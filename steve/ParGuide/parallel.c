/**************************************************************
*Desc : process a script
* <!> : All this code is reentrant
*/
#include "nb:pre.h"
#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define VERSI0N "\0$VER: 1.2"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE

/***********************************************************************
* All Variables have been declared within a structure to establish
* re-entrant coding
*/

struct MemVar
{
  struct Library	*MLMMULibBase;
  MLSYSTEM			*MLSystem;
  struct ScriptInfoRecord *SIR;
  struct List		*SegList;	// Ptr to global list of resident worker/guide segments 
  struct List 		*PIList;	// List of guides/workers belonging to this guide	
  struct List		*SNRList;

  PROCESSINFO *ThisPI;			// Ptr to this processinfo blk (as used in our parent's list)

  struct WBStartup *Msg_RWB;	// Replymessage from the child after termination  
  struct MsgPort *RepP_WBStartup;// Reply port used when a child terminates
								// There is one port for all children
  struct MsgPort *Port_CtoP;	// Port for this task which is used by
								// the child to send messages to this parent
								// There is one port for all children 	
  struct MsgPort *RepP_Guide;	// Reply port for both child and parent
								// when they receive a message from this guide
  SNR *CurSNR;

  // Global, this number is used by Addworker()/AddGuide() to pass on a process number
  // These values are increamented by PreloadObj()
  int CurWorkNr;					// Process number of next Worker

  struct WBStartup 	*Msg_RepWB;		// Reply message from process
  PROCDIALOGUE 		*Msg_SerDial[DIAL_MAXSER];	
									// Dialogue used to communicate with parent
  PROCDIALOGUE		*Msg_RProcDial;	// A message from either a child or our parent
  ULONG Sig_PtoC,					// Signal, parent to child
	    Sig_CtoP,					// Signal, child to parent (us)
        SigR_Guide,					// Signal, a reply on a msg we've send to our parent/child
	    SigR_WBStartup,				// Signal, a child commited suicide
		Sig_Forced,
	    SigRecvd;					// Signals received

  int PLL;
  BOOL B_TermOK;					// If TRUE, we may terminate ourselves
 																			
  int  SystemError;			
  PROCESSINFO *CurPI;				// Free to be used as temporarily PI ptr
};

#define MLMMULibBase		MyMem->MLMMULibBase
#define MLSystem			MyMem->MLSystem
#define SIR					MyMem->SIR
#define SPList				MyMem->SPList
#define	SegList 			MyMem->SegList
#define	PIList 				MyMem->PIList
#define	SNRList 			MyMem->SNRList
#define Msg_SerDial 		MyMem->Msg_SerDial
#define ThisPI 				MyMem->ThisPI
#define Msg_RWB 			MyMem->Msg_RWB
#define RepP_WBStartup		MyMem->RepP_WBStartup
#define Port_CtoP			MyMem->Port_CtoP
#define RepP_Guide			MyMem->RepP_Guide
#define	CurSNR				MyMem->CurSNR
#define CurWorkNr			MyMem->CurWorkNr
#define Msg_RepWB			MyMem->Msg_RepWB
#define Msg_RProcDial		MyMem->Msg_RProcDial
#define Sig_PtoC			MyMem->Sig_PtoC
#define Sig_CtoP			MyMem->Sig_CtoP
#define SigR_Guide			MyMem->SigR_Guide
#define SigR_WBStartup		MyMem->SigR_WBStartup
#define Sig_Forced			MyMem->Sig_Forced
#define SigRecvd			MyMem->SigRecvd
#define PLL					MyMem->PLL
#define B_TermOK			MyMem->B_TermOK
#define SystemError			MyMem->SystemError
#define CurPI				MyMem->CurPI

/*************************************************
*Func : Free all global structures used by this guide
*in   : -
*out  : -
*/
void FreeParGuide( MyMem, Error)
struct MemVar *MyMem;
int Error;
{
	if(PIList)
		FreeMem(PIList,sizeof(struct List));
	if(Msg_SerDial[0])
		FreeMem(Msg_SerDial[0], DIAL_MAXSER * sizeof(PROCDIALOGUE));
	if(RepP_Guide)
		DeletePort(RepP_Guide);
	if(Port_CtoP)
		DeletePort(Port_CtoP);
	if(RepP_WBStartup)
		DeletePort(RepP_WBStartup);
	if(MLMMULibBase)
		CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = Error;
}

/******************************************************
*Func : Initialize the parallel guide process structures
*		All structures are Global 
*in   : -
*out  : TRUE -> ok
*		FALSE -> Error in ThisPI->pi_Arguments.ar_RetErr
*/
int InitParGuide( MyMem)
struct MemVar *MyMem;
{
  int i;

	if( (MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return(ERR_NOMEM);

	// Make a ReplyPort for our children, used only to find out a guide/worker
	// has terminated. It needs not to be public since we will pass the Ptr on to 
    // the children. (Done by AddGuide())
	if( (RepP_WBStartup = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_NOMEM);

	// Port belonging to this guide. The child may send
	// messages through this port to reach its guide
	if( (Port_CtoP = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_NOMEM);

	// When this guide has sent a message to a child or parent, the child/parent may
	// reply through this port
	if( (RepP_Guide = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_NOMEM);

	// Make a Dialogue to communicate with either our parent or our children
	if( (Msg_SerDial[0] = (PROCDIALOGUE *)AllocMem(DIAL_MAXSER * sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(ERR_NOMEM);
	for(i = 1; i < DIAL_MAXSER; i++)
		Msg_SerDial[i] = (PROCDIALOGUE *)((ULONG)Msg_SerDial[i-1] + (ULONG)sizeof(PROCDIALOGUE));

	// Set up a List for ProcessInfo structures
	if( (PIList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(ERR_NOMEM);
 	NewList(PIList);

	return(NO_ERROR);
}

/*****************************************************
*Func : Find out if a child is still a member of our 
*		PIList
*in   : CheckPI-> ChildPI
*out  : TRUE -> child is still a member
*		FALSE -> Child is no member
*/
BOOL FindChild( MyMem, CheckPI)
struct MemVar *MyMem;
PROCESSINFO *CheckPI;
{
  PROCESSINFO *PI;

	// Send a command message to all our children
	// For us this means, send it to our child-guide
	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
		if(CheckPI == PI)
			return(TRUE);

	return(FALSE);
}

/****************************************************
*Func : Find the next free process dialogue for
*		sending a command to the child
*in   : PI -> PI of child
*out  : Ptr to free dialogue 
*		NULL -> no dialogue available
*/
PROCDIALOGUE *GetFreeChildDial( PI)
PROCESSINFO *PI;
{
  int i;

	for(i = 0; i < DIAL_MAXPTOC; i++)
		if(!((PROCDIALOGUE *)PI->pi_PtoCDial[i])->pd_InUse)
		{	
			((PROCDIALOGUE *)PI->pi_PtoCDial[i])->pd_InUse = TRUE;
			return( (PROCDIALOGUE *)PI->pi_PtoCDial[i]);
		}
	return(NULL);
}

/****************************************************
*Func : Find the next free Proc dialogue for
*		sending a command to the parent or process controller
*in   : -
*out  : Ptr to free dialogue 
*		NULL -> no dialogue available
*/
PROCDIALOGUE *GetFreeSerDial( MyMem)
struct MemVar *MyMem;
{
  int i;

	for(i = 0; i < DIAL_MAXSER; i++)
		if(!Msg_SerDial[i]->pd_InUse)
		{	
			Msg_SerDial[i]->pd_InUse = TRUE;
			return( Msg_SerDial[i]);
		}
	return(NULL);
}

/*********************************************************
*Func : Find out if a PI is in the PILIst
*in   : PIL -> Ptr to PILIst
*		CheckPI -> Ptr to ProcessInfo
*out  : TRUE -> PI in list
*		FALSE -> PI not in List
*/
BOOL FindPI( PIL, CheckPI)
struct List *PIL;
PROCESSINFO *CheckPI;
{
  PROCESSINFO *PI;

	for(PI = (PROCESSINFO *)PIL->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
		if(PI == CheckPI)
			return(TRUE);

	return(FALSE);
}

/***************************************************
*Func : Send a message to the PC
*in   : Cmd -> Cmd to be send to the PC
*out  : -
*/
void TalkToPC( MyMem, Cmd)
struct MemVar *MyMem;
int Cmd;
{
  PROCDIALOGUE *PD;	

	// Send a message to the Process Controller to indicate we are ready to start
	PD = GetFreeSerDial( MyMem);
	PD->pd_Cmd = Cmd;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)PD);
}


/*****************************************************
*Func : Find all children that just killed themselves
*		and remove them from the PI list
* <!> : All vars have been declared globally
*in   : -
*out  : -
*/
void KillProcess( MyMem)
struct MemVar *MyMem;
{
	// A child commited suicide
	// Remove its corps and send it to the graveyard
	// Get the messages from the childprocesses that just terminated itself
	while( (Msg_RWB = (struct WBStartup *)GetMsg(RepP_WBStartup)) != NULL)
	{
		CurPI = (PROCESSINFO *)((ULONG)Msg_RWB - (sizeof(struct Node)) - sizeof(UWORD));
#if _PRINTF
		printf("%s> removing [%s], error %d\n\r",ThisPI->pi_Name,CurPI->pi_Name, CurPI->pi_Arguments.ar_RetErr);
#endif
		Remove((struct Node *)CurPI);
		CurPI->pi_SNR->ProcInfo = NULL;	// SNR no longer active
		MLUnLoadSeg(SegList, CurPI->pi_Segment);
		UnLock(CurPI->pi_Startup.sm_ArgList[0].wa_Lock);

		DeleteTaskPort(CurPI->pi_Port_PtoC);
		MLMMU_FreeMem(CurPI->pi_PtoCDial[0]);
		MLMMU_FreeMem(CurPI->pi_Startup.sm_ArgList);
		MLMMU_FreeMem(CurPI->pi_Name);
		MLMMU_FreeMem(CurPI);
	}
}


/**********************************************************
*Func : A child has send a dialogue to us
*in   : -
*out  : -
*/
void ChildTalk( MyMem)
struct MemVar *MyMem;
{
	// Get the message from each child
	while( (Msg_RProcDial = (PROCDIALOGUE *)GetMsg(Port_CtoP)) != NULL)
	{	
#if _PRINTF
		printf("ChildTalk cmd %d from %s",Msg_RProcDial->pd_Cmd,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif

		if(FindChild(MyMem,Msg_RProcDial->pd_ChildPI))
		{
			switch(Msg_RProcDial->pd_Cmd)
			{
				case DCI_CHILDREADY:
#if _PRINTF
							printf("%s> Received DCI_CHILDREADY from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							// Our child is ready to go.
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
							Msg_RProcDial->pd_ChildPI->pi_State = ST_READY;
							break;
				case DCI_CHILDTERM:
#if _PRINTF
							printf("%s> Received DCI_CHILDTERM from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							// Our child want to commit suicide
							Msg_RProcDial->pd_Cmd = DCC_DOTERM;
							Msg_RProcDial->pd_ChildPI->pi_State = ST_TERM;
							// The child is free to let its spirit fly
							break;
			case DCI_SEVEREPROBLEM:
#if _PRINTF
							printf("PC> received DCI_SEVEREPROBLEM from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
							TalkToPC(MyMem,DCI_SEVEREPROBLEM);
							break;
				default: 
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
#if _PRINTF
							printf("%s> Received unknown request from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							break;
			}

			Msg_RProcDial->pd_ChildPI = NULL;			
			ReplyMsg((struct Message *)Msg_RProcDial);
		}
#if _PRINTF
		else
			printf("%s> Child doesn't exist anymore\n\r",ThisPI->pi_Name);
#endif
	}
}


/***********************************************************
*Func : Send A command to all our children
*in   : MyMem
*	    Cmd -> Command to be send
*out  : -
*/
void CmdChildren( MyMem, Cmd)
struct MemVar *MyMem;
int Cmd;
{
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;

	// Send a command message to all our children
	// For us this means, send it to our child-guide
	for(PI = (PROCESSINFO *)PIList->lh_Head;
		(PROCESSINFO *)PI->pi_Node.ln_Succ;
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		// send msg only to PI's that are not ready to commit suicide
		if(PI->pi_State != ST_TERM)
		{ 	
			if(PD = GetFreeChildDial(PI))
			{
				PD->pd_ChildPI = NULL;
				PD->pd_Cmd = Cmd;
				PD->pd_Msg.mn_ReplyPort = RepP_Guide;
				PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
				printf("%s> Sending cmd to child %s\n\r",ThisPI->pi_Name,PI->pi_Name);
#endif
			}
		}
	}
}

/**********************************************************
*Func : Our guide has something to say to us
*in   : -
*out  : -
*/
void ParentTalk( MyMem)
struct MemVar *MyMem;
{
  int Err;
  BOOL B_Reply;

	// The ProcContr has something to say to us
	while( (Msg_RProcDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
	{
		B_Reply = TRUE;
		Msg_RProcDial->pd_ChildPI = ThisPI;
		switch(Msg_RProcDial->pd_Cmd)
		{
			// Following cases are for this guide only
			case DCC_DOTERM:
#if _PRINTF
						printf("%s> Received DCC_DOTERM from guide\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDTERM;
						CmdChildren(MyMem, DCC_DOTERM);
						B_TermOK = TRUE;
						break;
			case DCC_DOCLEANUP:
#if _PRINTF
						printf("%s> Received DCC_DOCLEANUP from PC\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDCLEANUP;
						if(!B_TermOK)
							CmdChildren(MyMem, DCC_DOTERM);
						break;
			case DCC_DOSTOP:
#if _PRINTF
						printf("%s> Received DCC_DOSTOP from guide\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDREADY;
						CmdChildren(MyMem, DCC_DOSTOP);
						break;
			case DCC_DOHOLD:
#if _PRINTF
						printf("%s> Received DCC_DOHOLD from guide\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDHOLDS;
						CmdChildren(MyMem, DCC_DOHOLD);
						break;
			case DCC_DORUN:
#if _PRINTF
						printf("%s> Ignoring DCC_DORUN from guide\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDHOLDS;
						break;
			case DCC_DOPREPARE:
#if _PRINTF
						printf("%s> Received DCC_DOPREPARE from guide\n\r",ThisPI->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDREADY;
						if( (Err = PrepareChildren(MyMem)) != NO_ERROR)
						{
							SystemError = Err;
							Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
						}
						break;
			case DCC_DORUNOBJ:
#if _PRINTF
						printf("%s> Received DCC_DORUNOBJ for %s from guide\n\r",ThisPI->pi_Name,((PROCESSINFO *)Msg_RProcDial->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_OBJRUNS;
						RunObject(MyMem,Msg_RProcDial->pd_Luggage.lu_SNR);
						break;
			case DCC_DOHOLDOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOHOLDOBJ for %s from guide\n\r",ThisPI->pi_Name,((PROCESSINFO *)Msg_RProcDial->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_OBJHOLDS;
						HoldObject(MyMem,Msg_RProcDial->pd_Luggage.lu_SNR);
						break;
			case DCC_DOTERMOBJ:
			case DCC_DOSTOPOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOTERM/STOPOBJ for %s from guide\n\r",ThisPI->pi_Name,((PROCESSINFO *)Msg_RProcDial->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_OBJTERMS;

						TermObject(MyMem,Msg_RProcDial->pd_Luggage.lu_SNR,NULL);
						break;
			case DCC_DOEASYTERMOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOEASYTERMOBJ for %s from guide\n\r",ThisPI->pi_Name,((PROCESSINFO *)Msg_RProcDial->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDEASYTERMOBJ;

						B_Reply = TermObject(MyMem,Msg_RProcDial->pd_Luggage.lu_SNR,Msg_RProcDial);
						break;
			case DCC_DOEASYSTOPOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOEASYSTOPOBJ for %s from guide\n\r",ThisPI->pi_Name,((PROCESSINFO *)Msg_RProcDial->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						Msg_RProcDial->pd_Cmd = DCI_CHILDEASYSTOPOBJ;

						B_Reply = TermObject(MyMem,Msg_RProcDial->pd_Luggage.lu_SNR,Msg_RProcDial);
						break;
			default:	
						Msg_RProcDial->pd_Cmd = DCI_IGNORE;
#if _PRINTF
						printf("%s> Received unknown command from PC\n\r",ThisPI->pi_Name);
#endif
						break;
		}
		if(B_Reply)
			ReplyMsg((struct Message *)Msg_RProcDial);
	}
}

/*****************************************************************************
*Func : A reply on a message send to our guide or child has been received
* <!> : DCC replies are always send by our parent with pd_ChildPI set to NULL
*		DCI info replies are always send by children with pd_ChildPI set
*in   : -
*out  : -
*/	
void GetReply( MyMem)
struct MemVar *MyMem;
{
	while( (Msg_RProcDial = (PROCDIALOGUE *)GetMsg(RepP_Guide)) != NULL)
	{
		switch(Msg_RProcDial->pd_Cmd)
		{
			case DCC_IGNORE:		// mostly received after sending a DCI_CHILDREADY
#if _PRINTF
						printf("%s> Received DCC_IGNORE reply from Parent\n",ThisPI->pi_Name);
#endif
						break;
			case DCC_DOTERM:		// received after sending a DCI_CHILDTERM request
#if _PRINTF
						printf("%s> Received DCC_DOTERM reply from Parent\n",ThisPI->pi_Name);
#endif
						B_TermOK = TRUE;
						break;
			case DCI_IGNORE:	// a child didn't support the last message
#if _PRINTF
						printf("%s> Received DCI_IGNORE reply from [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;	// probably a continues aPP with no DOHOLD support
			case DCI_CHILDRUNS:	// reply on sending DCC_DORUN
						Msg_RProcDial->pd_ChildPI->pi_State = ST_RUNNING;
#if _PRINTF
						printf("%s> Received DCI_CHILDRUNS reply from [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDHOLDS:// reply on sending DCC_DOHOLD
						Msg_RProcDial->pd_ChildPI->pi_State = ST_HOLD;
#if _PRINTF
						printf("%s> Received DCI_CHILDHOLDS reply from [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDTERM: // reply on sending DCC_DOTERM to child
						Msg_RProcDial->pd_ChildPI->pi_State = ST_TERM;
#if _PRINTF
						printf("%s> Received DCI_CHILDTERM at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)Msg_RProcDial,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						Msg_RProcDial->pd_InUse = FALSE;
						Signal(&Msg_RProcDial->pd_ChildPI->pi_Process->pr_Task, SIGF_ABORT);
						break;
			case DCI_CHILDPREPARES: // reply on sending DCC_DOPREPARE to children
						Msg_RProcDial->pd_ChildPI->pi_State = ST_INIT;
#if _PRINTF
						printf("%s> Received DCI_CHILDPREPARES reply from [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDEASYTERM: // reply on sending DCC_DOEASYTERM to child
						Msg_RProcDial->pd_ChildPI->pi_State = ST_TERM;
#if _PRINTF
						printf("%s> Received DCI_EASYCHILDTERM at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)Msg_RProcDial,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						Msg_RProcDial->pd_InUse = FALSE;

						if(
							(Msg_RProcDial->pd_Luggage.lu_Dial != NULL) && 
							(((PROCDIALOGUE *)Msg_RProcDial->pd_Luggage.lu_Dial)->pd_Msg.mn_Node.ln_Type == NT_MESSAGE)
						  )
						{
#if _PRINTF
							printf("Returning original dialogue to PC\n");
#endif
							ReplyMsg(Msg_RProcDial->pd_Luggage.lu_Dial);
						}	

						Signal(&Msg_RProcDial->pd_ChildPI->pi_Process->pr_Task, SIGF_ABORT);
						break;
			case DCI_CHILDEASYSTOP: // reply on sending DCC_DOEASYSTOP to child
						Msg_RProcDial->pd_ChildPI->pi_State = ST_READY;
#if _PRINTF
						printf("%s> Received DCI_EASYCHILDSTOP at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)Msg_RProcDial,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						if(
							(Msg_RProcDial->pd_Luggage.lu_Dial != NULL) && 
							(((PROCDIALOGUE *)Msg_RProcDial->pd_Luggage.lu_Dial)->pd_Msg.mn_Node.ln_Type == NT_MESSAGE)
						  )
						{
#if _PRINTF
							printf("Returning original dialogue to PC\n");
#endif
							ReplyMsg(Msg_RProcDial->pd_Luggage.lu_Dial);
						}	
						break;
			default:
#if _PRINTF
						printf("%s> Received unknown reply from [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
		}
		Msg_RProcDial->pd_InUse = FALSE;
	}
	// no reply since this IS the reply 
}



/********************************************************
*Func : Load a new object into memory
*in   : Object -> SNR of object to be loaded
*out  : Error
*/
int LoadObject( MyMem, Object)
struct MemVar *MyMem;
SNR *Object;
{
  int Err;
  ULONG SigRec;
  PROCESSINFO *PI;

	/****** THIS IS NEW *********/

	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		if( PI->pi_SNR == Object )
		{
#if _PRINTF
			printf("Object already loaded, returning at once\n");
#endif
			return(0);
		}
	}

	/****** THIS IS NEW *********/

	Err = AddWorker(MLMMULibBase,MLSystem, SegList, PIList, Object, Port_CtoP, RepP_WBStartup,
					ThisPI->pi_Arguments.ar_Guide.ag_MainDirPath);

#if _PRINTF
	printf("> AddWorker result code = %d\n\r",Err);
#endif
	if(Err != NO_ERROR)
		return(Err);
		
	SigRec = Wait(Sig_CtoP|SIGF_ABORT);
	// Receive information from our children
	if(SigRec & Sig_CtoP)
		ChildTalk(MyMem);
	Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
	return(NO_ERROR);
}

/*********************************************************
*Func : Terminate a object
*in   : Object -> Object to be terminated
*out  : -
*/
BOOL TermObject( MyMem, Object, RepDial)
struct MemVar *MyMem;
SNR *Object;
PROCDIALOGUE *RepDial;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;
  BOOL B_Reply;

	PI = (PROCESSINFO *)Object->ProcInfo;
	if(PI == NULL)
		return(FALSE);

	// if the current active object is not terminating itself right now
	// then make it terminate itself.
#if _PRINTF
	printf("> TermObject, state of %s is %d\n\r",PI->pi_Name,PI->pi_State);
#endif
	B_Reply = TRUE;
	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Luggage.lu_Dial = RepDial;

			// Preload object must not be terminated but just stop with what they are doing
			if(RepDial == NULL)
				PD->pd_Cmd = PI->pi_Preload ? DCC_DOSTOP: DCC_DOTERM;
			else
			{
				B_Reply = FALSE;
				PD->pd_Cmd = PI->pi_Preload ? DCC_DOEASYSTOP: DCC_DOEASYTERM;
			}
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;

			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> TermObj, sending DCC_DOTERM %d to child %s\n\r",PD->pd_Cmd,PI->pi_Name);
#endif
		}
	}
	return(B_Reply);
}

/*********************************************************
*Func : Hold a object as in Obj
*in   : Object -> Object to be Hold
*out  : -
*/
void HoldObject( MyMem, Object)
struct MemVar *MyMem;
SNR *Object;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;

	PI = (PROCESSINFO *)Object->ProcInfo;
	if(PI == NULL)
		return;

	// if the current active object is not terminating itself right now
	// then make it hold
#if _PRINTF
	printf("> HoldObject, state of %s is %d\n\r",PI->pi_Name,PI->pi_State);
#endif

	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Cmd = DCC_DOHOLD;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;
	
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> HoldObj, sending DCC_DOTERM to child %s\n\r",PI->pi_Name);
#endif
		}
	}
}

/*********************************************************
*Func : Run the object as in Obj
*		For each object run, a new object may be preloaded		
*in   : Object -> Object to be run
*out  : -
*/
void RunObject( MyMem, Object)
struct MemVar *MyMem;
SNR *Object;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;

	PI = (PROCESSINFO *)Object->ProcInfo;
	if(PI == NULL)
		return;

	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Cmd = DCC_DORUN;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;

			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> RunObj, sending DCC_DORUN to child %s\n\r",PI->pi_Name);
#endif
		}
	}
}


/***************************************************************
*Func : Preload objects
*in   : -
*out  : NO_ERROR
*		Error number
*/
int PreloadObjects( MyMem)
struct MemVar *MyMem;
{
  int Err;

	for(CurSNR = (SNR *)SNRList->lh_Head;
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
		switch(CurSNR->nodeType)
		{
			// standard XaPP
			case TALK_ANIM:
			case TALK_AREXX:
			case TALK_DOS:
			case TALK_VARS:
			case TALK_PAGE:
			case TALK_SOUND:
			case TALK_USERAPPLIC:
			case TALK_STARTPAR:
						if((Err = LoadObject(MyMem,CurSNR)) != NO_ERROR)
							return(Err);
						if(PLL >= 3)
							((PROCESSINFO *)CurSNR->ProcInfo)->pi_Preload = TRUE;
						else
							((PROCESSINFO *)CurSNR->ProcInfo)->pi_Preload = FALSE;
						break;
		}
	} 
	return(NO_ERROR);
}

/***************************************************
*Func : prepare all children for a new run
*		If they are loaded then send a DCC_DOPREPARE
*		else load them again.
*in   : -
*out  : Error
*/
int PrepareChildren( MyMem)
struct MemVar *MyMem;
{
  int Err;
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;

	for(CurSNR = (SNR *)SNRList->lh_Head;
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
		// if not loaded then load it now
		if(CurSNR->ProcInfo == NULL)
		{
			switch(CurSNR->nodeType)
			{
				case TALK_ANIM:
				case TALK_AREXX:
				case TALK_DOS:
				case TALK_VARS:
				case TALK_PAGE:
				case TALK_SOUND:
				case TALK_USERAPPLIC:
				case TALK_STARTPAR:
						if((Err = LoadObject(MyMem,CurSNR)) != NO_ERROR)
							return(Err);
						((PROCESSINFO *)CurSNR->ProcInfo)->pi_Preload = FALSE;
						break;
			}
		}
		else
		{
			// if loaded then tell it to prepare
			PI = (PROCESSINFO *)CurSNR->ProcInfo;

			// send msg only to PI's that are not ready to commit suicide
			if(PI->pi_State != ST_TERM)
			{ 	
				if(PD = GetFreeChildDial(PI))
				{
					PD->pd_ChildPI = NULL;
					PD->pd_Cmd = DCC_DOPREPARE;
					PD->pd_Msg.mn_ReplyPort = RepP_Guide;
					PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
					printf("%s> Sending DOPREPARE to child %s\n\r",ThisPI->pi_Name,PI->pi_Name);
#endif
				}
			}
		}
	} 
#if _PRINTF
	printf("%s> This parguide is now prepared to run\n",ThisPI->pi_Name);
#endif
	return(NO_ERROR);
}
 
/****************************************************************
*Func : Main entry for parallel guide processors
*		All objects from the objectlist are scanned
*		and subsequent workers/guides are started
*		All coordination for higher levels is
*		controlled by this guide
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{
  
  int i, SigNum_Forced;
  PROCESSINFO *TPI, *PI;
  int Err, PLErr;
  struct MemVar *MyMem;		// ptr to all variables used in this program
							// All global vars must are placed in this struct.
  BOOL B_InitDone;			// If True all workers have been initialised

	if( (TPI = (PROCESSINFO *)ml_FindBaseAddr( argc, argv)) == NULL)
		return;	

	if( (MyMem = (struct MemVar *)AllocMem(sizeof(struct MemVar), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		TPI->pi_Arguments.ar_RetErr = ERR_NOMEM;
		TPI->pi_State = ST_TERM;	// force terminate state
		return;
	}
	ThisPI = TPI;
				
	if( (Err = InitParGuide(MyMem)) != NO_ERROR)
	{
		FreeParGuide(MyMem,Err);
		FreeMem(MyMem,sizeof(struct MemVar));
		return;
	}
	// get public resident segment list
	SegList = ThisPI->pi_Arguments.ar_Guide.ag_SegList;

	SIR = ThisPI->pi_Arguments.ar_Guide.ag_SIR;
	PLL = ThisPI->pi_Arguments.ar_Guide.ag_PLL;
	MLSystem =  ThisPI->pi_Arguments.ar_Guide.ag_MLSystem;
	SNRList = ThisPI->pi_SNR->list;

	for(i = 0; i < DIAL_MAXSER; i++)
	{
		// Set up the Dialogue message to our guide to indicate we are ready
		Msg_SerDial[i]->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		Msg_SerDial[i]->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
		// Attach the replyport to the message dialogue
		Msg_SerDial[i]->pd_Msg.mn_ReplyPort = RepP_Guide;
		Msg_SerDial[i]->pd_ChildPI = ThisPI;
	}
	if( (SigNum_Forced = AllocSignal(-1)) != -1)
		Sig_Forced = 1 << SigNum_Forced;
	else
		Sig_Forced = 0;

	SigR_WBStartup = 1 << RepP_WBStartup->mp_SigBit;	// Reply from a child process
	SigR_Guide 	= 1 << RepP_Guide->mp_SigBit;			// reply on message send to either 
														// parent or child.
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;	// message from our parent
	Sig_CtoP = 1 << Port_CtoP->mp_SigBit;				// message from a child

	SystemError = NO_ERROR;
	
	if((PLErr = PreloadObjects(MyMem)) != NO_ERROR)
	{
		Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
#if _PRINTF
		printf("Error preloading\n");
#endif
	}

	B_InitDone = FALSE;
	B_TermOK = FALSE;

	if((struct List *)PIList->lh_TailPred == (struct List *)PIList)
	{
		TalkToPC(MyMem,DCI_CHILDREADY);
		B_InitDone = TRUE;
	}

	// Main loop, continue this loop until all workerprocesses have removed themselves
	while( 
			(!B_TermOK) ||
			((struct List *)PIList->lh_TailPred != (struct List *)PIList)
		 )
	{
		SigRecvd = Wait(Sig_Forced |
						SigR_WBStartup |
						SigR_Guide |
						Sig_PtoC |
						Sig_CtoP |
						SIGF_ABORT);

		// Wait for workers to get ready
		if(!B_InitDone)
		{
			B_InitDone = TRUE;
			for(PI = (PROCESSINFO *)PIList->lh_Head; 
				(PROCESSINFO *)PI->pi_Node.ln_Succ;
				PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
				if(PI->pi_State != ST_READY)
				{
#if _PRINTF
					printf("Worker %s not ready yet\n",PI->pi_Name);
#endif
					B_InitDone = FALSE;
					Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
					break;
				}

			if(B_InitDone)
			{
#if _PRINTF
				printf("Parguide Init Done\n");
#endif
				if(PLErr == NO_ERROR)
					TalkToPC(MyMem,DCI_CHILDREADY);
				else	
					TalkToPC(MyMem,DCI_CHILDREADY_MEMPROBLEM);
			}
		}

		if(SystemError != NO_ERROR)
			TalkToPC(MyMem,DCI_SEVEREPROBLEM);

		// Get replies from Process Controller and children 
		if(SigRecvd & SigR_Guide)
			GetReply(MyMem);

		if(SigRecvd & SigR_WBStartup)
			KillProcess(MyMem);

		// Check for request on DCI_CHILDTERM or DCI_CHILDREADY	
		if(SigRecvd & Sig_CtoP)
			ChildTalk(MyMem);

		if(SigRecvd & Sig_PtoC)
			ParentTalk(MyMem);

		// Command our children
		if(SigRecvd & SIGF_ABORT)
		{
			B_TermOK = TRUE;
			CmdChildren(MyMem, DCC_DOTERM);
		}
	}

	if(Sig_Forced)
		FreeSignal(Sig_Forced);

	// Clean up outstanding messages

	FreeParGuide(MyMem,SystemError);
	FreeMem(MyMem,sizeof(struct MemVar));
	return;
}
