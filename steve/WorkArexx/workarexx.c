/******************************************************
*Desc : Perform a ScriptTalk "AREXX" command
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
#include "ph:rexx_pragma.h"
#include "ph:rexx_proto.h"
#include "ph:rexx.h"
#include "external.h"

#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>

#define VERSI0N "\0$VER: 1.2"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE

#define MAXSTRING 256

int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer);

/******** MakeFullPath() ********/
/*

 * Makes from 'anim:' and 'ape' the string 'anim:ape' and
 * makes from 'anim:test/pics/hires' and 'ape' the
 * string 'anim:test/pics/hires/ape'. Even more, the string
 * 'anim:test/pics/hires/' (note the slash) and 'ape' is correctly
 * converted (no double slashes). This seems trivial but this routine
 * should be used instead of trying to connect them yourself.
 *
 */

void MakeFullPath(STRPTR path, STRPTR name, STRPTR answer)
{
int len;

	len = strlen(path);
	if ( path[len-1]==':' || path[len-1]=='/' )
		sprintf(answer, "%s%s", path, name);
	else if (path!=NULL && path[0]!='\0')
		sprintf(answer, "%s/%s", path, name);
	else
		strcpy(answer, name);
}

/*********************************************************
*Func : Perform a Rexx command
* <!> : This command will set up a process for
*		the command and leave on its own.
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{
  char FileName[MAXSTRING];
  PROCDIALOGUE 	*Msg_RexxDial,	// Our dialogue 
				*Msg_LastRexxDial,	// Used for double checking function only
				*Msg_EasyStop,
				*Msg_RRexxDial;	// Our dialogue when our guide replies
  PROCESSINFO *ThisPI;			// ptr to this processinfo blk (as used in our parent's list)
  struct MsgPort *RepP_WorkRexx;	// Reply port for our parent when replying to our messages
  ULONG Sig_PtoC,				// A parent to child signal
    	SigR_CtoP,				// A reply to a msg we send to our parent
		SigR_RXtoXaPP,			// A reply from the arexx handler 
		SigRecvd;				// Signals received
  int 	RexxErr, i;
  char 	*RexxCmd;				// Command string to be executed
  int 	SizeOfRexxCmd;			// Length of command string
  BOOL 	B_ReInit,				// if TRUE, re-initialise data
		B_Term,					// If TRUE, we are free to terminate
		B_Run,			
  		B_Remove;				// If True, our guide wants us to clean up
  struct FileLock 		*RexxFileLock;
  struct FileInfoBlock  RexxFileFIB;
  AREXXCONTEXT *RexxContext;
  BOOL AllOK=FALSE;
  struct Library *MLMMULibBase = NULL;
  char err[128];	
  MLSYSTEM *MLSystem;	
  char Varcont[100];
 	
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	// Create a reply port for our guide, needs not to be public cause its base
	// ptr is passed on to the guide when sending a message
	if( (RepP_WorkRexx = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		// Return a general error 
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	// Make a Dialogue 
	if( (Msg_RexxDial = (PROCDIALOGUE *)AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		DeletePort(RepP_WorkRexx);
		// Return a general error 
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}
	
	// Open mlmmu

	MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0L);

	// Program init

	RexxErr = NO_ERROR;
	RexxCmd = NULL;
	SizeOfRexxCmd = 0;

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	// Now built a command string for the rexx command
	// Check mode (script or command)
	switch(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[1])
	{
		case ARGUMENT_COMMAND:
					SizeOfRexxCmd = strlen(ThisPI->pi_Arguments.ar_Worker.aw_Path) + 17 +
									strlen(&ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[50]);
					AllOK = TRUE;
					break;
		case ARGUMENT_SCRIPT:
					if ( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] == '@' )
					{
						AllOK = TRUE;
						if ( FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_Name,
											 MLSystem->VIList, Varcont) )
							strcpy(FileName,Varcont);
					}
					else
						MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,
									 ThisPI->pi_Arguments.ar_Worker.aw_Name,FileName);

				    if( (RexxFileLock = (struct FileLock *)Lock(FileName,ACCESS_READ)) != NULL)
					{
						if(Examine((BPTR)RexxFileLock,&RexxFileFIB))			
						{
							UnLock((BPTR)RexxFileLock);
							SizeOfRexxCmd = RexxFileFIB.fib_Size+10;
							AllOK = TRUE;
						}
						else
							UnLock((BPTR)RexxFileLock);
					}
					break;
	}

	if ( !AllOK && SizeOfRexxCmd==0 )
	{
		if(MLMMULibBase)
		{
			sprintf(err,"ARexx script '%s' is missing.",FileName); 
			MLMMU_AddMsgToQueue(err,1);
		}
	}

	if ( AllOK && SizeOfRexxCmd>0 )
		RexxCmd = MakeRexxCmd(SizeOfRexxCmd,ThisPI,FileName);

	// Set up the Dialogue message
	Msg_RexxDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_RexxDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	// Attach the replyport to the message dialogue
	Msg_RexxDial->pd_Msg.mn_ReplyPort = RepP_WorkRexx;
	Msg_RexxDial->pd_ChildPI = ThisPI;
	Msg_RexxDial->pd_InUse = TRUE;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkRexx->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	SigR_RXtoXaPP = 0;

	// Send a message to the guide to indicate we are ready to start
	Msg_RexxDial->pd_Cmd = DCI_CHILDREADY;
#if _PRINTF
	printf("Sending cmd DCI_CHILDREADY to guide\n");
#endif
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_RexxDial);

	// main 	
	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	Msg_EasyStop = NULL;
	RexxContext = NULL;

	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SigR_RXtoXaPP | SIGF_ABORT);

		if( (SigRecvd & SigR_RXtoXaPP) && (RexxContext != NULL))
		{
			// received reply on msg send, free the rexxmessage
#if _PRINTF
			printf("Received Arexx reply\n");
#endif
			GetARexxMsg(RexxContext);
			FreeARexx(RexxContext);
			RexxContext = NULL;
			SigR_RXtoXaPP = 0;

			if(Msg_EasyStop != NULL)
			{
				ReplyMsg((struct Message *)Msg_EasyStop);
				B_Remove = TRUE;
				Msg_EasyStop = NULL;
			}
		}

		if(SigRecvd & Sig_PtoC)
		{
#if _PRINTF
			printf("Received signal PtoC\n");
#endif
			Msg_LastRexxDial = NULL;
			if( (Msg_RRexxDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RRexxDial->pd_ChildPI = ThisPI;
#if _PRINTF
				printf("Msg at %x in use ? %d\n",(int)Msg_RRexxDial,Msg_RRexxDial->pd_InUse);
				printf("Received cmd %d from guide\n",Msg_RRexxDial->pd_Cmd);
#endif
				// Lets see what we've got
				switch(Msg_RRexxDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RRexxDial->pd_Cmd = DCI_CHILDPREPARES;	
							B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							B_Run = TRUE;
							Msg_RRexxDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOEASYTERM:
							Msg_RRexxDial->pd_Cmd = DCI_CHILDEASYTERM;
							if(RexxContext != NULL)
								Msg_EasyStop = Msg_RRexxDial;
							else
								B_Remove = TRUE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RRexxDial->pd_Cmd = DCI_CHILDEASYSTOP;
							if(RexxContext != NULL)
								Msg_EasyStop = Msg_RRexxDial;
							break;
					case DCC_DOTERM:
							Msg_RRexxDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							break;
					case DCC_DOSTOP:
							Msg_RRexxDial->pd_Cmd = DCI_CHILDREADY;	
							break;
					default:
							// simply ignore what we don't understand
							Msg_RRexxDial->pd_Cmd = DCC_IGNORE;	
							break;
				}
#if _PRINTF
				printf("Sending reply %d to guide\n",Msg_RRexxDial->pd_Cmd);
#endif
				if(Msg_EasyStop == NULL)
					ReplyMsg((struct Message *)Msg_RRexxDial);
			}
		}

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RRexxDial = (PROCDIALOGUE *)GetMsg(RepP_WorkRexx)) != NULL)
			{
				Msg_RRexxDial->pd_InUse = FALSE;
#if _PRINTF
				printf("Received reply %d from guide\n",Msg_RRexxDial->pd_Cmd);
#endif
				switch(Msg_RRexxDial->pd_Cmd)
				{
					case DCC_IGNORE:
							break;
					default:
							break;
				}
			}
		}

		if(B_Run)
		{
			// couldn't setup rexxcmd list earlier, final attempt

			if ( AllOK )
			{
				// START NEW VARS
				if ( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] == '@' )
				{
					if(RexxCmd)	// Free memory allocated earlier
						FreeMem(RexxCmd,SizeOfRexxCmd);
					if ( FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_Name,
										 MLSystem->VIList, Varcont) )
						strcpy(FileName,Varcont);
				    if( (RexxFileLock = (struct FileLock *)Lock(FileName,ACCESS_READ)) != NULL)
					{
						if(Examine((BPTR)RexxFileLock,&RexxFileFIB))			
						{
							UnLock((BPTR)RexxFileLock);
							SizeOfRexxCmd = RexxFileFIB.fib_Size+10;
							RexxCmd = MakeRexxCmd(SizeOfRexxCmd,ThisPI,FileName);
						}
						else
							UnLock((BPTR)RexxFileLock);
					}
				}
				// END NEW VARS

				if(RexxCmd == NULL)
					RexxCmd = MakeRexxCmd(SizeOfRexxCmd,ThisPI,FileName);
				if((RexxCmd != NULL) && (RexxContext == NULL))
					if( (RexxContext = PerformRexxCmd(RexxCmd)) != NULL)
						SigR_RXtoXaPP = 1<<RexxContext->ARexxPort->mp_SigBit;
			}

			B_Run = FALSE;
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			if(Msg_EasyStop != NULL)
				ReplyMsg((struct Message *)Msg_EasyStop);
			Msg_EasyStop = NULL;

			B_Term = TRUE;
			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
				{
#if _PRINTF
					printf("PD %d in use with command %d\n",i,((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_Cmd);
#endif
					B_Term = FALSE;
				}
			if(Msg_RexxDial->pd_InUse)
			{	
#if _PRINTF
				printf("RexxDial in use with command %d\n",i,((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_Cmd);
#endif
				B_Term = FALSE;
			}
		}
		else
		{
			if(B_ReInit)
			{
				if ( AllOK )
				{
					if(RexxCmd == NULL)
						RexxCmd = MakeRexxCmd(SizeOfRexxCmd,ThisPI,FileName);
				}

				if(!Msg_RexxDial->pd_InUse)
				{
					B_ReInit = FALSE;
					Msg_RexxDial->pd_ChildPI = ThisPI;
					Msg_RexxDial->pd_InUse = TRUE;
					Msg_RexxDial->pd_Cmd = DCI_CHILDREADY;
#if _PRINTF
					printf("Sending cmd DCI_CHILDREADY to guide\n");
#endif
					PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_RexxDial);
				}
			}
		}

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	// Free
	if(RexxContext)
		FreeARexx(RexxContext);

	if(RexxCmd)
		FreeMem(RexxCmd,SizeOfRexxCmd);

	FreeMem(Msg_RexxDial,sizeof(PROCDIALOGUE));
	DeletePort(RepP_WorkRexx);

	if(MLMMULibBase)
		CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = RexxErr;
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
