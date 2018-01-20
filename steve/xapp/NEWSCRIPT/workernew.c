#include <exec/exec.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <stdlib.h>
#include <stdio.h>

#include "demo:Gen/MINC/types.h"
#include "demo:Gen/MINC/Errors.h"
#include "demo:Gen/MINC/process.h"

#include "demo:Gen/MLMMU/mlmmu.h"
#include "demo:Gen/MLMMU/mlmmu_pragma.h"
#include "demo:Gen/MLMMU/mlmmu_proto.h"

#include "demo:Gen/mllib/medialinklib_proto.h"
#include "demo:Gen/mllib/medialinklib_pragma.h"

#include "demo:Gen/general_protos.h"

#include "nb:parser.h"

extern void XappSetup(PROCESSINFO *ThisPI);
extern int XappDoIt(PROCESSINFO *ThisPI);
extern void GetExtraData(PROCESSINFO *ThisPI, char *path);
int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer);

/******** load_file() ********/

void load_file( char *name , struct Library *MLMMULibBase )
{
char cmd[150];
struct Library *medialinkLibBase;

	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
	if ( medialinkLibBase )
	{
		sprintf(cmd,"NEWSCRIPT %s",name);
		Permit();
		UA_IssueRexxCmd_V2("MP_NEWSCR","MEDIAPOINT",cmd,NULL,NULL);
		UA_IssueRexxCmd_V2("MP_NEWSCR","MEDIAPOINT","QUIT",NULL,NULL);
		Forbid();
		CloseLibrary((struct Library *)medialinkLibBase);
	}
}

void main( int argc, char *argv[] )
{
  char FileName[300];

  int 			ErrSysPass,
				ErrLoad;	

  PROCDIALOGUE 	*Msg_Dial,		// Our dialogue 
				*Msg_Dial2,	
				*Msg_RDial;		// Our dialogue when our guide replies

  MLSYSTEM	  	*MLSystem;	
  PROCESSINFO 	*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

  struct MsgPort *RepP_Work;	// Reply port for our parent when replying to our messages
  ULONG 		Sig_PtoC,		// A parent to child signal
    			SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
  int 			Err;
  BOOL			B_HasRun,
				B_ReInit,		// if TRUE, re-initialise data
				B_Term,			// If TRUE, we are free to terminate
				B_Run,			
				B_Stop,
				B_Remove;		// If True, our guide wants us to clean up
  struct Library *MLMMULibBase;
  char Varcont[100];

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

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	GetExtraData( ThisPI, FileName );

	MLMMULibBase = NULL;
	Msg_Dial = NULL;
	Msg_Dial2 = NULL;
	RepP_Work = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_Work = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_Dial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL) ||
		((Msg_Dial2 = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_Dial2);
		MLMMU_FreeMem(Msg_Dial);
		if(RepP_Work)
			DeletePort(RepP_Work);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	ErrSysPass = TRUE;
    ErrLoad = TRUE;

	Err = NO_ERROR;
	// Set up the Dialogue message
	Msg_Dial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_Dial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_Dial->pd_Msg.mn_ReplyPort = RepP_Work;
	Msg_Dial2->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_Dial2->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_Dial2->pd_Msg.mn_ReplyPort = RepP_Work;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_Work->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_Dial,ThisPI,DCI_CHILDREADY);

	// main 	
	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	B_Stop = FALSE;
	B_HasRun = TRUE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RDial->pd_ChildPI = ThisPI;
				switch(Msg_RDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
							{
								B_Run = TRUE;
							}
							Msg_RDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
							Msg_RDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							B_HasRun = TRUE;
							break;
					case DCC_DOSTOP:
							Msg_RDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							B_HasRun = TRUE;
							break;
					case DCC_DOEASYTERM:
							Msg_RDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RDial);
			}
		}

		// get a reply from our guide or the TRANSITION module

		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RDial = (PROCDIALOGUE *)GetMsg(RepP_Work)) != NULL)
			{
				Msg_RDial->pd_InUse = FALSE;
				switch(Msg_RDial->pd_Cmd)
				{
					case DCC_IGNORE:
							break;
					default:
							break;
				}
			}
		}

		if(B_Stop)
			B_Stop = FALSE;

		if( (!B_Remove && !B_Term) || !B_HasRun)
		{
			if(B_Run)
			{
				//ObtainSemaphore(&MLSystem->ms_Sema_Transition);
				//XappDoIt(ThisPI);
				//B_HasRun = TRUE;
				//ReleaseSemaphore(&MLSystem->ms_Sema_Transition);

				if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0]=='@' )
				{
					if ( FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,MLSystem->VIList,Varcont) )
						strcpy(FileName,Varcont);
				}

				load_file( FileName, MLMMULibBase );
				B_Run = FALSE;
			}

			if(B_ReInit)
				B_ReInit = SendDialogue(Msg_Dial,ThisPI,DCI_CHILDREADY);
		}

		if(B_Remove)
			B_Term = TRUE;

		// Check if there are still messages in the portlist
		// if so then signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	MLMMU_FreeMem(Msg_Dial2);
	MLMMU_FreeMem(Msg_Dial);
	DeletePort(RepP_Work);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = Err;
}

/******** FindVarContents() ********/

int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer)
{
VIR *this_vir;
TEXT local_varName[20];
int i=0;

	answer[0] = '\0';
	local_varName[0] = '\0';

	for(i=0; i<19; i++)
	{
		if ( varName[i+1] != ')' )	// +1 skips @
			local_varName[i] = varName[i+1];
		else
		{
			local_varName[i] = ')';
			break;
		}
	}

	local_varName[i+1] = '\0';
	if ( local_varName[0] == '\0' )
		return(0);

	i = strlen(local_varName);

	if ( i<3 || local_varName[0] != '(' || local_varName[ i-1 ] != ')' )
		return(0);

	stccpy(local_varName, &local_varName[1], i-1);	// str=(str) -> copy from 'str'

	for(this_vir = (VIR *)VIList->lh_Head; 
		(VIR *)this_vir->vir_Node.ln_Succ;	
		this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		if ( !stricmp(local_varName, this_vir->vir_Name) )
		{
			if ( this_vir->vir_Type == VCT_STRING )
				stccpy(answer, this_vir->vir_String, 49);
			else
				sprintf(answer,"%d",this_vir->vir_Integer);
		}
	}

	if ( !answer[0] )
		strcpy(answer," ");

	return( (int)(strlen( local_varName ) + 3) );		// add @ + ( and )
}
