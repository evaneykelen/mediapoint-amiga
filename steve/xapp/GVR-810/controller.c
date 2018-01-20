#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "protos.h"
#include "structs.h"
#define GUI_DEFS
#include "setup.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct RendezVousRecord *rvrec;

STATIC void PrintFrameCode(	struct GVR_record *GVR_rec,
														struct GadgetRecord *Controller_GR );

/**** functions ****/

/******** OpenController() ********/

struct Window *OpenController(struct UserApplicInfo *UAI, STRPTR dragBarTitle,
															struct GadgetRecord *GR, struct Image *image)
{
	/**** open screen ****/

	UAI->screenWidth		= 640;
	if ( rvrec->capsprefs->PalNtsc==PAL_MODE )
		UAI->screenHeight = 256;
	else
		UAI->screenHeight = 200;
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

	UA_SwitchFlagsOn(UAI->userWindow, IDCMP_CLOSEWINDOW);

	SetWindowTitles(UAI->userWindow, dragBarTitle, NULL);

	/**** draw gadgets ****/

	UA_DrawGadgetList(UAI->userWindow, GR);
	DrawImage(UAI->userWindow->RPort, image, 3, 2);

	return(UAI->userWindow);
}

/******** CloseController() ********/

void CloseController(struct UserApplicInfo *UAI, struct GadgetRecord *GR)
{
	UA_SwitchFlagsOff(UAI->userWindow, IDCMP_CLOSEWINDOW);

	if (UAI->userWindow!=NULL)
		UA_CloseWindow(UAI);

	if (UAI->userScreen!=NULL)
		UA_CloseScreen(UAI);
}

/******** MonitorController() ********/

BOOL MonitorController(	struct UserApplicInfo *UAI, struct Window *window,
												struct GVR_record *GVR_rec,
												struct GadgetRecord *Controller_GR, UWORD *mypattern1)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
struct EventData CED;
int ID;
BOOL loop=TRUE, paused=FALSE;
struct GVR_record copy_of_GVR_rec;
ULONG action=0;
TEXT frameStr[31];

	UA_DisableButton(window, &Controller_GR[2], mypattern1);	// play backward
	UA_DisableButton(window, &Controller_GR[4], mypattern1);	// step backward

	// A copy is made here of of GVR_rec because it contains pointers to
	// e.g. serial data comm stuff. The controller uses the same perform
	// function as the preview and player does. To preserve settings done in
	// the user interface the copy is made.

	CopyMem(GVR_rec, &copy_of_GVR_rec, sizeof(struct GVR_record));

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);
	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			while( message = (struct IntuiMessage *)GetMsg(window->UserPort) )
			{
				CED.Class = message->Class;
				CED.Code = message->Code;
				CED.Qualifier = message->Qualifier;
				CED.MouseX = message->MouseX;
				CED.MouseY = message->MouseY;
				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case CLOSEWINDOW:
					case MOUSEBUTTONS:
						action=CED.Class;
						break;
				}
			}
		}

		if ( action==CLOSEWINDOW )
		{
			loop=FALSE;
		}
		else if ( action==MOUSEBUTTONS )
		{
			if (CED.Code==SELECTDOWN)
			{
				CED.MouseY -= window->BorderTop;
				ID = UA_CheckGadgetList(window, Controller_GR, &CED);

				/**** if paused and next command is not pause then	****/
				/**** switch off pause														  ****/

				if ( paused && ID!=-1 && ID!=5 )
				{
					paused = FALSE;

					copy_of_GVR_rec.cmd	= GVR_PAUSE;	// if paused, next issued pause will play again
					UA_SetSprite(window, SPRITE_BUSY);
					PerformActions(&copy_of_GVR_rec);
					UA_SetSprite(window, SPRITE_NORMAL);

					ID=-1;
				}

				switch(ID)
				{
					case 1:	// stop
						UA_HiliteButton(window, &Controller_GR[ID]);
						copy_of_GVR_rec.cmd	= GVR_STOP;
						UA_SetSprite(window, SPRITE_BUSY);
						PerformActions(&copy_of_GVR_rec);
						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);
						UA_SetSprite(window, SPRITE_NORMAL);
						break;

					// case 2 - play backwards is not supported by GVR-810

					case 3:	// play fwd
						UA_HiliteButton(window, &Controller_GR[ID]);
						copy_of_GVR_rec.cmd	= GVR_PLAY;
						UA_SetSprite(window, SPRITE_BUSY);
						PerformActions(&copy_of_GVR_rec);
						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);
						UA_SetSprite(window, SPRITE_NORMAL);
						break;

					// case 4 - step backwards is not supported by GVR-810

					case 5:	// step forwards
						UA_HiliteButton(window, &Controller_GR[ID]);

						UA_SetSprite(window, SPRITE_BUSY);

						// The GVR must be paused in order to step
						if (!paused)
						{
							paused = TRUE;
							copy_of_GVR_rec.cmd	= GVR_PAUSE;
							PerformActions(&copy_of_GVR_rec);
						}

						copy_of_GVR_rec.cmd	= GVR_STEP;
						PerformActions(&copy_of_GVR_rec);

						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);

						UA_SetSprite(window, SPRITE_NORMAL);
	
						break;

					case 6:	// fast rev
						UA_HiliteButton(window, &Controller_GR[ID]);
						copy_of_GVR_rec.cmd	= GVR_FAST_REWIND;
						UA_SetSprite(window, SPRITE_BUSY);
						PerformActions(&copy_of_GVR_rec);
						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);
						UA_SetSprite(window, SPRITE_NORMAL);
						break;

					case 7:	// fast fwd
						UA_HiliteButton(window, &Controller_GR[ID]);
						UA_HiliteButton(window, &Controller_GR[ID]);
						copy_of_GVR_rec.cmd	= GVR_FAST_FORWARD;
						UA_SetSprite(window, SPRITE_BUSY);
						PerformActions(&copy_of_GVR_rec);
						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);
						UA_SetSprite(window, SPRITE_NORMAL);
						break;

					case 8:	// pause
						UA_HiliteButton(window, &Controller_GR[ID]);
						UA_SetSprite(window, SPRITE_BUSY);
						if (!paused)
						{
							paused = TRUE;
							UA_HiliteButton(window, &Controller_GR[ID]);
							copy_of_GVR_rec.cmd	= GVR_PAUSE;
							PerformActions(&copy_of_GVR_rec);
						}
						PrintFrameCode(&copy_of_GVR_rec, Controller_GR);
						UA_SetSprite(window, SPRITE_NORMAL);
						break;
				}
			}
		}
	}

	// return the current frame in GVR_rec->current

	UA_SetSprite(window, SPRITE_BUSY);

  GetFrameCode(&copy_of_GVR_rec, frameStr);
  if( (frameStr[2]==':') && (frameStr[8]==':') )
		strcpy(GVR_rec->current, frameStr);
	else
		sprintf(GVR_rec->current,"00:00:00:00");

	UA_SetSprite(window, SPRITE_NORMAL);

	UA_EnableButton(window, &Controller_GR[2]);	// play backward
	UA_EnableButton(window, &Controller_GR[4]);	// step backward

	return(TRUE);
}

/******** PrintFrameCode() ********/

STATIC void PrintFrameCode(	struct GVR_record *GVR_rec,
														struct GadgetRecord *Controller_GR )
{
TEXT frameStr[31];

  GetFrameCode(GVR_rec, frameStr);	// get 00:00:00:00
  if( (frameStr[2]==':') && (frameStr[8]==':') )	// Check for good frame code
	{
		frameStr[11] = 0x00;
		UA_ClearButton(GVR_rec->window, &Controller_GR[9], AREA_PEN); 
		UA_DrawSpecialGadgetText(GVR_rec->window, &Controller_GR[9], frameStr, SPECIAL_TEXT_CENTER);
	}
}

/******** E O F ********/
