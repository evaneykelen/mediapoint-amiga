//	File		:	setup.c
//	Uses		:	pre.h midi_gadgets.h minc:types.h Errors.h process.h
//	Date		:	-92 29-04-93
//	Author	:	E. van Eykelen
//	Desc.		:	setup the Midi GUI
//

#include "nb:pre.h"
#include "gen:support_protos.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#define VERSI0N "\0$VER: MediaPoint MIDI xapp 1.1"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath, int *action);
void PutExtraData(PROCESSINFO *ThisPI, STRPTR fullPath, int action);

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
		UA_DoubleGadgetDimensions(MIDI_GR);

	UA_TranslateGR(MIDI_GR, msgs);

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= MIDI_GR[0].x2;
	UAI.windowHeight	= MIDI_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, MIDI_GR);

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
int ID,action;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH];
TEXT filename[SIZE_FILENAME];
TEXT fullpath[SIZE_FULLPATH], temp[SIZE_FULLPATH];
UWORD *mypattern1;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->sample_Path);
	filename[0] = '\0';

	FRR.path							= path;
	FRR.fileName					= filename;
	FRR.title							= msgs[Msg_MIDI_1-1]; // "Select a MIDI file:"
	FRR.opts							= DIR_OPT_MIDI;
	FRR.multiple					= FALSE;

	/**** get arguments from extraData ****/

	action=0;
	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData( ThisPI, fullpath, &action );
		strcpy(temp, fullpath);
		UA_ShortenString(window->RPort, temp, (MIDI_GR[2].x2-MIDI_GR[2].x1)-16);
		UA_DrawText(window, &MIDI_GR[2], temp);
		UA_EnableButton(window, &MIDI_GR[5]);	// play
		UA_SplitFullPath(fullpath, path, filename);
		UA_SetCycleGadgetToVal(window, &MIDI_GR[6], action);
		if (action==2)	// stop
			UA_DisableButton(window, &MIDI_GR[2], mypattern1);	// no filename on stop
		UA_MakeFullPath(path,filename,temp);	// used for ghosting when cycling
	}
	else
	{
		UA_DrawText(window, &MIDI_GR[2], msgs[Msg_X_5-1]);
		UA_DisableButton(window, &MIDI_GR[5], mypattern1);	// play
		strcpy(temp,msgs[Msg_X_5-1]);
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, MIDI_GR, &CED);
			switch(ID)
			{
				case 2:	// file requester
					UA_HiliteButton(window, &MIDI_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, filename, fullpath);
						strcpy(temp,fullpath);
						UA_ShortenString(window->RPort, fullpath, (MIDI_GR[2].x2-MIDI_GR[2].x1)-16);
						UA_ClearButton(window, &MIDI_GR[2], AREA_PEN);
						UA_DrawText(window, &MIDI_GR[2], fullpath);
						UA_EnableButton(window, &MIDI_GR[5]);	// play
					}
					break;

				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &MIDI_GR[3]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &MIDI_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 5: // Play
					UA_InvertButton(window, &MIDI_GR[ID]);
					Delay(100L);
					UA_InvertButton(window, &MIDI_GR[ID]);
					break;

				case 6:
					UA_ProcessCycleGadget(window, &MIDI_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&MIDI_GR[6],&action);
					if (action==2)	// STOP
						UA_DisableButton(window, &MIDI_GR[2], mypattern1);	// no filename on stop
					else
					{
						UA_EnableButton(window, &MIDI_GR[2]);	// no filename on stop
						UA_ClearButton(window, &MIDI_GR[2], AREA_PEN);
						UA_ShortenString(window->RPort, temp, (MIDI_GR[2].x2-MIDI_GR[2].x1)-16);
						UA_DrawText(window, &MIDI_GR[2], temp);
					}
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
		UA_MakeFullPath(path, filename, fullpath);
		PutExtraData(ThisPI, fullpath, action);

		if ( action == 0 )				// wait
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 1;
		else if ( action == 1 )		// loop
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 2;
		else											// stop
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 1;
	}

	return(retVal);
}

/******** GetExtraData() ********/
/*
 * MIDI extra data must look like:
 *
 * "work:carl_screams.mid" action
 *
 */

void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath, int *action)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	fullPath[0] = '\0';
	*action = 0; // Wait,Loop,Stop

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;	// sorry Cees; no file means no showing either.

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
		if ( numChars>=1 && numChars<SIZE_FULLPATH )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					RemoveQuotes(tmp);
					strcpy(fullPath,tmp);
					break;

				case 1:
					sscanf(tmp,"%d",action);
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
 * MIDI extra data must look like:
 *
 * "work:carl_screams.mid" action
 *
 */

void PutExtraData(PROCESSINFO *ThisPI, STRPTR fullPath, int action)
{
UBYTE scrStr[512];

	StrToScript(fullPath, scrStr);
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, "\\\"%s\\\" %d", scrStr, action);
}

/******** E O F ********/
