/******************************************************
*Desc : Perform a ScriptTalk "DOS" command
*		This routine will either wait for the command
*		to be finished or  
* <!> : This module is resident and re-entrant
*		Compile without -b1 and without -y options
*		Link with cres.o in stead of c.o
*		Also compile umain.c : lc -b1 umain 
*
*/

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
#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>

#define VERSI0N "\0$VER: 1.2"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE

int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer);

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

/*************************************************
*Func : Perform a dos command
* <!> : This command will set up a process for
*		the command and leave on its own.
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{
  PROCDIALOGUE 	*Msg_DosDial,	// Our dialogue 
				*Msg_LastDosDial,	// Used for double checking function only
				*Msg_RDosDial;	// Our dialogue when our guide replies

  PROCESSINFO *ThisPI;			// ptr to this processinfo blk (as used in our parent's list)
  struct MsgPort *RepP_WorkDos;	// Reply port for our parent when replying to our messages

  ULONG Sig_PtoC,				// A parent to child signal
    	SigR_CtoP,				// A reply to a msg we send to our parent
		SigRecvd;				// Signals received
  int 	DosErr, i;
  char 	*DosCmd;				// Command string to be executed
  int 	SizeOfDosCmd;			// Length of command string
  BOOL 	B_ReInit,				// if TRUE, re-initialise data
		B_Term,					// If TRUE, we are free to terminate
		B_Run,			
		B_Remove;				// If True, our guide wants us to clean up
  char Varcont[100];
  MLSYSTEM *MLSystem;	

	DosErr = ERR_WORKER;
	RepP_WorkDos = NULL;
	Msg_DosDial = NULL;
	DosCmd = NULL;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	// string memory size for the dos command line
	SizeOfDosCmd = strlen(ThisPI->pi_Arguments.ar_Worker.aw_Path)
				   + strlen("c:Execute ")
				   + strlen(ThisPI->pi_Arguments.ar_Worker.aw_Name) + 10
				   + strlen("c:Stack ?????");

	// allocate various memory blocks
	if( 
		((RepP_WorkDos = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_DosDial = (PROCDIALOGUE *)AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((DosCmd = (char *)AllocMem(SizeOfDosCmd, MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	  )
		goto LeaveWorkDos;

	// make command string
	if(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[1] == ARGUMENT_SCRIPT)
	{
		MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;
		if (ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] > 0)
			sprintf(DosCmd, "Stack %d\nExecute \"", ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2]);
		else
			strcpy(DosCmd, "Execute \"");
		MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,
					 ThisPI->pi_Arguments.ar_Worker.aw_Name,
					 &DosCmd[strlen(DosCmd)]);
		strcat(DosCmd,"\"");	
	}
	else
	{
		if (ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] > 0)
			sprintf(DosCmd, "Stack %d\n%s\n",
					ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2],
					ThisPI->pi_Arguments.ar_Worker.aw_Path);
		else
			strcpy(DosCmd, ThisPI->pi_Arguments.ar_Worker.aw_Path);
	}
	
	// Set up the Dialogue message
	Msg_DosDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_DosDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	// Attach the replyport to the message dialogue
	Msg_DosDial->pd_Msg.mn_ReplyPort = RepP_WorkDos;
	Msg_DosDial->pd_ChildPI = ThisPI;
	Msg_DosDial->pd_InUse = TRUE;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkDos->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	Msg_DosDial->pd_Cmd = DCI_CHILDREADY;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_DosDial);

	// main 	
	DosErr = NO_ERROR;
	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			Msg_LastDosDial = NULL;

			if( (Msg_RDosDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RDosDial->pd_ChildPI = ThisPI;
				// Lets see what we've got
				switch(Msg_RDosDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RDosDial->pd_Cmd = DCI_CHILDPREPARES;	
							B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							B_Run = TRUE;
							Msg_RDosDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOEASYTERM:
							Msg_RDosDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RDosDial->pd_Cmd = DCI_CHILDEASYSTOP;
							break;
					case DCC_DOTERM:
							Msg_RDosDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							break;
					case DCC_DOSTOP:
							Msg_RDosDial->pd_Cmd = DCI_CHILDREADY;	
							break;
					default:
							// simply ignore what we don't understand
							Msg_RDosDial->pd_Cmd = DCC_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RDosDial);
			}
		}

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RDosDial = (PROCDIALOGUE *)GetMsg(RepP_WorkDos)) != NULL)
				Msg_RDosDial->pd_InUse = FALSE;

		if(B_Run)
		{
			// START NEW VAR

			if ( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] == '@' )
			{
				if (ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] > 0)
					sprintf(DosCmd, "Stack %d\nExecute \"", ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2]);
				else
					strcpy(DosCmd, "Execute \"");
				if( FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_Name,
									MLSystem->VIList, Varcont ) )
					strcat(DosCmd,Varcont);
				strcat(DosCmd,"\"");	
//KPrintF("NEW dos cmd = [%s]\n", DosCmd);
			}

			// END NEW VAR

			// Start the dos command
			if(!Execute(DosCmd,NULL,NULL))														
				DosErr = ERR_WORKER;
			B_Run = FALSE;
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;
			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
					B_Term = FALSE;

			if(Msg_DosDial->pd_InUse)
				B_Term = FALSE;
		}
		else
		{
			if(B_ReInit)
			{
				// Since this worker is always ready we will
				// send a DCI_CHILDREADY to our guide to indicate we
				// are prepared to run. 
				// Other Xapps may first want to re-init their data before
				// sending a new DCI_CHILDREADY request to the guide.
				if(!Msg_DosDial->pd_InUse)
				{
					B_ReInit = FALSE;
					Msg_DosDial->pd_ChildPI = ThisPI;
					Msg_DosDial->pd_InUse = TRUE;
					Msg_DosDial->pd_Cmd = DCI_CHILDREADY;
					PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_DosDial);
				}
			}
		} // if B_Remove

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	// Free all
LeaveWorkDos:
	if(DosCmd)
		FreeMem(DosCmd,SizeOfDosCmd);
	if(Msg_DosDial)
		FreeMem(Msg_DosDial,sizeof(PROCDIALOGUE));
	if(RepP_WorkDos)
		DeletePort(RepP_WorkDos);

	ThisPI->pi_Arguments.ar_RetErr = DosErr;
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

/******** E O F ********/
