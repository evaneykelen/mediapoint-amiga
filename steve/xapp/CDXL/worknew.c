/******************************************************
*Desc : Perform a ScriptTalk "CDXL" command
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
#include	"gp:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define _PRINTF FALSE
#define _PRINTDEB FALSE

#define MAXSTRING 256

void XappSetup(PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath);

/*************************************************
*Func : Play an cdxl
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( int argc, char *argv[] )
{

  char 			FileName[MAXSTRING];

  UBYTE 		*CDxlDataSegment;
  int 			ErrSysPass,
				ErrLoadCDxl;	

  PROCDIALOGUE 	*Msg_CDxlDial,	// Our dialogue 
				*Msg_CDxlDial2,	
				*Msg_RCDxlDial;	// Our dialogue when our guide replies

  MLSYSTEM	  	*MLSystem;	
  PROCESSINFO 	*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

  struct MsgPort *RepP_WorkCDxl;// Reply port for our parent when replying to our messages
  ULONG 		Sig_PtoC,		// A parent to child signal
    			SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
  int 			CDxlErr, i;
  BOOL			B_CDxlHasRun,
				B_ReInit,		// if TRUE, re-initialise data
				B_Term,			// If TRUE, we are free to terminate
				B_Run,			
				B_Stop,
				B_Remove;		// If True, our guide wants us to clean up

	struct Library *MLMMULibBase;
	long oldpri;
	struct Task *task;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	/**** this is called when showing the GUI ****/

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}

	// Program init

	// GetExtraData (see bottom of file setup.c) gets the path, and
	// numArgs 9, 10 and 11. 

	GetExtraData(ThisPI, FileName);
	if ( FileName[0] == '\0' )
	{
    ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		//KPrintF("return direct\n");
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	MLMMULibBase = NULL;
	Msg_CDxlDial = NULL;
	Msg_CDxlDial2 = NULL;
	RepP_WorkCDxl = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkCDxl = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_CDxlDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL) ||
		((Msg_CDxlDial2 = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_CDxlDial2);
		MLMMU_FreeMem(Msg_CDxlDial);
		if(RepP_WorkCDxl)
			DeletePort(RepP_WorkCDxl);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	CDxlDataSegment = NULL;
	ErrSysPass = TRUE;
    ErrLoadCDxl = TRUE;
	if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
	{
		if( (CDxlDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
		{
#if _PRINTF
			printf("Data segment alloc ok\n");
#endif
//			if((ErrSysPass = (int)pass_mlsystem(MLSystem,CDxlDataSegment, ThisPI->pi_Port_PtoC)) == 0)
//			{
#if _PRINTF
				printf("pass system ok\n");	
#endif
//				ErrLoadCDxl = load_cdxl(FileName,CDxlDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
//			}
		}
	}

#if _PRINTF
	printf("Datasegment at %x\n",(int)CDxlDataSegment);
	printf("pass system error %d\n",ErrSysPass);
	printf("Load cdxl error %d\n",ErrLoadCDxl);
#endif

	CDxlErr = NO_ERROR;
	// Set up the Dialogue message
	Msg_CDxlDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_CDxlDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_CDxlDial->pd_Msg.mn_ReplyPort = RepP_WorkCDxl;
	Msg_CDxlDial2->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_CDxlDial2->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_CDxlDial2->pd_Msg.mn_ReplyPort = RepP_WorkCDxl;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkCDxl->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_CDxlDial,ThisPI,DCI_CHILDREADY);

	//KPrintF("Starting the cdxl xapp\n");

	// main 	
	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	B_Stop = FALSE;
	B_CDxlHasRun = TRUE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RCDxlDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{

#if _PRINTF
			printf("received cmd %d from guide\n",(int)Msg_RCDxlDial->pd_Cmd);
#endif
				Msg_RCDxlDial->pd_ChildPI = ThisPI;
				switch(Msg_RCDxlDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
							{
								// see if the CDxl is NOT in memory yet
								// Last effort to get the picture into memory!
								if(ErrLoadCDxl)
								{
									if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
									{
										if(CDxlDataSegment == NULL)
										{
											CDxlDataSegment = (UBYTE *)MLMMU_AllocMem(5000L, MEMF_PUBLIC|MEMF_CLEAR,NULL);
#if _PRINTF
											printf("ralloc datasegment at %x\n",(int)CDxlDataSegment);
#endif
										}	
										if(CDxlDataSegment != NULL)
										{
											if(ErrSysPass != 0)
											{
//												ErrSysPass = (int)pass_mlsystem(MLSystem,CDxlDataSegment, ThisPI->pi_Port_PtoC);
#if _PRINTF
												printf("rpass system error = %d\n",ErrSysPass);
#endif
											}
//											if(ErrSysPass == 0)
//												ErrLoadCDxl = (int)load_cdxl(FileName,CDxlDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
										}
									}
								}
								if(ErrLoadCDxl == 0)
								{
									B_Run = TRUE;
								}
							}
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							B_CDxlHasRun = TRUE;
							break;
					case DCC_DOSTOP:
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							B_CDxlHasRun = TRUE;
							break;
					case DCC_DOEASYTERM:
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RCDxlDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RCDxlDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
#if _PRINTF
				printf("Sending reply %d to guide\n",Msg_RCDxlDial->pd_Cmd);
#endif
				ReplyMsg((struct Message *)Msg_RCDxlDial);
			}
		}

		// get a reply from our guide or the TRANSITION module

		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RCDxlDial = (PROCDIALOGUE *)GetMsg(RepP_WorkCDxl)) != NULL)
			{
				Msg_RCDxlDial->pd_InUse = FALSE;
#if _PRINTF
				printf("Received reply %d from guide\n",Msg_RCDxlDial->pd_Cmd);
#endif
				// Lets see what we've got.
				// Right now we don't need to check the reply cmds
				// since we don't use them.
				switch(Msg_RCDxlDial->pd_Cmd)
				{
					case DCC_IGNORE:
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
			if(!ErrLoadCDxl)
			{
#if _PRINTF
				printf("unload file\n");
#endif
				unload_cdxl(CDxlDataSegment);
			}
			ErrLoadCDxl = TRUE;	
			if(!ErrSysPass)
			{
#if _PRINTF
				printf("release slide\n");
#endif
				release_cdxl(CDxlDataSegment);
			}
			ErrSysPass = TRUE;
			MLMMU_FreeMem(CDxlDataSegment);
			CDxlDataSegment = NULL;
#if _PRINTF
			printf("End Stop\n");
#endif
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term) || !B_CDxlHasRun)
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
				task = FindTask( 0 );
				oldpri = SetTaskPri(task, 21 );
//				do_cdxl(CDxlDataSegment);
				SetTaskPri( task,oldpri );

				B_CDxlHasRun = TRUE;
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
					if(CDxlDataSegment == NULL)
					{
						CDxlDataSegment = (UBYTE *)MLMMU_AllocMem(5000L,MEMF_PUBLIC|MEMF_CLEAR,NULL);
#if _PRINTF
						printf("alloc datasegment at %x\n",(int)CDxlDataSegment);
#endif
					}	
					if(CDxlDataSegment != NULL)
					{
						if(ErrSysPass)
						{
//							ErrSysPass = (int)pass_mlsystem(MLSystem,CDxlDataSegment,ThisPI->pi_Port_PtoC);
#if _PRINTF
							printf("pass system error = %d\n",ErrSysPass);
#endif
						}		
						if(!ErrSysPass)
						{
//							if(ErrLoadCDxl)
//								ErrLoadCDxl = (int)load_cdxl(FileName,CDxlDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
						}
					}
				}
				B_ReInit = SendDialogue(Msg_CDxlDial,ThisPI,DCI_CHILDREADY);
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
			if(Msg_CDxlDial->pd_InUse && (Msg_CDxlDial->pd_Cmd == DCI_CHILDREADY))
			{	
#if _PRINTF
				printf("CDxlDial in use with command %d\n",i,((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_Cmd);
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
	if(!ErrLoadCDxl)
	{
#if _PRINTF
		printf("unload file\n");
#endif
		unload_cdxl(CDxlDataSegment);
	}
	if(!ErrSysPass)
	{
#if _PRINTF
		printf("release slide\n");
#endif
		release_cdxl(CDxlDataSegment);
	}
	MLMMU_FreeMem(CDxlDataSegment);
#if _PRINTF
	printf("End Term\n");
#endif

	MLMMU_FreeMem(Msg_CDxlDial2);
	MLMMU_FreeMem(Msg_CDxlDial);
	DeletePort(RepP_WorkCDxl);
	CloseLibrary(MLMMULibBase);

//	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = CBITS;
	ThisPI->pi_Arguments.ar_RetErr = CDxlErr;
}
