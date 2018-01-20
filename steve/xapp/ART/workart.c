/******************************************************
*Desc : Perform a ScriptTalk "Art" command
*		This routine will either wait for the command
*		to be finished or  
* <!> : This module is resident and re-entrant
*		Compile without -b1 and without -y options
*		Link with cres.o in stead of c.o
*		Also compile umain.c : lc -b1 umain 
*
*/

#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "gen:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define VERSI0N "\0$VER: MediaPoint Interlude xapp 1.2"          
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE
#define _PRINTDEB FALSE

void XappSetup(PROCESSINFO *ThisPI);

/*************************************************
*Func : Display the par: input
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{

	UBYTE	*ARTDataSegment;
	int	ErrSysPass,
			ErrLoadART;	

	PROCDIALOGUE	*Msg_ARTDial,	// Our dialogue 
						*Msg_RARTDial;	// Our dialogue when our guide replies

	MLSYSTEM			*MLSystem;	
	PROCESSINFO		*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

	struct MsgPort	*RepP_WorkART;// Reply port for our parent when replying to our messages

	ULONG		Sig_PtoC,		// A parent to child signal
				SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
	int		i;
	BOOL		B_ReInit,		// if TRUE, re-initialise data
				B_Term,			// If TRUE, we are free to terminate
				B_Run,			
				B_Stop,
				B_Remove;		// If True, our guide wants us to clean up

	struct Library *MLMMULibBase;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	/**** this is called when showing the GUI ****/
	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	MLMMULibBase = NULL;
	Msg_ARTDial = NULL;
	RepP_WorkART = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkART = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_ARTDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_ARTDial);
		if(RepP_WorkART)
			DeletePort(RepP_WorkART);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	ARTDataSegment = NULL;
	ErrSysPass = TRUE;
	ErrLoadART = TRUE;
	if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
	{
		if( (ARTDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
		{
			if((ErrSysPass = (int)pass_mlsystem(MLSystem,ARTDataSegment, ThisPI->pi_Port_PtoC)) == 0)
				ErrLoadART = load_art(NULL,ARTDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
		}
	}

	// Set up the Dialogue message
	Msg_ARTDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_ARTDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_ARTDial->pd_Msg.mn_ReplyPort = RepP_WorkART;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkART->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_ARTDial,ThisPI,DCI_CHILDREADY);

	// main 	
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	B_Stop = FALSE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RARTDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RARTDial->pd_ChildPI = ThisPI;
				switch(Msg_RARTDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RARTDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
							{
								if(ErrLoadART)
								{
									if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
									{
										if(ARTDataSegment == NULL)
											ARTDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL);

										if(ARTDataSegment != NULL)
										{
											if(ErrSysPass != 0)
												ErrSysPass = (int)pass_mlsystem(MLSystem,ARTDataSegment, ThisPI->pi_Port_PtoC);
											if(ErrSysPass == 0)
												ErrLoadART = (int)load_art(NULL,ARTDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
										}
									}
								}

								if(ErrLoadART == 0)
										B_Run = TRUE;
							}
							Msg_RARTDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
						Msg_RARTDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOSTOP:
							Msg_RARTDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYTERM:
							Msg_RARTDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RARTDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RARTDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RARTDial);
			}
		}

		// get a reply from our guide or the TRANSITION module
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RARTDial = (PROCDIALOGUE *)GetMsg(RepP_WorkART)) != NULL)
				Msg_RARTDial->pd_InUse = FALSE;

		if(B_Stop)
		{
			if(!ErrLoadART)
				unload_art(ARTDataSegment);
			ErrLoadART = TRUE;	
			if(!ErrSysPass)
				release_art(ARTDataSegment);
			ErrSysPass = TRUE;
			MLMMU_FreeMem(ARTDataSegment);
			ARTDataSegment = NULL;
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term ) )
		{
			if(B_Run)
			{
				ObtainSemaphore(&MLSystem->ms_Sema_Transition);
				do_art(ARTDataSegment);
				ReleaseSemaphore(&MLSystem->ms_Sema_Transition);
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
				{
					if(ARTDataSegment == NULL)
						ARTDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(),MEMF_PUBLIC|MEMF_CLEAR,NULL);
					if(ARTDataSegment != NULL)
					{
						if(ErrSysPass)
							ErrSysPass = (int)pass_mlsystem(MLSystem,ARTDataSegment,ThisPI->pi_Port_PtoC);
						if(!ErrSysPass)
							if(ErrLoadART)
								ErrLoadART = (int)load_art(NULL,ARTDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
					}
				}
				B_ReInit = SendDialogue(Msg_ARTDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;

			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
					B_Term = FALSE;

		if(Msg_ARTDial->pd_InUse && (Msg_ARTDial->pd_Cmd == DCI_CHILDREADY))
				B_Term = FALSE;
		}

		// Check if there are still messages in the portlist
		// if so then signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	if(!ErrLoadART)
		unload_art(ARTDataSegment);
	if(!ErrSysPass)
		release_art(ARTDataSegment);
	MLMMU_FreeMem(ARTDataSegment);

	MLMMU_FreeMem(Msg_ARTDial);
	DeletePort(RepP_WorkART);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
}
