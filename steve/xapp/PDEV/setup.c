#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"
#include "defs.h"
#include "player.h"
#include "playercode.h"

#define VERSI0N "\0$VER: 1.3"          
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Pdev_record *Pdev_rec, struct UserApplicInfo *UAI);

/**** external function declarations ****/

extern void GetVarsFromPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;
struct Library *PlayerBase					= NULL;
BOOL failure=FALSE, pdev_opened=FALSE;

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
struct Pdev_record Pdev_rec;
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

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** open special libraries or devices ****/

	pdev_opened = OpenPlayerDevice(&Pdev_rec);

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** copy image to chip memory ****/

	chipMem = (UBYTE *)AllocMem(sizeof(ImageDataControler), MEMF_CHIP);
	if (chipMem==NULL)
	{
		ClosePlayerDevice(&Pdev_rec);
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
		UA_DoubleGadgetDimensions(Pdev_GR);

	UA_TranslateGR(Pdev_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Pdev_GR[0].x2;
	UAI.windowHeight	= Pdev_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	Pdev_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Pdev_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &Pdev_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** free chip mem ****/

	FreeMem(chipMem, sizeof(ImageDataControler));

	/**** close specials libraries or devices ****/

	if (pdev_opened)	// && !failure)
		ClosePlayerDevice(&Pdev_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Pdev_record *Pdev_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID, frameReturn;
UWORD *mypattern1;
struct Window *controlerWindow;
TEXT dragBarTitle[25];
struct UserApplicInfo c_UAI;

	CopyMem(UAI,&c_UAI,sizeof(struct UserApplicInfo));

	strcpy(dragBarTitle, msgs[Msg_X_LD_1-1]);

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(Pdev_rec, ThisPI);
	else
	{
		Pdev_rec->videoOn			= TRUE;
		Pdev_rec->audio1On		= TRUE;
		Pdev_rec->audio2On		= TRUE;
		Pdev_rec->indexOn			= TRUE;
		Pdev_rec->action			= pd_Play;
		Pdev_rec->param1			= 1000;
		Pdev_rec->param2			= 1200;
		Pdev_rec->blank				= FALSE;
		Pdev_rec->init				= FALSE;
	}

	/**** set buttons ****/

	if (Pdev_rec->videoOn)
		UA_InvertButton(window, &Pdev_GR[7]);

	if (Pdev_rec->audio1On)
		UA_InvertButton(window, &Pdev_GR[8]);

	if (Pdev_rec->audio2On)
		UA_InvertButton(window, &Pdev_GR[9]);

	if (Pdev_rec->indexOn)
		UA_InvertButton(window, &Pdev_GR[10]);

	UA_SetStringGadgetToVal(window, &Pdev_GR[12], Pdev_rec->param1);
	UA_SetStringGadgetToVal(window, &Pdev_GR[13], Pdev_rec->param2);

	UA_SetCycleGadgetToVal(window, &Pdev_GR[16], Pdev_rec->action);

	if (Pdev_rec->init)
		UA_InvertButton(window, &Pdev_GR[18]);

	if (Pdev_rec->action != pd_Play && Pdev_rec->action != pd_Search)
	{
		UA_DisableButton(window, &Pdev_GR[12], mypattern1);	// start
		UA_DisableButton(window, &Pdev_GR[14], mypattern1);	// start
	}
	else
	{
		UA_EnableButton(window, &Pdev_GR[12]);	// start
		UA_EnableButton(window, &Pdev_GR[14]);	// start
	}
	if (Pdev_rec->action != pd_Play)
	{
		UA_DisableButton(window, &Pdev_GR[13], mypattern1);	// end
		UA_DisableButton(window, &Pdev_GR[15], mypattern1);	// end
	}
	else
	{
		UA_EnableButton(window, &Pdev_GR[13]);	// end
		UA_EnableButton(window, &Pdev_GR[15]);	// end
	}

	if ( !pdev_opened )
	{
		UA_DisableButton(window, &Pdev_GR[ 5], mypattern1);
		UA_DisableButton(window, &Pdev_GR[11], mypattern1);
		UA_DisableButton(window, &Pdev_GR[14], mypattern1);
		UA_DisableButton(window, &Pdev_GR[15], mypattern1);
		GiveMessage(window, msgs[Msg_PdevMisses-1]);
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			/**** CDTV window gadgets ****/

			ID = UA_CheckGadgetList(window, Pdev_GR, &CED);
			switch(ID)
			{
				case 4:	// OK
do_ok:
					UA_HiliteButton(window, &Pdev_GR[4]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 5:	// Preview
					UA_InvertButton(window, &Pdev_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					failure=FALSE;
					if (!PerformActions(Pdev_rec,(1L << capsport->mp_SigBit)))	//NULL,NULL,NULL))
					{
						failure=TRUE;
						GiveMessage(window, msgs[Msg_X_3-1]);
					}
					UA_SetSprite(window, SPRITE_NORMAL);
					UA_InvertButton(window, &Pdev_GR[ID]);
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Pdev_GR[6]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 7:	// video
					UA_InvertButton(window, &Pdev_GR[ID]);
					if (Pdev_rec->videoOn)
						Pdev_rec->videoOn=FALSE;
					else
						Pdev_rec->videoOn=TRUE;
					break;

				case 8:	// audio 1
					UA_InvertButton(window, &Pdev_GR[ID]);
					if (Pdev_rec->audio1On)
						Pdev_rec->audio1On=FALSE;
					else
						Pdev_rec->audio1On=TRUE;
					break;

				case 9:	// audio 2
					UA_InvertButton(window, &Pdev_GR[ID]);
					if (Pdev_rec->audio2On)
						Pdev_rec->audio2On=FALSE;
					else
						Pdev_rec->audio2On=TRUE;
					break;

				case 10:	// index
					UA_InvertButton(window, &Pdev_GR[ID]);
					if (Pdev_rec->indexOn)
						Pdev_rec->indexOn=FALSE;
					else
						Pdev_rec->indexOn=TRUE;
					break;

				case 11:	// controler
					UA_InvertButton(window, &Pdev_GR[ID]);
					controlerWindow = OpenController(&c_UAI,dragBarTitle,Controler_GR,&ImageControler);
					if (controlerWindow!=NULL)
					{
						frameReturn=-1;	// use current frame
						Pdev_rec->window = NULL;
						failure=FALSE;
						if ( !MonitorController(	&c_UAI, controlerWindow, Pdev_rec,
																			&frameReturn, Controler_GR) )
							failure=TRUE;
						Pdev_rec->window = window;
						CloseController(&c_UAI,Controler_GR);
					}
					UA_InvertButton(window, &Pdev_GR[ID]);
					break;

				case 12:	// start
					UA_ProcessStringGadget(window, Pdev_GR, &Pdev_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&Pdev_GR[ID], &Pdev_rec->param1);
					break;

				case 13:	// end
					UA_ProcessStringGadget(window, Pdev_GR, &Pdev_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&Pdev_GR[ID], &Pdev_rec->param2);
					break;

				case 14:	// start
				case 15:	// end
					UA_HiliteButton(window, &Pdev_GR[ID]);
					UA_InvertButton(window, &Pdev_GR[11]);
					controlerWindow = OpenController(&c_UAI,dragBarTitle,Controler_GR,&ImageControler);
					if (controlerWindow!=NULL)
					{
						if (ID==14)
							frameReturn=Pdev_rec->param1;
						else
							frameReturn=Pdev_rec->param2;
						Pdev_rec->window = NULL;
						MonitorController(&c_UAI, controlerWindow, Pdev_rec,
															&frameReturn, Controler_GR);
						Pdev_rec->window = window;
						CloseController(&c_UAI,Controler_GR);
					}
					UA_InvertButton(window, &Pdev_GR[11]);
					if (ID==14)	// start
					{
						Pdev_rec->param1 = frameReturn;
						UA_SetStringGadgetToVal(window, &Pdev_GR[12], Pdev_rec->param1);
					}
					else	// end
					{
						Pdev_rec->param2 = frameReturn;
						UA_SetStringGadgetToVal(window, &Pdev_GR[13], Pdev_rec->param2);
					}
					break;

				case 16:
					UA_ProcessCycleGadget(window, &Pdev_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Pdev_GR[ID], &(Pdev_rec->action));
					if (Pdev_rec->action != pd_Play && Pdev_rec->action != pd_Search)
					{
						UA_DisableButton(window, &Pdev_GR[12], mypattern1);	// start
						UA_DisableButton(window, &Pdev_GR[14], mypattern1);	// start
					}
					else
					{
						UA_EnableButton(window, &Pdev_GR[12]);	// start
						UA_EnableButton(window, &Pdev_GR[14]);	// start
					}
					if (Pdev_rec->action != pd_Play)
					{
						UA_DisableButton(window, &Pdev_GR[13], mypattern1);	// end
						UA_DisableButton(window, &Pdev_GR[15], mypattern1);	// end
					}
					else
					{
						UA_EnableButton(window, &Pdev_GR[13]);	// end
						UA_EnableButton(window, &Pdev_GR[15]);	// end
					}
					break;

				case 18:	// init
					UA_InvertButton(window, &Pdev_GR[ID]);
					if (Pdev_rec->init)
						Pdev_rec->init=FALSE;
					else
						Pdev_rec->init=TRUE;
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
		PutVarsToPI(Pdev_rec, ThisPI);

	return(retVal);
}

/******** E O F ********/
