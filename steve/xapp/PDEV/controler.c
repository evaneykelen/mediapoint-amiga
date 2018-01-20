#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "player.h"
#include "playercode.h"
#include "defs.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;
extern struct RendezVousRecord *rvrec;
extern struct MsgPort *capsport;
extern UBYTE **msgs;

#define PlayerBase Pdev_rec->PlayerBase

/**** functions ****/

/******** OpenController() ********/

struct Window *OpenController(struct UserApplicInfo *UAI, STRPTR dragBarTitle,
															struct GadgetRecord *GR, struct Image *image)
{
	/**** open screen ****/

	UAI->screenWidth		= 640;
/*
	if ( rvrec->capsprefs->PalNtsc==PAL_MODE )
		UAI->screenHeight = 256;
	else
		UAI->screenHeight = 200;
*/
	UAI->screenDepth		= 2;
	UAI->screenModes		= HIRES;
	UA_OpenScreen(UAI);
	if (UAI->userScreen==NULL)
		return(NULL);

	SetFont(&(UAI->userScreen->RastPort), UAI->small_TF);

	/**** open window ****/

	UAI->windowModes	= 3;	// open on the first (frontmost) screen
	UAI->windowX			= -1;	// -1 means center on screen
	UAI->windowY			= -1;	// -1 means center on screen
	UAI->windowWidth	= GR[0].x2;
	UAI->windowHeight	= GR[0].y2 + UAI->userScreen->RastPort.TxBaseline + 1;
	UAI->wflg					= WFLG_ACTIVATE | WFLG_DRAGBAR | WFLG_CLOSEGADGET |
											WFLG_GIMMEZEROZERO | WFLG_RMBTRAP;
	UA_OpenWindow(UAI);

	/**** modify defaults ****/

	UA_SwitchFlagsOn(UAI->userWindow, IDCMP_INTUITICKS | IDCMP_CLOSEWINDOW);

	SetWindowTitles(UAI->userWindow, dragBarTitle, NULL);

	/**** draw gadgets ****/

	UA_DrawGadgetList(UAI->userWindow, GR);
	DrawImage(UAI->userWindow->RPort, image, 3, 2);

	return(UAI->userWindow);
}

/******** CloseController() ********/

void CloseController(struct UserApplicInfo *UAI, struct GadgetRecord *GR)
{
	UA_SwitchFlagsOff(UAI->userWindow, IDCMP_INTUITICKS | IDCMP_CLOSEWINDOW);

	if (UAI->userWindow!=NULL)
		UA_CloseWindow(UAI);

	if (UAI->userScreen!=NULL)
		UA_CloseScreen(UAI);
}

/******** MonitorController() ********/

BOOL MonitorController(	struct UserApplicInfo *UAI, struct Window *window,
												struct Pdev_record *Pdev_rec,
												int *frameReturn, struct GadgetRecord *Controler_GR)
{
ULONG signals, signalMask;
struct EventData CED;
int ID, frameNum, lastCmd=-1, scanCode=-1, ticks=0;
BOOL loop=TRUE, videoOn = TRUE, paused = FALSE;
TEXT frameStr[30];
struct IntuiMessage *message;

	if ( *frameReturn!=-1 )
	{
		if (!CommandPlayer(	Pdev_rec, AC_AFGOST, *frameReturn, 0, (1L << capsport->mp_SigBit)))
			return(FALSE);
		if (!CommandPlayer(	Pdev_rec, AC_AQENDM, 0, 0, (1L << capsport->mp_SigBit)))
			return(FALSE);
	}

	/**** show current frame ****/

	if ( !CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit)) )	// Frame code request
		return(FALSE);
	sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
	if (frameNum != 99999)
	{
		UA_ClearButton(window, &Controler_GR[9], AREA_PEN);
		stccpy(frameStr, Pdev_rec->IOblock->iop_Argout, 20);

		if ( strnicmp(frameStr,"internal",8)==0 )
		{
			GiveMessage(window,msgs[Msg_X_3-1]);
			return(FALSE);
		}

		UA_DrawSpecialGadgetText(	window, &Controler_GR[9],
															frameStr,	SPECIAL_TEXT_CENTER);

		/**** init player ****/

		if (!CommandPlayer(Pdev_rec, AC_AVVIOF,0,0, (1L << capsport->mp_SigBit)))		// Video off
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_DCLEAR,0,0, (1L << capsport->mp_SigBit)))		// Clear internal buffers
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_AVPION,0,0, (1L << capsport->mp_SigBit)))		// Picture index on
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_AVCION,0,0, (1L << capsport->mp_SigBit)))		// Chapter index on
			return(FALSE);

		/**** go back to start counter ****/

		if (frameNum != 99999)
		{
			if (!CommandPlayer(Pdev_rec, AC_AFGOST, frameNum, 0, (1L << capsport->mp_SigBit)))
				return(FALSE);
			if (!CommandPlayer(Pdev_rec, AC_AQENDM, 0, 0, (1L << capsport->mp_SigBit)))
				return(FALSE);
		}

		if (!CommandPlayer(Pdev_rec, AC_AVVION, 0, 0, (1L << capsport->mp_SigBit)))	// Video on
			return(FALSE);

		/**** event handler ****/

		signalMask = (1L << capsport->mp_SigBit);
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
					CED.MouseX		= message->MouseX;
					CED.MouseY		= message->MouseY;
					ReplyMsg((struct Message *)message);

					switch(CED.Class)
					{
						case CLOSEWINDOW:
							loop=FALSE;
							break;

						case INTUITICKS:
							/**** if frameNum is 99999 then this device supports ****/
							/**** no frame numbers ****/ 
							if (frameNum != 99999)
							{
								if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit))) // Frame code request
									return(FALSE);
								UA_ClearButton(window, &Controler_GR[9], AREA_PEN); 
								stccpy(frameStr, Pdev_rec->IOblock->iop_Argout, 20);
								UA_DrawSpecialGadgetText(window, &Controler_GR[9],
																				 frameStr, SPECIAL_TEXT_CENTER);
							}
							if (ticks>1)
							{
								if (scanCode==1)
								{
									while ( !((*((char *)0xbfe001))&64) )
									{
										if (!CommandPlayer(Pdev_rec, AC_AXCRVS,0,0, (1L << capsport->mp_SigBit)))	// scan revers
											return(FALSE);
									}
									if (!CommandPlayer(Pdev_rec, AC_ASSTIL,0,0, (1L << capsport->mp_SigBit)))
										return(FALSE);
									scanCode=-1;
								}
								else if (scanCode==2)
								{
									while ( !((*((char *)0xbfe001))&64) )
									{
										if (!CommandPlayer(Pdev_rec, AC_AXCFWD,0,0, (1L << capsport->mp_SigBit)))	// scan forward
											return(FALSE);
									}
									if (!CommandPlayer(Pdev_rec, AC_ASSTIL,0,0, (1L << capsport->mp_SigBit)))
										return(FALSE);
									scanCode=-1;
								}
							}
							else
								ticks++;
							break;

						case MOUSEBUTTONS:
							if (CED.Code==SELECTUP)
								scanCode=-1;
							else if (CED.Code==SELECTDOWN)
							{
								CED.MouseY -= window->BorderTop;

								ID = UA_CheckGadgetList(window, Controler_GR, &CED);
		
								/**** if paused and next command is not pause then ****/
								/**** swith off pause ****/

								if (ID>=1 && ID<8 && paused)
								{
									paused = FALSE;
									UA_InvertButton(window, &Controler_GR[8]);
								}
	
								if (ID!=6 && ID!=7)
									scanCode=-1;

								switch(ID)
								{
									case 1:	/* stop */
										UA_HiliteButton(window, &Controler_GR[ID]);
										if (!CommandPlayer(Pdev_rec, AC_AVVIOF,0,0, (1L << capsport->mp_SigBit)))		// Video off
											return(FALSE);
										if (!CommandPlayer(Pdev_rec, AC_ASSTIL,0,0, (1L << capsport->mp_SigBit)))		// Still
											return(FALSE);
										videoOn=FALSE;
										lastCmd=-1;
										break;

									case 2:	/* play rev */
										UA_HiliteButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
										{
											if (frameNum != 99999)
											{
												if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit))) // Frame code request
													return(FALSE);
												sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
												frameNum -= 1000;
												if (frameNum < 1)
													frameNum = 1;
												if (!CommandPlayer(Pdev_rec, AC_AFGOST,frameNum,0, (1L << capsport->mp_SigBit))) // search
													return(FALSE);
												if (!CommandPlayer(Pdev_rec, AC_AQENDM,frameNum,0, (1L << capsport->mp_SigBit))) // wait
													return(FALSE);
											}
										}
										if (!CommandPlayer(Pdev_rec, AC_AMPNR,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=ID;
										break;
	
									case 3:	/* play fwd */
										UA_HiliteButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
										{
											if (frameNum != 99999)
											{
												if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit))) // Frame code request
													return(FALSE);
												sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
												frameNum += 1000;
												if (frameNum > 53999)
													frameNum = 53999;
												if (!CommandPlayer(Pdev_rec, AC_AFGOST,frameNum,0, (1L << capsport->mp_SigBit)))	// search
													return(FALSE);
												if (!CommandPlayer(Pdev_rec, AC_AQENDM,frameNum,0, (1L << capsport->mp_SigBit)))	// wait
													return(FALSE);
											}
										}
										if (!CommandPlayer(Pdev_rec, AC_AMPNF,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=ID;
										break;

									case 4:	/* step rev */
										UA_InvertButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if (!CommandPlayer(Pdev_rec, AC_AMSTR,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=-1;
										UA_InvertButton(window, &Controler_GR[ID]);
										break;

									case 5:	/* step fwd */
										UA_InvertButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if (!CommandPlayer(Pdev_rec, AC_AMSTF,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=-1;
										UA_InvertButton(window, &Controler_GR[ID]);
										break;

									case 6:	/* fast rev */
										UA_HiliteButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
										{
											if (frameNum != 99999)
											{
												if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit))) // Frame code request
													return(FALSE);
												sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
												frameNum -= 10000;
												if (frameNum < 1)
													frameNum = 1;
												if (!CommandPlayer(Pdev_rec, AC_AFGOST,frameNum,0, (1L << capsport->mp_SigBit)))	// search
													return(FALSE);
												if (!CommandPlayer(Pdev_rec, AC_AQENDM,frameNum,0, (1L << capsport->mp_SigBit)))	// wait
													return(FALSE);
											}
										}
										if (!CommandPlayer(Pdev_rec, AC_AMPFR,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=ID;
										scanCode=1;
										ticks=0;
										break;

									case 7:	/* fast fwd */
										UA_HiliteButton(window, &Controler_GR[ID]);
										if (!videoOn)
										{
											videoOn = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
												return(FALSE);
										}
										if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
										{
											if (frameNum != 99999)
											{
												if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit))) // Frame code request
													return(FALSE);
												sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
												frameNum += 10000;
												if (frameNum > 53999)
													frameNum = 53999;
												if (!CommandPlayer(Pdev_rec, AC_AFGOST,frameNum,0, (1L << capsport->mp_SigBit)))	// search
													return(FALSE);
												if (!CommandPlayer(Pdev_rec, AC_AQENDM,frameNum,0, (1L << capsport->mp_SigBit)))	// wait
													return(FALSE);
											}
										}
										if (!CommandPlayer(Pdev_rec, AC_AMPFF,0,0, (1L << capsport->mp_SigBit)))
											return(FALSE);
										lastCmd=ID;
										scanCode=2;
										ticks=0;
										break;

									case 8:	/* pause */
										UA_InvertButton(window, &Controler_GR[ID]);
										if (!paused)
										{
											paused = TRUE;
											if (!CommandPlayer(Pdev_rec, AC_ASSTIL,0,0, (1L << capsport->mp_SigBit)))
												return(FALSE);
										}
										else
										{
											paused = FALSE;
											if (!videoOn)
											{
												videoOn = TRUE;
												if (!CommandPlayer(Pdev_rec, AC_AVVION,0,0, (1L << capsport->mp_SigBit)))	// Video on
													return(FALSE);
											}
											if (lastCmd==2)
											{
												if (!CommandPlayer(Pdev_rec, AC_AMPNR,0,0, (1L << capsport->mp_SigBit)))	// play rev
													return(FALSE);
											}
											else if (lastCmd==3)
											{
												if (!CommandPlayer(Pdev_rec, AC_AMPNF,0,0, (1L << capsport->mp_SigBit)))	// play fwd
													return(FALSE);
											}
											else if (lastCmd==6)
											{
												if (!CommandPlayer(Pdev_rec, AC_AMPFR,0,0, (1L << capsport->mp_SigBit)))	// fast rev
													return(FALSE);
											}
											else if (lastCmd==7)
											{
												if (!CommandPlayer(Pdev_rec, AC_AMPFF,0,0, (1L << capsport->mp_SigBit)))	// fast fwd
													return(FALSE);
											}
										}
										break;
								}
							}
							break;
					}
				}
			}
		}
	}

	if (frameNum != 99999)
	{
		if (!CommandPlayer(Pdev_rec, AC_AFREQ,0,0, (1L << capsport->mp_SigBit)))	// Frame code request
			return(FALSE);
		sscanf((char*)Pdev_rec->IOblock->iop_Argout, "%d", &frameNum);
		*frameReturn = frameNum;
	}
	else
		*frameReturn = 0;

	return(TRUE);
}

/******** E O F ********/
