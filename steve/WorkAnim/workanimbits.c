//	File		:	workanim.c
//	Uses		:	external.h mlmmu.h
//	Date		:	-92 ( 08-05-1993 )
//	Author	:	S. Vanderhorst ( adaption atempt C. Lieshout )
//	Desc.		:	Display a animation
//

#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/all.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include	"pg:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define b_AHR		0x01
#define b_ReInit	0x02
#define b_Term		0x04
#define b_Run		0x08
#define b_Stop		0x10
#define b_Remove	0x20

#define _PRINTF FALSE
#define _PRINTDEB FALSE

#define MAXSTRING 256

/*******************************************
*	Func	:	Send a dialogue to our guide
*	in		:	Msg_Dial	-> the dialogue
*				PI			-> PI of task owner
*				Cmd		-> the command to be send
*	out	:	TRUE	-> ok
*				FALSE	-> error
*/
BOOL SendDialogue( PROCDIALOGUE *Msg_Dial, PROCESSINFO *PI, int Cmd )
{
	if(Msg_Dial->pd_InUse)
		return(FALSE);

	Msg_Dial->pd_ChildPI = PI;
	Msg_Dial->pd_InUse = TRUE;
	Msg_Dial->pd_Cmd = Cmd;
	PutMsg(PI->pi_Port_CtoP,(struct Message *)Msg_Dial);
	return(TRUE);
}

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
			ErrLoadAnim;	

	PROCDIALOGUE	*Msg_AnimDial,		// Our dialogue 
						*Msg_AnimDial2,	
						*Msg_RAnimDial;	// Our dialogue when our guide replies

	MLSYSTEM			*MLSystem;	
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk (as used in our parent's list)

	struct MsgPort	*RepP_WorkAnim,	// Reply port for our parent when replying to our messages
						*Port_toTrans;		// to get to  the TRANSITION module	
	ULONG	Sig_PtoC,						// A parent to child signal
			SigR_CtoP,						// A reply to a msg we send to our parent
			SigRecvd;						// Signals received

	WORD	b;									// NEW store the control bits here

	WORD	CBITS;
	struct Library *MLMMULibBase;
	long	oldpri;
	struct Task *task;

	if( ( ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL )
		return;

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	Msg_AnimDial = NULL;
	Msg_AnimDial2 = NULL;
	RepP_WorkAnim = NULL;
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
	MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,
						ThisPI->pi_Arguments.ar_Worker.aw_Name,FileName);

	CBITS = ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11];

	AnimDataSegment = NULL;
	ErrSysPass = TRUE;
	ErrLoadAnim = TRUE;
	if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
		if( (AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
			if((ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment, ThisPI->pi_Port_PtoC)) == 0)
				ErrLoadAnim = load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);

	// Set up the Dialogue message
	Msg_AnimDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_AnimDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_AnimDial->pd_Msg.mn_ReplyPort = RepP_WorkAnim;
	Msg_AnimDial2->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_AnimDial2->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_AnimDial2->pd_Msg.mn_ReplyPort = RepP_WorkAnim;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkAnim->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_AnimDial,ThisPI,DCI_CHILDREADY);

	// main 	
	b = b_AHR;

	while( !( b & b_Term ) )
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RAnimDial->pd_ChildPI = ThisPI;
				switch(Msg_RAnimDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(! ( b & ( b_Remove | b_Term ) ) )
								b |= b_ReInit;
							break;
					case DCC_DORUN:
							if( !( b & ( b_Remove | b_Term ) ) )
							{
								// see if the anim is NOT in memory yet
								// Last effort to get the picture into memory!
								if( ErrLoadAnim )
								{
									if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
									{
										if(AnimDataSegment == NULL)
											AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL);
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
										if(!Msg_AnimDial2->pd_InUse)
										{
											Msg_AnimDial2->pd_InUse = TRUE;
											Msg_AnimDial2->pd_Cmd = DCC_DOTRANSITION;
											Msg_AnimDial2->pd_Luggage.lu_SNR = ThisPI->pi_SNR;
											Msg_AnimDial2->pd_Luggage.lu_Dial = (struct Message *)(*((ULONG *)AnimDataSegment)+12L);
											PutMsg(Port_toTrans,(struct Message *)Msg_AnimDial2);
											ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = CBITS | 0x40;
											b &= ~b_AHR;
										}
									}
									else
										b |= b_Run ;
								}
							}
							Msg_RAnimDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDTERM;
							b |= b_Remove | b_AHR;
							b &= ~b_Run;
							break;
					case DCC_DOSTOP:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDREADY;	
							b |= b_Stop | b_AHR;
							b &= ~b_Run;
							break;
					case DCC_DOEASYTERM:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYTERM;
							b |= b_Remove;
							b &= ~b_Run;
							break;
					case DCC_DOEASYSTOP:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							b |= b_Stop;
							b &= ~b_Run;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RAnimDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RAnimDial);
			}
		}

		// get a reply from our guide or the TRANSITION module
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(RepP_WorkAnim)) != NULL)
			{
				Msg_RAnimDial->pd_InUse = FALSE;
				if(Msg_RAnimDial->pd_Cmd == DCI_TRANSITION )
					if( !( b & ( b_Remove | b_Term ) ) )
						b |= b_Run;
			}

		if( b & b_Stop )
		{
			if(!ErrLoadAnim)
				unload_anim(AnimDataSegment);
			ErrLoadAnim = TRUE;	
			if(!ErrSysPass)
				release_anim(AnimDataSegment);
			ErrSysPass = TRUE;
			MLMMU_FreeMem(AnimDataSegment);
			AnimDataSegment = NULL;
			b &= ~b_Stop;
		}

		if( ( !( b & ( b_Remove | b_Term ) ) ) || ! ( b & b_AHR ))
		{
			if( b & b_Run )
			{
				ObtainSemaphore(&MLSystem->ms_Sema_Transition);
				task = FindTask( 0 );
				oldpri = SetTaskPri(task, 21 );
				do_anim(AnimDataSegment);
				SetTaskPri( task,oldpri );
				b |= b_AHR;
				ReleaseSemaphore(&MLSystem->ms_Sema_Transition);
				b &= ~b_Run;
			}

			if( b & b_ReInit )
			{
				if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
				{
					if(AnimDataSegment == NULL)
						AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L,MEMF_PUBLIC|MEMF_CLEAR,NULL);
					if(AnimDataSegment != NULL)
					{
						if(ErrSysPass)
							ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment,ThisPI->pi_Port_PtoC);
						if(!ErrSysPass)
							if(ErrLoadAnim)
								ErrLoadAnim = (int)load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
					}
				}
				b &= ~b_ReInit;
				if( SendDialogue(Msg_AnimDial,ThisPI,DCI_CHILDREADY ) )
					b |= b_ReInit; 
			}
		}

		if( b & b_Remove )			// wait till all dialogues used
			b |= b_Term;				// to send commands to us have been freed

		// Check if there are still messages in the portlist
		// if so then signal ourself
		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

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
