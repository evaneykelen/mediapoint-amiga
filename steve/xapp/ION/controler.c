#include "nb:pre.h"
#include "protos.h"
#include "structs.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;
extern struct RendezVousRecord *rvrec;
extern struct MsgPort *capsport;

/**** functions ****/

/******** OpenControler() ********/

struct Window *OpenControler(	struct UserApplicInfo *UAI, STRPTR dragBarTitle,
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

	//if ( UAI->userScreen->ViewPort.Modes & LACE )
	//	SetFont(&(UAI->userScreen->RastPort), UAI->large_TF);
	//else
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

	//if ( UAI->userScreen->ViewPort.Modes & LACE )
	//	SetFont(UAI->userWindow->RPort, UAI->large_TF);
	//else
	SetFont(UAI->userWindow->RPort, UAI->small_TF);

	/**** modify defaults ****/

	UA_SwitchFlagsOn(UAI->userWindow, IDCMP_CLOSEWINDOW);

	SetWindowTitles(UAI->userWindow, dragBarTitle, NULL);

	/**** draw gadgets ****/

	UA_DrawGadgetList(UAI->userWindow, GR);
	DrawImage(UAI->userWindow->RPort, image, 3, 2);

	return(UAI->userWindow);
}

/******** CloseControler() ********/

void CloseControler(struct UserApplicInfo *UAI, struct GadgetRecord *GR)
{
int i;

	UA_SwitchFlagsOff(UAI->userWindow, IDCMP_CLOSEWINDOW);

	for(i=1; i<9; i++)
		UA_EnableButton(UAI->userWindow, &GR[i]);

	if (UAI->userWindow!=NULL)
		UA_CloseWindow(UAI);

	if (UAI->userScreen!=NULL)
		UA_CloseScreen(UAI);
}

/******** MonitorControler() ********/

BOOL MonitorControler(struct UserApplicInfo *UAI, struct Window *window,
											struct Ion_record *Ion_rec,
											struct GadgetRecord *Controler_GR, UWORD *mypattern1,
											STRPTR unitStr)
{
ULONG signals, signalMask;
struct EventData CED;
int ID, frame;
BOOL loop=TRUE;
TEXT frameStr[30], buffer[50];
struct IntuiMessage *message;

	UA_DisableButton(window, &Controler_GR[1], mypattern1);	// stop
	UA_DisableButton(window, &Controler_GR[4], mypattern1);	// fast rew & play
	UA_DisableButton(window, &Controler_GR[5], mypattern1);	// fast fwd & play
	UA_DisableButton(window, &Controler_GR[8], mypattern1);	// pause

	/**** init RV321 ****/

	if (!sendIV321code(Ion_rec, "T1"))	// track display on
		return(FALSE);
	Delay(CMD_FINISHED_DELAY);
	if (!sendIV321code(Ion_rec, "M0"))	// cancel image muting
		return(FALSE);
	Delay(CMD_FINISHED_DELAY);

	frame = Ion_rec->frame;

	sprintf(buffer, "A%02d", frame);
	if (!sendIV321code(Ion_rec, buffer))
		return(FALSE);
	Delay(100L);

	/**** show current frame ****/

	UA_ClearButton(window, &Controler_GR[9], AREA_PEN);
	sprintf(frameStr, "%d\n", frame);
	UA_DrawSpecialGadgetText(window, &Controler_GR[9], frameStr, SPECIAL_TEXT_CENTER);

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
		
					case MOUSEBUTTONS:
						if (CED.Code==SELECTDOWN)
						{
							CED.MouseY -= window->BorderTop;
							ID = UA_CheckGadgetList(window, Controler_GR, &CED);
							switch(ID)
							{
								case 2:	// play rev
									if (frame>1)
									{
										frame--;
										UA_InvertButton(window, &Controler_GR[ID]);
										sendIV321code(Ion_rec, "R");
										Delay(25L);
										UA_InvertButton(window, &Controler_GR[ID]);
										sprintf(frameStr, "%d\n", frame);
										UA_ClearButton(window, &Controler_GR[9], AREA_PEN);
										UA_DrawSpecialGadgetText(	window, &Controler_GR[9],
																							frameStr, SPECIAL_TEXT_CENTER);
									}
									break;
		
								case 3:	// play fwd
									if (frame<50)
									{
										frame++;
										UA_InvertButton(window, &Controler_GR[ID]);
										sendIV321code(Ion_rec, "F");
										Delay(25L);
										UA_InvertButton(window, &Controler_GR[ID]);
										sprintf(frameStr, "%d\n", frame);
										UA_ClearButton(window, &Controler_GR[9], AREA_PEN); 
										UA_DrawSpecialGadgetText(	window, &Controler_GR[9],
																							frameStr, SPECIAL_TEXT_CENTER);
									}
									break;
		
								case 6:	// fast rev
									UA_InvertButton(window, &Controler_GR[ID]);
									if (frame-10 < 1)
										frame=1;
									else
										frame-=10;
									sprintf(buffer, "A%02d", frame);
									sendIV321code(Ion_rec, buffer);
									Delay(50L);
									UA_InvertButton(window, &Controler_GR[ID]);
									sprintf(frameStr, "%d\n", frame);
									UA_ClearButton(window, &Controler_GR[9], AREA_PEN); 
									UA_DrawSpecialGadgetText(	window, &Controler_GR[9],
																						frameStr, SPECIAL_TEXT_CENTER);
									break;
		
								case 7:	// fast fwd
									UA_InvertButton(window, &Controler_GR[ID]);
									if (frame+10 > 50)
										frame=50;
									else
										frame+=10;
									sprintf(buffer, "A%02d", frame);
									sendIV321code(Ion_rec, buffer);
									Delay(50L);
									UA_InvertButton(window, &Controler_GR[ID]);
									sprintf(frameStr, "%d\n", frame);
									UA_ClearButton(window, &Controler_GR[9], AREA_PEN); 
									UA_DrawSpecialGadgetText(	window, &Controler_GR[9],
																						frameStr, SPECIAL_TEXT_CENTER);
									break;
							}
						}
						break;
				}
			}
		}
	}

	return(TRUE);
}

/******** E O F ********/
