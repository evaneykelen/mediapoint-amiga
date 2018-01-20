#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Ion_record *Ion_rec,
									struct UserApplicInfo *UAI);

/**** external function declarations ****/

extern void GetVarsFromPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate, STRPTR unitStr);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
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
UBYTE *chipMem;
struct Ion_record Ion_rec;
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

	/**** parse .info file for special settings ****/

	GetInfoFile(ThisPI, "ION", Ion_rec.devName, &(Ion_rec.portNr), &(Ion_rec.baudRate),
							Ion_rec.unitStr);

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort((struct standard_record *)&Ion_rec,8,8,2) )
		return;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** copy image to chip memory ****/

	chipMem = (UBYTE *)AllocMem(sizeof(ImageDataControler), MEMF_CHIP);
	if (chipMem==NULL)
	{
		Close_SerialPort((struct standard_record *)&Ion_rec);
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
		UA_DoubleGadgetDimensions(CSV_GR);

	UA_TranslateGR(CSV_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= CSV_GR[0].x2;
	UAI.windowHeight	= CSV_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	Ion_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, CSV_GR);
	UA_DrawSpecialGadgetText(UAI.userWindow, &CSV_GR[7], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &Ion_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** free chip mem ****/

	FreeMem(chipMem, sizeof(ImageDataControler));

	/**** close specials libraries or devices ****/

	Close_SerialPort((struct standard_record *)&Ion_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Ion_record *Ion_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal, tab;
struct EventData CED;
int ID;
UWORD *mypattern1; //, *waitPtr;
struct Window *controlerWindow;
TEXT dragBarTitle[25];
struct UserApplicInfo c_UAI;

	CopyMem(UAI,&c_UAI,sizeof(struct UserApplicInfo));

	strcpy(dragBarTitle, msgs[Msg_X_I_1-1]); // "Canon RV-321 controller"

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(Ion_rec, ThisPI);
	else
	{
		Ion_rec->action			= 1;
		Ion_rec->counterOn	= TRUE;
		Ion_rec->frame			= 1;
		Ion_rec->startFrame	= 1;
		Ion_rec->endFrame		= 2;
		Ion_rec->delay			= 1;
		Ion_rec->blank			= FALSE;
	}

	/**** set buttons ****/

	if (Ion_rec->counterOn)
		UA_InvertButton(window, &CSV_GR[17]);

	if (Ion_rec->blank)
		UA_InvertButton(window, &CSV_GR[8]);

	UA_SetStringGadgetToVal(window, &CSV_GR[4], Ion_rec->frame);
	UA_SetStringGadgetToVal(window, &CSV_GR[5], Ion_rec->startFrame);
	UA_SetStringGadgetToVal(window, &CSV_GR[6], Ion_rec->endFrame);
	UA_SetStringGadgetToVal(window, &CSV_GR[7], Ion_rec->delay);

	if (Ion_rec->action == 1)
	{
		UA_InvertButton(window, &CSV_GR[2]);
		UA_DisableButton(window, &CSV_GR[5], mypattern1);	// from
		UA_DisableButton(window, &CSV_GR[6], mypattern1);	// to
	}
	else
	{
		UA_InvertButton(window, &CSV_GR[3]);
		UA_DisableButton(window, &CSV_GR[4], mypattern1);	// from
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, CSV_GR, &CED);
			switch(ID)
			{
				case 2:	// mode 1 (show frame)
					if ( Ion_rec->action==2 )
					{
						UA_InvertButton(window, &CSV_GR[2]);
						UA_InvertButton(window, &CSV_GR[3]);
						Ion_rec->action=1;
						UA_EnableButton(window, &CSV_GR[4]);
						UA_DisableButton(window, &CSV_GR[5], mypattern1);
						UA_DisableButton(window, &CSV_GR[6], mypattern1);
						goto from;
					}
					break;

				case 3:	// mode 2 (show from...to)
					if ( Ion_rec->action==1 )
					{
						UA_InvertButton(window, &CSV_GR[2]);
						UA_InvertButton(window, &CSV_GR[3]);
						Ion_rec->action=2;
						UA_DisableButton(window, &CSV_GR[4], mypattern1);
						UA_EnableButton(window, &CSV_GR[5]);
						UA_EnableButton(window, &CSV_GR[6]);
						goto start;
					}
					break;

				case 4:	// from
from:
					UA_ProcessStringGadget(window, CSV_GR, &CSV_GR[4], &CED);
					UA_SetValToStringGadgetVal(&CSV_GR[4], &Ion_rec->frame);
					if (Ion_rec->frame<1)
					{	
						Ion_rec->frame=1;
						UA_SetStringGadgetToVal(window, &CSV_GR[4], 1);
					}
					else if (Ion_rec->frame>50)
					{	
						Ion_rec->frame=50;
						UA_SetStringGadgetToVal(window, &CSV_GR[4], 50);
					}
					break;

				case 5:	// start
start:
					tab = UA_ProcessStringGadget(window, CSV_GR, &CSV_GR[5], &CED);
					UA_SetValToStringGadgetVal(&CSV_GR[5], &Ion_rec->startFrame);
					if (Ion_rec->startFrame<1)
					{	
						Ion_rec->startFrame=1;
						UA_SetStringGadgetToVal(window, &CSV_GR[5], 1);
					}
					else if (Ion_rec->startFrame>50)
					{	
						Ion_rec->startFrame=50;
						UA_SetStringGadgetToVal(window, &CSV_GR[5], 50);
					}
					if (tab)
						goto end;							
					break;

				case 6:	// end
end:
					tab = UA_ProcessStringGadget(window, CSV_GR, &CSV_GR[6], &CED);
					UA_SetValToStringGadgetVal(&CSV_GR[6], &Ion_rec->endFrame);
					if (Ion_rec->endFrame<1)
					{	
						Ion_rec->endFrame=1;
						UA_SetStringGadgetToVal(window, &CSV_GR[6], 1);
					}
					else if (Ion_rec->endFrame>50)
					{	
						Ion_rec->endFrame=50;
						UA_SetStringGadgetToVal(window, &CSV_GR[6], 50);
					}
					if (tab)
						goto start;							
					break;

				case 7:	// duration
					UA_ProcessStringGadget(window, CSV_GR, &CSV_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&CSV_GR[ID], &Ion_rec->delay);
					break;

				case 8:	// blank
					UA_InvertButton(window, &CSV_GR[ID]);
					if ( Ion_rec->blank )
						Ion_rec->blank=FALSE;
					else
						Ion_rec->blank=TRUE;
					break;

				case 9:	// OK
do_ok:
					UA_HiliteButton(window, &CSV_GR[9]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 10:	// Preview
					UA_InvertButton(window, &CSV_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					if (!PerformActions(Ion_rec))
						GiveMessage(window, msgs[Msg_X_3-1]); // "Communication with device aborted."
					UA_SetSprite(window, SPRITE_NORMAL);
					UA_InvertButton(window, &CSV_GR[ID]);
					break;

				case 11:	// Cancel
do_cancel:
					UA_HiliteButton(window, &CSV_GR[11]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 17:	// counter
					UA_InvertButton(window, &CSV_GR[ID]);
					if ( Ion_rec->counterOn )
						Ion_rec->counterOn=FALSE;
					else
						Ion_rec->counterOn=TRUE;
					break;
					
				case 18:	// controler
					UA_InvertButton(window, &CSV_GR[ID]);
					controlerWindow = OpenControler(&c_UAI,dragBarTitle,Controler_GR,&ImageControler);
					if (controlerWindow!=NULL)
					{
						Ion_rec->window = NULL;
						if (!MonitorControler(&c_UAI, controlerWindow, Ion_rec,
																	Controler_GR, mypattern1,
																	Ion_rec->unitStr))
							GiveMessage(window, msgs[Msg_X_3-1]);	// "Communication with device aborted."
						Ion_rec->window = window;
						CloseControler(&c_UAI, Controler_GR);
					}
					UA_InvertButton(window, &CSV_GR[ID]);
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
		PutVarsToPI(Ion_rec, ThisPI);

	return(retVal);
}

/******** E O F ********/
