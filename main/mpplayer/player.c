#include "nb:pre.h"
#include "nb:gui.h"
#include "nb:vectors.h"
#include "minc:types.h"
#include "minc:system.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

extern struct List *LoadMLSegments(struct List *, struct FileLock *, ULONG);
extern void UnLoadMLSegments(struct List *, ULONG);
extern BOOL playScript(struct ScriptInfoRecord *, UBYTE, BOOL, BOOL, BOOL);	// see ph:playscript.c
extern BOOL SpecialAllocMLSystem(void);
extern void SpecialFreeMLSystem(void);
extern void ProcessDeInitializer(void);

int AllocDongle(void);
UWORD ReadDongle(UWORD code);
void FreeDongle(void);
void RunScript(int argc, char **argv, STRPTR scriptName);
BOOL pre_player(void);
void post_player(void);
void FlushTheSucker(void);

/**** defines ****/

#define VERSION	"\0$VER: 1.4"
#define custom (*((struct Custom *)0xdff000))
#define PLAYER_IS_UPDATEABLE TRUE

/**** globals ****/

static UBYTE *vers = VERSION;
struct ScriptNodeRecord *fromSNR=NULL;	// see also proccont.c

/**** externals ****/

extern struct List *SegList;		// defined in proccont.c, initialised in Initsegment.c
extern struct CapsPrefs CPrefs;
extern struct Process *process;
extern APTR tempWdwPtr;
extern struct Library *medialinkLibBase;
extern struct Library *MLMMULibBase;
extern char *scriptCommands[];
extern ULONG allocFlags;
extern UBYTE **msgs;   
extern struct ObjectInfo ObjectRecord;
extern TEXT newScript[];
extern BOOL BlockAllInput;
extern struct Window *playWindow;
extern struct RendezVousRecord rvrec;
extern struct Screen *playScreen;
extern MLSYSTEM	*MLSystem;
extern BOOL alt_ctrl_esc_pressed;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct MsgPort *port;
TEXT scriptName[SIZE_FULLPATH];

	scriptName[0] = '\0';

	Forbid();
	port = FindPort(ML_RENDEZ_VOUS);
	Permit();
	if (port!=NULL)	// can't run when MediaPoint is running!
		exit(0);

	/**** init vars ****/

	SegList = NULL;	// No segments resident yet (process controler)

	/**** look how we started ****/

	if (argc==0)
		CPrefs.fromWB = TRUE;
	else 
		CPrefs.fromWB = FALSE;

	/**** get resources ****/

	if ( !StartUpFuncs(TRUE) )
		FreeAndExit();

	/**** get the tool types ****/

	GetWorkbenchTools(argc, argv, scriptName);

	/**** get OS related stuff and open Locale (if present) ****/
	/**** also relies on fonts being opened.								****/

	if ( !DoReleaseTwo() )
		FreeAndExit();

	/**** init and get config ****/

	SetConfigDefaults();

	if ( !SetMonitorDefaults() )
		FreeAndExit();

	GetConfigFile();

	if ( !SetMonitorFromConfig() )
		FreeAndExit();

	/**** translate app ****/

	if ( !TranslateApp(TRUE) )
		exit(0);
	DoReleaseTwoSecondPart();

	/**** Preload system xapps with their LOADATSTART tool attribute set to YES ****/

	if( (SegList = LoadMLSegments(SegList,CPrefs.appdirLock,OTT_PRELOAD)) == NULL)
		FreeAndExit();

	/**** check stack size ****/

	CheckStack();

	/**** close the Workbench screen ****/

	if ( !CPrefs.WorkBenchOn )
		CloseWorkBench();

	/**** start commodity ****/

	if ( !OpenMagicBroker() )
		UA_WarnUser(-1);

	/**** handle events ****/

	if ( SpecialAllocMLSystem() )
	{
		RunScript(argc,argv,scriptName);
		SpecialFreeMLSystem();
	}

	/**** stop commodity ****/

	if ( CPrefs.SystemTwo )
		CloseMagicBroker();

	/**** reopen the WB ****/

	OpenWorkBench();

	/**** restore pointer to CLI window ****/

	process->pr_WindowPtr = tempWdwPtr;

	/**** exit ****/

	FreeAndExit();
}

/******** CheckStack() ********/

void CheckStack(void)
{
struct CommandLineInterface *CLI;

	if ( !CPrefs.fromWB )
	{	
		CLI = (struct CommandLineInterface *)BADDR(process->pr_CLI);
		if ( (CLI->cli_DefaultStack*4) < 20000 )
			Message( "The stack size is too small. Set it to at least 40000 bytes." );
	}
}

/******** FreeAndExit() ********/

void FreeAndExit(void)
{
	UnLoadTranslationFile();
	if(SegList)
		UnLoadMLSegments(SegList, OTT_AFTERLOAD | OTT_PRELOAD);
	removeRendezVous();
	FreeGlobalAllocs(TRUE);

	if ( IntuitionBase->ActiveWindow )
		ClearPointer( IntuitionBase->ActiveWindow );
	RethinkDisplay();

	CloseLibraries();
	FlushTheSucker();
	exit(0);
}

/******** DonglePresent() ********/

BOOL DonglePresent(void)
{
	if(!AllocDongle())
		return(FALSE);

	if(ReadDongle(5) != 23)
		return(FALSE);

	if(ReadDongle(6) != 11)
		return(FALSE);

	if(ReadDongle(7) != 1992)
		return(FALSE);

	FreeDongle();

	return(TRUE);
}

/******** RunScript() ********/

void RunScript(int argc, char **argv, STRPTR scriptName)
{
TEXT path[SIZE_FULLPATH];
TEXT filename[SIZE_FILENAME];
TEXT active[SIZE_FULLPATH];
TEXT temp[SIZE_FULLPATH];
TEXT ass[SIZE_FULLPATH];
int len;
FILE *fp;
BOOL initializers=TRUE;

	newScript[0] = '\0';
	active[0] = '\0';

	if (argc==0)	// from WB
		strcpy(newScript,scriptName);
	else if ( (argc==2 && argv[1][0]=='?') || argc!=2 )
	{
		printf("MediaPoint Player © Copyright 1992-'93-'94 by MediaPoint Int.\n");
		printf("Usage: %s \"script name\"\n", argv[0]);
		return;
	}
	else
		strcpy(newScript,argv[1]);

	OpenPlayScreen(CPrefs.playerMonitorID);
	PlayScreenToFront();

#if PLAYER_IS_UPDATEABLE

	len = strlen(newScript);
	if ( newScript[len-2]=='.' &&	( newScript[len-1]=='1' || newScript[len-1]=='2' ) )
		;
	else
		newScript[0] = '\0';

	// After a reboot there is a RUNS_xxx file. Read the script indicated by this file.

	if ( newScript[0] )
	{
		UA_SplitFullPath(newScript, path, filename);	// eg. 'data2:' and 'script.1'
		len = strlen(filename);
		filename[len-2]='\0';													// remove '.1' or '.2' part 

		sprintf(temp,"RUNS_%s.1",filename);						// eg. RUNS_script.1
		UA_MakeFullPath(path,temp,active);				
		fp = fopen(active,"r");
		if (fp)
		{
			UA_MakeFullPath(path,&temp[5],newScript);
			fclose(fp);
		}
		else
		{
			sprintf(temp,"RUNS_%s.2",filename);					// eg. RUNS_script.2
			UA_MakeFullPath(path,temp,active);				
			fp = fopen(active,"r");
			if (fp)
			{
				UA_MakeFullPath(path,&temp[5],newScript);
				fclose(fp);
			}
		}
	}

#endif

	if ( !MLMMU_OpenMsgQueue() )
		return;

	while( newScript[0] != '\0' ) 
	{
		UA_SplitFullPath(newScript, path, filename);

#if !PLAYER_IS_UPDATEABLE
		newScript[0] = '\0';	// arexx may fill it again.
#else
		if ( active[0] )
			DeleteFile(active);

		sprintf(temp,"RUNS_%s",filename);
		UA_MakeFullPath(path,temp,active);				
		fp = fopen(active,"w");
		if (fp)
		{
			fprintf(fp," ");
			fclose(fp);
		}			

		// START - SET ASSIGN

//printf("Assign to [%s]\n",newScript);

		len = strlen(newScript);
		if ( newScript[len-1]=='1' )
		{
			sprintf(ass,"MP_RA:script1");
			AssignPath("ALIAS",ass);
//printf("1 Made assign to [%s]\n",ass);
		}
		else if ( newScript[len-1]=='2' )
		{
			sprintf(ass,"MP_RA:script2");
			AssignPath("ALIAS",ass);
//printf("2 Made assign to [%s]\n",ass);
		}

		// END - SET ASSIGN

#endif

		if ( ReadScript(path, filename, scriptCommands) )
		{
			fromSNR = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[1]->lh_Head;
			if ( pre_player() )
			{
				if ( ValidateSER(&(ObjectRecord.scriptSIR),TRUE,TRUE) )
				{
					playScript( &(ObjectRecord.scriptSIR),1,initializers,FALSE,FALSE );
				}
				if ( initializers )
					initializers=FALSE;
				post_player();
			}
			freeScript();
		}
		else if ( argc>0 )	// print only to CLI
			printf("Script '%s' can't be found.\n",filename);

#if PLAYER_IS_UPDATEABLE
		len = strlen(newScript);
		if ( newScript[len-2]!='.' )
			newScript[0]='\0';
		else
		{
			if ( newScript[len-1]=='1' )
				newScript[len-1]='2';
			else
				newScript[len-1]='1';
		}
#endif

		if ( alt_ctrl_esc_pressed )
			newScript[0]='\0';
	}

	PlayScreenToFront();

	if ( !initializers )
		ProcessDeInitializer();

	ClosePlayScreen();
	WBenchToFront();

	ProcessMsgQueue();
	MLMMU_CloseMsgQueue();
}

/******** pre_player() ********/

BOOL pre_player(void)
{
ULONG numObjects;
int numButtons;

	CloseWorkBench();

	if ( !ValidateSER(&(ObjectRecord.scriptSIR),TRUE,TRUE) )
		return(FALSE);

	numObjects = CreateUsedXappList(&(ObjectRecord.scriptSIR));
	if ( numObjects==1 )
	{
		CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
		CPrefs.mousePointer |= 8;	// put input to none
	}

	numButtons = Validate_All_LE(&(ObjectRecord.scriptSIR),TRUE);
	if ( numButtons>0 )
	{
		if ( !(CPrefs.mousePointer & 8) )	// keep none
		{
			CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
			CPrefs.mousePointer |= 1;	// cursor
		}
	}

	Forbid();
	process->pr_WindowPtr = -1;
	Permit();

	// ONLY ENABLE SWITCHING TO WB WHEN INPUT IS CURSOR+MOUSE OR CURSOR

	if ( (CPrefs.mousePointer & 2) || (CPrefs.mousePointer & 8) )
	{
		Forbid();
		BlockAllInput=TRUE;
		Permit();
	}

	rvrec.pagescreen = playScreen;

	return(TRUE);
}

/******** post_player() ********/

void post_player(void)
{
	FreeUsedXappList();
	Free_All_LE( &(ObjectRecord.scriptSIR) );
}

/******** FlushTheSucker() ********/

void FlushTheSucker(void)
{
int i;
UBYTE *ptr;

	for(i=0; i<10; i++)
	{
		ptr = (UBYTE *)AllocMem(0x7ffffff0,MEMF_PUBLIC);
		if (ptr)
			FreeMem(ptr,0x7ffffff0);
	}
}

/******** E O F ********/
