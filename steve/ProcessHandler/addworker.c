/***********************************************************
*Desc : Add a new worker to the current guide list.
*/
#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

/*********************************************************************
*Func : Set up a new guide, this function is first called from the
*		ProcessController and may later be called by a ProcessGuide.
* <!> : This routine can never be called within a Parallel-ProcessGuide
*in   : MLSystem		-> ptr to MediaLink System struct
*		SegList			-> ptr to list of resident segments
*		PIList			-> List of guides/workers belonging to this guide	
*		NewObj	 		-> ptr to SNR of new child
*		Port_CtoP		-> Port belonging to the parent task which
*						   will be used by the child the send messages
*				  	       to its parent. 
*		RepP_WBStartup 	-> To which port should the new worker 
*					 	   send its WBStartup message when terminating
*		MainPath		-> ptr to full path name of main directory
*out  : ERR_NONE -> OK
*		else ErrorCode
*/
int AddWorker(MLMMULibBase, MLSystem, SegList, PIList, NewObj, Port_CtoP, RepP_WBStartup, MainPath)
struct Library *MLMMULibBase;
MLSYSTEM *MLSystem;
struct List *SegList;
struct List *PIList;
SNR *NewObj;
struct MsgPort *Port_CtoP;
struct MsgPort *RepP_WBStartup;
char *MainPath;
{
  ULONG StackSize;
  ULONG i;
  PROCESSINFO *NPI;			// New worker process	
  struct WBArg *WorkerArgs;	// Ptr to array of Arguments used by this new worker
							// This is only used to maintain some sort of compatibility
							// with Workbench process startups
  char WorkerLoadName[300];


	if(MLMMU_AvailMem(MEMF_PUBLIC) < 50000)	
	{
		if(MLMMU_PleaseFree(100000) < 50000)
			return(ERR_NOXAPPMEM);
	}

	// Allocate a ProcessInfo structure for this new worker
	if( (NPI = (PROCESSINFO *)MLMMU_AllocMem(
								sizeof(PROCESSINFO),MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_NOXAPPMEM);

	// Allocate argument list for the new worker
	NPI->pi_Startup.sm_NumArgs = 1;
	if( (WorkerArgs = (struct WBArg *)MLMMU_AllocMem(
								NPI->pi_Startup.sm_NumArgs * sizeof(struct WBArg),
								MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	{
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}	
	NPI->pi_Startup.sm_ArgList = WorkerArgs;

	switch(NewObj->nodeType)
	{
			case TALK_ANIM:
						strcpy(WorkerLoadName,"Animation");
						break;
			case TALK_AREXX:
						strcpy(WorkerLoadName,"ARexx");
						break;
			case TALK_DOS:
						strcpy(WorkerLoadName,"DOS");
						break;
			case TALK_VARS:
						strcpy(WorkerLoadName,"Variables");
						break;
			case TALK_PAGE:
						strcpy(WorkerLoadName,"Transitions");
						break;
			case TALK_SOUND:
						strcpy(WorkerLoadName,"Music");
						break;
			case TALK_USERAPPLIC:
						strcpy(WorkerLoadName,NewObj->objectPath);
						break;
	}

	//              WorkerLoadName      LevelStr  "."  (MAX stci_d size) + "\0"
	NPI->pi_SizeOfName = strlen(WorkerLoadName) + 1 + 7 + 1;

	NPI->pi_SizeOfName += 8;
	NPI->pi_SizeOfName &= 0xfffffff8;	// roundup at 8 bytes

	if( (NPI->pi_Name = (char *)MLMMU_AllocMem(NPI->pi_SizeOfName,MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	{
		MLMMU_FreeMem(WorkerArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);	
	}

	// Set up the name for this Worker
	strcpy(NPI->pi_Name,WorkerLoadName);			// start name with "Worker"
	strcat(NPI->pi_Name,".");
	stci_d(&NPI->pi_Name[strlen(NPI->pi_Name)],NewObj->PageNr);	// atach PageNr

	NPI->pi_Node.ln_Type = NT_WORKERPROC;	// Node type
	NPI->pi_Node.ln_Name = NPI->pi_Name;	// Our Name, if used by the List	
	NPI->pi_Node.ln_Pri = 0;				// Treat our children equal, give love,
											// cherish and nourish them.

	// Allocate memory for a messageport through which the parent can reach its child
	if( (NPI->pi_Port_PtoC = (struct MsgPort *)AllocMem(sizeof(struct MsgPort), 
												   MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
	{
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(WorkerArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);	
	}

	NPI->pi_Port_CtoP = Port_CtoP;
	NPI->pi_Unit = NewObj->PageNr;		// Not used by the system right now but may be 
										// used in future to see at which page we are
	
	// Load the Worker process into memory.
	if( (NPI->pi_Segment = MLLoadSeg(SegList, WorkerLoadName,&StackSize)) == NULL)
	{
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(WorkerArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_XAPPNOTFOUND);	
	}

	// allocate memory for the new child which its parent may use to send
	// process dialogues.
	if( (NPI->pi_PtoCDial[0] = (ULONG *)MLMMU_AllocMem(DIAL_MAXPTOC * sizeof(PROCDIALOGUE), 
												   MEMF_CLEAR|MEMF_PUBLIC,NULL)) == NULL)
	{
		MLUnLoadSeg(SegList, NPI->pi_Segment);
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(WorkerArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}
	((PROCDIALOGUE *)NPI->pi_PtoCDial[0])->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	((PROCDIALOGUE *)NPI->pi_PtoCDial[0])->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);

	for(i = 1; i < DIAL_MAXPTOC; i++)
	{
		NPI->pi_PtoCDial[i] = (ULONG *)((ULONG)NPI->pi_PtoCDial[i-1] + (ULONG)sizeof(PROCDIALOGUE));
		((PROCDIALOGUE *)NPI->pi_PtoCDial[i])->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		((PROCDIALOGUE *)NPI->pi_PtoCDial[i])->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	}

	// Start the new Workerprocess, it will automatically wait for a Message
	if( (NPI->pi_Process = (struct Process *) 
							((ULONG)CreateProc(NPI->pi_Name,0,NPI->pi_Segment,StackSize) - 
							 (ULONG)sizeof(struct Task))) == NULL)
	{
		MLMMU_FreeMem(NPI->pi_PtoCDial[0]);
		MLUnLoadSeg(SegList, NPI->pi_Segment);
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(WorkerArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}
	NPI->pi_Quiet = MLGetQuietSeg(SegList, NPI->pi_Segment);

	// Initialise the TaskMsgPort previously allocated and
	// assign a signal to it
	CreateTaskPort(NPI->pi_Process,NPI->pi_Port_PtoC,NPI->pi_Name);

	// Initialise a WBMessage like message to be send to the new WorkerProcess
	NPI->pi_Startup.sm_Process = &NPI->pi_Process->pr_MsgPort;
	NPI->pi_Startup.sm_Segment = NPI->pi_Segment;
	NPI->pi_Startup.sm_ToolWindow = NULL;
	NPI->pi_Startup.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	NPI->pi_Startup.sm_Message.mn_Length = sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD);
	NPI->pi_Startup.sm_Message.mn_ReplyPort = RepP_WBStartup;

	// initialise arguments for the new guide
    WorkerArgs[0].wa_Lock = (BPTR)Lock(MainPath,ACCESS_READ);
	WorkerArgs[0].wa_Name = NPI->pi_Name;

	NPI->pi_Arguments.ar_Worker.aw_NumArgs = 		NewObj->numericalArgs;
	NPI->pi_Arguments.ar_Worker.aw_Path = 			NewObj->objectPath;
	NPI->pi_Arguments.ar_Worker.aw_Name = 			NewObj->objectName;
	NPI->pi_Arguments.ar_Worker.aw_ExtraData = 		NewObj->extraData;
	NPI->pi_Arguments.ar_Worker.aw_ExtraDataSize =	&NewObj->extraDataSize;
	NPI->pi_Arguments.ar_Worker.aw_List =			NewObj->list;
	NPI->pi_Arguments.ar_Worker.aw_MLSystem = 		MLSystem;
	NPI->pi_Arguments.ar_Worker.aw_Origin = 		ORG_PROCHANDLER;

	// Add the new worker at the end of our ProcessInfoList 
	AddTail((struct List *)PIList,(struct Node *)NPI);	

	NPI->pi_State = ST_INIT;

	// make a cyclic ptr reference
	NewObj->ProcInfo = (ULONG *)NPI;
	NPI->pi_SNR = NewObj;

	// The new worker has been set up, send a message with information
	// and let it do its job
	PutMsg(NPI->pi_Startup.sm_Process,(struct Message *)&NPI->pi_Startup);
	return(NO_ERROR);
}

