#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "protos.h"
#include "structs.h"
#include "setup.h"

/**** function declarations ****/

extern void GetVarsFromPI(struct GVR_record *, PROCESSINFO *);
extern void PutVarsToPI(struct GVR_record *, PROCESSINFO *);
BOOL MonitorUser(struct Window *, PROCESSINFO *, struct GVR_record *, struct UserApplicInfo *);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName,
									int *portNr, int *baudRate );

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

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
UBYTE *chipMem;
struct GVR_record GVR_rec;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort("MP rendez-vous");
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

	GetInfoFile(ThisPI, "GVR", GVR_rec.devName, &(GVR_rec.portNr), &(GVR_rec.baudRate) );

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort((struct standard_record *)&GVR_rec,8,8,2) )
		return;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** copy image to chip memory ****/

	chipMem = (UBYTE *)AllocMem(sizeof(ImageDataController), MEMF_CHIP);
	if (chipMem==NULL)
	{
		Close_SerialPort((struct standard_record *)&GVR_rec);
		return;
	}

	CopyMem(ImageDataController, chipMem, sizeof(ImageDataController));
	ImageController.ImageData = (SHORT *)chipMem;	

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

	capsport = (struct MsgPort *)FindPort("MediaPointPort");
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(GVR_GR);

	/**** open the window ****/

	UAI.windowX			 = -1;	/* -1 means center on screen */
	UAI.windowY			 = -1;	/* -1 means center on screen */
	UAI.windowWidth	 = GVR_GR[0].x2;
	UAI.windowHeight = GVR_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	GVR_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, GVR_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &GVR_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** free chip mem ****/

	FreeMem(chipMem, sizeof(ImageDataController));

	/**** close specials libraries or devices ****/

	Close_SerialPort((struct standard_record *)&GVR_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct GVR_record *GVR_rec, struct UserApplicInfo *UAI)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
UWORD *mypattern1;
struct Window *controllerWindow;
TEXT dragBarTitle[25];
struct UserApplicInfo c_UAI;

	/**** init vars, alloc memory ****/

	CopyMem(UAI,&c_UAI,sizeof(struct UserApplicInfo));

	strcpy(dragBarTitle, "GVR Controller");

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(GVR_rec, ThisPI);
	else
	{
		GVR_rec->video	= TRUE;
		GVR_rec->audio	= TRUE;
		GVR_rec->blank	= FALSE;
		GVR_rec->init		= FALSE;
		strcpy(GVR_rec->start, "00:00:00:00");
		strcpy(GVR_rec->end, "00:00:00:00");
		strcpy(GVR_rec->current, "00:00:00:00");
		GVR_rec->cmd		= GVR_PLAY;
	}

	/**** set buttons ****/

	if (GVR_rec->video)
		UA_InvertButton(window, &GVR_GR[7]);

	if (GVR_rec->audio)
		UA_InvertButton(window, &GVR_GR[8]);

	if (GVR_rec->blank)
		UA_InvertButton(window, &GVR_GR[16]);

	if (GVR_rec->init)
		UA_InvertButton(window, &GVR_GR[17]);

	UA_SetStringGadgetToString(window, &GVR_GR[13], GVR_rec->start);
	UA_SetStringGadgetToString(window, &GVR_GR[14], GVR_rec->end);

	UA_SetCycleGadgetToVal(window, &GVR_GR[10], GVR_rec->cmd);

	if ( EnDisAbleList[GVR_rec->cmd] == 0 )
	{
		UA_DisableButton(window, &GVR_GR[11], mypattern1);
		UA_DisableButton(window, &GVR_GR[12], mypattern1);
		UA_DisableButton(window, &GVR_GR[13], mypattern1);
		UA_DisableButton(window, &GVR_GR[14], mypattern1);
	}
	else if ( EnDisAbleList[GVR_rec->cmd] == 1 )
	{
		UA_DisableButton(window, &GVR_GR[12], mypattern1);
		UA_DisableButton(window, &GVR_GR[14], mypattern1);
	}

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);

	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			while( message = (struct IntuiMessage *)GetMsg(window->UserPort) )
			{
				CED.Class			= message->Class;
				CED.Code			= message->Code;
				CED.Qualifier	= message->Qualifier;
				CED.MouseX 		= message->MouseX;
				CED.MouseY 		= message->MouseY;
				ReplyMsg((struct Message *)message);

				if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, GVR_GR, &CED);
					switch(ID)
					{
						case 4:		// OK
do_ok:
							UA_HiliteButton(window, &GVR_GR[4]);
							retVal=TRUE;
							loop=FALSE;
							break;

						case 5:		// Preview
							UA_InvertButton(window, &GVR_GR[ID]);
							UA_SetSprite(window, SPRITE_BUSY);
							if (!PerformActions(GVR_rec))
								GiveMessage(window, "Communication with device aborted.");
							UA_SetSprite(window, SPRITE_NORMAL);
							Delay(4L);
							UA_InvertButton(window, &GVR_GR[ID]);
							break;

						case 6:		// Cancel
do_cancel:
							UA_HiliteButton(window, &GVR_GR[6]);
							retVal=FALSE;
							loop=FALSE;
							break;

						case 7:		// video toggle
							UA_InvertButton(window, &GVR_GR[ID]);
							if (GVR_rec->video) GVR_rec->video=FALSE; else GVR_rec->video=TRUE;
							break;

						case 8:		// audio toggle
							UA_InvertButton(window, &GVR_GR[ID]);
							if (GVR_rec->audio) GVR_rec->audio=FALSE; else GVR_rec->audio=TRUE;
							break;

						case 9:		// controller
						case 11:	// controller (start)
						case 12:	// controller (end)
							UA_InvertButton(window, &GVR_GR[ID]);
							controllerWindow = OpenController(&c_UAI, dragBarTitle,
																								Controller_GR,
																								&ImageController);
							if (controllerWindow!=NULL)
							{
								if ( ID==11 )	// copy start frame to current frame
									strcpy(GVR_rec->current, GVR_rec->start);
								else if ( ID==12 )	// copy end frame to current frame
									strcpy(GVR_rec->current, GVR_rec->end);

								if (!MonitorController(&c_UAI, controllerWindow, GVR_rec,
																			Controller_GR, mypattern1))
								{
									GiveMessage(window, "Communication with device aborted.");
								}
								else
								{
									if ( ID==11 )	// looking for start frame
									{
										strcpy(GVR_rec->start, GVR_rec->current);
										UA_SetStringGadgetToString(window, &GVR_GR[13], GVR_rec->start);
									}
									else if ( ID==12 )	// looking for end frame
									{
										strcpy(GVR_rec->end, GVR_rec->current);
										UA_SetStringGadgetToString(window, &GVR_GR[14], GVR_rec->end);
									}
								}
								CloseController(&c_UAI, Controller_GR);
							}
							UA_InvertButton(window, &GVR_GR[ID]);
							break;

						case 10:	// commands cycle
							UA_ProcessCycleGadget(window, &GVR_GR[ID], &CED);
							UA_SetValToCycleGadgetVal(&GVR_GR[ID], &GVR_rec->cmd);
							if ( EnDisAbleList[GVR_rec->cmd] == 0 )
							{
								UA_DisableButton(window, &GVR_GR[11], mypattern1);
								UA_DisableButton(window, &GVR_GR[12], mypattern1);
								UA_DisableButton(window, &GVR_GR[13], mypattern1);
								UA_DisableButton(window, &GVR_GR[14], mypattern1);
							}
							else if ( EnDisAbleList[GVR_rec->cmd] == 1 )
							{
								if ( UA_IsGadgetDisabled( &GVR_GR[13] ) )	// start
								{
									UA_EnableButton(window, &GVR_GR[11]);
									UA_EnableButton(window, &GVR_GR[13]);
								}
								UA_DisableButton(window, &GVR_GR[12], mypattern1);
								UA_DisableButton(window, &GVR_GR[14], mypattern1);
							}
							else if ( EnDisAbleList[GVR_rec->cmd] == 2 )
							{
								if ( UA_IsGadgetDisabled( &GVR_GR[13] ) )	// end
								{
									UA_EnableButton(window, &GVR_GR[11]);
									UA_EnableButton(window, &GVR_GR[13]);
								}
								if ( UA_IsGadgetDisabled( &GVR_GR[14] ) )	// end
								{
									UA_EnableButton(window, &GVR_GR[12]);
									UA_EnableButton(window, &GVR_GR[14]);
								}
							}
							break;

						case 13:	// starting frame code
							UA_ProcessStringGadget(window, GVR_GR, &GVR_GR[ID], &CED);
							UA_SetStringToGadgetString(&GVR_GR[ID], GVR_rec->start);
							break;                                                
						case 14:	// ending frame code
							UA_ProcessStringGadget(window, GVR_GR, &GVR_GR[ID], &CED);
							UA_SetStringToGadgetString(&GVR_GR[ID], GVR_rec->end);
							break;

						case 16:	// blank after show toggle
							UA_InvertButton(window, &GVR_GR[ID]);
							if (GVR_rec->blank) GVR_rec->blank=FALSE; else GVR_rec->blank=TRUE;
							break;

						case 17:	// init player toggle
							UA_InvertButton(window, &GVR_GR[ID]);
							if (GVR_rec->init) GVR_rec->init=FALSE; else GVR_rec->init=TRUE;
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
		}
	}

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutVarsToPI(GVR_rec, ThisPI);

	return(retVal);
}

/******** E O F ********/
