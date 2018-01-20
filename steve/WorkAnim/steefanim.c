/******************************************************
*Desc : Perform a ScriptTalk "ANIM" command
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
#include <proto/all.h>

#include "minc:snr.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define _PRINTF FALSE
#define _PRINTDEB FALSE

#define MAXSTRING 256


/******************************************************
*Func : Makes from 'anim:' and 'ape' the string 'anim:ape' and
* 		makes from 'anim:test/pics/hires' and 'ape' the
* 		string 'anim:test/pics/hires/ape'. Even more, the string
* 		'anim:test/pics/hires/' (note the slash) and 'ape' is correctly
* 		converted (no double slashes). This seems trivial but this routine
* 		should be used instead of trying to connect them yourself.
*in   : Path
*		Name
*		Dest
*out  : -
*/
void MakeFullPath(Path, Name, Dest) 
STRPTR Path;
STRPTR Name;
STRPTR Dest;
{
  int PathLength;

	PathLength = strlen(Path);
	if(Path[PathLength-1] == ':' || Path[PathLength-1] == '/')
	{
		strcpy(Dest,Path);
		strcat(Dest,Name);
	}
	else 
	{
		if(Path != NULL && Path[0] != '\0')
		{
			strcpy(Dest,Path);
			strcat(Dest,"/");
			strcat(Dest,Name);
		}
		else
			strcpy(Dest, Name);
	}	
}

/*******************************************
*Func : Send a dialogue to our guide
*in   : Msg_Dial -> the dialogue
*		PI 		 -> PI of task owner
*		Cmd 	 -> the command to be send
*out  : TRUE -> ok
*		FALSE -> error
*/
BOOL SendDialogue(Msg_Dial, PI, Cmd)
PROCDIALOGUE *Msg_Dial;
PROCESSINFO *PI;
int Cmd;
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
*Func : Play an animation
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{

  char 			FileName[MAXSTRING];

  UBYTE 		*AnimDataSegment;
  int 			ErrSysPass,
				ErrLoadAnim;	

  PROCDIALOGUE 	*Msg_AnimDial,	// Our dialogue 
				*Msg_AnimDial2,	
				*Msg_RAnimDial;	// Our dialogue when our guide replies

  MLSYSTEM	  	*MLSystem;	
  PROCESSINFO 	*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

  struct MsgPort *RepP_WorkAnim,// Reply port for our parent when replying to our messages
 				 *Port_toTrans;	// to get to  the TRANSITION module	
  ULONG 		Sig_PtoC,		// A parent to child signal
    			SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
  int 			AnimErr, i;
  BOOL			B_AnimHasRun,
				B_ReInit,		// if TRUE, re-initialise data
				B_Term,			// If TRUE, we are free to terminate
				B_Run,			
				B_Stop,
				B_Remove;		// If True, our guide wants us to clean up
  WORD 			FPS;
  WORD			CBITS;
  struct Library *MLMMULibBase;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	MLMMULibBase = NULL;
	Msg_AnimDial = NULL;
	Msg_AnimDial2 = NULL;
	RepP_WorkAnim = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mlmmu.library",0)) == NULL) ||
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

	/**** turn FPS into jiffies ****/
	FPS = ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9];
	CBITS = ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10];

	if(FPS != 0)
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = 50 / FPS;
	else
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = 0;

	AnimDataSegment = NULL;
	ErrSysPass = TRUE;
    ErrLoadAnim = TRUE;
	if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
	{
		if( (AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
		{
#if _PRINTF
			printf("Data segment alloc ok\n");
#endif
			if((ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment, ThisPI->pi_Port_PtoC)) == 0)
			{
#if _PRINTF
				printf("pass system ok\n");	
#endif
				ErrLoadAnim = load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
			}
		}
	}

#if _PRINTF
	printf("Datasegment at %x\n",(int)AnimDataSegment);
	printf("pass system error %d\n",ErrSysPass);
	printf("Load anim error %d\n",ErrLoadAnim);
#endif

	AnimErr = NO_ERROR;
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
	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	B_Stop = FALSE;
	B_AnimHasRun = TRUE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
#if _PRINTF
				printf("received cmd %d from guide\n",(int)Msg_RAnimDial->pd_Cmd);
#endif
				Msg_RAnimDial->pd_ChildPI = ThisPI;
				switch(Msg_RAnimDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
							{
								// see if the anim is NOT in memory yet
								// Last effort to get the picture into memory!
								if(ErrLoadAnim)
								{
									if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
									{
										if(AnimDataSegment == NULL)
										{
											AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL);
#if _PRINTF
											printf("ralloc datasegment at %x\n",(int)AnimDataSegment);
#endif
										}	
										if(AnimDataSegment != NULL)
										{
											if(ErrSysPass != 0)
											{
												ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment, ThisPI->pi_Port_PtoC);
#if _PRINTF
												printf("rpass system error = %d\n",ErrSysPass);
#endif
											}
											if(ErrSysPass == 0)
												ErrLoadAnim = (int)load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
										}
									}
								}

								if(ErrLoadAnim == 0)
								{
									// if the effectnr is a cut then skip the effect
									if(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] != 46)
									{
#if _PRINTF
										printf("about to send DCC_TRANSITION\n");
#endif
										//First Send a TRANSITION request to the TRANSITION module
										if(!Msg_AnimDial2->pd_InUse)
										{
											Msg_AnimDial2->pd_InUse = TRUE;
											Msg_AnimDial2->pd_Cmd = DCC_DOTRANSITION;
											Msg_AnimDial2->pd_Luggage.lu_SNR = ThisPI->pi_SNR;
											Msg_AnimDial2->pd_Luggage.lu_Dial = (struct Message *)(*((ULONG *)AnimDataSegment)+12L);
#if _PRINTF	
											printf("sending DCC_TRANSITION, PicSeg at %x\n",(int)Msg_AnimDial2->pd_Luggage.lu_Dial);
#endif
											PutMsg(Port_toTrans,(struct Message *)Msg_AnimDial2);
											ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = CBITS | 0x40;
											B_AnimHasRun = FALSE;
										}
									}
									else
										B_Run = TRUE;
								}
							}
							Msg_RAnimDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							B_AnimHasRun = TRUE;
							break;
					case DCC_DOSTOP:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							B_AnimHasRun = TRUE;
							break;
					case DCC_DOEASYTERM:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RAnimDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RAnimDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
#if _PRINTF
				printf("Sending reply %d to guide\n",Msg_RAnimDial->pd_Cmd);
#endif
				ReplyMsg((struct Message *)Msg_RAnimDial);
			}
		}

		// get a reply from our guide or the TRANSITION module
		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RAnimDial = (PROCDIALOGUE *)GetMsg(RepP_WorkAnim)) != NULL)
			{
				Msg_RAnimDial->pd_InUse = FALSE;
#if _PRINTF
				printf("Received reply %d from replier [%s]\n",Msg_RAnimDial->pd_Cmd,Msg_RAnimDial->pd_ChildPI);
#endif
				// Lets see what we've got.
				// Right now we don't need to check the reply cmds
				// since we don't use them.
				switch(Msg_RAnimDial->pd_Cmd)
				{
					case DCC_IGNORE:
							break;
					case DCI_TRANSITION:
#if _PRINTF
							printf("Received run reply from the TRANSITION module\n");
#endif
							if(!B_Remove && !B_Term)
								B_Run = TRUE;
							break;	
					default:
							break;
				}
			}
		}

		if(B_Stop)
		{
#if _PRINTF
			printf("Stop\n");
#endif
			if(!ErrLoadAnim)
			{
#if _PRINTF
				printf("unload file\n");
#endif
				unload_anim(AnimDataSegment);
			}
			ErrLoadAnim = TRUE;	
			if(!ErrSysPass)
			{
#if _PRINTF
				printf("release slide\n");
#endif
				release_anim(AnimDataSegment);
			}
			ErrSysPass = TRUE;
			MLMMU_FreeMem(AnimDataSegment);
			AnimDataSegment = NULL;
#if _PRINTF
			printf("End Stop\n");
#endif
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term) || !B_AnimHasRun)
		{
			if(B_Run)
			{
#if _PRINTF
				printf("sema at %x\n",(int)&MLSystem->ms_Sema_Transition);
#endif
				ObtainSemaphore(&MLSystem->ms_Sema_Transition);
#if _PRINTF
				printf("obtained sema at %x\n",(int)&MLSystem->ms_Sema_Transition);
#endif
				do_anim(AnimDataSegment);
				B_AnimHasRun = TRUE;
				ReleaseSemaphore(&MLSystem->ms_Sema_Transition);
#if _PRINTF
				printf("freed sema at %x\n",(int)&MLSystem->ms_Sema_Transition);
#endif

				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
				{
					if(AnimDataSegment == NULL)
					{
						AnimDataSegment = (UBYTE *)MLMMU_AllocMem(5000L,MEMF_PUBLIC|MEMF_CLEAR,NULL);
#if _PRINTF
						printf("alloc datasegment at %x\n",(int)AnimDataSegment);
#endif
					}	
					if(AnimDataSegment != NULL)
					{
						if(ErrSysPass)
						{
							ErrSysPass = (int)pass_mlsystem(MLSystem,AnimDataSegment,ThisPI->pi_Port_PtoC);
#if _PRINTF
							printf("pass system error = %d\n",ErrSysPass);
#endif
						}		
						if(!ErrSysPass)
						{
							if(ErrLoadAnim)
								ErrLoadAnim = (int)load_anim(FileName,AnimDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
						}
					}
				}
				B_ReInit = SendDialogue(Msg_AnimDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;
			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
				{
#if _PRINTF
					printf("PD %d in use with command %d\n",i,((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_Cmd);
#endif
//					B_Term = FALSE;
				}
			if(Msg_AnimDial->pd_InUse && (Msg_AnimDial->pd_Cmd == DCI_CHILDREADY))
			{	
#if _PRINTF
				printf("AnimDial in use with command %d\n",i,((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_Cmd);
#endif
//				B_Term = FALSE;
			}
		}
		// Check if there are still messages in the portlist
		// if so then signal ourself
		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
		{
#if _PRINTF
			printf("Signalling myself\n");	
#endif
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
		}
	}

#if _PRINTF
	printf("Term\n");
#endif
	if(!ErrLoadAnim)
	{
#if _PRINTF
		printf("unload file\n");
#endif
		unload_anim(AnimDataSegment);
	}
	if(!ErrSysPass)
	{
#if _PRINTF
		printf("release slide\n");
#endif
		release_anim(AnimDataSegment);
	}
	MLMMU_FreeMem(AnimDataSegment);
#if _PRINTF
	printf("End Term\n");
#endif

	MLMMU_FreeMem(Msg_AnimDial2);
	MLMMU_FreeMem(Msg_AnimDial);
	DeletePort(RepP_WorkAnim);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = FPS;
	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = CBITS;
	ThisPI->pi_Arguments.ar_RetErr = AnimErr;
}
