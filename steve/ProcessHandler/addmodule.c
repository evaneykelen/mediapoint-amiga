/***********************************************************
*Desc : Run a module (synchronizer, geventprocessor etc)
*/
#include <workbench/startup.h>
#include <exec/types.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <libraries/dosextens.h>

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

#include "rexx_pragma.h"
#include "rexx_proto.h"
#include "rexx.h"
#include "external.h"

/*********************************************************************
*Func : Set up a new module
*in   : SegList			-> ptr to list of resident segments
*		PIList			-> ptr to list of PROCESSINFO struct
*		ModName			-> ptr to modulename		
*		ScriptInfo;		-> ptr to ScriptInfo record
*		RepP_WBStartup 	-> To which port should the module
*					 	   send its WBStartup message when terminating
*		MainPath		-> Ptr to full main path  
*		NewPI			-> ptr to ptr of just added PI
*		MLSystem		-> ptr to system structure
*out  : ERR_NONE -> OK
*		else ErrorCode
*/
int AddModule( SegList, PIList, ModName, ScriptInfo, RepP_WBStartup, MainPath, NewPI, MLSystem)
struct List *SegList;
struct List *PIList;
char *ModName;
struct ScriptInfoRecord *ScriptInfo;
struct MsgPort *RepP_WBStartup;
char *MainPath;
PROCESSINFO **NewPI;
MLSYSTEM *MLSystem;
{
  ULONG StackSize;
  PROCESSINFO *NPI;			// New module process	
  struct WBArg *ModArgs;	// Ptr to array of Arguments used by this module
							// We only setup an argument list to maintain some sort
							// of compatibility with Workbench processes

	//printf("Add module, MLSystem at %x\n",(int)MLSystem);

	if(NewPI != NULL)
		*NewPI = NULL;

	if(AvailMem(MEMF_PUBLIC) < 50000)	
	{
//KPrintF("NO MEM FOR AddModule\n");
		return(ERR_NOXAPPMEM);
	}

	// Allocate a ProcessInfo structure for this module
	if( (NPI = (PROCESSINFO *)MLMMU_AllocMem(
								sizeof(PROCESSINFO),MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_NOXAPPMEM);

	// Allocate argument list for the module 
	NPI->pi_Startup.sm_NumArgs = 1;
	if( (ModArgs = (struct WBArg *)MLMMU_AllocMem(
								NPI->pi_Startup.sm_NumArgs * sizeof(struct WBArg),
								MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	{
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}	
	NPI->pi_Startup.sm_ArgList = ModArgs;

	NPI->pi_Name = ModName;
	NPI->pi_SizeOfName = 0;					// not an allocated name array

	NPI->pi_Node.ln_Type = NT_MODULE;

	NPI->pi_Node.ln_Name = NPI->pi_Name;	// Our Name, if used by the List		
	NPI->pi_Node.ln_Pri = 0;		

	// Load the standard module into memory
	if( (NPI->pi_Segment = MLLoadSeg(SegList,ModName,&StackSize)) == NULL)
	{
		MLMMU_FreeMem(ModArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_XAPPNOTFOUND);
	}

 	// Start the new Guideprocess
	if( (NPI->pi_Process = (struct Process *) 
							((ULONG)CreateProc(NPI->pi_Name,0,NPI->pi_Segment,StackSize) - 
							 (ULONG)sizeof(struct Task))) == NULL)
	{
		MLUnLoadSeg(SegList, NPI->pi_Segment);
		MLMMU_FreeMem(ModArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}

	// Initialise a WBMessage like message to be send to the new GuideProcess
	NPI->pi_Startup.sm_Process = &NPI->pi_Process->pr_MsgPort;
	NPI->pi_Startup.sm_Segment = NPI->pi_Segment;
	NPI->pi_Startup.sm_ToolWindow = NULL;
	NPI->pi_Startup.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	NPI->pi_Startup.sm_Message.mn_Length = sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD);
	NPI->pi_Startup.sm_Message.mn_ReplyPort = RepP_WBStartup;

	// initialise arguments for the new module
    ModArgs[0].wa_Lock = (BPTR)Lock(MainPath,ACCESS_READ);
	ModArgs[0].wa_Name = NPI->pi_Name;

	NPI->pi_Arguments.ar_Module.am_SIR = ScriptInfo;
 	NPI->pi_Arguments.ar_Module.am_TempoType = TempoType;
 	NPI->pi_Arguments.ar_Module.am_MLSystem = MLSystem;

	// Add the new module at the end of our ProcessInfoList 
	AddTail((struct List *)PIList,(struct Node *)NPI);	

	NPI->pi_State = ST_INIT;
	if(NewPI != NULL)
		*NewPI = NPI;

	// The new guide has been set up, send a message with information
	// and let it do its job
	PutMsg(NPI->pi_Startup.sm_Process,(struct Message *)&NPI->pi_Startup);
	return(NO_ERROR);
}

