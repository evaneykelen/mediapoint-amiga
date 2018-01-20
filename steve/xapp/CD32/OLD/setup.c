#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "gen:support_protos.h"

#define VERSI0N "\0$VER: MediaPoint CDTV xapp 1.2"          
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct CDTV_record *CDTV_rec);
void SetButtons(struct CDTV_record *CDTV_rec, struct Window *window,
								UWORD *mypattern);

/**** external function declarations ****/

extern void GetVarsFromPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct CDTV_record CDTV_rec;
UBYTE *chipMem;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;

	/**** link with it ****/

	list	= &(port->mp_MsgList);
	node	= list->lh_Head;
	rvrec	= (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** parse .info file for special settings ****/

	GetInfoFile("CDTV", ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath,
							CDTV_rec.devName, &(CDTV_rec.portNr), &(CDTV_rec.baudRate));

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort(&CDTV_rec) )
		return;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** copy image to chip memory ****/

	chipMem = (UBYTE *)AllocMem(sizeof(ImageDataControler), MEMF_CHIP);
	if (chipMem==NULL)
	{
		Close_SerialPort(&CDTV_rec);
		return;
	}
	CopyMem(ImageDataControler, chipMem, sizeof(ImageDataControler));
	ImageControler.ImageData = (SHORT *)chipMem;	

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
		UA_DoubleGadgetDimensions(CDTV_GR);

	UA_TranslateGR(CDTV_GR, msgs);

	UA_AdjustGadgetCoords(Controler_GR, CDTV_GR[25].x1-3, CDTV_GR[25].y1-2);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= CDTV_GR[0].x2;
	UAI.windowHeight	= CDTV_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	CDTV_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, CDTV_GR);
	DrawImage(UAI.userWindow->RPort, (struct Image *)&ImageControler,
						CDTV_GR[25].x1, CDTV_GR[25].y1);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &CDTV_rec);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** free chip mem ****/

	FreeMem(chipMem, sizeof(ImageDataControler));

	/**** close specials libraries or devices ****/

	Close_SerialPort(&CDTV_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct CDTV_record *CDTV_rec)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID, song, oldSong;
UWORD *mypattern1;
//UWORD *waitPtr;
struct FileLock *fileLock;
struct Window *wdw;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

#if 0
	waitPtr = (UWORD *)AllocMem(sizeof(WaitPointer), MEMF_CHIP);
	if (waitPtr==NULL)
		return(FALSE);
	CopyMem(WaitPointer, waitPtr, sizeof(WaitPointer));
#endif

	/**** init CDTV_rec ****/

	CDTV_rec->IOReq1			= NULL;
	CDTV_rec->IOReq2			= NULL;
	CDTV_rec->IOPort			= NULL;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(CDTV_rec, ThisPI);
	else
	{
		CDTV_rec->action			= 0;			// play song
		CDTV_rec->command			= 0;			// fade out slow
		CDTV_rec->song				= 1;
		CDTV_rec->from				= 1;
		CDTV_rec->to					= 2;
		strcpy(CDTV_rec->start, "00:05:00");
		strcpy(CDTV_rec->end, "00:10:00");
		CDTV_rec->fadeIn			= FALSE;
		CDTV_rec->control			= 0;			// serial
	}

	/**** set buttons ****/

	SetButtons(CDTV_rec, window, mypattern1);

	UA_DisableButton(window, &Controler_GR[2], mypattern1);
	UA_DisableButton(window, &Controler_GR[6], mypattern1);
	UA_DisableButton(window, &Controler_GR[7], mypattern1);

	/**** treat radio buttons separately ****/

	UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);

	if ( CDTV_rec->fadeIn )
		UA_InvertButton(window, &CDTV_GR[14]);

	/**** get info from CDTV ****/

	wdw = UA_OpenMessagePanel(window, msgs[Msg_CDTV_12-1]); // "Looking for the CDTV..."

	UA_SetSprite(wdw, SPRITE_BUSY);
	GetNewCD(window, CDTV_rec);
	UA_SetSprite(wdw, SPRITE_NORMAL);

	if (wdw)
		UA_CloseMessagePanel(wdw);

	/**** if cdtv.device is not present, disable cycle ****/

	fileLock = (struct FileLock *)Lock((STRPTR)"devs:cdtv.device",(LONG)ACCESS_READ);
	if (fileLock == NULL)
		UA_DisableButton(window, &CDTV_GR[16], mypattern1);
	else
		UnLock((BPTR)fileLock);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			/**** CDTV window gadgets ****/

			ID = UA_CheckGadgetList(window, CDTV_GR, &CED);
			switch(ID)
			{
				case 4:	// play song
				case 5:	// play from...to
				case 6:	// play start...end
				case 7:	// direct command
					if ( CDTV_rec->action != ID-4 )
					{
						UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);
						CDTV_rec->action=ID-4;								
						UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);
						SetButtons(CDTV_rec, window, mypattern1);
					}
					break;

				case 8:		// song
				case 9:		// from
				case 10:	// to
				case 11:	// start
				case 12:	// end
					UA_ProcessStringGadget(window, CDTV_GR, &CDTV_GR[ID], &CED);
					if (ID==8)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->song);
					else if (ID==9)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->from);
					else if (ID==10)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->to);
					else if (ID==11)
						UA_SetStringToGadgetString(&CDTV_GR[ID], CDTV_rec->start);
					else if (ID==12)
						UA_SetStringToGadgetString(&CDTV_GR[ID], CDTV_rec->end);
					break;

				case 13:	// command
				case 16:	// control
					UA_ProcessCycleGadget(window, &CDTV_GR[ID], &CED);
					if (ID==13)
						UA_SetValToCycleGadgetVal(&CDTV_GR[ID], &CDTV_rec->command);
					else if (ID==16)
					{
						UA_SetValToCycleGadgetVal(&CDTV_GR[ID], &CDTV_rec->control);
						if ( CDTV_rec->control==CONTROL_VIA_SERIAL )
							CloseCDTV(CDTV_rec);
						else if ( CDTV_rec->control==CONTROL_VIA_CDROM )
						{
							if ( !OpenCDTV(CDTV_rec) )
								GiveMessage(CDTV_rec->window, msgs[Msg_CDTV_14-1]); // "The CDTV doesn't respond!"
							else
							{
								UA_SetSprite(window, SPRITE_BUSY);
								GetNewCD(window, CDTV_rec);
								UA_SetSprite(window, SPRITE_NORMAL);
							}
						}
					}
					break;

				case 14:
					UA_InvertButton(window, &CDTV_GR[ID]);
					if ( CDTV_rec->fadeIn )
						CDTV_rec->fadeIn=FALSE;
					else
						CDTV_rec->fadeIn=TRUE;
					break;

				case 15:	// new cd
					UA_SetSprite(window, SPRITE_BUSY);
					GetNewCD(window, CDTV_rec);
					UA_SetSprite(window, SPRITE_NORMAL);
					break;

				case 17:	// OK
do_ok:
					UA_HiliteButton(window, &CDTV_GR[17]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 18:	// Preview
					UA_SetSprite(window, SPRITE_BUSY);
					UA_InvertButton(window, &CDTV_GR[ID]);
					if ( !ControlCDTV(CDTV_rec) )
						GiveMessage(CDTV_rec->window, msgs[Msg_CDTV_14-1]); // "The CDTV device doesn't respond!"
					else
						ShowTrack(window, CDTV_rec);
					UA_InvertButton(window, &CDTV_GR[ID]);
					UA_SetSprite(window, SPRITE_NORMAL);
					break;

				case 19:	// Cancel
do_cancel:
					UA_HiliteButton(window, &CDTV_GR[19]);
					loop=FALSE;
					retVal=FALSE; 
					break;
			}

			/**** Controler gadgets ****/

			ID =  UA_CheckGadgetList(window, Controler_GR, &CED);
			switch(ID)
			{
				case 1:	// Stop
					UA_InvertButton(window, &Controler_GR[ID]);
					CDTV_Stop(CDTV_rec);
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 3:	// Play Fwd
					UA_InvertButton(window, &Controler_GR[ID]);
					CDTV_PlayTrack(CDTV_rec);
					ShowTrack(window, CDTV_rec);
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 4:	// Play prev
					UA_InvertButton(window, &Controler_GR[ID]);
					CDTV_GetPrevSong(CDTV_rec, &song);
					oldSong = CDTV_rec->song;							
					CDTV_rec->song = song;							
					CDTV_PlayTrack(CDTV_rec);
					ShowTrack(window, CDTV_rec);
					CDTV_rec->song = oldSong;							
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 5:	// Play next
					UA_InvertButton(window, &Controler_GR[ID]);
					CDTV_GetNextSong(CDTV_rec, &song);
					oldSong = CDTV_rec->song;							
					CDTV_rec->song = song;							
					CDTV_PlayTrack(CDTV_rec);
					ShowTrack(window, CDTV_rec);
					CDTV_rec->song = oldSong;							
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 8:	// Pause
					UA_InvertButton(window, &Controler_GR[ID]);
					CDTV_Pause(CDTV_rec);
					ShowTrack(window, CDTV_rec);
					UA_InvertButton(window, &Controler_GR[ID]);
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

	//FreeMem(waitPtr, sizeof(WaitPointer));

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutVarsToPI(CDTV_rec, ThisPI);

	return(retVal);
}

/******** SetButtons() ********/

void SetButtons(struct CDTV_record *CDTV_rec, struct Window *window,
								UWORD *mypattern1)
{
BOOL disable[30];
int i;

	for(i=0; i<30; i++)
		disable[i] = FALSE;	

	for(i=8; i<15; i++)
		UA_EnableButton(window, &CDTV_GR[i]);

	if ( CDTV_rec->fadeIn )
		UA_InvertButton(window, &CDTV_GR[14]);	// fade

	switch( CDTV_rec->action )	// play song, play from...to etc.
	{
		case 0:	// play song
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToVal(window, &CDTV_GR[8], CDTV_rec->song);
			break;

		case 1:	// play from...to
			disable[ 8] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToVal(window, &CDTV_GR[9], CDTV_rec->from);
			UA_SetStringGadgetToVal(window, &CDTV_GR[10], CDTV_rec->to);
			break;

		case 2:	// play start...end
			disable[ 8] = TRUE;
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToString(window, &CDTV_GR[11], CDTV_rec->start);
			UA_SetStringGadgetToString(window, &CDTV_GR[12], CDTV_rec->end);
			break;

		case 3:	// do command
			disable[ 8] = TRUE;
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[14] = TRUE;	// fade
			UA_SetCycleGadgetToVal(window, &CDTV_GR[13], CDTV_rec->command);
			break;
	}

	if ( !UA_IsGadgetDisabled( &CDTV_GR[16] ) ) 
		UA_SetCycleGadgetToVal(window, &CDTV_GR[16], CDTV_rec->control);

	for(i=8; i<15; i++)
		if ( disable[i] )
			UA_DisableButton(window, &CDTV_GR[i], mypattern1);
}

/******** E O F ********/
