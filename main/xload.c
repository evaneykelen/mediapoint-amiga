/**************************************************************
*Desc : process a script
* <!> : All this code is reentrant
*/

#include "pre.h"
#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include "nb:pre.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "ph:rexx_pragma.h"
#include "ph:rexx_proto.h"
#include "ph:rexx.h"
#include "ph:external.h"

// from file : xload.c
GLOBAL PROCESSINFO *AddXaPP(char *, struct ScriptNodeRecord *, struct MsgPort *);
GLOBAL void XIKillProcess( struct MsgPort *);
GLOBAL BOOL InitXaPP( char *, struct ScriptNodeRecord *, BOOL);

extern struct List *SegList;
extern MLSYSTEM	*MLSystem;				// MediaLink Global System structure
extern struct RendezVousRecord rvrec;
extern UBYTE **msgs;   
extern struct ObjectInfo ObjectRecord;

/****************************************************************
*Func : Add a new Xapp 
*in   : -
*out  : -
*/
PROCESSINFO *AddXaPP(WorkerLoadName, NewObj, RepP_WBStartup)
char *WorkerLoadName;
struct ScriptNodeRecord *NewObj;
struct MsgPort *RepP_WBStartup;
{
  ULONG StackSize;
  PROCESSINFO *NPI;			// New worker process	
  struct WBArg *WorkerArgs;	// Ptr to array of Arguments used by this new worker
							// This is only used to maintain some sort of compatibility
							// with Workbench process startups

	// Allocate a ProcessInfo structure for this new worker
	if( (NPI = (PROCESSINFO *)AllocMem(
								sizeof(PROCESSINFO),MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(NULL);

	// Allocate argument list for the new worker
	NPI->pi_Startup.sm_NumArgs = 1;
	if( (WorkerArgs = (struct WBArg *)AllocMem(
								NPI->pi_Startup.sm_NumArgs * sizeof(struct WBArg),
								MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		FreeMem(NPI, sizeof(PROCESSINFO));	
		return(NULL);
	}	
	NPI->pi_Startup.sm_ArgList = WorkerArgs;

	NPI->pi_SizeOfName = 32;

	if( (NPI->pi_Name = (char *)AllocMem(NPI->pi_SizeOfName,MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		FreeMem(WorkerArgs, NPI->pi_Startup.sm_NumArgs*sizeof(struct WBArg));	
		FreeMem(NPI, sizeof(PROCESSINFO));	
		return(NULL);
	}

	// Set up the name for this Worker
	strcpy(NPI->pi_Name,"Worker.1");			// start name with "Worker"

	NPI->pi_Node.ln_Type = NT_WORKERPROC;	// Node type
	NPI->pi_Node.ln_Name = NPI->pi_Name;	// Our Name, if used by the List		
	NPI->pi_Node.ln_Pri = 0;				// Treat our children equal, give love,
											// cherish and nourish them.
	
	// Load the Worker process into memory.
	if( (NPI->pi_Segment = MLLoadSeg(SegList,WorkerLoadName,&StackSize)) == NULL)
	{
		FreeMem(NPI->pi_Name, NPI->pi_SizeOfName);
		FreeMem(WorkerArgs, NPI->pi_Startup.sm_NumArgs*sizeof(struct WBArg));	
		FreeMem(NPI, sizeof(PROCESSINFO));	
		return(NULL);
	}

	// Start the new Workerprocess, it will automatically wait for a Message
	if( (NPI->pi_Process = (struct Process *) 
							((ULONG)CreateProc(NPI->pi_Name,0,NPI->pi_Segment,StackSize) - 
							 (ULONG)sizeof(struct Task))) == NULL)
	{
		MLUnLoadSeg(SegList,NPI->pi_Segment);
		FreeMem(NPI->pi_Name, NPI->pi_SizeOfName);
		FreeMem(WorkerArgs, NPI->pi_Startup.sm_NumArgs*sizeof(struct WBArg));	
		FreeMem(NPI, sizeof(PROCESSINFO));	
		return(NULL);
	}

	// Initialise a WBMessage like message to be send to the new WorkerProcess
	NPI->pi_Startup.sm_Process = &NPI->pi_Process->pr_MsgPort;
	NPI->pi_Startup.sm_Segment = NPI->pi_Segment;
	NPI->pi_Startup.sm_ToolWindow = NULL;
	NPI->pi_Startup.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	NPI->pi_Startup.sm_Message.mn_Length = sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD);
	NPI->pi_Startup.sm_Message.mn_ReplyPort = RepP_WBStartup;

	// initialise arguments for the new guide
	if( (WorkerArgs[0].wa_Lock = (BPTR)Lock("",ACCESS_READ)) == NULL)
		; //printf("Couldn't lock the main dir for this xapp\n");
	WorkerArgs[0].wa_Name = NPI->pi_Name;

	NPI->pi_Arguments.ar_Worker.aw_NumArgs = 	NewObj->numericalArgs;
	NPI->pi_Arguments.ar_Worker.aw_Path = 		NewObj->objectPath;
	NPI->pi_Arguments.ar_Worker.aw_Name = 		NewObj->objectName;
	NPI->pi_Arguments.ar_Worker.aw_ExtraData = 	NewObj->extraData;
	NPI->pi_Arguments.ar_Worker.aw_ExtraDataSize = &NewObj->extraDataSize;
	NPI->pi_Arguments.ar_Worker.aw_MLSystem = 	MLSystem;
	NPI->pi_Arguments.ar_Worker.aw_Origin = 	ORG_SCRIPTEDITOR;

	NPI->pi_State = ST_INIT;

	// make a cyclic ptr reference
	NewObj->ProcInfo = (ULONG *)NPI;
	NPI->pi_SNR = NewObj;

	// The new worker has been set up, send a message with information
	// and let it do its job
	PutMsg(NPI->pi_Startup.sm_Process,(struct Message *)&NPI->pi_Startup);
	return(NPI);
}

/*****************************************************
*Func : Kill child
*in   : RepP_WBStartup -> Port through which the child returns
*out  : -
*/
void XIKillProcess( RepP_WBStartup)
struct MsgPort *RepP_WBStartup;
{
  struct WBStartup *Msg_RWB;
  PROCESSINFO *CurPI;

	while( (Msg_RWB = (struct WBStartup *)GetMsg(RepP_WBStartup)) != NULL)
	{
		CurPI = (PROCESSINFO *)((ULONG)Msg_RWB - (sizeof(struct Node)) - sizeof(UWORD));

		CurPI->pi_SNR->ProcInfo = NULL;	// SNR no longer active
		MLUnLoadSeg(SegList,CurPI->pi_Segment);

		UnLock(CurPI->pi_Startup.sm_ArgList[0].wa_Lock);
		FreeMem(CurPI->pi_Startup.sm_ArgList, 
				CurPI->pi_Startup.sm_NumArgs*sizeof(struct WBArg));	
		FreeMem(CurPI->pi_Name, CurPI->pi_SizeOfName);
		FreeMem(CurPI, sizeof(PROCESSINFO));	
	}
}

/*************************************************
*Func : Load a xapp and have it initialised
*in   : XaPPName -> Name of this Xapp
*		ThisObj -> Ptr to SNR of this object
*out  : TRUE -> OK
*		FALSE -> error
*/
BOOL InitXaPP( XappName, ThisObj, tiny)
char *XappName;
struct ScriptNodeRecord *ThisObj;
BOOL tiny;
{
  PROCESSINFO *ThisPI;
  struct MsgPort *RepP_WBStartup;
  ULONG SigRecvd, SigR_WBStartup;
  BOOL TermOK;

	rvrec.msgs = msgs;   

	CreateUsedXappList(&(ObjectRecord.scriptSIR));

	LoadMLSegments(SegList,CPrefs.appdirLock, OTT_AFTERLOAD);

	if( (RepP_WBStartup = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		UnLoadMLSegments(SegList, OTT_AFTERLOAD);
		FreeUsedXappList();
		return(FALSE);
	}

	// Allocate the MediaLink global system structure	
	if((MLSystem =(MLSYSTEM *)AllocMem(sizeof(MLSYSTEM),MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		DeletePort(RepP_WBStartup);
		UnLoadMLSegments(SegList, OTT_AFTERLOAD);
		FreeUsedXappList();
		return(FALSE);
	}	

 	SigR_WBStartup = 1 << RepP_WBStartup->mp_SigBit;	// Reply from a child process

	if ( tiny )
	{
		if(TinyProcessInitializer() != NO_ERROR)
		{
			TinyProcessDeInitializer();
			FreeMem(MLSystem, sizeof(MLSYSTEM));
			DeletePort(RepP_WBStartup);
			UnLoadMLSegments(SegList, OTT_AFTERLOAD);
			FreeUsedXappList();
			return(FALSE);
		}
	}
	else
	{
		if(ProcessInitializer(&(ObjectRecord.scriptSIR),TRUE) != NO_ERROR)
		{
			ProcessDeInitializer();
			FreeMem(MLSystem, sizeof(MLSYSTEM));
			DeletePort(RepP_WBStartup);
			UnLoadMLSegments(SegList, OTT_AFTERLOAD);
			FreeUsedXappList();
			return(FALSE);
		}
	}

	if( (ThisPI = AddXaPP(XappName,ThisObj, RepP_WBStartup)) == NULL)
	{
		if ( tiny )
			TinyProcessDeInitializer();
		else
			ProcessDeInitializer();
		FreeMem(MLSystem, sizeof(MLSYSTEM));
		DeletePort(RepP_WBStartup);
		UnLoadMLSegments(SegList, OTT_AFTERLOAD);
		FreeUsedXappList();
		return(FALSE);
	}

	// wait for the worker to get ready
	SigRecvd = 0;
 	TermOK = FALSE;

	while(!TermOK)
	{
		SigRecvd = Wait(SigR_WBStartup);

		if(SigRecvd & SigR_WBStartup)
		{
			XIKillProcess(RepP_WBStartup);
			TermOK = TRUE;
		}
	}

	if ( tiny )
		TinyProcessDeInitializer();
	else
		ProcessDeInitializer();
	FreeMem(MLSystem, sizeof(MLSYSTEM));
	DeletePort(RepP_WBStartup);

	UnLoadMLSegments(SegList, OTT_AFTERLOAD);
	FreeUsedXappList();

	return(TRUE);
}
