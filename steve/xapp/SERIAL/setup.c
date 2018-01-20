#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "setup.h"
#include "structs.h"
#include "demo:gen/support_protos.h"

#define VERSI0N "\0$VER: 1.0"          
static UBYTE *vers = VERSI0N;

extern void GetVarsFromPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI);
extern BOOL DoSerial(struct SerRecord *ser_rec);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);

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
void chkabort(void) { }

/**** functions ****/

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

	list	= &(port->mp_MsgList);
	node	= list->lh_Head;
	rvrec	= (struct RendezVousRecord *)node->ln_Name;

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
		UA_DoubleGadgetDimensions(Ser_GR);
	UA_TranslateGR(Ser_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Ser_GR[0].x2;
	UAI.windowHeight	= Ser_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Ser_GR);
	UA_DrawSpecialGadgetText(UAI.userWindow, &Ser_GR[ 8], msgs[Msg_Ser9 -1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(UAI.userWindow, &Ser_GR[15], msgs[Msg_Ser10-1], SPECIAL_TEXT_AFTER_STRING);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal=FALSE;
struct EventData CED;
int ID;
UWORD *mypattern1;
struct SerRecord ser_rec;
struct StringRecord Ser_DATA_SR;
struct PopUpRecord PUR;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(&ser_rec, ThisPI);
	else
	{
		ser_rec.data[0]			= '\0';
		ser_rec.baudrate		= 9600;
		ser_rec.handshaking	= 2;		// none
		ser_rec.parity			= 0;		// none
		ser_rec.bits_char		= 1;		// 8
		ser_rec.stop_bits		= 0;		// 1
		ser_rec.unit				= 1;		// 1
		ser_rec.pacing			= 0;		// 
		strcpy(ser_rec.devName,"serial.device");
	}

	UA_SetStringGadgetToVal(window, &Ser_GR[9], ser_rec.baudrate);
	UA_SetStringGadgetToVal(window, &Ser_GR[15], ser_rec.pacing);
	UA_SetStringGadgetToString(window, &Ser_GR[17], ser_rec.devName);

	UA_SetCycleGadgetToVal(window, &Ser_GR[10], ser_rec.handshaking);
	UA_SetCycleGadgetToVal(window, &Ser_GR[11], ser_rec.parity);
	UA_SetCycleGadgetToVal(window, &Ser_GR[12], ser_rec.bits_char);
	UA_SetCycleGadgetToVal(window, &Ser_GR[13], ser_rec.stop_bits);
	UA_SetCycleGadgetToVal(window, &Ser_GR[14], ser_rec.unit);

	SetAPen(window->RPort,LO_PEN);
	SetDrMd(window->RPort,JAM1);

	if ( window->WScreen->ViewPort.Modes & LACE )
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	Move(window->RPort, Ser_GR[16].x1+4, Ser_GR[16].y1+window->RPort->TxBaseline+4+(window->RPort->TxHeight+3)*0);
	Text(window->RPort, msgs[ Msg_Ser11-1 ], strlen(msgs[ Msg_Ser11-1 ]));
	Move(window->RPort, Ser_GR[16].x1+4, Ser_GR[16].y1+window->RPort->TxBaseline+4+(window->RPort->TxHeight+3)*1);
	Text(window->RPort, msgs[ Msg_Ser12-1 ], strlen(msgs[ Msg_Ser12-1 ]));
	Move(window->RPort, Ser_GR[16].x1+4, Ser_GR[16].y1+window->RPort->TxBaseline+4+(window->RPort->TxHeight+3)*2);
	Text(window->RPort, msgs[ Msg_Ser13-1 ], strlen(msgs[ Msg_Ser13-1 ]));
	Move(window->RPort, Ser_GR[16].x1+4, Ser_GR[16].y1+window->RPort->TxBaseline+4+(window->RPort->TxHeight+3)*3);
	Text(window->RPort, msgs[ Msg_Ser14-1 ], strlen(msgs[ Msg_Ser14-1 ]));

	if ( window->WScreen->ViewPort.Modes & LACE )
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);

	Ser_DATA_SR.maxLen = 255;
	Ser_DATA_SR.buffer = (UBYTE *)AllocMem(256,MEMF_ANY|MEMF_CLEAR);
	if ( !Ser_DATA_SR.buffer )
		loop=FALSE;
	Ser_GR[8].ptr = (struct GadgetRecord *)&Ser_DATA_SR;
	if ( ser_rec.data[0] )
		UA_SetStringGadgetToString(window, &Ser_GR[8], ser_rec.data);

	/**** pur ****/

	PUR.window	= NULL;
	PUR.GR			= PopUp_GR;
	PUR.ptr			= msgs[ Msg_BaudList-1 ];
	PUR.active	= 6;
	PUR.number	= 15;
	PUR.width		= 8;
	PUR.fit			= 0;
	PUR.top			= 0;

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Ser_GR, &CED);
			switch(ID)
			{
				case 2:		// OK
do_ok:
					UA_HiliteButton(window, &Ser_GR[2]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 3:		// Preview
					UA_InvertButton(window, &Ser_GR[ID]);
					DoSerial(&ser_rec);
					UA_InvertButton(window, &Ser_GR[ID]);
					break;
					
				case 4:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Ser_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 8:		// data
				case 9:		// baudrate
				case 15:	// pacing
				case 17:	// device name
					UA_ProcessStringGadget(window,Ser_GR,&Ser_GR[ID],&CED);
					if (ID== 8) UA_SetStringToGadgetString(&Ser_GR[ID],ser_rec.data);
					if (ID== 9) UA_SetValToStringGadgetVal(&Ser_GR[ID],&ser_rec.baudrate);
					if (ID==15) UA_SetValToStringGadgetVal(&Ser_GR[ID],&ser_rec.pacing);
					if (ID==17) UA_SetStringToGadgetString(&Ser_GR[ID],ser_rec.devName);
					break;

				case 10:	// handshaking
				case 11:	// parity
				case 12:	// bits/char
				case 13:	// stop bits
				case 14:	// unit
					UA_ProcessCycleGadget(window,&Ser_GR[ID],&CED);
					if (ID==10) UA_SetValToCycleGadgetVal(&Ser_GR[ID],&ser_rec.handshaking);
					if (ID==11) UA_SetValToCycleGadgetVal(&Ser_GR[ID],&ser_rec.parity);
					if (ID==12) UA_SetValToCycleGadgetVal(&Ser_GR[ID],&ser_rec.bits_char);
					if (ID==13) UA_SetValToCycleGadgetVal(&Ser_GR[ID],&ser_rec.stop_bits);
					if (ID==14) UA_SetValToCycleGadgetVal(&Ser_GR[ID],&ser_rec.unit);
					break;

				case 18:	// pop up
					UA_InvertButton(window, &Ser_GR[ID]);
					if ( UA_OpenPopUpWindow(window, &Ser_GR[ID], &PUR) )
					{
						UA_Monitor_PopUp(&PUR);
						UA_ClosePopUpWindow(&PUR);
						UA_SetStringGadgetToString(window, &Ser_GR[9], PUR.ptr+PUR.active*PUR.width);
						sscanf(PUR.ptr+PUR.active*PUR.width,"%d",&ser_rec.baudrate);
					}
					UA_InvertButton(window, &Ser_GR[ID]);
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

	if ( retVal )
		PutVarsToPI(&ser_rec, ThisPI);

	return(retVal);
}

/******** E O F ********/
