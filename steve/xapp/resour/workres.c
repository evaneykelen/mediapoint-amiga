/******************************************************
*Desc : Perform a ScriptTalk "Resource" command
*		This routine will either wait for the command
*		to be finished or  
* <!> : This module is resident and re-entrant
*		Compile without -b1 and without -y options
*		Link with cres.o in stead of c.o
*		Also compile umain.c : lc -b1 umain 
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
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

#define _PRINTF FALSE
#define _PRINTDEB FALSE

void XappSetup(PROCESSINFO *ThisPI);
extern void GetExtraData(PROCESSINFO *ThisPI, char *path, int *mode);

void load_file( char *name , struct Library *MLMMULibBase )
{
	struct Library *DOSBase;
	char err[170];
	FILE *fp;
	MEMTAG *memtag;
	int t;
	struct FileInfoBlock *Finfo;
	BPTR	lock;
	long size = 0;
	char *t_data;

	// First check the name in the MLMMU lib

	memtag = MLMMU_FindMem( name );
	if( memtag == NULL )
	{

		DOSBase  = OpenLibrary( "dos.library", 0 );

		size = 0;
		Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );

		if( Finfo )
		{
			lock = Lock( name, ACCESS_READ );
			if( lock != NULL )
			{
				Examine( lock, Finfo );
				size = Finfo->fib_Size;
				UnLock( lock );
			}
			FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
		}

		//KPrintF("Size file is %ld\n",size );

		fp = fopen( name, "r" );
		if( fp != NULL )
		{
			t_data = (char *)MLMMU_AllocMem( size, MEMF_PUBLIC|MEMF_FREE_ALWAYS, name );
			if( t_data != NULL )
			{
				if( (t = fread( t_data , 1, size , fp )) != size );
				MLMMU_SetMemStat( MTF_SETCLR | MTF_INIT, t_data );
				//MLMMU_FreeMem(t_data);
			}
			fclose( fp );
		}
		else
		{
			strcpy(err,"File not found : " );
			strcat(err, name );
			//KPrintF("%s\n",err );
			//MLMMU_AddMsgToQueue(err, 1 );
		}
		CloseLibrary( DOSBase );
	}
	//else
	//	KPrintF("Data already in MMU\n");
}

void unload_file( char *name , struct Library *MLMMULibBase )
{
	struct Library *DOSBase;
	char err[170];
	FILE *fp;
	MEMTAG *memtag;
	struct FileInfoBlock *Finfo;
	BPTR	lock;
	long size = 0;
	char *t_data;

	// First check the name in the MPMMU lib

	DOSBase  = OpenLibrary( "dos.library", 0 );

	memtag = MLMMU_FindMem( name );
	if( memtag != NULL )
	{
		size = 0;
		Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );

		if( Finfo )
		{
			lock = Lock( name, ACCESS_READ );
			if( lock != NULL )
			{
				Examine( lock, Finfo );
				size = Finfo->fib_Size;
				UnLock( lock );
			}
			FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
		}

		fp = fopen( name, "r" );
		if( fp != NULL )
		{
			t_data = (char *)MLMMU_AllocMem( size, MEMF_PUBLIC, name );
			if ( t_data )
			{
				MLMMU_FreeMem( t_data );		// just free twice
				MLMMU_FreeMem( t_data );		// just free twice
			}
			fclose( fp );
		}
		else
		{
			strcpy(err,"File not found : " );
			strcat(err, name );
			//MLMMU_AddMsgToQueue(err, 1 );
		}

	}
	//else
	//	KPrintF("Data not in the MPMMU\n");

	CloseLibrary( DOSBase );
}

/*************************************************
*Func : Display the par: input
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{

	char FileName[300];
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
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	GetExtraData( ThisPI, FileName, &action );

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
				if( action==0 )
					load_file( FileName, MLMMULibBase );
				else
					unload_file( FileName, MLMMULibBase );
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
