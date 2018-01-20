/******************************************************
*Desc :	Perform a ScriptTalk "ANIM" command
*			This routine will either wait for the command
*			to be finished or  
* <!> :	This module is resident and re-entrant
*			Compile without -b1 and without -y options
*			Link with cres.o in stead of c.o
*			Also compile umain.c : lc -b1 umain 
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

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE
#define _PRINTDEB FALSE

#define MAXSTRING 256

/*************************************************
*	Func	:	Play an animation
*	in		:	Argv -> Ptr to PROCESSINFO.pi_Startup
*	out	:	-
*/
void main( int argc, char *argv[] )
{
	char	FileName[MAXSTRING];

	UBYTE	*AnimDataSegment;
	int	ErrSysPass,
			ErrLoadAnim, i;	

	PROCDIALOGUE	*Msg_AnimDial,		// Our dialogue 
						*Msg_AnimDial2,	
						*Msg_RAnimDial = NULL;	// Our dialogue when our guide replies

	PROCDIALOGUE	*Msg_Pending = NULL;
	
	MLSYSTEM			*MLSystem;	
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk (as used in our parent's list)

	struct MsgPort	*RepP_WorkAnim,	// Reply port for our parent when replying to our messages
						*Port_toTrans;		// to get to  the TRANSITION module	
	ULONG	Sig_PtoC,						// A parent to child signal
			SigR_CtoP,						// A reply to a msg we send to our parent
			SigRecvd;						// Signals received
	BOOL	B_AnimHasRun,
			B_ReInit,						// if TRUE, re-initialise data
			B_Term,							// If TRUE, we are free to terminate
			B_Run,			
			B_Stop,
			B_SetAfterState,				// if TRUE, anim will be performed before cleaning up
			B_EffectHasRun,				// effect for this anim is running or done
			B_StopAfter,
			B_RemoveAfter,
			B_WStop,
			B_Remove;						// If True, our guide wants us to clean up

	int MPend;

	WORD		CBITS;
	struct	Library *MLMMULibBase;
	long		oldpri;
	struct	Task *task;
	struct	wjif WJIF;
	LONG		jiffies;
	ULONG		wsig = 0;

	if( ( ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL )
		return;

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	MLMMULibBase	= NULL;
	Msg_AnimDial	= NULL;
	Msg_AnimDial2	= NULL;
	RepP_WorkAnim	= NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((Port_toTrans = FindPort("Port_Transition")) == NULL) ||
		((RepP_WorkAnim = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_AnimDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL) ||
		((Msg_AnimDial2 = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_AnimDial2);
		MLMMU_FreeMem(Msg_AnimDial);
		if(RepP_WorkAnim)
			DeletePort(RepP_WorkAnim);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	// Program init
	MakeFullPath(	ThisPI->pi_Arguments.ar_Worker.aw_Path,
						ThisPI->pi_Arguments.ar_Worker.aw_Name, FileName);

	CBITS = ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11];

	AnimDataSegment 	= NULL;
	ErrSysPass			= TRUE;
   ErrLoadAnim			= TRUE;

	if( MLSystem->miscFlags & 1L << 6 )
		if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
			if( (AnimDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
				if((ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment, ThisPI->pi_Port_PtoC)) == 0)
					ErrLoadAnim = load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);

	// Set up the Dialogue message
	Msg_AnimDial->pd_Msg.mn_Node.ln_Type	= NT_MESSAGE;
	Msg_AnimDial->pd_Msg.mn_Length			= sizeof(PROCDIALOGUE);
	Msg_AnimDial->pd_Msg.mn_ReplyPort		= RepP_WorkAnim;
	Msg_AnimDial2->pd_Msg.mn_Node.ln_Type	= NT_MESSAGE;
	Msg_AnimDial2->pd_Msg.mn_Length			= sizeof(PROCDIALOGUE);
	Msg_AnimDial2->pd_Msg.mn_ReplyPort		= RepP_WorkAnim;

	// Our guide will reply to us when we must start
	SigR_CtoP	= 1 << RepP_WorkAnim->mp_SigBit;
	Sig_PtoC		= 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_AnimDial, ThisPI, DCI_CHILDREADY);

	// main 	

/* ADDED */
	B_EffectHasRun		= FALSE;
	B_SetAfterState	= FALSE;
/*********/
	B_ReInit			= FALSE;
	B_Run				= FALSE;
	B_Term			= FALSE;
	B_Remove			= FALSE;
	B_Stop 			= FALSE;
	B_AnimHasRun	= TRUE;
	B_WStop 			= TRUE;
	MPend 			= 0;

	WJIF.signum = 0;

	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT | wsig );

		if( SigRecvd & wsig )
		{
			remove50hz( &WJIF );			// 50hz signal say's time is up

			B_WStop = TRUE;

			if( MPend == 1 )				// easy stop pending
			{
				B_Stop = TRUE;
				B_Run = FALSE;
				B_AnimHasRun = TRUE;
				ReplyMsg((struct Message *)Msg_Pending);
				MPend = 0;
			}
			else
				if( MPend == 2 )			// easy term pending
				{
					B_Remove = TRUE;
					B_Run = FALSE;
					B_AnimHasRun = TRUE;
					ReplyMsg((struct Message *)Msg_Pending);
					MPend = 0;
				}
		}

		if(SigRecvd & Sig_PtoC)
		{
			if( MPend == 1 )				// easy stop found
			{
				B_Stop = TRUE;
				B_Run = FALSE;
				B_AnimHasRun = TRUE;
				ReplyMsg((struct Message *)Msg_Pending);
				MPend = 0;
			}
			else
				if( MPend == 2 )			// easy term found
				{
					B_Remove = TRUE;
					B_Run = FALSE;
					B_AnimHasRun = TRUE;
					ReplyMsg((struct Message *)Msg_Pending);
					MPend = 0;
				}

			if( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RAnimDial->pd_ChildPI = ThisPI;
				switch(Msg_RAnimDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							MPend = 0;
							B_WStop = FALSE;
							B_SetAfterState = FALSE;
							if(!B_Remove && !B_Term)
							{
								if(ErrLoadAnim)			// see if the anim is NOT in memory yet
								{
									if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
									{
										if(AnimDataSegment == NULL)
											AnimDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL);
										if(AnimDataSegment != NULL)
										{
											if(ErrSysPass != 0)
												ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment, ThisPI->pi_Port_PtoC);
											if(ErrSysPass == 0)
												ErrLoadAnim = (int)load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
										}
									}
								}

								if(ErrLoadAnim == 0)
								{
									// if the effectnr is a cut then skip the effect
									if(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] != 0 )
									{
										//First Send a TRANSITION request to the TRANSITION module
										if( !Msg_AnimDial2->pd_InUse )
										{
											Msg_AnimDial2->pd_InUse = TRUE;
											Msg_AnimDial2->pd_Cmd = DCC_DOTRANSITION;
											Msg_AnimDial2->pd_Luggage.lu_SNR = ThisPI->pi_SNR;
											Msg_AnimDial2->pd_Luggage.lu_Dial = (struct Message *)(*((ULONG *)AnimDataSegment)+12L);
//		KPrintF("Send do T %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
											PutMsg(Port_toTrans,(struct Message *)Msg_AnimDial2);
											ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = CBITS | 0x40;
											B_AnimHasRun = FALSE;
											B_EffectHasRun = TRUE;
										}
//										else
//					KPrintF("Animdial in USE ???????\n");
									}
									else
										B_Run = TRUE;
								}
							}
							Msg_RAnimDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
//	KPrintF("Got a term %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
							Msg_RAnimDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							B_AnimHasRun = TRUE;
							break;
					case DCC_DOSTOP:
//	KPrintF("Got a stop %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
							Msg_RAnimDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							B_AnimHasRun = TRUE;
							break;
					case DCC_DOEASYTERM:
//	KPrintF("Got a easyterm %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYTERM;
							if( B_WStop )	// send the easyterm return command when you are finished
							{
								if(!B_EffectHasRun)
								{
									B_Remove = TRUE;
									B_Run = FALSE;
								}
								else
								{
									B_SetAfterState = TRUE;
									B_StopAfter = FALSE;
									B_RemoveAfter = TRUE;
								}
							}
							else
							{
//	KPrintF("Pending animdial 2\n");
								Msg_Pending = Msg_RAnimDial;
								MPend = 2;
							}
							break;
					case DCC_DOEASYSTOP:
//	KPrintF("Got a easystop %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							if( B_WStop )	// same for the easystop command 
							{
								if(!B_EffectHasRun)
								{
									B_Stop = TRUE;
									B_Run = FALSE;
								}
								else
								{
									B_SetAfterState = TRUE;
									B_StopAfter = TRUE;
									B_RemoveAfter = FALSE;
								}	
							}
							else
							{
//	KPrintF("Pending animdial 1\n");
								MPend = 1;
								Msg_Pending = Msg_RAnimDial;
							}
							break;
					default:
							// simply ignore what we don't understand
							Msg_RAnimDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				if( MPend == 0 )
					ReplyMsg((struct Message *)Msg_RAnimDial);
//				else
//					KPrintF("Reply differted\n");
			}
		}

		if(SigRecvd & SigR_CtoP)			// get a reply from guide or TRANSITION
		{
			while( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(RepP_WorkAnim)) != NULL)
			{
				Msg_RAnimDial->pd_InUse = FALSE;
				// Lets see what we've got.
				// Right now we don't need to check the reply cmds
				// since we don't use them.
				switch(Msg_RAnimDial->pd_Cmd)
				{
					case DCC_IGNORE:
							break;
					case DCI_TRANSITION:					// | in case of a stop before you could run !!!!!!
							if(!B_Remove && !B_Term && !B_AnimHasRun )	// the effect is ready you can start now
							{
								B_Run = TRUE;
//		KPrintF("run=TRUE %s\n",ThisPI->pi_Arguments.ar_Worker.aw_Name );
							}
							break;	
					default:
							break;
				}
			}
		}

		if(B_Stop)
		{
			if(!ErrLoadAnim)
				unload_anim(AnimDataSegment);
			ErrLoadAnim = TRUE;	
			if(!ErrSysPass)
				release_anim(AnimDataSegment);
			ErrSysPass = TRUE;
			MLMMU_FreeMem(AnimDataSegment);
			AnimDataSegment = NULL;
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term) || !B_AnimHasRun)
		{
			if(B_Run)
			{
				ObtainSemaphore(&MLSystem->ms_Sema_Transition);
				task = FindTask( 0 );
				oldpri = SetTaskPri(task, 21 );

				if(ThisPI->pi_Port_PtoC->mp_MsgList.lh_Head->ln_Succ != NULL)
					Signal(&(ThisPI->pi_Process->pr_Task),Sig_PtoC);
				do_anim(AnimDataSegment);
				SetTaskPri( task,oldpri );
				B_AnimHasRun = TRUE;
				ReleaseSemaphore(&MLSystem->ms_Sema_Transition);

// anim is ready now wait for a moment
				jiffies = WaitJiffies( ThisPI->pi_SNR, MLSystem );
				if( jiffies != 0 )
				{
					wsig = set50hz( &WJIF, jiffies );
					B_WStop = FALSE;
				}
				else
					B_WStop = TRUE;

				B_EffectHasRun = FALSE;
				if(B_SetAfterState)
				{
					B_SetAfterState = FALSE;
					B_Stop = B_StopAfter;
					B_Remove = B_RemoveAfter;
				}
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
				{
					if(AnimDataSegment == NULL)
						AnimDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(),MEMF_PUBLIC|MEMF_CLEAR,NULL);
					if(AnimDataSegment != NULL)
					{
						if(ErrSysPass)
							ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment,ThisPI->pi_Port_PtoC);
						if(!ErrSysPass)
							if(ErrLoadAnim)
								ErrLoadAnim = (int)load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
					}
				}
				B_ReInit = SendDialogue(Msg_AnimDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if( B_Remove )			// wait till all dialogues used
			B_Term = TRUE;		// to send commands to us have been freed

		// Check if there are still messages in the portlist 
		// if so, signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	remove50hz( &WJIF );

	if(!ErrLoadAnim)
		unload_anim(AnimDataSegment);
	if(!ErrSysPass)
		release_anim(AnimDataSegment);
	MLMMU_FreeMem(AnimDataSegment);

	MLMMU_FreeMem(Msg_AnimDial2);
	MLMMU_FreeMem(Msg_AnimDial);
	DeletePort(RepP_WorkAnim);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = CBITS;
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
}

