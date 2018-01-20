/***********************************************************
*Desc : Add a new guide to the current process guide or 
*		process controller.
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
*in   : MLSystem		-> ptr to MediaLink system struct
*		SegList			-> ptr to list of resident segments
*		PIList			-> List of child guides/workers belonging to this guide	
*		SIR				-> ptr to overal system ScriptInfo
*		GuideType 		-> GT_SER or GT_PAR
*		Object 			-> ptr to the SNR for this process, NULL for masterguide
*		Port_CtoP		-> Port belonging to the parent task which
*						   will be used by the child the send messages
*				  	       to its parent. 
*		RepP_WBStartup 	-> To which port should the new guide 
*					 	   send its WBStartup message when terminating
*		MainPath		-> Full pathname of the main directory
*		PLL				-> PreLoadLevel
*out  : NO_ERROR -> OK
*		else ErrorCode
*/
int AddGuide(MLMMULibBase, MLSystem, SegList, PIList, SIR, GuideType, Object, Port_CtoP, RepP_WBStartup, MainPath, PLL)
struct Library *MLMMULibBase;
MLSYSTEM	*MLSystem;
struct List *SegList;
struct List *PIList;
struct ScriptInfoRecord *SIR;
int GuideType;
SNR *Object;
struct MsgPort *Port_CtoP;
struct MsgPort *RepP_WBStartup;
char *MainPath;
int PLL;
{
  ULONG StackSize;
  char GuideName[200];
  ULONG i;
  PROCESSINFO *NPI;			// New guide process	
  struct WBArg *GuideArgs;	// Ptr to array of Arguments used by this new guide
							// We only setup an argument list to maintain some sort
							// of compatibility with Workbench processes

	if(MLMMU_AvailMem(MEMF_PUBLIC) < 50000)	
	{
		if(MLMMU_PleaseFree(100000) == 0)
			return(ERR_NOXAPPMEM);
	}

	// Allocate a ProcessInfo structure for this new guide
	if( (NPI = (PROCESSINFO *)MLMMU_AllocMem(
								sizeof(PROCESSINFO),MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_NOXAPPMEM);

	// Allocate argument list for the new guide
	NPI->pi_Startup.sm_NumArgs = 1;
	if( (GuideArgs = (struct WBArg *)MLMMU_AllocMem(
								NPI->pi_Startup.sm_NumArgs * sizeof(struct WBArg),
								MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	{
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}	
	NPI->pi_Startup.sm_ArgList = GuideArgs;

	if(GuideType == GT_SER)
		strcpy(GuideName,"Serial");
	else
		strcpy(GuideName,"Parallel");

	//              Guidename  "."  (MAX stci_d size) + "\0"
	NPI->pi_SizeOfName = strlen(GuideName) + 1 + 7 + 1;

	NPI->pi_SizeOfName += 8;
	NPI->pi_SizeOfName &= 0xfffffff8;	// roundup at 8 bytes

	if( (NPI->pi_Name = (char *)MLMMU_AllocMem(NPI->pi_SizeOfName,MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	{
		MLMMU_FreeMem(GuideArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);	
	}

	// Set up the name for this Guide
	strcpy(NPI->pi_Name,GuideName);			// start name with "Guide"
	strcat(NPI->pi_Name,".");
	stci_d(&NPI->pi_Name[strlen(NPI->pi_Name)],Object->PageNr);	// atach PageNr

	if(GuideType == GT_SER)
		NPI->pi_Node.ln_Type = NT_SERGUIDEPROC;	// serial guide
	else
		NPI->pi_Node.ln_Type = NT_PARGUIDEPROC;	// parallel guide

	NPI->pi_Node.ln_Name = NPI->pi_Name;	// Our Name, if used by the List
	NPI->pi_Node.ln_Pri = 0;				// Treat our children equal, give love,
											// cherish and nourish them.

	// Allocate memory for a messageport through which the parent can reach its child
	if( (NPI->pi_Port_PtoC = (struct MsgPort *)AllocMem(sizeof(struct MsgPort), 
												   MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
	{
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(GuideArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);	
	}

	NPI->pi_Port_CtoP = Port_CtoP;
	NPI->pi_Unit = Object->PageNr;
	
	// Load the standard Guide process into memory
	if( (NPI->pi_Segment = MLLoadSeg(SegList,GuideName,&StackSize)) == NULL)
	{
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(GuideArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_XAPPNOTFOUND);	
	}

	// allocate memory for the new child which its parent may use to send
	// process dialogues
	if( (NPI->pi_PtoCDial[0] = (ULONG *)MLMMU_AllocMem(DIAL_MAXPTOC * sizeof(PROCDIALOGUE), 
												   MEMF_CLEAR|MEMF_PUBLIC,NULL)) == NULL)
	{
		MLUnLoadSeg(SegList, NPI->pi_Segment);
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(GuideArgs);
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

	// Start the new Guideprocess, it will automatically wait for a Message
	if( (NPI->pi_Process = (struct Process *) 
							((ULONG)CreateProc(NPI->pi_Name,0,NPI->pi_Segment,StackSize) - 
							 (ULONG)sizeof(struct Task))) == NULL)
	{
		MLMMU_FreeMem(NPI->pi_PtoCDial[0]);
		MLUnLoadSeg(SegList, NPI->pi_Segment);
		FreeMem(NPI->pi_Port_PtoC,sizeof(struct MsgPort));
		MLMMU_FreeMem(NPI->pi_Name);
		MLMMU_FreeMem(GuideArgs);
		MLMMU_FreeMem(NPI);
		return(ERR_NOXAPPMEM);
	}
	NPI->pi_Quiet = MLGetQuietSeg(SegList, NPI->pi_Segment);
	//printf("Quiet = %s\n",NPI->pi_Quiet ? "YES":"NO");

	// Initialise the TaskMsgPort previously allocated and
	// assign a signal to it
	CreateTaskPort((struct Task *)NPI->pi_Process,NPI->pi_Port_PtoC,NPI->pi_Name);

	// Initialise a WBMessage like message to be send to the new GuideProcess
	NPI->pi_Startup.sm_Process = &NPI->pi_Process->pr_MsgPort;
	NPI->pi_Startup.sm_Segment = NPI->pi_Segment;
	NPI->pi_Startup.sm_ToolWindow = NULL;
	NPI->pi_Startup.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
	NPI->pi_Startup.sm_Message.mn_Length = sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD);
	NPI->pi_Startup.sm_Message.mn_ReplyPort = RepP_WBStartup;

	// initialise arguments for the new guide
    GuideArgs[0].wa_Lock = (BPTR)Lock(MainPath,ACCESS_READ);
	GuideArgs[0].wa_Name = NPI->pi_Name;

	NPI->pi_Arguments.ar_Guide.ag_SegList = SegList;
	NPI->pi_Arguments.ar_Guide.ag_SIR = SIR;
	NPI->pi_Arguments.ar_Guide.ag_MainDirPath = MainPath;
	NPI->pi_Arguments.ar_Guide.ag_PLL = PLL;
	NPI->pi_Arguments.ar_Guide.ag_MLSystem = MLSystem;

	// Add the new pupil-guide at the end of our ProcessInfoList 
	AddTail((struct List *)PIList,(struct Node *)NPI);	

	NPI->pi_State = ST_INIT;

	Object->ProcInfo = (ULONG *)NPI;
	NPI->pi_SNR = Object;

	// The new guide has been set up, send a message with information
	// and let it do its job
	PutMsg(NPI->pi_Startup.sm_Process,(struct Message *)&NPI->pi_Startup);

	if(NPI->pi_Node.ln_Type == NT_SERGUIDEPROC)
	{
		;	//PrintSer("PlayTrak %x, LoadTrack %x\n",Object->PlayTrackSNR,Object->LoadTrackSNR);
	}

	return(NO_ERROR);
}
