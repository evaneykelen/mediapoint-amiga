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
#include "minc:sync.h"
#include "minc:external.h"
#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE

#define MINIMUMSIZE      150000
#define MINIMUMSIZE_HALF  75000

/***********************************************************************
* All Variables have been declared within a structure to establish
* re-entrant coding
*/

struct MemVar
{
  struct Library 	*MLMMULibBase;
  MLSYSTEM			*MLSystem;
  struct ScriptInfoRecord	*SIR;
  struct List		*SegList;	// Ptr to global list of resident worker/guide segments
  struct List 		*PIList;	// List of guides/workers belonging to this guide
  struct List		*SNRList;

  PROCESSINFO *ThisPI;			// Ptr to this processinfo blk (as used in our parent's list)

  struct MsgPort *RepP_WBStartup;// Reply port used when a child terminates
								// There is one port for all children
  struct MsgPort *Port_CtoP;	// Port for this task which is used by
								// the child to send messages to this parent
								// There is one port for all children
  struct MsgPort *RepP_Guide;	// Reply port for both child and parent
								// when they receive a message from this guide
  struct WBStartup 	*Msg_RWB;		// Reply message from process
  PROCDIALOGUE 		*Msg_SerDial[DIAL_MAXSER];	
									// Dialogue used to communicate with parent
  PROCDIALOGUE		*Msg_RProcDial;	// A message from either a child or our parent
  ULONG Sig_PtoC,					// Signal, parent to child
	    Sig_CtoP,					// Signal, child to parent (us)
        SigR_Guide,					// Signal, a reply on a msg we've send to our parent/child
	    SigR_WBStartup,				// Signal, a child commited suicide
		Sig_Forced,
	    SigRecvd;					// Signals received

  SNR *Obj_Load;					// Object to be loaded
  SNR *Obj_Run;						// Object to be run
  SNR *Obj_Hold;					// Object to be holded
  SNR *Obj_Term;					// Object to be terminated

  BOOL B_TermOK;					// If TRUE, we may terminate ourselves
  BOOL B_InitDone;					// if true, all workers have been loaded
  BOOL B_CleanUp;					// if true, this guide must cleanup workers
									// from the end of the list to the beginning
									// and stop when anough memory becomes available.
  int  SystemError;			
  PROCESSINFO *CurPI;				// Free to be used as temporarily PI ptr
  int PLL;							// PreLoadLevel
  int PLErr;
};

#define MLMMULibBase		MyMem->MLMMULibBase
#define MLSystem			MyMem->MLSystem
#define SIR					MyMem->SIR

#define SPList				MyMem->SPList
#define	SegList 			MyMem->SegList
#define	PIList 				MyMem->PIList
#define SNRList				MyMem->SNRList
#define ThisPI 				MyMem->ThisPI
#define CurPI				MyMem->CurPI

#define RepP_WBStartup		MyMem->RepP_WBStartup
#define RepP_Guide			MyMem->RepP_Guide

#define Port_CtoP			MyMem->Port_CtoP

#define CurWorkNr			MyMem->CurWorkNr
#define CurGuideNr			MyMem->CurGuideNr

#define Msg_RWB				MyMem->Msg_RWB
#define Msg_RProcDial		MyMem->Msg_RProcDial

#define Msg_SerDial 		MyMem->Msg_SerDial

#define Sig_PtoC			MyMem->Sig_PtoC
#define Sig_CtoP			MyMem->Sig_CtoP
#define Sig_Forced			MyMem->Sig_Forced
#define SigR_Guide			MyMem->SigR_Guide
#define SigR_WBStartup		MyMem->SigR_WBStartup

#define SigRecvd			MyMem->SigRecvd

#define Obj_Load			MyMem->Obj_Load
#define Obj_Run				MyMem->Obj_Run
#define Obj_Hold			MyMem->Obj_Hold
#define Obj_Term			MyMem->Obj_Term

#define B_TermOK			MyMem->B_TermOK
#define B_InitDone			MyMem->B_InitDone
#define B_CleanUp			MyMem->B_CleanUp
#define SystemError			MyMem->SystemError
#define PLL					MyMem->PLL
#define PLErr				MyMem->PLErr

/*************************************************
*Func : Free all global structures used by this guide
*in   : -
*out  : -
*/
void FreeSerGuide( MyMem, Error)
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
int InitSerGuide( MyMem)
struct MemVar *MyMem;
{
  int i;

	if( (MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return(ERR_NOMEM);

	// Make a ReplyPort for our children, used only to find out a guide/worker
	// has terminated. It needs not to be public since we will pass the Ptr on to 
    // the children. 
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
	{
		if(!((PROCDIALOGUE *)PI->pi_PtoCDial[i])->pd_InUse)
		{	
			((PROCDIALOGUE *)PI->pi_PtoCDial[i])->pd_InUse = TRUE;
			((PROCDIALOGUE *)PI->pi_PtoCDial[i])->pd_Luggage.lu_Dial = NULL;
			return( (PROCDIALOGUE *)PI->pi_PtoCDial[i]);
		}
	}
#if _PRINTF
	KPrintF("No free dialogue for child %s\n",PI->pi_Name);
#endif
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

	//KPrintF("GetFreeSerDial failed\n");

	return(NULL);
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

//KPrintF("FindChild failed\n");

	return(FALSE);
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
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;

	// A child commited suicide
	// Remove its corps and send it to the graveyard
	// Get the messages from the childprocesses that just terminated itself
	while( (Msg_RWB = (struct WBStartup *)GetMsg(RepP_WBStartup)) != NULL)
	{
		CurPI = (PROCESSINFO *)((ULONG)Msg_RWB - (sizeof(struct Node)) - sizeof(UWORD));
#if _PRINTF
		KPrintF("%s> removing [%s], error %d\n",ThisPI->pi_Name,CurPI->pi_Name, CurPI->pi_Arguments.ar_RetErr);
#endif
		Remove((struct Node *)CurPI);
		if((CurPI->pi_SNR != NULL) && (CurPI == (PROCESSINFO *)(CurPI->pi_SNR->ProcInfo)))
			CurPI->pi_SNR->ProcInfo = NULL;	// SNR no longer active

		MLUnLoadSeg(SegList, CurPI->pi_Segment);
		UnLock(CurPI->pi_Startup.sm_ArgList[0].wa_Lock);

		DeleteTaskPort(CurPI->pi_Port_PtoC);
		MLMMU_FreeMem(CurPI->pi_PtoCDial[0]);
		MLMMU_FreeMem(CurPI->pi_Startup.sm_ArgList);
		MLMMU_FreeMem(CurPI->pi_Name);
		MLMMU_FreeMem(CurPI);
	}

	// Search for children that have lost their parent and kill them
	// it;s nature's law, only the strong will survive
	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		// a Darcy McDonald, lets burry him
		if(
			((PI->pi_SNR == NULL) || (PI != (PROCESSINFO *)(PI->pi_SNR->ProcInfo))) &&
			PI->pi_State != ST_TERM
		  )
		{
			if(PD = GetFreeChildDial(PI))
			{
#if _PRINTF
				KPrintF("Burying child %s PI %x, pi_SNR %x\n",PI->pi_Name,(int)PI,(int)PI->pi_SNR);
#endif
				PD->pd_ChildPI = NULL;
				PD->pd_Cmd = DCC_DOTERM;
				PD->pd_Msg.mn_ReplyPort = RepP_Guide;
				PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
			}
		}
	}
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
	if( (PD = GetFreeSerDial( MyMem)) != NULL)
	{
		PD->pd_ChildPI = ThisPI;
		PD->pd_Cmd = Cmd;
		PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)PD);
	}
}

/******************************************************
*Func : Clean up workers from the end of the list
*		until enough memory becomes available
*in   : -
*out  : -
*/
void CleanUpMem(MyMem)
struct MemVar *MyMem;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;

	if(MLMMU_AvailMem(MEMF_PUBLIC) > MINIMUMSIZE)	//100000)
	{
		B_CleanUp = FALSE;
#if _PRINTF
		printf("Clean up procedure, over 100Kbyte ram available\n");	
#endif
		PLErr = NO_ERROR;
		Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
		MLMMU_PleaseFree(MINIMUMSIZE);	//100000);
		return;
	}

	for(PI = (PROCESSINFO *)PIList->lh_TailPred;
		((PROCESSINFO *)PI->pi_Node.ln_Pred)->pi_Node.ln_Pred;
		PI = (PROCESSINFO *)PI->pi_Node.ln_Pred)
	{		
		if(PI->pi_State != ST_TERM && PI->pi_State != ST_RUNNING)
		{	
			PI->pi_State = ST_TERM;			// force state of worker
			if(PD = GetFreeChildDial(PI))
			{
				PD->pd_ChildPI = NULL;
				PD->pd_Cmd = DCC_DOTERM;
				PD->pd_Msg.mn_ReplyPort = RepP_Guide;
				PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
				printf("Clean up procedure, removing child %s\n",PI->pi_Name);
#endif				
				MLMMU_PleaseFree(MINIMUMSIZE);	//100000);
				return;
			}
		}
	}

	if(((PROCESSINFO *)PI->pi_Node.ln_Pred)->pi_Node.ln_Pred == NULL)
	{
		B_CleanUp = FALSE;
#if _PRINTF
		printf("Clean up procedure, removed all but first in PILIST, memleft = %d\n",(int)MLMMU_AvailMem(MEMF_PUBLIC));	
#endif
		if(MLMMU_AvailMem(MEMF_PUBLIC) < MINIMUMSIZE_HALF)	//50000)
		{
			TalkToPC(MyMem,DCI_SEVEREPROBLEM);
			PLErr = NO_ERROR; 
		}	
		MLMMU_PleaseFree(MINIMUMSIZE);	//100000);
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
		if(FindChild(MyMem,Msg_RProcDial->pd_ChildPI))
		{
			switch(Msg_RProcDial->pd_Cmd)
			{
				case DCI_CHILDREADY:
#if _PRINTF
							printf("%s> Received DCI_CHILDREADY from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							//KPrintF("%s> Received DCI_CHILDREADY from child [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
							// Our child is ready to go.
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
							Msg_RProcDial->pd_ChildPI->pi_State = ST_READY;
							break;
				case DCI_CHILDTERM:
#if _PRINTF
							printf("%s> Received DCI_CHILDTERM from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							//KPrintF("%s> Received DCI_CHILDTERM from child [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
							// Our child want to commit suicide
							Msg_RProcDial->pd_Cmd = DCC_DOTERM;
							Msg_RProcDial->pd_ChildPI->pi_State = ST_TERM;
							// The child is free to let its spirit fly
							break;
				case DCI_SEVEREPROBLEM:
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
#if _PRINTF
							printf("PC> received DCI_SEVEREPROBLEM from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							//KPrintF("PC> received DCI_SEVEREPROBLEM from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
							TalkToPC(MyMem,DCI_SEVEREPROBLEM);
							break;
				default: 
							Msg_RProcDial->pd_Cmd = DCC_IGNORE;
#if _PRINTF
							printf("%s> Received unknown request from child [%s]\n\r",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
							//KPrintF("%s> Received unknown request from child [%s]\n",ThisPI->pi_Name,Msg_RProcDial->pd_ChildPI->pi_Name);
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
#if _PRINTF
			else
				printf("%s> unable to Send cmd to child %s no PD free\n\r",ThisPI->pi_Name,PI->pi_Name);
#endif
		}
#if _PRINTF
		else
			printf("%s> unable to Send cmd to child %s state is ST_TERM\n\r",ThisPI->pi_Name,PI->pi_Name);
#endif
	}
}

/***********************************************************
*Func : Send A command to all our children
*in   : MyMem
*out  : -
*/
void ResetPreloadChildren( MyMem)
struct MemVar *MyMem;
{
  PROCESSINFO *PI;

	// Send a command message to all our children
	// For us this means, send it to our child-guide
	for(PI = (PROCESSINFO *)PIList->lh_Head;
		(PROCESSINFO *)PI->pi_Node.ln_Succ;
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
		PI->pi_Preload = FALSE;
}

/*************************************************
*Func : The ProcessController has something to say to us
*in   : -
*out  : -
*/
void PCTalk( MyMem)
struct MemVar *MyMem;
{
  int Err;
  PROCESSINFO *PI;
  PROCDIALOGUE *RPD;
  BOOL B_Reply;

	// The ProcContr has something to say to us
	while( (RPD = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
	{
		B_Reply = TRUE;
		RPD->pd_ChildPI = ThisPI;
		switch(RPD->pd_Cmd)
		{
			// Following cases are for this guide only
			case DCC_DOTERM:
#if _PRINTF
						printf("%s> Received DCC_DOTERM from PC\n\r",ThisPI->pi_Name);
#endif
						RPD->pd_Cmd = DCI_CHILDTERM;
						if(!B_TermOK)
							CmdChildren(MyMem, DCC_DOTERM);
						B_TermOK = TRUE;
						break;
			case DCC_DOCLEANUP:
#if _PRINTF
						printf("%s> Received DCC_DOCLEANUP from PC\n\r",ThisPI->pi_Name);
#endif
						RPD->pd_Cmd = DCI_CHILDCLEANUP;
						if(!B_TermOK)
							CmdChildren(MyMem, DCC_DOTERM);
						break;
			case DCC_DOCLEANUPEASY:
#if _PRINTF
						printf("%s> Received DCC_DOCLEANUPEASY from PC\n\r",ThisPI->pi_Name);
#endif
						ResetPreloadChildren(MyMem);
						RPD->pd_Cmd = DCI_CHILDCLEANUP;
						B_CleanUp = TRUE;
						B_InitDone = FALSE;
						ThisPI->pi_State = ST_INIT;
						Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
						break;
			// Following cases are for object control only
			// DOLOADOBJ  MUST  always preceed DORUNOBJ
			case DCC_DOTERMALLOBJS:
#if _PRINTF
						printf("%s> Received DCC_DOTERMALLOBJS from PC\n\r",ThisPI->pi_Name);
#endif
						RPD->pd_Cmd = DCI_TERMALLOBJS;
						// Terminates all objects except for CONTINUE objects
						// Also, the object in pd_Luggage.lu_SNR will not be terminated

						for(PI = (PROCESSINFO *)PIList->lh_Head;
							(PROCESSINFO *)PI->pi_Node.ln_Succ;
							PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
							if(PI->pi_SNR != RPD->pd_Luggage.lu_SNR && PI->pi_State != ST_TERM)
								TermObject(MyMem,PI->pi_SNR,NULL);
						break;
			case DCC_DOLOADOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOLOADOBJ for page %d from PC\n\r",ThisPI->pi_Name,RPD->pd_Luggage.lu_SNR->PageNr);
#endif
						RPD->pd_Cmd = DCI_OBJLOAD;
						if((Err = LoadObject(MyMem,RPD->pd_Luggage.lu_SNR)) != NO_ERROR)
						{
#if _PRINTF
							printf("Error afterloading\n");
#endif
							Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
							SystemError = Err;
						}
						break;
			case DCC_DORUNOBJ:
#if _PRINTF
						printf("%s> Received DCC_DORUNOBJ for page %d,%s from PC \n\r",ThisPI->pi_Name,RPD->pd_Luggage.lu_SNR->PageNr,((PROCESSINFO *)RPD->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						//KPrintF("%s> Received DCC_DORUNOBJ for page %d,%s from PC \n",ThisPI->pi_Name,RPD->pd_Luggage.lu_SNR->PageNr,((PROCESSINFO *)RPD->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
						RPD->pd_Cmd = DCI_OBJRUNS;
						if(RPD->pd_Luggage.lu_SNR->ParentSNR->nodeType == TALK_STARTSER)
						{
#if _PRINTF
							printf("Object is attached to SERIAL guide\n");	
#endif
							RunObject(MyMem,RPD->pd_Luggage.lu_SNR);
						}			
						else
						{
#if _PRINTF
							printf("Object is attached to PARALLEL guide\n");	
#endif
							RunParObject(MyMem,RPD->pd_Luggage.lu_SNR);
						}
						break;
			case DCC_DOHOLDOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOHOLDOBJ from PC\n\r",ThisPI->pi_Name);
#endif
						RPD->pd_Cmd = DCI_OBJHOLDS;
						if(RPD->pd_Luggage.lu_SNR->ParentSNR->nodeType == TALK_STARTSER)
							HoldObject(MyMem,RPD->pd_Luggage.lu_SNR);
						else
							HoldParObject(MyMem,RPD->pd_Luggage.lu_SNR);
						break;
			case DCC_DOTERMOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOTERMOBJ from PC for %s\n\r",ThisPI->pi_Name,((PROCESSINFO *)RPD->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						RPD->pd_Cmd = DCI_OBJTERMS;
						if(RPD->pd_Luggage.lu_SNR->ParentSNR->nodeType == TALK_STARTSER)
							B_Reply = TermObject(MyMem,RPD->pd_Luggage.lu_SNR,RPD);
						else
							B_Reply = TermParObject(MyMem,RPD->pd_Luggage.lu_SNR,RPD);
						
#if _PRINTF
						printf("B_Reply after term request = %s\n",B_Reply ? "TRUE":"FALSE");
#endif
						break;
			case DCC_DOPREPOBJ:
#if _PRINTF
						printf("%s> Received DCC_DOPREPOBJ from PC for %s\n\r",ThisPI->pi_Name,((PROCESSINFO *)RPD->pd_Luggage.lu_SNR->ProcInfo)->pi_Name);
#endif
						RPD->pd_Cmd = DCI_OBJPREPS;
						PrepObject(MyMem,RPD->pd_Luggage.lu_SNR);
						break;
			default:	
						RPD->pd_Cmd = DCI_IGNORE;
#if _PRINTF
						printf("%s> Received unknown command from PC\n\r",ThisPI->pi_Name);
#endif
						break;
		}
		if(B_Reply)
			ReplyMsg((struct Message *)RPD);
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
  PROCDIALOGUE *RPD;

	while( (RPD = (PROCDIALOGUE *)GetMsg(RepP_Guide)) != NULL)
	{
		switch(RPD->pd_Cmd)
		{
			case DCC_IGNORE:		// mostly received after sending a DCI_CHILDREADY
#if _PRINTF
						printf("%s> Received DCC_IGNORE reply from Parent\n\r",ThisPI->pi_Name);
#endif
						break;
			case DCC_DOTERM:		// received after sending a DCI_CHILDTERM request
#if _PRINTF
						printf("%s> Received DCC_DOTERM reply from Parent\n\r",ThisPI->pi_Name);
#endif
						B_TermOK = TRUE;
						break;

			case DCI_IGNORE:	// a child didn't support the last message
#if _PRINTF
						printf("%s> Received DCI_IGNORE reply from [%s]\n\r",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
#endif
						break;	// probably a continues aPP with no DOHOLD support
			case DCI_CHILDREADY:	// reply on sending DCC_DORUN
									// Some Xapps are fast enough to start their
									// job and be ready to be run again (like WorkDos)
						RPD->pd_ChildPI->pi_State = ST_READY;
#if _PRINTF
						printf("%s> Received DCI_CHILDREADY reply from [%s]\n\r",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDRUNS:	// reply on sending DCC_DORUN
						RPD->pd_ChildPI->pi_State = ST_RUNNING;
#if _PRINTF
						printf("%s> Received DCI_CHILDRUNS reply from [%s]\n\r",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
#endif
						//KPrintF("%s> Received DCI_CHILDRUNS reply from [%s]\n",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
						break;
			case DCI_CHILDHOLDS:// reply on sending DCC_DOHOLD
						RPD->pd_ChildPI->pi_State = ST_HOLD;
#if _PRINTF
						printf("%s> Received DCI_CHILDHOLDS reply from [%s]\n\r",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDTERM: // reply on sending DCC_DOTERM to child
						RPD->pd_ChildPI->pi_State = ST_TERM;
#if _PRINTF
						printf("%s> Received DCI_CHILDTERM at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)RPD,RPD->pd_ChildPI->pi_Name);
#endif
						RPD->pd_InUse = FALSE;
						Signal(&RPD->pd_ChildPI->pi_Process->pr_Task, SIGF_ABORT);
						break;
			case DCI_CHILDPREPARES:// reply on sending DCC_DOPREPARE
						RPD->pd_ChildPI->pi_State = ST_INIT;
#if _PRINTF
						printf("%s> Received DCI_CHILDPREPS reply from [%s]\n\r",ThisPI->pi_Name,RPD->pd_ChildPI->pi_Name);
#endif
						break;

			case DCI_CHILDEASYTERM: // reply on sending DCC_DOEASYTERM to child
			case DCI_CHILDEASYTERMOBJ: // reply on sending DCC_DOEASYTERMOBJ to child
						if(RPD->pd_Cmd == DCI_CHILDEASYTERM) 
							RPD->pd_ChildPI->pi_State = ST_TERM;
#if _PRINTF
						printf("%s> Received DCI_EASYCHILDTERM at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)RPD,RPD->pd_ChildPI->pi_Name);
#endif
						RPD->pd_InUse = FALSE;

						if(
							(RPD->pd_Luggage.lu_Dial != NULL) && 
							(((PROCDIALOGUE *)RPD->pd_Luggage.lu_Dial)->pd_Msg.mn_Node.ln_Type == NT_MESSAGE)
						  )
						{
#if _PRINTF
							printf("Returning original dialogue to PC\n");
#endif
							ReplyMsg(RPD->pd_Luggage.lu_Dial);
						}	

						Signal(&RPD->pd_ChildPI->pi_Process->pr_Task, SIGF_ABORT);
						break;
			case DCI_CHILDEASYSTOP: // reply on sending DCC_DOEASYSTOP to child
			case DCI_CHILDEASYSTOPOBJ: // reply on sending DCC_DOEASYSTOPOBJ to child
						if(RPD->pd_Cmd == DCI_CHILDEASYSTOP) 
							RPD->pd_ChildPI->pi_State = ST_READY;
#if _PRINTF
						printf("%s> Received DCI_EASYCHILDSTOP at %x reply from [%s]\n\r",ThisPI->pi_Name,(int)RPD,RPD->pd_ChildPI->pi_Name);
#endif
						if(
							(RPD->pd_Luggage.lu_Dial != NULL) && 
							(((PROCDIALOGUE *)RPD->pd_Luggage.lu_Dial)->pd_Msg.mn_Node.ln_Type == NT_MESSAGE)
						  )
						{
#if _PRINTF
							printf("Returning original dialogue to PC\n");
#endif
							ReplyMsg(RPD->pd_Luggage.lu_Dial);
						}	
						break;
			default:
#if _PRINTF
						printf("%s> Received unknown reply from Parent/Child\n\r",ThisPI->pi_Name);
#endif
						break;
		}
		RPD->pd_InUse = FALSE;

		if(SetSignal(0,0) & SIGF_ABORT)
			break;
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
PROCESSINFO *PI;

	/****** START NEW *********/

	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		if( PI->pi_SNR == Object )
			return(0);
	}

	/****** END NEW *********/

	if(Object->nodeType != TALK_STARTPAR)
		Err = AddWorker(MLMMULibBase,MLSystem, SegList, PIList, Object, Port_CtoP, RepP_WBStartup,
						ThisPI->pi_Arguments.ar_Guide.ag_MainDirPath);
	else
		Err = AddGuide(MLMMULibBase,MLSystem, SegList,PIList,SIR,GT_PAR,Object,Port_CtoP,RepP_WBStartup,
					   ThisPI->pi_Arguments.ar_Guide.ag_MainDirPath,PLL);

	return(Err);
}

/*********************************************************
*Func : Terminate a object
*in   : Object -> Object to be terminated
*out  : TRUE -> Send a reply at once
*		FALSE -> don't send a reply yet
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
		return(TRUE);

	B_Reply = TRUE;
	// if the current active object is not terminating itself right now
	// then make it terminate itself.
#if _PRINTF
	printf("> TermObject, state of %s is %d\n\r",PI->pi_Name,PI->pi_State);
#endif
	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Luggage.lu_Dial = NULL;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;

			// Preload object must not be terminated but just stop with what they are doing
			PD->pd_Cmd = PI->pi_Preload ? DCC_DOSTOP: DCC_DOTERM;

			if(PI->pi_Quiet)
			{
				if(PI->pi_SNR->numericalArgs[0] == ARGUMENT_DEFER)
					PD->pd_Cmd = PI->pi_Preload ? DCC_DOSTOP: DCC_DOTERM;
				else
					PD->pd_Cmd = DCC_IGNORE;
			}
			else
			{
				if(
					(PI->pi_SNR->numericalArgs[0] == ARGUMENT_DEFER) &&
					(RepDial != NULL)
				  )
				{
					B_Reply = FALSE;	
					PD->pd_Luggage.lu_Dial = RepDial;
					PD->pd_Cmd = PI->pi_Preload ? DCC_DOEASYSTOP: DCC_DOEASYTERM;
				}
				else
					PD->pd_Cmd = PI->pi_Preload ? DCC_DOSTOP: DCC_DOTERM;
			}
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
		}
	}
#if _PRINTF
	printf("> TermObj ending, sending DCC_DOTERM %d to child %s\n\r",PD->pd_Cmd,PI->pi_Name);
#endif
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
*Func : Prep a object as in Obj
*in   : Object -> Object to be Prepped
*out  : -
*/
void PrepObject( MyMem, Object)
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
	printf("> PrepObject, state of %s is %d\n\r",PI->pi_Name,PI->pi_State);
#endif
	if(PI->pi_State != ST_TERM && PI->pi_State != ST_INIT)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Cmd = DCC_DOPREPARE;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;
	
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
		}
	}
#if _PRINTF
	printf("> PrepObj end, sending DCC_DOPREPARE to child %s\n\r",PI->pi_Name);
#endif
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
	{	
		//KPrintF("RO 1 failed\n");
		return;
	}

#if _PRINTF
	{ char dbug[100]; sprintf(dbug,"> RunObject, state of %s is %d\n",PI->pi_Name,PI->pi_State); KPrintF(dbug); }
#endif

	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Cmd = DCC_DORUN;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;

			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
		}
		//else
		//	KPrintF("RO 3 failed\n");
	}
	//else
	//	KPrintF("RO 2 failed\n");

#if _PRINTF
	printf("> RunObj ending, sending DCC_DORUN to child %s\n\r",PI->pi_Name);
#endif
}



/*********************************************************
*Func : Terminate a object
*in   : Object -> Object to be terminated
*out  : -
*/
BOOL TermParObject( MyMem, Object, RepDial)
struct MemVar *MyMem;
SNR *Object;
PROCDIALOGUE *RepDial;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI, *PIO;
  SNR *ParentSNR;
  BOOL B_Reply;

	B_Reply = TRUE;

	ParentSNR = Object->ParentSNR;

	PIO = (PROCESSINFO *)Object->ProcInfo;
	PI = (PROCESSINFO *)ParentSNR->ProcInfo;
	if(PI == NULL || PIO == NULL)
		return(TRUE);

	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;
			PD->pd_Luggage.lu_SNR = Object;
			PD->pd_Luggage.lu_Dial = RepDial;

			if(PIO->pi_Quiet)
			{
				if(PIO->pi_SNR->numericalArgs[0] == ARGUMENT_DEFER)
					PD->pd_Cmd = PIO->pi_Preload ? DCC_DOSTOPOBJ: DCC_DOTERMOBJ;
				else
					PD->pd_Cmd = DCC_IGNORE;
			}
			else
			{
				if(
					(PIO->pi_SNR->numericalArgs[0] == ARGUMENT_DEFER) &&
					(RepDial != NULL)
				  )
				{
					B_Reply = FALSE;
					PD->pd_Cmd = PIO->pi_Preload ? DCC_DOEASYSTOPOBJ: DCC_DOEASYTERMOBJ;
				}
				else
					PD->pd_Cmd = PIO->pi_Preload ? DCC_DOSTOPOBJ: DCC_DOTERMOBJ;
			}
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> TermparObj, sending DCC_TERM %d for page %d to parguide %s\n\r",PD->pd_Cmd,Object->PageNr,PI->pi_Name);
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
void HoldParObject( MyMem, Object)
struct MemVar *MyMem;
SNR *Object;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;
  SNR *ParentSNR;

	ParentSNR = Object->ParentSNR;

	PI = (PROCESSINFO *)ParentSNR->ProcInfo;
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
			PD->pd_Cmd = DCC_DOHOLDOBJ;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;
			PD->pd_Luggage.lu_SNR = Object;
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> Holdobj, sending DCC_DORUNOBJ to parguide %s\n\r",PI->pi_Name);
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
void RunParObject( MyMem, Object)
struct MemVar *MyMem;
SNR *Object;
{
  PROCDIALOGUE *PD;
  PROCESSINFO *PI;
  SNR *ParentSNR;

	ParentSNR = Object->ParentSNR;

	PI = (PROCESSINFO *)ParentSNR->ProcInfo;
	if(PI == NULL)
		return;

	if(PI->pi_State != ST_TERM)
	{
		if(PD = GetFreeChildDial(PI))
		{
			PD->pd_ChildPI = NULL; 					// from parent
			PD->pd_Cmd = DCC_DORUNOBJ;
			PD->pd_Msg.mn_ReplyPort = RepP_Guide;
			PD->pd_Luggage.lu_SNR = Object;
			PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
#if _PRINTF
			printf("> RunparObj, sending DCC_DORUNOBJ for page %d to parguide %s\n\r",Object->PageNr,PI->pi_Name);
#endif
		}
	}
}

/***************************************************************
*Func : Preload objects
*in   : -
*out  : NO_ERROR
*		Error
*/		
int PreloadObjects( MyMem)
struct MemVar *MyMem;
{
  int Err;
  SNR *CurSNR;
  BOOL LoadObj;
  ULONG SigRec;
 
	LoadObj = TRUE;		// first object in list must always be preloaded

	for(CurSNR = (SNR *)SNRList->lh_Head;
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
					// From PLL5, Load all objects
		if(PLL >= 5)
			LoadObj = TRUE;
		else		// From PLL4, Load all objects if this is the main level
			if(PLL >= 4 && ThisPI->pi_Unit == 1)
				LoadObj = TRUE;
			else	 	// From PLL3, also load parallel guides
				if(PLL >= 3 && CurSNR->nodeType == TALK_STARTPAR)
					LoadObj = TRUE;
				else 		// From PLL 2, load first executable object behind a label
					if(PLL >= 2 && CurSNR->nodeType == TALK_LABEL)
						LoadObj = TRUE;

		if (	(AvailMem(MEMF_CHIP | MEMF_LARGEST) < MINIMUMSIZE) ||
				(AvailMem(MEMF_FAST | MEMF_LARGEST) < MINIMUMSIZE) )
		{
#if _PRINTF
			KPrintF("avail would have failed\n");
#endif
			LoadObj = FALSE;
		}

		if(LoadObj)
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
							LoadObj = FALSE;
							if((Err = LoadObject(MyMem,CurSNR)) != NO_ERROR)
 								return(Err);
							SigRec = Wait(Sig_CtoP|SIGF_ABORT);
							// Receive information from our children
							if(SigRec & Sig_CtoP)
								ChildTalk(MyMem);
							Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
							if(PLL >= 2)
								((PROCESSINFO *)CurSNR->ProcInfo)->pi_Preload = TRUE;
							break;
			}
		}
	} 

#if 0
	if(PLL >= 4)
	{
		GlobEvList = SIR->globallocalEvents;
		CurSER = *GlobEvList;

		while(CurSER != NULL)
		{
			if(CurSER->labelSNR->ParentSNR == ThisPI->pi_SNR)
			{
				if(CurSER->labelSNR->ProcInfo == NULL)
				{
					if((Err = LoadObject(MyMem,CurSER->labelSNR)) != NO_ERROR)
						return(Err);
					else
					{
						SigRec = Wait(Sig_CtoP|SIGF_ABORT);
						// Receive information from our children
						if(SigRec & Sig_CtoP)
							ChildTalk(MyMem);
						Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
						((PROCESSINFO *)CurSNR->ProcInfo)->pi_Preload = TRUE;
					}
				}
			}
			GlobEvList++;				// next global event
			CurSER = *GlobEvList;
		}
	}
#endif
	return(NO_ERROR);
}

/****************************************************************
*Func : Main entry for serial guide processors
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
  int Err;
  PROCESSINFO *TPI,*PI;
  struct MemVar *MyMem;		// ptr to all variables used in this program
							// All global vars must are placed in this struct.
  
						
	if( (TPI = (PROCESSINFO *)ml_FindBaseAddr( argc, argv)) == NULL)
		return;

	if( (MyMem = (struct MemVar *)AllocMem(sizeof(struct MemVar), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		TPI->pi_Arguments.ar_RetErr = ERR_NOMEM;
		TPI->pi_State = ST_TERM;	// force terminate state
		return;
	}
	ThisPI = TPI;

	if( (Err = InitSerGuide(MyMem)) != NO_ERROR)
	{
		FreeSerGuide(MyMem,Err);
		FreeMem(MyMem,sizeof(struct MemVar));
		TPI->pi_State = ST_TERM;	// force terminate state
		return;
	}
	// get public resident segment list
	SegList = ThisPI->pi_Arguments.ar_Guide.ag_SegList;
	SIR =  ThisPI->pi_Arguments.ar_Guide.ag_SIR;
	PLL =  ThisPI->pi_Arguments.ar_Guide.ag_PLL;

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

	SigR_WBStartup 	= 1 << RepP_WBStartup->mp_SigBit;	// Reply from a child process
	SigR_Guide 		= 1 << RepP_Guide->mp_SigBit;		// reply on message send to either 
														// parent or child.
	Sig_PtoC 		= 1 << ThisPI->pi_Port_PtoC->mp_SigBit;	// message from our parent
	Sig_CtoP 		= 1 << Port_CtoP->mp_SigBit;		// message from a child

	PLErr = PreloadObjects(MyMem);

	B_InitDone = FALSE;
	B_TermOK = FALSE;
	B_CleanUp = FALSE;

	// In case of an empty list, send CHILDREADY 
	if((struct List *)PIList->lh_TailPred == (struct List *)PIList)
	{
		ThisPI->pi_State = ST_READY;
		TalkToPC(MyMem,DCI_CHILDREADY);
		B_InitDone = TRUE;
	}	

	// Main loop, continue this loop until all workers have removed themselves
	while( 
			(!B_TermOK) ||
			((struct List *)PIList->lh_TailPred != (struct List *)PIList)
		 )
	{
		SigRecvd = Wait(SigR_WBStartup |
						SigR_Guide |
						Sig_PtoC |
						Sig_CtoP |
						Sig_Forced |
						SIGF_ABORT);
	
		// Wait for workers to get ready
		if(!B_InitDone)
		{
#if _PRINTF
			printf("check for initdone\n");
#endif
			if(!B_CleanUp)
			{
				B_InitDone = TRUE;
				for(PI = (PROCESSINFO *)PIList->lh_Head; 
					(PROCESSINFO *)PI->pi_Node.ln_Succ;
					PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
					if(PI->pi_State != ST_READY)
					{
#if _PRINTF
						printf("worker %s not ready yet, state %d\n",PI->pi_Name,PI->pi_State);
#endif
						B_InitDone = FALSE;
						Signal(&(ThisPI->pi_Process->pr_Task),Sig_Forced);
						break;
					}
			}
			if(B_InitDone)
			{
				if(PLErr == NO_ERROR)
				{
#if _PRINTF
					printf("Init done. sending childready to pc\n");
#endif
					ThisPI->pi_State = ST_READY;
					TalkToPC(MyMem,DCI_CHILDREADY);
				}
				else
				{
#if _PRINTF
					printf("Init done. sending childready memproblem to pc\n");
#endif
					TalkToPC(MyMem,DCI_CHILDREADY_MEMPROBLEM);
				}
			}
		}

		if(B_CleanUp)
			CleanUpMem(MyMem);

		if(SystemError != NO_ERROR)
			TalkToPC(MyMem,DCI_SEVEREPROBLEM);

		// Get replies from Process Controller and children 
		if(SigRecvd & SigR_Guide)
			GetReply(MyMem);

		// A child corps has to be burried
		if(SigRecvd & SigR_WBStartup)
			KillProcess(MyMem);

		// Receive commands from process controller
		if(SigRecvd & Sig_PtoC)
			PCTalk(MyMem);

		// Receive information from our children
		if(SigRecvd & Sig_CtoP)
			ChildTalk(MyMem);

		// Command our children
		if(SigRecvd & SIGF_ABORT)
		{
			B_TermOK = TRUE;
			CmdChildren(MyMem, DCC_DOTERM);
		}
	}

	if(Sig_Forced)
		FreeSignal(Sig_Forced);

	FreeSerGuide(MyMem,SystemError);
	FreeMem(MyMem,sizeof(struct MemVar));
	return;
}
