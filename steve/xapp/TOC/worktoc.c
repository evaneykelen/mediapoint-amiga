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
#include <exec/libraries.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <utility/tagitem.h>
#include <libraries/toccata.h>
#include <clib/exec_protos.h>
#include <clib/toccata_protos.h>
#include <pragmas/toccata_pragmas.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "demo:gen/general_protos.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define _PRINTF FALSE
#define _PRINTDEB FALSE

#define MAXSTRING 256

struct TagItem tocTags[] =
{
	{ TT_FileName, NULL	},
	{ TT_IoPri,127 },
	{ TT_BufferSize,30*1024 },
	{ TT_SmartPlay, TRUE },
	{ NULL,NULL		},
};

void XappSetup(PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath);

/*************************************************
*Func : Play an cdxl
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( int argc, char *argv[] )
{

	struct ToccataBase *ToccataBase=NULL;

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
	tocTags[0].ti_Data = FileName;


	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	MLMMULibBase = NULL;
	Msg_CDxlDial = NULL;
	Msg_CDxlDial2 = NULL;
	RepP_WorkCDxl = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkCDxl = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_CDxlDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL) ||
		((Msg_CDxlDial2 = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL) ||
		((ToccataBase = (struct ToccataBase *)OpenLibrary("toccata.library",0))== NULL )
	  )
	{
		MLMMU_FreeMem(Msg_CDxlDial2);
		MLMMU_FreeMem(Msg_CDxlDial);
		if(RepP_WorkCDxl)
			DeletePort(RepP_WorkCDxl);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		if( ToccataBase )
			CloseLibrary( ToccataBase );

		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	CDxlDataSegment = NULL;
	ErrSysPass = TRUE;
  ErrLoadCDxl = TRUE;

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
									B_Run = TRUE;
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
	//						B_Stop = TRUE;
	//						B_Run = FALSE;
	//						B_CDxlHasRun = TRUE;
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
			KPrintF("Stop toc\n");
			T_Stop( 0 );
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term))
		{
			if(B_Run)
			{
				T_Playback( tocTags );
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				B_ReInit = SendDialogue(Msg_CDxlDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			T_Stop( 0 );
			B_Term = TRUE;

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

	MLMMU_FreeMem(Msg_CDxlDial2);
	MLMMU_FreeMem(Msg_CDxlDial);
	DeletePort(RepP_WorkCDxl);
	CloseLibrary(MLMMULibBase);
	CloseLibrary( ToccataBase );

	ThisPI->pi_Arguments.ar_RetErr = CDxlErr;
}
