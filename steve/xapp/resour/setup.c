//	File		:	setup.c
//	Uses		:	pre.h minc:types.h Errors.h process.h
//	Date		:	-92 29-04-93
//	Author	:	E. van Eykelen
//	Desc.		:	setup the Resource GUI
//

#include "nb:pre.h"
#include "demo:gen/support_protos.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#define VERSI0N "\0$VER: 1.0"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, char *path, int *mode);
void PutExtraData(PROCESSINFO *ThisPI, char *path, int mode);
int GetFileSize(STRPTR path);
void MakeNiceNumber(char *in, char *out);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open and load the medialink font ***/

	UAI.small_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI.small_TA.ta_YSize = 10;
	UAI.small_TA.ta_Style = FS_NORMAL;
	UAI.small_TA.ta_Flags = FPF_DESIGNED;
	UAI.small_TF = rvrec->smallfont;

	UAI.large_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI.large_TA.ta_YSize = 20;
	UAI.large_TA.ta_Style = FS_NORMAL;
	UAI.large_TA.ta_Flags = FPF_DESIGNED;
	UAI.large_TF = rvrec->largefont;

	/**** open a window ****/

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(Res_GR);

	UA_TranslateGR(Res_GR, msgs);

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Res_GR[0].x2;
	UAI.windowHeight	= Res_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Res_GR);
	Res_GR[5].type=BUTTON_GADGET;

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID,mode;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH];
TEXT filename[SIZE_FILENAME];
TEXT fullpath[SIZE_FULLPATH], temp[SIZE_FULLPATH], sizestr[40];
UWORD *mypattern1;
long size;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->import_picture_Path);
	path[0] = '\0';
	filename[0] = '\0';
	fullpath[0] = '\0';
	mode=0;

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.title			= msgs[Msg_SelectAFile-1];
	FRR.opts			= DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_THUMBS;
	FRR.multiple	= FALSE;

	/**** get arguments from extraData ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData( ThisPI, fullpath, &mode );
		UA_SplitFullPath(fullpath, path, filename);
		strcpy(temp, fullpath);
		UA_ShortenString(window->RPort, temp, (Res_GR[2].x2-Res_GR[2].x1)-16);
		UA_DrawText(window, &Res_GR[2], temp);
		UA_SetCycleGadgetToVal(window,&Res_GR[6],mode);
		size = GetFileSize(fullpath);
		sprintf(temp,"%d",size);
		MakeNiceNumber(temp,sizestr);
		UA_DrawText(window, &Res_GR[5], sizestr);
	}
	else
		UA_DrawText(window, &Res_GR[2], msgs[Msg_X_5-1]);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Res_GR, &CED);
			switch(ID)
			{
				case 2:	// file requester
					UA_HiliteButton(window, &Res_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, filename, fullpath);
						strcpy(temp,fullpath);
						UA_ShortenString(window->RPort, temp, (Res_GR[2].x2-Res_GR[2].x1)-16);
						UA_ClearButton(window, &Res_GR[2], AREA_PEN);
						UA_DrawText(window, &Res_GR[2], temp);
						size = GetFileSize(fullpath);
						sprintf(temp,"%d",size);
						MakeNiceNumber(temp,sizestr);
						UA_ClearButton(window, &Res_GR[5], AREA_PEN);
						UA_DrawText(window, &Res_GR[5], sizestr);
					}
					break;

				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &Res_GR[3]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Res_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 6:
					UA_ProcessCycleGadget(window, &Res_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Res_GR[6],&mode);
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	FreeMem(mypattern1, 4L);

	if ( retVal && fullpath[0] != '\0' )
	{
		PutExtraData(ThisPI, fullpath, mode);
	}

	return(retVal);
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, char *path, int *mode)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;

	ScriptToStr(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, scrStr);

	strPtr = scrStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;

	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if ( numChars>=1 && numChars<USERAPPLIC_MEMSIZE )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					RemoveQuotes(tmp);
					strcpy(path, tmp);
					break;
				case 1:
					sscanf(tmp, "%d", mode);
					break;
			}
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, char *path, int mode)
{
UBYTE scrStr1[SIZE_FULLPATH+10];

	StrToScript(path, scrStr1);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"\\\"%s\\\" %d",
					path,
					mode );
}

/******** GetFileSize() ********/

int GetFileSize(STRPTR path)
{
struct FileInfoBlock *Finfo;
BPTR lock;
int size;

	size=0;
	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );
	if( Finfo )
	{
		lock = Lock( path, ACCESS_READ );
		if( lock != NULL )
		{
			Examine( lock, Finfo );
			size = Finfo->fib_Size;
			UnLock( lock );
		}
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
	}

	return(size);	
}

/******** MakeNiceNumber() ********/

void MakeNiceNumber(char *in, char *out)
{
int i,len,comma,pos;
char sign;

	if (	rvrec->capsprefs->lanCode==1 )	// English
		sign=',';
	else
		sign='.';

	len=strlen(in);
	if(len<=3)
	{
		strcpy(out,in);
		return;
	}
	comma=2;
	pos=0;
	for(i=len-1; i>=0; i--)
	{
		out[pos]=in[i];
		pos++;
		if (comma==0)
		{
			comma=2;
			out[pos]=sign;		
			pos++;
		}
		else
			comma--;
	}
	out[pos]='\0';
	if (comma==2)
		out[pos-1]='\0';
	strrev(out);
}

/******** E O F ********/
