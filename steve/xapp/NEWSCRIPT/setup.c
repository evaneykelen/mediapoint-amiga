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
void GetExtraData(PROCESSINFO *ThisPI, char *path);
void PutExtraData(PROCESSINFO *ThisPI, char *path);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

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
		UA_DoubleGadgetDimensions(NewScript_GR);

	UA_TranslateGR(NewScript_GR, msgs);

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= NewScript_GR[0].x2;
	UAI.windowHeight	= NewScript_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, NewScript_GR);

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
int ID;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH], filename[SIZE_FILENAME], fullpath[SIZE_FULLPATH], temp[SIZE_FULLPATH];
UWORD *mypattern1;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->script_Path);
	//path[0] = '\0';
	filename[0] = '\0';
	fullpath[0] = '\0';

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.title			= msgs[Msg_SelectAScript-1];
	FRR.opts			= DIR_OPT_SCRIPTS;
	FRR.multiple	= FALSE;

	/**** get arguments from extraData ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData( ThisPI, fullpath );
		UA_SplitFullPath(fullpath, path, filename);

		if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0]=='@' )
			strcpy(temp, msgs[Msg_VarPath_5-1]);
		else
			strcpy(temp, fullpath);

		UA_ShortenString(window->RPort, temp, (NewScript_GR[2].x2-NewScript_GR[2].x1)-16);
		UA_DrawText(window, &NewScript_GR[2], temp);

		if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0]=='@' )
			UA_DisableButton(window, &NewScript_GR[2], mypattern1);	// filename path
	}
	else
		UA_DrawText(window, &NewScript_GR[2], msgs[Msg_X_5-1]);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, NewScript_GR, &CED);
			switch(ID)
			{
				case 2:	// file requester
					UA_HiliteButton(window, &NewScript_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, filename, fullpath);
						strcpy(temp,fullpath);
						UA_ShortenString(window->RPort, temp, (NewScript_GR[2].x2-NewScript_GR[2].x1)-16);
						UA_ClearButton(window, &NewScript_GR[2], AREA_PEN);
						UA_DrawText(window, &NewScript_GR[2], temp);
					}
					break;

				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &NewScript_GR[3]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &NewScript_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
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

	if (	retVal && fullpath[0] != '\0' &&
				ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0]!='@' )
		PutExtraData(ThisPI, fullpath);

	return(retVal);
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, char *path)
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

void PutExtraData(PROCESSINFO *ThisPI, char *path)
{
UBYTE scrStr1[SIZE_FULLPATH+10];

	StrToScript(path, scrStr1);
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, "\\\"%s\\\"", path);
}

/******** E O F ********/
