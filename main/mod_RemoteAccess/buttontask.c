#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <dos/dostags.h>

STATIC VOID __saveds ButtonServer(void);

/**** externals ****/

extern struct Library *medialinkLibBase;
extern UBYTE **msgs;
extern struct RendezVousRecord *rvrec;
extern struct GadgetRecord RA2_GR[];
extern struct EventData CED;
extern UWORD chip mypattern1[];
extern struct Gadget PropSlider1;
extern struct Process *process;
extern struct ExecBase *SysBase;
extern struct Window *reportWdw;
extern struct MsgPort *capsport;

/**** globals ****/

static struct Task *ButtonTask;
static struct Task *keptTask;
int buttonTouched;

/**** gadgets ****/

extern struct GadgetRecord RA2_GR[];

/**** functions ****/

/******** StartButtonTask() ********/

BOOL StartButtonTask(void)
{
	buttonTouched = FALSE;

	ButtonTask = (struct Task *)CreateTask("MP_Button_Task",0,ButtonServer,4096);
	if (!ButtonTask)
		return(FALSE);
	else
		Wait(SIGBREAKF_CTRL_C);
	if(!ButtonTask)
		return(FALSE);

	Forbid();
	keptTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask("MP_Button_Task");
	Permit();

	return(TRUE);
}

/******** StopButtonTask() ********/

void StopButtonTask(void)
{
	Signal(ButtonTask, SIGBREAKF_CTRL_C);
	Wait(SIGBREAKF_CTRL_C);

	Forbid();
	capsport->mp_SigTask = keptTask;
	Permit();
}

/******** ButtonServer() ********/

STATIC VOID __saveds ButtonServer(void)
{
BYTE KeepGoing = TRUE;
ULONG signals;

	Signal((struct Task *)process,SIGBREAKF_CTRL_C);

	while(KeepGoing)
	{
		signals = UA_doStandardWaitExtra(reportWdw, &CED, SIGBREAKF_CTRL_C);
		if ( signals & SIGBREAKF_CTRL_C )
		{
			KeepGoing = FALSE;
			SetSignal(0,SIGBREAKF_CTRL_C);
		}
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			if ( !buttonTouched && UA_CheckGadget(reportWdw,RA2_GR,&RA2_GR[5],&CED)==5 )
			{
				UA_HiliteButton(reportWdw, &RA2_GR[5]);
				buttonTouched = TRUE;
			}
		}
	}

	Forbid();
	Signal((struct Task *)process,SIGBREAKF_CTRL_C);
	ButtonTask = NULL;
	RemTask(SysBase->ThisTask);
}

/******** IsTheButtonHit() ********/

BOOL IsTheButtonHit(void)
{
	return( (BOOL)buttonTouched );
}

/******** E O F ********/
