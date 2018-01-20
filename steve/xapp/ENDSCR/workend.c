/******************************************************
*Desc : Perform a ScriptTalk "Resource" command
*				This routine will either wait for the command
*				to be finished or  
* <!> : This module is resident and re-entrant
*				Compile without -b1 and without -y options
*				Link with cres.o in stead of c.o
*				Also compile umain.c : lc -b1 umain 
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "demo:gen/general_protos.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "mllib:medialinklib_proto.h"
#include "mllib:medialinklib_pragma.h"

#define _PRINTF FALSE
#define _PRINTDEB FALSE

int CopyNewScriptEND( char *path, struct Library *DOSBase );
int ClearArchiveEND( char *path, struct Library *DOSBase );

//===============================================
//	Name		: SayRun
//	Function	: Check which script runs
//	Inputs	: 
//	Result	: 1 or 2 or 0 when file not found
//	Updated	: 19 - 09 - 1994
//
int SayRun ( void )
{
	char str[256];
	FILE *f;	
	int check=0;

	f = fopen("MP_RA:RUNS_script.1","r");

	if( f )
	{
		check =1;
		fclose( f );
	}

	if( check ==  0 )
	{
		f = fopen("MP_RA:RUNS_script.2","r");
		if( f )
		{
			check =2;
			fclose( f );
		}
	}
	return( check );
}

void CheckAndChange( void )
{
	BPTR lock;
	char tt[256];
	int run;
	struct Library *DOSBase;
	struct Library *medialinkLibBase;
	FILE *fh;

	DOSBase = OpenLibrary("dos.library",0 );
	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
	if ( !medialinkLibBase )
		return;

//	KPrintF("Open base\n");

	if( DOSBase )
	{
		lock = Lock( "MP_RA:Swap", ACCESS_READ );
		if( lock )
		{
			UnLock( lock );
			run = SayRun();

			if( run )
			{
				DeleteFile( "MP_RA:swap" );

				fh = fopen("MP_RA:swapping","w");
				if ( fh )
				{
					fprintf(fh," ");
					fclose(fh);
				}

				//SendArexx( "MEDIAPOINT","quit" );

				// changed Send the quit when you have copied the files
				//UA_IssueRexxCmd_V2("MP_ENDSCR","MEDIAPOINT","QUIT",NULL,NULL);
				// end changed

				if( run == 1 )									// set alias to other script
					run = 2;
				else
					run = 1;

				// Moved to the player
				// sprintf(tt,"assign ALIAS: MP_RA:script%d\n",run );
				// Execute( tt,0,0 );							// do  alias

				// Delete and copy all files from script1 to script2

				if( run == 1 )
					strcpy(tt,"MP_RA:script2");
				else
					strcpy(tt,"MP_RA:script1");

				ClearArchiveEND( tt, DOSBase );		// clear all the archive bits old dir

				if( run == 1 )
					strcpy(tt,"MP_RA:script1");
				else
					strcpy(tt,"MP_RA:script2");

				CopyNewScriptEND( tt, DOSBase );		// copy and delete all the files

				//Delay(30*50);
				UA_IssueRexxCmd_V2("MP_ENDSCR","MEDIAPOINT","QUIT",NULL,NULL);

				DeleteFile( "MP_RA:swapping" );
			}	
		}
	CloseLibrary((struct Library *)medialinkLibBase);
	CloseLibrary( DOSBase );
	}
}

void main( int argc, char **argv)
{

	int		ErrLoadVSC;	

	PROCDIALOGUE	*Msg_VSCDial,	// Our dialogue 
						*Msg_RVSCDial;	// Our dialogue when our guide replies

	MLSYSTEM			*MLSystem;	
	PROCESSINFO		*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

	struct MsgPort	*RepP_WorkVSC;// Reply port for our parent when replying to our messages

	ULONG		Sig_PtoC,		// A parent to child signal
				SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
	int		i,action;
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
	Msg_VSCDial = NULL;
	RepP_WorkVSC = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkVSC = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_VSCDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_VSCDial);
		if(RepP_WorkVSC)
			DeletePort(RepP_WorkVSC);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	ErrLoadVSC = TRUE;

	// Set up the Dialogue message
	Msg_VSCDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_VSCDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_VSCDial->pd_Msg.mn_ReplyPort = RepP_WorkVSC;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkVSC->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_VSCDial,ThisPI,DCI_CHILDREADY);

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
			if( (Msg_RVSCDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RVSCDial->pd_ChildPI = ThisPI;
				switch(Msg_RVSCDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
								B_Run = TRUE;
							Msg_RVSCDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
						Msg_RVSCDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOSTOP:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYTERM:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RVSCDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RVSCDial);
			}
		}

		// get a reply from our guide or the TRANSITION module
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RVSCDial = (PROCDIALOGUE *)GetMsg(RepP_WorkVSC)) != NULL)
				Msg_RVSCDial->pd_InUse = FALSE;

		if(B_Stop)
		{
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term ) )
		{
			if(B_Run)
			{
				CheckAndChange();
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				B_ReInit = SendDialogue(Msg_VSCDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;

			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
					B_Term = FALSE;

		if(Msg_VSCDial->pd_InUse && (Msg_VSCDial->pd_Cmd == DCI_CHILDREADY))
				B_Term = FALSE;
		}

		// Check if there are still messages in the portlist
		// if so then signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	MLMMU_FreeMem(Msg_VSCDial);
	DeletePort(RepP_WorkVSC);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
}
