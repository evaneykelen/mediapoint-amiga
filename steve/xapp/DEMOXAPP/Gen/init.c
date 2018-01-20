#include <exec/exec.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/serial.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <workbench/startup.h>
// CLIB
#include <clib/exec_protos.h>
// PRAGMAS
#include <pragmas/exec_pragmas.h>
// USER
#include "demo:Gen/mp.h"

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;
STATIC struct Task *oldTask;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** OpenStuff() ********/

BOOL OpenStuff(struct GadgetRecord *GR, struct UserApplicInfo *UAI)
{
struct MsgPort *port;
struct Node *node;
struct List *list;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(FALSE);

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

	UA_InitStruct(UAI);
	UAI->IB = IntuitionBase;

	/**** open and load the medialink font ***/

	UAI->small_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI->small_TA.ta_YSize = 10;
	UAI->small_TA.ta_Style = FS_NORMAL;
	UAI->small_TA.ta_Flags = FPF_DESIGNED;
	UAI->small_TF = rvrec->smallfont;

	UAI->large_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI->large_TA.ta_YSize = 20;
	UAI->large_TA.ta_Style = FS_NORMAL;
	UAI->large_TA.ta_Flags = FPF_DESIGNED;
	UAI->large_TF = rvrec->largefont;

	/**** open a window ****/

	if (UA_HostScreenPresent(UAI))
		UAI->windowModes = 1;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(UAI))
		UA_DoubleGadgetDimensions(GR);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI->windowX			= -1;
	UAI->windowY			= -1;
	UAI->windowWidth	= GR[0].x2;
	UAI->windowHeight	= GR[0].y2;
	UAI->wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(UAI);

	if ( !UAI->userWindow )
		return(FALSE);

	return(TRUE);
}

/******** CloseStuff() ********/

void CloseStuff(struct UserApplicInfo *UAI)
{
	UA_CloseWindow(UAI);
	capsport->mp_SigTask = oldTask;
}

/******** E O F ********/
