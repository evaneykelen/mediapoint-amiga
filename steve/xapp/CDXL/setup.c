#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

TEXT FPSList[] = { "\
1\0    2\0    3\0    4\0    5\0    6\0    7\0    8\0    9\0    10\0   \
11\0   12\0   13\0   14\0   15\0   16\0   17\0   18\0   19\0   20\0   \
21\0   22\0   23\0   24\0   25\0   26\0   27\0   28\0   29\0   30\0   \
31\0   32\0   33\0   34\0   35\0   36\0   37\0   38\0   39\0   40\0   \
41\0   42\0   43\0   44\0   45\0   46\0   47\0   48\0   49\0   50\0   \
51\0   52\0   53\0   54\0   55\0   56\0   57\0   58\0   59\0   60\0   " };

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct CycleRecord *CR;
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

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(CDXL_GR);

	UA_TranslateGR(CDXL_GR, msgs);

	CR = (struct CycleRecord *)CDXL_GR[8].ptr;
	CR->ptr = FPSList;

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= CDXL_GR[0].x2;
	UAI.windowHeight	= CDXL_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, CDXL_GR);

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
int ID, value;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH];
TEXT filename[SIZE_FILENAME];
TEXT fullpath[SIZE_FULLPATH], temp[SIZE_FULLPATH];
UWORD *mypattern1;
UBYTE speedMode;
BOOL pfd, enlarge;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] = 46;		// transition
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[6] = 0;			// x offset (relative to centered position)
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[7] = 0;			// y offset (relative to centered position)
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = 14;		// FPS
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = 1;		// Rotations
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = 1;		// CBITS
	}

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->anim_Path);
	filename[0] = '\0';

	FRR.path							= path;
	FRR.fileName					= filename;
	FRR.title							= msgs[Msg_CDXL_6-1];
	FRR.opts							= DIR_OPT_ALL;
	FRR.multiple					= FALSE;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData( ThisPI, fullpath );
		strcpy(temp, fullpath);
		UA_ShortenString(window->RPort, temp, (CDXL_GR[2].x2-CDXL_GR[2].x1)-16);
		UA_DrawText(window, &CDXL_GR[2], temp);
		//UA_EnableButton(window, &CDXL_GR[5]);	// preview
		UA_SplitFullPath(fullpath, path, filename);
	}
	else
	{
		UA_DrawText(window, &CDXL_GR[2], msgs[Msg_X_5-1]);
		//UA_DisableButton(window, &CDXL_GR[5], mypattern1);	// preview
	}

	/**** set FPS cycle ****/

	UA_SetCycleGadgetToVal(	window, &CDXL_GR[8],
													ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] );

	/**** set loops cycle ****/

	value = ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10];
	if (value==-1)
		value=0;	// infinite
	UA_SetCycleGadgetToVal(	window, &CDXL_GR[9], value);

	speedMode = 1;	// FPS
	if ( ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] & 1 )
		speedMode = 2;	// CDXL speed;
	UA_InvertButton(window, &CDXL_GR[speedMode+5]);	// 6=FPS mode, 7=CDXL speed
	if (speedMode==2)
		UA_DisableButton(window, &CDXL_GR[8], mypattern1);	// FPS cycle

	if ( ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] & 2 )
	{
		pfd = TRUE;
		UA_InvertButton(window, &CDXL_GR[10]);	// play from disk
	}
	else
		pfd = FALSE;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] & 4 )
	{
		enlarge = TRUE;
		UA_InvertButton(window, &CDXL_GR[11]);	// enlarge
	}
	else
		enlarge = FALSE;

	if ( !DoWeHaveAtLeastA68020() )
		UA_DisableButton(window, &CDXL_GR[11], mypattern1);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, CDXL_GR, &CED);
			switch(ID)
			{
				case 2:
					UA_HiliteButton(window, &CDXL_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, filename, fullpath);
						UA_ShortenString(	window->RPort, fullpath,
															(CDXL_GR[2].x2-CDXL_GR[2].x1)-16);
						UA_ClearButton(window, &CDXL_GR[2], AREA_PEN);
						UA_DrawText(window, &CDXL_GR[2], fullpath);
						//UA_EnableButton(window, &CDXL_GR[5]);	// preview
					}
					break;

				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &CDXL_GR[3]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &CDXL_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;
/*
				case 5: // Preview
					UA_HiliteButton(window, &CDXL_GR[ID]);
					break;
*/
				case 6:	// FPS mode
					if (speedMode==2)
					{
						UA_InvertButton(window, &CDXL_GR[6]);
						UA_InvertButton(window, &CDXL_GR[7]);
						speedMode=1;
						UA_EnableButton(window, &CDXL_GR[8]);		// FPS cycle
					}
					break;

				case 7:	// CDXL speed mode
					if (speedMode==1)
					{
						UA_InvertButton(window, &CDXL_GR[6]);
						UA_InvertButton(window, &CDXL_GR[7]);
						speedMode=2;
						UA_DisableButton(window, &CDXL_GR[8], mypattern1);	// FPS cycle
					}
					break;

				case 8:	// FPS cycle
				case 9:	// loops cycle
					UA_ProcessCycleGadget(window, &CDXL_GR[ID], &CED);
					break;

				case 10:	// play from disk
					UA_InvertButton(window, &CDXL_GR[ID]);
					if (pfd)
						pfd=FALSE;
					 else
						pfd=TRUE;
					break;

				case 11:	// enlarge
					UA_InvertButton(window, &CDXL_GR[11]);	// enlarge
					if (enlarge)
						enlarge=FALSE;
					else
						enlarge=TRUE;
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

	if ( retVal && filename[0] != '\0' )
	{
		UA_SetValToCycleGadgetVal(&CDXL_GR[8], &value);	// FPS
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = value;

		UA_SetValToCycleGadgetVal(&CDXL_GR[9], &value);	// loops
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = value;
		if (value==0)	// infinite chosen
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = -1;	// this is infinite for Cees

		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = 0;	// clear CBITS
		if (speedMode==2)
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = 1;	// CDXL speed
		if (pfd)
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] |= 2;	// Play from disk
		if (enlarge)
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] |= 4;	// Enlarge

		UA_MakeFullPath(path, filename, fullpath);

		PutExtraData(ThisPI, fullpath);
	}

	return(retVal);
}

/******** GetExtraData() ********/
/*
 * CDXL extra data must look like:
 *
 * 'work:carl_screams' speed loops bits
 *
 */

void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath)
{
int numChars, argNum, len, numSeen, value;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		fullPath[0] = '\0';
		return;	// sorry Cees; no file means no showing either.
	}

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
					strcpy(fullPath,tmp);
					break;
				case 1:	// speed		(store in argNum+8 == 9)
				case 2:	// loops		(store in argNum+8 == 10)
				case 3:	// CBITS		(store in argNum+8 == 11)
					sscanf(tmp, "%d", &value);
				 	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[argNum+8] = value;
					break; 
				default:
					return;	// get the hell out, s'thing is terribly wrong!
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
/*
 * CDXL extra data must look like:
 *
 * 'work:carl_screams' speed loops bits
 *
 */

void PutExtraData(PROCESSINFO *ThisPI, STRPTR fullPath)
{
UBYTE scrStr[512];

	StrToScript(fullPath, scrStr);
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"\\\"%s\\\" %d %d %d",
					scrStr,
					ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9],					
					ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10],					
					ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11]);
}

/******** DoWeHaveAtLeastA68020() ********/

BOOL DoWeHaveAtLeastA68020(void)
{
struct ExecBase *exec;

	exec = (struct ExecBase *)OpenLibrary("exec.library",0L);
	if ( !exec )
		return(FALSE);

	if ( exec->AttnFlags & AFF_68020 )
	{
		CloseLibrary((struct Library *)exec);
		return(TRUE);
	}

	CloseLibrary((struct Library *)exec);
	return(FALSE);
}

/******** E O F ********/
