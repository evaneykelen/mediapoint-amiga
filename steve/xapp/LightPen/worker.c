
#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "lightpen.h"

#include <stdlib.h>
#include <stdio.h>

#define _PRINTF FALSE

extern void XappSetup(PROCESSINFO *ThisPI);
extern PROCESSINFO *ml_FindBaseAddr(int , char **);

#define KILLOLD_XAPP FALSE

#define XAPPPORTNAME "LightXapp"

/*************************************************
*Func : 
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*       More info in geninit.c
*out  : -
*/

void main(int argc, char **argv)
{

	PROCDIALOGUE	*Msg_XappDial,		// Dialogue to be send to an already active Xapp
						*Msg_RXappDial,	// Dialogue received by another Xapp
						*Msg_WorkDial,		// Our dialogue 
						*Msg_RWorkDial;	// Dialogue when our guide replies
												// This is actually a copy of the ptr we sent.  
												// Also: dialogue from our parent
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk 
												// (as used in our parent's list)
	struct MsgPort	*Port_Xapp,			// Inter-Xapp communication receive port
						*Port_OtherXapp,	// Port of older Xapp
						*RepP_Xapp,			// Inter-Xapp communication reply port
						*RepP_Worker;		// Reply port for our parent when 
												// replying to our messages

	ULONG				Sig_PtoC,			// A parent to child signal
						SigR_CtoP,			// A reply to a msg we send to our parent
						SigRecvd,			// Signals received
						SigLp;

	int	Error;                
	BOOL	B_ReInit,						// if TRUE, re-initialise data
			B_Term,							// If TRUE, we are free to terminate
			B_Run,			
			B_Remove,						// If True, our guide wants us to clean up
			Int_installed;

	struct lp lp;

	lp.offset_x = oldx;
	lp.offset_y = oldy;
	Int_installed = 0;

	Msg_XappDial	= NULL;
	Msg_RXappDial	= NULL;
	Msg_WorkDial	= NULL;
	Msg_RWorkDial	= NULL;
	ThisPI			= NULL;
	Port_Xapp		= NULL;
	Port_OtherXapp	= NULL;
	RepP_Xapp		= NULL;
	RepP_Worker		= NULL;
	SigLp 			= 0;

	Forbid();
	if( FindPort( XAPPPORTNAME ) )
	{
		Permit();
		KPrintF("Port bestaat al\n");
		return 0;
	}
	Port_Xapp = CreatePort(XAPPPORTNAME, 0 );
	Permit();

	if( Port_Xapp == NULL )
		return;

	// Get our PROCESSINFO base ptr
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}

	// Create a reply port for your guide, needs not to be public cause its base
	// ptr is passed on to the guide when sending a message
	if( (RepP_Worker = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		// Return a general error 
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	// Make a Dialogue 
	if( (Msg_WorkDial = (PROCDIALOGUE *)AllocMem(sizeof(PROCDIALOGUE), 
                                            MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		if (	RepP_Worker != NULL )
			DeletePort(RepP_Worker);
		// Return a general error 
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	// You can do anything you want, set up interrupts, tasks or processes.
	// As long as you can reach them and let them perform your commands. Your
	// only reponsibility is not to loose you own children.


/*****************************************************************************/
// Send init-ready message to your guide

	// Set up the Dialogue message
	Msg_WorkDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_WorkDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	// Attach the replyport to the message dialogue
	Msg_WorkDial->pd_Msg.mn_ReplyPort = RepP_Worker;
	Msg_WorkDial->pd_ChildPI = ThisPI;
	// Send a message to your guide to indicate you are ready to start
	Msg_WorkDial->pd_InUse = TRUE;
	Msg_WorkDial->pd_Cmd = DCI_CHILDREADY;

	// Reply signal from your guide
	SigR_CtoP  =   1 << RepP_Worker->mp_SigBit;
	// Command signal from your guide
	Sig_PtoC     =   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_WorkDial);

	Error = NO_ERROR;

	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	while(!B_Term)
	{
		// Add your own signals to this list 

		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);
		if(SigRecvd & SIGF_ABORT)
			break;
        
		if(SigRecvd & Sig_PtoC)
		{
			// Our parent has something to say to us
			while( (Msg_RWorkDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RWorkDial->pd_ChildPI = ThisPI;	
				switch(Msg_RWorkDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
						Msg_RWorkDial->pd_Cmd = DCI_CHILDPREPARES;                 
						B_ReInit = TRUE;
						break;                          
					case DCC_DORUN:
						// Either start or re-run from pause
						B_Run = TRUE;
						Msg_RWorkDial->pd_Cmd = DCI_CHILDRUNS;                 
						break;                          
					case DCC_DOHOLD:
						// Hold your horses and wait till a 
						// a DCC_DORUN arrives
						Msg_RWorkDial->pd_Cmd = DCI_CHILDHOLDS;                 
						break;
					case DCC_DOTERM:
						Msg_RWorkDial->pd_Cmd = DCI_CHILDTERM;                  
						B_Remove = TRUE;
						break;
					case DCC_DOSTOP:
						Msg_RWorkDial->pd_Cmd = DCI_CHILDREADY;	
						break;
					default:
						// simply ignore what we don't understand
						Msg_RWorkDial->pd_Cmd = DCI_IGNORE;	
						break;
				}
                ReplyMsg((struct Message *)Msg_RWorkDial);
			}
		}

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RWorkDial = (PROCDIALOGUE *)GetMsg(RepP_Worker)) != NULL)
			{
				Msg_RWorkDial->pd_InUse = FALSE;
				// Lets see what we've got.
				// Right now we don't need to check the reply cmds
				// since we don't use them.
				switch(Msg_RWorkDial->pd_Cmd)
				{
					case DCC_IGNORE:
						break;
					default:
						break;
				}
			}
		}

		if( signal & SigLp )
		{
			set_mouse( &lp );
		}

		if(B_Run)
		{
			if( Int_installed )
			{
				if( init_mouse( &lp ) )
				{
					Int_installed = 1;
					setlightpen(&lp);
					SigRecvd |= lp.signal;
					SigLp = lp.signal )
				}
				else
				{
					KPrinF("Int failed free mouse\n");
					free_mouse( &lp );
				}
				B_Run = FALSE;
			}
			else
				KPrintF("Already installed\n");
		}

		if(B_ReInit)
		{
			if(!Msg_WorkDial->pd_InUse)
			{
				B_ReInit = FALSE;
				Msg_WorkDial->pd_ChildPI = ThisPI;
				Msg_WorkDial->pd_InUse = TRUE;
				Msg_WorkDial->pd_Cmd = DCI_CHILDREADY;
				PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_WorkDial);
			}
		}

		if(SigRecvd & SIGF_ABORT)
			break;
	}

    /*
    XaPPFree();
    */

	if( Int_installed )
	{
		removelightpen(&lp);
		free_mouse( &lp );
		Int_installed = 0;
	}

	if ( Msg_WorkDial != NULL )
	    FreeMem(Msg_WorkDial,sizeof(PROCDIALOGUE));

	if ( RepP_Worker != NULL )
	    DeletePort(RepP_Worker);

	if(Port_Xapp != NULL)
	    DeletePort(Port_Xapp);

	ThisPI->pi_Arguments.ar_RetErr = Error;

	return;
}
