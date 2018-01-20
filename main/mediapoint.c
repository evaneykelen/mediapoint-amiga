#include "nb:pre.h"
#include "nb:gui.h"
#include "nb:vectors.h"
#include "minc:types.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

extern struct List *LoadMLSegments(struct List *, struct FileLock *, ULONG);
extern void UnLoadMLSegments(struct List *SegList, ULONG);

/**** globals ****/

#define VERSION	"\0$VER: 3.8"
static UBYTE *vers = VERSION;

/**** externals ****/

extern struct List *SegList;		// defined in proccont.c, initialised in Initsegment.c
extern struct CapsPrefs CPrefs;
extern struct Process *process;
extern APTR tempWdwPtr;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern ULONG allocFlags;
extern struct IOClipReq *clipboard;
extern int HANDLESNIF;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct MsgPort *port;

/*printf("snr %d\n",sizeof(struct ScriptNodeRecord));
printf("ser %d\n",sizeof(struct ScriptEventRecord));
printf("vcr %d\n",sizeof(struct VarCondRecord));
printf("per %d\n",sizeof(struct PageEventRecord));*/

	Forbid();
	port = FindPort(ML_RENDEZ_VOUS);
	Permit();
	if (port!=NULL)
		exit(0);

	/**** init vars ****/

	SegList = NULL;	// No segments resident yet (process controler)

	/**** look how we started ****/

	if (argc==0)
		CPrefs.fromWB = TRUE;
	else 
		CPrefs.fromWB = FALSE;

	/**** get resources ****/

	if ( !StartUpFuncs(FALSE) )
		FreeAndExit();

	/**** get the tool types ****/

	GetWorkbenchTools(argc, argv, NULL);

	/**** get OS related stuff and open Locale (if present) ****/
	/**** also relies on fonts being opened.								****/

	if ( !DoReleaseTwo() )
		FreeAndExit();

	/**** init and get config ****/

	SetConfigDefaults();

	if ( !SetMonitorDefaults() )
	{
		Early_WarnUser("The configuration file contains an unsupported monitor");
		FreeAndExit();
	}

	GetConfigFile();

	if ( !SetMonitorFromConfig() )
	{
		Early_WarnUser("The configuration file contains an unsupported monitor");
		FreeAndExit();
	}

	/**** translate app ****/

	if ( !TranslateApp(TRUE) )
		exit(0);
	DoReleaseTwoSecondPart();

	/**** Preload system xapps with their LOADATSTART tool attribute set to YES ****/

	if( (SegList = LoadMLSegments(SegList,CPrefs.appdirLock,OTT_PRELOAD)) == NULL)
		FreeAndExit();

	/**** init menus ****/

	OpenMenus();

	/**** check stack size ****/

	CheckStack();

	/**** Most Recent Opened stuff ****/

	InitMRO();

	/**** open clipboard ****/

	//clipboard = CBOpen(0);

	/**** handle events ****/

	HandleEvents(NULL);

	/**** close clipboard ****/

	//CBClose(clipboard);	

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
			Message( msgs[ Msg_StackTooLow-1 ] );
	}
}

/******** FreeAndExit() ********/

void FreeAndExit(void)
{
	TERemoveScrap();	// Free undo/clipboard of text editor

	UnLoadTranslationFile();

	FreeInfoFromPageXaPP();

	if(SegList)
		UnLoadMLSegments(SegList, OTT_AFTERLOAD | OTT_PRELOAD);

	CloseMenus();

	removeRendezVous();

	if ( TestBit(allocFlags, FONTS_SCANNED_FLAG) )
		UA_freeScannedFonts();

	FreeFontList();

	FreeGlobalAllocs(FALSE);

	CloseLibraries();

	FlushTheSucker();

	exit(0);
}

/******** E O F ********/
