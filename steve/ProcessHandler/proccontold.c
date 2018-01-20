
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/libraries.h>
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

#define printf PrintSer

#define _PRINTF FALSE
#define _PRINTSER FALSE

#define HOST_TRANSITIONS "Transitions"

extern struct CapsPrefs CPrefs;
extern struct ScriptNodeRecord *fromSNR;

#define MAINDIRPATHSIZE	500


MLSYSTEM			*MLSystem;				// MediaLink Global System structure

SNR 				*Guide1SNR;
struct ScriptInfoRecord *SIR;
char				*MainDirPath;			// Ptr to main directory of medialink

/** VAR AREA **/

struct List *DelayedVIList;					// Updated after a new object has send
											// a RUN info to  its guide.
struct List	*VIList;						// ptr of variables area of medialink
											// this list and nodes are copied when the
											// script is run.This list will copied
											// into the MLSystem.vilist for access by users 
struct MsgPort *Port_ToVI;					// global port to Variable area
											// may be used by XaPPs to gain access
											//  to the variables		
/** VAR AREA END **/


struct List			*SegList;				// List of resident modules/segments
struct List 		*PIList;				// List of guides/workers belonging to 
											// the process controller
SYNCDIALOGUE 		*Msg_SyncDial[DIAL_MAXSYNC];
											// Syncdialogue for communication with synchro
GEDIALOGUE	 		*Msg_GEDial[DIAL_MAXGE];// GEdialogue for communication with GlobeProc
SYNCDIALOGUE		*Msg_RSyncDial;
struct WBStartup 	*Msg_RWBStartup;		// Replymessage from the guide after termination
PROCESSINFO 		*CurPI;					// Ptr to a ProcessInfo 	

struct MsgPort 		*RepP_WBStartup,		// Reply port used when a guide terminates
					*Port_GEtoPC,			// Port through which the Globalevent
											// processor can reach the Processcontroller
											// Public port
					*Port_CtoP,				// Port for this task which is used by
											// the child to send messages to this parent
											// There is one port for all children 	
					*RepP_PtoC,				// Reply port for child when the parent
											// has send a message to it
					*Port_StoPC,			// Port through which the Synchro can reach
											// the PC
					*Port_PCtoS,			// Port through which we can reach the synchro
					*Port_PCtoGE,			// Port through which the PC can reach the GE
					*RepP_PCtoGE,			// and a reply from the GE
					*RepP_PCtoS;

int					Err_Transition;			// error from AddTransition()
int 				GEErr;					// Error from InitGlobalEventProcessor()
int 				SCErr;					// Error from InitSyncProcessor()
int 				GuideErr;				// Error from Addguide()

ULONG	 			Sig_GEtoPC,				// Signal, GlobalEvent handler to Proccont
					SigR_WBStartup,			// Signal, process termination 
					Sig_CtoP, 				// Signal, child->parent message
					SigR_PtoC,				// Signal, Child replies to parent			
					SigR_PCtoS,				// Signal, synchro replies
					Sig_StoPC,				// Signal, synchro to PC
					Sig_RXtoPC,				// Signal, AREXX to PC
					SigR_PCtoGE,				// Signal, GE replies				
  					SigRecvd;				// Signals received

int 				Cnt;
SNR					*SNR_Punch;				// Object to be punched
SNR					*SNR_GE;				// Next object according to the GlobEProc
SNR 				*SNR_Script;			// Next object according to the script
											// follower.
SNR					*SNR_Play;				// Object currently playing
SNR					*SNR_Load;				// object last loaded.

SNR					*SNR_TrackPlay;			// Unlike SNR_Play, this var exactly
											// keep track of the SNR currently played
											// it is set to NULL if no object is
											// currently active.

BOOL				B_SNRGE;				// if True, GlobEProc was invoked
BOOL				B_PlayObj;				// If true -> play the next object
											// else wait till next round
BOOL				B_Terminate;			// If true, escape from the current show
BOOL				B_LoadObj;				// If true, a new object must be loaded
BOOL				B_PunchObj;				// If TRUE, send punches of object to
BOOL				B_NoLoadParse;			// Set when the SNR_Next is the next object
											// to be loaded. The Scriptforwardfollower
											// will not take the succeeding object
											// at entry. Set upon an 
BOOL				B_NoPlayParse;			// as B_NoLoadParse
BOOL				B_MemProblem;

int 				RunState;				// Current Status of the player
											// RS_INIT, RS_CONTROL, RS_REMOVE

SNRSTACK			PlayStack,
					LoadStack;

// System setup vars
int 				SNRStackSize;
ULONG				TempoType;
BOOL				ObjectDateCheck;		// if true, date of object should be checked
											// to determine wether it should be displayed.
int 				PreLoadLevel;			// Preload level
ULONG				MaxMemSize;
int 				RunMode;				// Presentation/interactive or timecode mode


char SyncName[256];

// AREXX 
AREXXCONTEXT 		*RexxContext;
char 				PublicRexxReturn[100];


#define FOLLOW_FORWARD 1
#define FOLLOW_REVERSE 2
int					FollowDirection;

/****************************************************
*Func : Find the next free process dialogue for
*		sending a command to the child
*in   : PI -> PI of child
*out  : Ptr to free dialogue 
*		NULL -> no dialogue available
*/
PROCDIALOGUE *GetFreeDial( PI)
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
*Func : Find the next free Sync dialogue for
*		sending a command to the child
*in   : -
*out  : Ptr to free dialogue 
*		NULL -> no dialogue available
*/
SYNCDIALOGUE *GetFreeSyncDial( void)
{
  int i;

	for(i = 0; i < DIAL_MAXSYNC; i++)
		if(!Msg_SyncDial[i]->sd_InUse)
		{	
			Msg_SyncDial[i]->sd_InUse = TRUE;
			return( Msg_SyncDial[i]);
		}
	return(NULL);
}

/****************************************************
*Func : Find the next free GE dialogue for
*		sending a command to the child
*in   : -
*out  : Ptr to free dialogue 
*		NULL -> no dialogue available
*/
GEDIALOGUE *GetFreeGEDial( void)
{
  int i;

	for(i = 0; i < DIAL_MAXGE; i++)
		if(!Msg_GEDial[i]->gd_InUse)
		{	
			Msg_GEDial[i]->gd_InUse = TRUE;
			return( Msg_GEDial[i]);
		}

	return(NULL);
}

/*******************************************
*Func : Signal the control modules to abort
*		Send a DCC_DOTERM to the guides
*in   : PIList
*out  : -
*/
void TerminateChildren( void)
{
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;

	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		if((PI->pi_Node.ln_Type == NT_MODULE) && (PI->pi_State != ST_TERM))
			Signal((struct Task *)PI->pi_Process, SIGF_ABORT);
		else
		{ 	
			if(PI->pi_State != ST_TERM)
			{
				if(PD = GetFreeDial(PI))
				{
					PD->pd_ChildPI = NULL;
					PD->pd_Cmd = DCC_DOTERM;
					PD->pd_Msg.mn_ReplyPort = RepP_PtoC;
					PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
				}
			}
		}
	}
}

/**************************************************
*Func : Command all guides to free as much memory
*		as possible
*		The Main guide (SerGuide.1) however will
*		keep in memory as much as possible 
*/
void CleanupChildren( void)
{
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;

	for(PI = (PROCESSINFO *)PIList->lh_Head; 
		(PROCESSINFO *)PI->pi_Node.ln_Succ;	
		PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
	{
		if((PI->pi_Node.ln_Type != NT_MODULE) && (PI->pi_State != ST_TERM))
		{
			PI->pi_State = ST_INIT;	
			if(PD = GetFreeDial(PI))
			{
				PD->pd_ChildPI = NULL;
				if(PI->pi_SNR == Guide1SNR)
					PD->pd_Cmd = DCC_DOCLEANUPEASY;	
				else
					PD->pd_Cmd = DCC_DOCLEANUP;
				PD->pd_Msg.mn_ReplyPort = RepP_PtoC;
				PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
			}
		}
	}
}

/*************************************************
*Func : add a certain synchronizer to the system
*		depending on the type of synchronizing we do
*in   : SegList -> ptr to segmentlist
*		PIList -> ptr to proccessinfo list
*		ScriptInfo -> used to determine the synchrotype
*		RepP_WBStartup -> replyport for the synchro
*						  when terminating
*out  : Error
*/
int AddSyncProcessor( SegList, PIList, SIR, RepP_WBStartup, SyncPI)
struct List *SegList;
struct List *PIList;
struct ScriptInfoRecord *SIR;
struct MsgPort *RepP_WBStartup;
PROCESSINFO **SyncPI;
{
	switch(SIR->timeCodeFormat)
	{
		case TIMEFORMAT_HHMMSS:
						strcpy(SyncName,"HHMMSST_Timer");
						break;
		case TIMEFORMAT_MIDI:
						strcpy(SyncName,"MIDI_Timer");
						break;
		case TIMEFORMAT_MLTC:
						strcpy(SyncName,"HHMMSSFF_Timer");
						break;
		case TIMEFORMAT_SMPTE:
						strcpy(SyncName,"SMPTE_Timer");
						break;
		case TIMEFORMAT_CUSTOM:
						strcpy(SyncName,CPrefs.customTimeCode);
						break;
	}

#if _PRINTF
	printf("Adding module [%s] as synchronizer\n",SyncName);
#endif

	return(AddModule(SegList, PIList, SyncName, SIR, RepP_WBStartup,MainDirPath,SyncPI,MLSystem));
}

/*************************************************
*Func : add the global event processor to the system
*in   : SegList -> ptr to segmentlist
*		PIList -> ptr to proccessinfo list
*		ScriptInfo -> used by to geventproc for hotkeys
*		RepP_WBStartup -> replyport for the geventproc
*						  when terminating
*out  : Error
*/
int AddGlobalEventProcessor( SegList, PIList, SIR, RepP_WBStartup)
struct List *SegList;
struct List *PIList;
struct ScriptInfoRecord *SIR;
struct MsgPort *RepP_WBStartup;
{
	return(AddModule(SegList, PIList, "Input", SIR, RepP_WBStartup,MainDirPath,NULL,MLSystem));
}

/*************************************************
*Func : add the transition xapp to the module list
*		This overall transition xapp is used by
*		the animation xapp to show screen effects
*in   : SegList -> ptr to segmentlist
*		PIList -> ptr to proccessinfo list
*		ScriptInfo -> used by to geventproc for hotkeys
*		RepP_WBStartup -> replyport for the geventproc
*						  when terminating
*out  : Error
*/
int AddTransition( SegList, PIList, SIR, RepP_WBStartup)
struct List *SegList;
struct List *PIList;
struct ScriptInfoRecord *SIR;
struct MsgPort *RepP_WBStartup;
{
	return(AddModule(SegList, PIList, HOST_TRANSITIONS, SIR, RepP_WBStartup,MainDirPath,NULL,MLSystem));
}

/** VAR AREA **/

/****************************************************
*Func : generate a list with copies of all variables
*		used in the script. This is done since
*		all variables must hold their original values
*		after a script has been run
*in   : -
*out  : TRUE ->  list successfully generated
*		FALSE -> error
*/
BOOL MakeCopyOfVars( void)
{
  VIR *OrgVI, *NewVI;

	VIList = NULL;
	if( (VIList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(FALSE);	
	NewList(VIList);

	DelayedVIList = NULL;
	if( (DelayedVIList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(FALSE);	
	NewList(DelayedVIList);
	
	for(OrgVI = (VIR *)SIR->VIList.lh_Head; 
		(VIR *)OrgVI->vir_Node.ln_Succ;	
		OrgVI = (VIR *)OrgVI->vir_Node.ln_Succ)
	{
		if( (NewVI = (VIR *)AllocMem(sizeof(VIR), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
			return(FALSE);
		*NewVI = *OrgVI;
		AddTail(VIList,(struct Node *)NewVI);

		if( (NewVI = (VIR *)AllocMem(sizeof(VIR), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
			return(FALSE);
		*NewVI = *OrgVI;
		AddTail(DelayedVIList,(struct Node *)NewVI);

	}
	return(TRUE);
}

/*********************************************************
*Func : Free the copies of the variables
*<!>	Note that the current values will be lost
*in   : -
*out  : -
*/
void FreeCopyOfVars( void)
{
  VIR *VI, *NextVI;

	if(VIList != NULL)
	{
		VI = (VIR *)VIList->lh_Head;
		while( NextVI = (VIR *)VI->vir_Node.ln_Succ)
		{
			FreeMem(VI, sizeof(VIR));
			VI = NextVI;
		}

		FreeMem(VIList,sizeof(struct List));
	}

	if(DelayedVIList != NULL)
	{
		VI = (VIR *)DelayedVIList->lh_Head;
		while( NextVI = (VIR *)VI->vir_Node.ln_Succ)
		{
			FreeMem(VI, sizeof(VIR));
			VI = NextVI;
		}

		FreeMem(DelayedVIList,sizeof(struct List));
	}
}

/***************************************************
*Func : Copy all variables from the VIList into
*		the DelayedVIList.
*in   : -
*out  : -
*/
void MakeCopyOfDelayedVars( void)
{
  VIR *ChgVI, *DlyVI, *NextChgVI, *NextDlyVI;

#if _PRINTF
	PrintSer("Copiing delayed vars\n");
#endif

	DlyVI = (VIR *)DelayedVIList->lh_Head;
	ChgVI = (VIR *)VIList->lh_Head;
	while( NextDlyVI = (VIR *)DlyVI->vir_Node.ln_Succ)
	{
#if _PRINTF
		printf("Copy [%s] to [%s]\n",ChgVI->vir_Name,DlyVI->vir_Name);
#endif
		DlyVI->vir_Contents = ChgVI->vir_Contents;	

		ChgVI = (VIR *)ChgVI->vir_Node.ln_Succ;
		DlyVI = NextDlyVI;
	}
}

/**********************************************
*Func : Answer the VARCONTENTS area of a variable
*		in to copied VIList list.
*in   : aVir -> ptr to a vir
*out  : VARCONTENTS -> ptr to a var
*/
VARCONTENTS *VCCopyOf(aVir)
VIR *aVir;
{
  VIR *VI;

	for(VI = (VIR *)VIList->lh_Head; 
		(VIR *)VI->vir_Node.ln_Succ;	
		VI = (VIR *)VI->vir_Node.ln_Succ)
	{
		if(!strnicmp(VI->vir_Name,aVir->vir_Name,20))
			return(&VI->vir_Contents);
	}
	return(NULL);
}

/***************************************************
*Func : add to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Add( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
  char Str[101];

	if(Src1->vc_Type == VCT_STRING)
	{
		// string + string
		if(Src2->vc_Type == VCT_STRING)
		{
			strcpy(Str,Src1->vc_Value.vv_String);
			strcat(Str,Src2->vc_Value.vv_String);
			strncpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else if(Src2->vc_Type == VCT_INTEGER)	// string + integer
		{
			sprintf(Str,"%s%d",Src1->vc_Value.vv_String,Src2->vc_Value.vv_Integer);			
			strncpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else	// VCT_EMPTY
		{
			strncpy(Dest->vc_Value.vv_String,Src1->vc_Value.vv_String,49);
			Dest->vc_Type = VCT_STRING;
		}
	}
	else
	{
		// integer + string
		if(Src2->vc_Type == VCT_STRING)
		{
			sprintf(Str,"%d%s",Src1->vc_Value.vv_Integer,Src2->vc_Value.vv_String);			
			strncpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else if(Src2->vc_Type == VCT_INTEGER)	// integer + integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer + Src2->vc_Value.vv_Integer; 
		}
		else	// VCT_EMPTY
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : subtract to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Subtract( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string - string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string - integer
		{
		}
	}
	else
	{
		// integer - string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer - integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer - Src2->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : multiply to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Multiply( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string * string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string * integer
		{
		}
	}
	else
	{
		// integer * string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer * integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer * Src2->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : divide to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Divide( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string / string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string / integer
		{
		}
	}
	else
	{
		// integer / string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer / integer
		{
			Dest->vc_Type = VCT_INTEGER;
			if(Src2->vc_Value.vv_Integer)
				Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer / Src2->vc_Value.vv_Integer;
			else
				Dest->vc_Value.vv_Integer = 0; 
		}
	}
}

/******************************************************
*Func : perform a calculation operand on a destination
*		C = A (oper) B
*in   : Src1 -> ptr to VARCONTENTS of A
*       Src2 -> ptr to VARCONTENTS of B
*		Dest -> ptr to VARCONTENTS of C
*
*		about complex var types:
*		C = A + B.	A = string: "plaatje"; B = integer: 3 => C = "plaatje3"
*		C = A + B.  A = integer: 3; B = string: "plaatje" => C = "3plaatje"
*
*		integer to integer just follows normal calc rules
*		"plaatje" + "test" => "plaatjetest"
*out  : -
*/ 
void CalculateOn(Src1,Src2,Dest,Oper)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
UWORD		Oper;
{
	switch(Oper)
	{
		case OPER_ADD:
				//PrintSer(" OPER ADD ");
				Add(Src1,Src2,Dest);
				break;
		case OPER_SUBTRACT:
				//PrintSer(" OPER SUB ");
				Subtract(Src1,Src2,Dest);
				break;
		case OPER_MULTIPLY:
				//PrintSer(" OPER MULT ");
				Multiply(Src1,Src2,Dest);
				break;
		case OPER_DIVIDE:
				//PrintSer(" OPER DIVIDE ");
				Divide(Src1,Src2,Dest);
				break;
		default:
				//PrintSer(" OPER ILLEGAL ");
				break;
	}
} 

/*****************************************************
*Func : Compare two VARCONTENTS struct
*		and return TRUE if the condition matches
*in   : Vc1
*		Vc2
*		Condition
*out  : True -> the condition was true
*		False -> the condition was false
*/
BOOL CheckCondition(Vc1, Vc2, Cond)
VARCONTENTS *Vc1,
			*Vc2;
UWORD Cond;
{
  int Res;

	// types must be the same
	if(Vc1->vc_Type == Vc2->vc_Type)
	{
		if(Vc1->vc_Type == VCT_STRING)
			Res = strcmpi(Vc1->vc_Value.vv_String,Vc2->vc_Value.vv_String);
		else
			Res = Vc1->vc_Value.vv_Integer - Vc2->vc_Value.vv_Integer;

		switch(Cond)
		{
			case COND_LESS:
					//PrintSer(" LESS ");
					if(Res < 0) return(TRUE);
					break;
			case COND_LESSEQUAL:
					//PrintSer(" LESSEQUAL ");
					if(Res <= 0) return(TRUE);
					break;
			case COND_EQUAL:
					//PrintSer(" EQUAL ");
					if(Res == 0) return(TRUE);
					break;
			case COND_GREATEREQUAL:
					//PrintSer(" GRTREQUAL ");
					if(Res >= 0) return(TRUE);
					break;
			case COND_GREATER:
					//PrintSer(" GREATER ");
					if(Res > 0) return(TRUE);
					break;
			case COND_NOTEQUAL:
					//PrintSer(" UNEQUAL ");
					if(Res != 0) return(TRUE);
					break;
			default:
					//PrintSer(" ILL ");
					break;
		}
	}
	return(FALSE);
}

/*******************************************************
*Func : check all conditions for a variable
*		and return a ptr to the VCR that caused the hit
*in   : CondList -> ptr to condition list
*		CheckVar -> Var that must be checked
*/
VCR *CheckConditionsOn(CondList,CheckVar)
struct List *CondList;
VARCONTENTS *CheckVar;
{
  VARCONTENTS *Vc;
  VCR *CurVCR;

	for(CurVCR = (VCR *)CondList->lh_Head; 
		(VCR *)CurVCR->vcr_Node.ln_Succ;	
		CurVCR = (VCR *)CurVCR->vcr_Node.ln_Succ)
	{
		if(CurVCR->vcr_CheckBits & VC_CHECKVIR)
			Vc = VCCopyOf(CurVCR->vcr_Check.vc_VIR);
		else
			Vc = &CurVCR->vcr_Check.vc_Direct;

		//PrintSer("Check On ");
		//PrintVC(Vc);

		if(CheckCondition(CheckVar, Vc,CurVCR->vcr_Condition))
			return(CurVCR);		
	}
	return(NULL);
}


/******************************************************
*Func : Print the varcontents
*in   : Vc -> ptr to VARCONTENTS
*out  : -
*/					
void PrintVC( VARCONTENTS *Vc)
{
#if _PRINTF
	if(Vc->vc_Type == VCT_STRING)
		PrintSer("Str[%s] ",Vc->vc_Value.vv_String);
	if(Vc->vc_Type == VCT_INTEGER)
		PrintSer("Int[%d] ",Vc->vc_Value.vv_Integer);
#endif
}

/******************************************************
*Func : For all expressions of the SNR, calculate the
*		new varaiable value
*in   : SNRVarList -> ptr to list of expressions
*out  : NULL -> no valid condition found
*		else -> ptr to VCR that caused the hit
*/					
VCR *CalculateVariablesOfSNR( SNRVarList)
struct List *SNRVarList;
{
  VAR *CurVar;
  VARCONTENTS *Source1, *Source2;
  VCR *HitVCR;

	//PrintSer("CalculateVariablesOfSNR 1\n");

	if(SNRVarList == NULL)
		return(NULL);

	HitVCR = NULL;
	for(CurVar = (VAR *)SNRVarList->lh_Head; 
		(VAR *)CurVar->var_Node.ln_Succ;	
		CurVar = (VAR *)CurVar->var_Node.ln_Succ)
	{
		//PrintSer("Calc on var\n");

		if(!(CurVar->var_SourceBits & SB_NOSOURCES))
		{
			if(CurVar->var_SourceBits & SB_SOURCE1VIR)
			{
				//PrintSer("VAR[%s] ",CurVar->var_Source1.vs_VIR->vir_Name);
				Source1 = VCCopyOf(CurVar->var_Source1.vs_VIR);
			}
			else
			{
				//PrintSer("DIR ");
				Source1 = &CurVar->var_Source1.vs_Direct;
			}

			//PrintVC(Source1);			

			if(CurVar->var_SourceBits & SB_SOURCE2VIR)
			{
				//PrintSer("VAR[%s] ",CurVar->var_Source2.vs_VIR->vir_Name);
				Source2 = VCCopyOf(CurVar->var_Source2.vs_VIR);
			}
			else
			{
				//PrintSer("DIR ");
				Source2 = &CurVar->var_Source2.vs_Direct;
			}
			//PrintVC(Source2);			
		}

		if(CurVar->var_CheckBeforeAssign)
		{
			HitVCR = CheckConditionsOn(&CurVar->var_CondList,VCCopyOf(CurVar->var_ResultVIR));
			if(!(CurVar->var_SourceBits & SB_NOSOURCES))
				CalculateOn(Source1,Source2,VCCopyOf(CurVar->var_ResultVIR),CurVar->var_Operator);
		}
		else
		{
			if(!(CurVar->var_SourceBits & SB_NOSOURCES))
				CalculateOn(Source1,Source2,VCCopyOf(CurVar->var_ResultVIR),CurVar->var_Operator);

			//PrintSer("RESVAR[%s] ",CurVar->var_ResultVIR->vir_Name);
			//PrintVC(VCCopyOf(CurVar->var_ResultVIR));
			//PrintSer("\n");

			HitVCR = CheckConditionsOn(&CurVar->var_CondList,VCCopyOf(CurVar->var_ResultVIR));
		}
		//PrintSer("\n");
		if(HitVCR)
			break;
	}

	//PrintSer("CalculateVariablesOfSNR 2 %x\n",HitVCR);

	return(HitVCR);
}

/** VAR AREA END **/

/****************************************************
*Func : Free all data for the processcontroller
*in   : Err -> Err upon return
 *out  : Err -> copy of input error
*/
int FreeProcessController( Err)
int Err;
{
  ULONG SigRecvd;
  int ChildError;

	if(PIList)
	{
		SNR_TrackPlay = NULL;
		TerminateChildren();

		SigR_WBStartup 	= 1 << RepP_WBStartup->mp_SigBit;
		while( (struct List *)PIList->lh_TailPred != (struct List *)PIList)
		{
			SigRecvd = Wait(SigR_WBStartup|SIGBREAKF_CTRL_E);

			if(SigRecvd & SigR_WBStartup)
				if((ChildError = RemoveChild()) != NO_ERROR)
					Err = ChildError;

			if(SigRecvd & SIGBREAKF_CTRL_E)
				break;
		}

		FreeMem(PIList,sizeof(struct List));
	}

	MLMMU_FreeMem(MainDirPath);
	if(RepP_PCtoGE)
		DeletePort(RepP_PCtoGE);
	if(Port_StoPC)
		DeletePort(Port_StoPC);	

	MLMMU_FreeMem(Msg_GEDial[0]);
	MLMMU_FreeMem(Msg_SyncDial[0]);

	if(RepP_PCtoS)
		DeletePort(RepP_PCtoS);
	if(RepP_PtoC)
		DeletePort(RepP_PtoC);
	if(Port_CtoP)
		DeletePort(Port_CtoP);
	if(RepP_WBStartup)
		DeletePort(RepP_WBStartup);
	if(Port_GEtoPC)
		DeletePort(Port_GEtoPC);	
	if(SegList)
		UnLoadMLSegments(SegList,OTT_AFTERLOAD);

	MLMMU_FreeMem(MLSystem);
	MLMMU_FreeMem(LoadStack.sn_Lower);
	MLMMU_FreeMem(PlayStack.sn_Lower);

	FreeARexx(RexxContext);

/** VAR AREA **/

	FreeCopyOfVars();

/** VAR AREA END **/

	return(Err);
}

/************************************************
*Func : open the MMU library and set the 
*		configuration.
*in   : -
*out  : NULL -> error
*		ptr to mlmmu.library
*/
#if 0
BOOL SetupMMU( void)
{
	if(MLMMULibBase != NULL)
	{
		// max 1 MByte
		MLMMU_SetMemRestrict(MaxMemSize);
		// MEMF_STAY-Memblks should stay in memory after a MLMMU_FreeMem was performed.
		// If set to FALSE also MEMF_STAY blks will be freed
		MLMMU_SetMemReside(TRUE);
		return(TRUE);
	}

	return(FALSE);
}
#endif

/*******************************************************
*Func : Initialise the processcontroller data
*in   : ScriptInfo -> Used for AddSynchro and AddGlobalevent
*out  : NO_ERROR -> ok
*		else error
*/
int InitProcessController( ScriptInfo)
struct ScriptInfoRecord *ScriptInfo;
{
  int i;
  PROCESSINFO *SyncPI;

	LoadStack.sn_Lower = NULL;
	PlayStack.sn_Lower = NULL;
	RexxContext		= NULL;
	MLSystem 		= NULL;
	Port_GEtoPC 	= NULL;
	RepP_WBStartup 	= NULL;
	PIList 			= NULL;
	Port_CtoP 		= NULL;
	RepP_PtoC 		= NULL;
	RepP_PCtoS 		= NULL;
	Msg_SyncDial[0]	= NULL;
	Msg_GEDial[0]	= NULL;
	Port_StoPC 		= NULL;
	MainDirPath		= NULL;
	Port_PCtoGE		= NULL;
	RepP_PCtoGE		= NULL;

	// If Arexx can't be initialized then simply ignore it	
	RexxContext = InitARexx();		

/** VAR AREA **/

	if(MakeCopyOfVars() == FALSE)
		return(ERR_MEMVILIST);

/** VAR AREA END **/
/*
	if(!SetupMMU())
		return(ERR_MLMMU);
*/
	if((MainDirPath = (char *)MLMMU_AllocMem(MAINDIRPATHSIZE,MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_MAINDIRMEM);
	getpath((BPTR)CPrefs.appdirLock, MainDirPath);
#if _PRINTF
	printf("Lock = %x,Main path = [%s]\n",(int)CPrefs.appdirLock,MainDirPath);
#endif

	// Allocate the MediaLink Play SNR stack
	if((PlayStack.sn_Lower =(SNRSTACKITEM *)MLMMU_AllocMem(sizeof(SNRSTACKITEM)*SNRStackSize,MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_SYSTEMMEM);
	PlayStack.sn_Ptr = PlayStack.sn_Lower;
	PlayStack.sn_Upper= (SNRSTACKITEM *)((ULONG)PlayStack.sn_Lower + (ULONG)(sizeof(SNRSTACKITEM)*SNRStackSize));

	// Allocate the MediaLink load SNR stack
	if((LoadStack.sn_Lower =(SNRSTACKITEM *)MLMMU_AllocMem(sizeof(SNRSTACKITEM)*SNRStackSize,MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_SYSTEMMEM);
	LoadStack.sn_Ptr = LoadStack.sn_Lower;
	LoadStack.sn_Upper= (SNRSTACKITEM *)((ULONG)LoadStack.sn_Lower + (ULONG)(sizeof(SNRSTACKITEM)*SNRStackSize));

	// Allocate the MediaLink global system structure	
	if((MLSystem =(MLSYSTEM *)MLMMU_AllocMem(sizeof(MLSYSTEM),MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_SYSTEMMEM);

/** VAR AREA **/

	// Make MLSystem->VIList point to the delayed copy of the VIR list.
	// This enables us to print variables on screen.
	MLSystem->VIList = (struct List *)DelayedVIList;

/** VAR AREA END **/

	// Load all Medialink resident-reentrant NaPP segments
	if(LoadMLSegments(SegList,CPrefs.appdirLock,OTT_AFTERLOAD) == NULL)
		return(ERR_MLSEGMENTS);

 	// Make a port through which the globalevent handler can reach the 
	// processcontroller.
	if( (Port_GEtoPC = (struct MsgPort *)CreatePort("Port_GEtoPC",0)) == NULL)
		return(ERR_PORTPROCCONT);

	// Make a replyport for our guide, used only to find out the guide
	// has terminated.
	// It doesn't have to be public since we will pass on the ptr to our
	// children ourselves.
	if( (RepP_WBStartup = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_REPPORTPROCCONT);

	// Actually, the processcontroller controls only 1 guide, but we have to be
	// consistent in our thoughts and therefore use a Listbased system anyway.
	// Set up a List for ProcessInfo structures
	if( (PIList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		return(ERR_MEMPILIST);
 	NewList(PIList);

	// Make a Msgport which the synchronizer may use to send us Synchronize signals
	if( (Port_StoPC = (struct MsgPort *)CreatePort("Port_StoPC",0)) == NULL)
		return(ERR_SYNCPORT);

	if((SCErr = AddSyncProcessor( SegList, PIList, ScriptInfo, RepP_WBStartup,&SyncPI)) != NO_ERROR)
		return(SCErr);

	// Port belonging to the ProcessController. The guides may send
	// messages to this port to reach the process controller
	if( (Port_CtoP = (struct MsgPort *)CreatePort("Port_ProcCont",0)) == NULL)
		return(ERR_CHILDTOPARENTPORT);

	// When the parent has sent a message to a child, the child may
	// reply through this port
	if( (RepP_PtoC = (struct MsgPort *)CreatePort(0,0)) == NULL) 
		return(ERR_REPPORTPARENTTOCHILD);

	// Replyport for synchronizer
	if( (RepP_PCtoS = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_REPPORT);

	// Replyport for global event processor
	if( (RepP_PCtoGE = (struct MsgPort *)CreatePort(0,0)) == NULL)
		return(ERR_REPPORT);

	// make a dialogue to communicate with the synchronizer
	if( (Msg_SyncDial[0] = (SYNCDIALOGUE *)MLMMU_AllocMem(DIAL_MAXSYNC * sizeof(SYNCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_SYNCDIALMEM);
	for(i = 1; i < DIAL_MAXSYNC; i++)
		Msg_SyncDial[i] = (SYNCDIALOGUE *)((ULONG)Msg_SyncDial[i-1] + (ULONG)sizeof(SYNCDIALOGUE));

	// make a dialogue to communicate with the Global Event Processor
	if( (Msg_GEDial[0] = (GEDIALOGUE *)MLMMU_AllocMem(DIAL_MAXGE * sizeof(GEDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
		return(ERR_NOMEM);
	for(i = 1; i < DIAL_MAXGE; i++)
		Msg_GEDial[i] = (GEDIALOGUE *)((ULONG)Msg_GEDial[i-1] + (ULONG)sizeof(GEDIALOGUE));

	// Initialise the globalevent handler
	if( (GEErr = AddGlobalEventProcessor( SegList, PIList, ScriptInfo, RepP_WBStartup)) != NO_ERROR)
		return(GEErr);

	// Initialise the transition xapp
	// used by the animation xapp to control screen effects
	if( (Err_Transition = AddTransition(SegList, PIList, ScriptInfo, RepP_WBStartup)) != NO_ERROR)
		return(Err_Transition);

	// Wait for a synchronizer port, time out after 5 seconds
	for(Cnt = 0; Cnt < 25; Cnt++)
	{
		if(SyncPI->pi_State == ST_TERM)
		{
#if _PRINTF
			printf("Synchro early terminated\n");
#endif
			break;
		}

		if( (Port_PCtoS = (struct MsgPort *)FindPort("Port_Synchro")) != NULL)
			break;
		else
			Delay(10);
	}
	if(Cnt == 25)
		return(ERR_SYNCHRO);

	// Wait for a Global event port, time out after 5 seconds
	for(Cnt = 0; Cnt < 25; Cnt++)
		if( (Port_PCtoGE = (struct MsgPort *)FindPort("Port_GlobEProc")) != NULL)
			break;
		else
			Delay(10);
	if(Cnt == 25)
		return(ERR_PORTPCTOGE);

	// Wait for the transition xapp, time out after 5 seconds
	for(Cnt = 0; Cnt < 25; Cnt++)
		if(FindPort("Port_Transition") != NULL)
			break;
		else
			Delay(10);

	if(Cnt == 25)
		return(ERR_PORTTRANSITION);

	return(NO_ERROR);
}

/***********************************************************
*Func : Find the next executable object 
*		This function is also used to find out if a new
*		guide has any executable objects in its list.
*in   : ObjList -> ptr to a list
*		CurObj -> Current Active object
*out  : Next object to become active
*		NULL -> No more objects to be run
*/
SNR *FindObject(ObjList, CurObj)
struct List *ObjList;
SNR *CurObj;
{
 	if(CurObj == NULL)
	{
		if(ObjList)
			CurObj = (SNR *)ObjList->lh_Head;
		else
			return(NULL);
	}	
	else
		CurObj = (SNR *)CurObj->node.ln_Succ;

	while(CurObj->node.ln_Succ != NULL)
	{
		switch(CurObj->nodeType)
		{
			case TALK_GOTO:
				break;
			// standard XaPP
			case TALK_ANIM:
			case TALK_AREXX:
			case TALK_DOS:
			case TALK_PAGE:
			case TALK_SOUND:
			case TALK_USERAPPLIC:
			case TALK_STARTPAR:
				return(CurObj);
				break;
			case TALK_STARTSER:
						if(FindObject(CurObj->list,NULL))
							return(CurObj);
				break;
		}
		CurObj = (SNR *)CurObj->node.ln_Succ;
	}

	return(NULL);
}

/**********************************************************
*Func : Scan the script and run all ser guides
*in   : CurList -> Ptr to list of SNRs
*out  : Error
*/
int AddGuides( CurList)
struct List *CurList;
{
  SNR *CurSNR;
  int Err;

	for(CurSNR = (SNR *)CurList->lh_Head;	
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
		if(CurSNR->nodeType == TALK_STARTSER)
		{
			if( (Err = AddGuide(MLMMULibBase,MLSystem,SegList,PIList,SIR,GT_SER,CurSNR,Port_CtoP,RepP_WBStartup,MainDirPath,PreLoadLevel)) != NO_ERROR)
				return(Err);
			if( (Err = AddGuides(CurSNR->list)) != NO_ERROR)
				return(Err);
		}
	}
	return(NO_ERROR);
}


/*********************************************************
*Func : Remove a child from the ProcessInfo list and
*		clean up its process
*in   : -
*out  : Error nr
*/
int RemoveChild( void)
{
  PROCESSINFO *CurPI;
  int Err;

	Err = NO_ERROR;
	// Get the messages from the childprocesses that just terminated itself
	while( (Msg_RWBStartup = (struct WBStartup *)GetMsg(RepP_WBStartup)) != NULL)
	{
		CurPI = (PROCESSINFO *)((ULONG)Msg_RWBStartup - (sizeof(struct Node)) - sizeof(UWORD));
#if _PRINTSER
		PrintSer("PC> removing [%s], error %d\n",CurPI->pi_Name, CurPI->pi_Arguments.ar_RetErr);
#endif
		if(CurPI->pi_Arguments.ar_RetErr != NO_ERROR)
			Err = CurPI->pi_Arguments.ar_RetErr;

		Remove((struct Node *)CurPI);
		MLUnLoadSeg(SegList,CurPI->pi_Segment); 
		UnLock(CurPI->pi_Startup.sm_ArgList[0].wa_Lock);
		DeleteTaskPort(CurPI->pi_Port_PtoC);
		MLMMU_FreeMem(CurPI->pi_PtoCDial[0]);
		MLMMU_FreeMem(CurPI->pi_Startup.sm_ArgList);
		MLMMU_FreeMem(CurPI->pi_Name);
		MLMMU_FreeMem(CurPI);
	}
	return(Err);
}

/***************************************************
*Func : Get the reply from a child
*in   : -
*out  : -
*/
void ChildReply( void)
{
  PROCDIALOGUE *Msg_RProcDial;			// Message received from child  

	while( (Msg_RProcDial = (PROCDIALOGUE *)GetMsg(RepP_PtoC)) != NULL)
	{
		switch(Msg_RProcDial->pd_Cmd)
		{
			// Common guide commands
			case DCI_IGNORE:
#if _PRINTF
						printf("PC> received DCI_IGNORE reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDTERM:
#if _PRINTF
						printf("PC> received DCI_CHILDTERM reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						Msg_RProcDial->pd_ChildPI->pi_State = ST_TERM;
						break;
			case DCI_CHILDCLEANUP:
#if _PRINTF
						printf("PC> received DCI_CHILDCLEANUP reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;	
			// Special object commands
			case DCI_TERMALLOBJS:
#if _PRINTF
						printf("PC> received DCI_TERMALLOBJS reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_OBJRUNS:
#if _PRINTF
						printf("PC> received DCI_OBJRUNS reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_OBJHOLDS:
#if _PRINTF
						printf("PC> received DCI_OBJHOLDS reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_OBJTERMS:
#if _PRINTF
						printf("PC> received DCI_OBJTERMS reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						// don't send a new punch if this SNR is part of a
						// parallel level.
						if(Msg_RProcDial->pd_Luggage.lu_SNR != NULL)
						{
#if _PRINTF
							printf("Received delayed reply\n");
#endif
							if(Msg_RProcDial->pd_Luggage.lu_SNR->ParentSNR->nodeType != TALK_STARTPAR)
							{
#if _PRINTF
								printf("Received OBJTERMS for page %d, sending new punches\n",Msg_RProcDial->pd_Luggage.lu_SNR->PageNr);
#endif
								B_PunchObj = TRUE;
							}
						}
						break;
			case DCI_OBJLOAD:
#if _PRINTF
						printf("PC> received DCI_OBJLOAD reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			default:	
#if _PRINTF
						printf("PC> received unknown reply from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
		}
		Msg_RProcDial->pd_InUse = FALSE;

	}
}

/*****************************************************
*Func : Get a request from a child
*in   : -
*out  : -
*/
void ChildTalks( void)
{
  PROCDIALOGUE *Msg_RProcDial;			// Message received from child  

	// Get the messages from the children
	while( (Msg_RProcDial = (PROCDIALOGUE *)GetMsg(Port_CtoP)) != NULL)
	{
		switch(Msg_RProcDial->pd_Cmd)
		{
			case DCI_CHILDREADY:
						Msg_RProcDial->pd_Cmd = DCC_IGNORE;
						Msg_RProcDial->pd_ChildPI->pi_State = ST_READY;
#if _PRINTF
						printf("PC> received DCI_CHILDREADY from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_CHILDREADY_MEMPROBLEM:
						Msg_RProcDial->pd_Cmd = DCC_IGNORE;
						Forbid();
						CleanupChildren();
						B_MemProblem = TRUE;
						Permit();
#if _PRINTF
						printf("PC> received DCI_CHILDREADY_MEMPROBLEM from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
			case DCI_SEVEREPROBLEM:
						B_Terminate = TRUE;
						Msg_RProcDial->pd_Cmd = DCC_IGNORE;
#if _PRINTF
						printf("PC> received DCI_SEVEREPROBLEM from [%s]\n",Msg_RProcDial->pd_ChildPI->pi_Name);
#endif
						break;
		}
		Msg_RProcDial->pd_ChildPI = NULL;
		ReplyMsg((struct Message *)Msg_RProcDial);
	}
}

/*****************************************************
*Func : Analyse a global message as received from
*		the global event processor or the Arexx
*		interpreter
*in   : Msg_GlobDial -> Ptr to GlobalEventDialoge
*		NoRMC -> Check for RunMode,	TRUE -> check off (used by Rexx)
*									FALSE-> check on  (used by GlobEProc)	
*out  : -
*/
void AnalyseGlobalMsg( Msg_GlobDial, NoRMC)			
GEVENTDIALOGUE *Msg_GlobDial;
BOOL NoRMC;
{
	switch(Msg_GlobDial->gd_Cmd)
	{
		case RCMDSTOPnJUMP:
				if(NoRMC || (RunMode != RM_TIMECODE))
				{		
					FollowDirection = FOLLOW_FORWARD;
					SNR_GE = Msg_GlobDial->gd_Label.gl_SNR;
					B_LoadObj = TRUE;
					B_PlayObj = TRUE;
					B_PunchObj = TRUE;
					B_SNRGE = TRUE;
					if(
					   SNR_GE->nodeType == TALK_STARTPAR ||
					   SNR_GE->nodeType == TALK_STARTSER
					  )
					{
						B_NoPlayParse = TRUE;
						B_NoLoadParse = TRUE;
					}
				}
				break;	
		case RCMDNEXTOBJ:
				if(NoRMC || (RunMode != RM_TIMECODE))
				{		
					if(FollowDirection == FOLLOW_REVERSE)
					{
						FollowDirection = FOLLOW_FORWARD;
						SNR_Load = SNR_Play;
					}
					if(SNR_Play)
						SNR_GE = SNR_Play;
					B_LoadObj = TRUE;
					B_PlayObj = TRUE;	
					B_PunchObj = TRUE;
					B_SNRGE = TRUE;
				}	
				break;
		case RCMDPREVOBJ:
				if(NoRMC || (RunMode != RM_TIMECODE))
				{		
					if(FollowDirection == FOLLOW_FORWARD)
					{
						FollowDirection = FOLLOW_REVERSE;
						SNR_Load = SNR_Play;
					}
					if(SNR_Play)
						SNR_GE = SNR_Play;
					B_LoadObj = TRUE;
					B_PlayObj = TRUE;
					B_PunchObj = TRUE;
					B_SNRGE = TRUE;
				}
				break;
		case RCMDESCAPE:
				B_Terminate = TRUE;
				break;
		default :
				SNR_GE = NULL;
	}
}

/********************************************************
*Func : Get a new SNR from the Global Event processor
*in   : -
*out  : SNR in SNR_GE
*/
void GETalks( void)
{
  GEVENTDIALOGUE *Msg_RGEDial;

	// Get the messages from the global event processor
	while( (Msg_RGEDial = (GEVENTDIALOGUE *)GetMsg(Port_GEtoPC)) != NULL)
	{
#if _PRINTF
		printf("PC> Command from GlobEProc = %d\n",(int)Msg_RGEDial->gd_Cmd);
#endif
		if(!B_SNRGE)
		{	
			AnalyseGlobalMsg(Msg_RGEDial,FALSE);
	
			if(RunMode == RM_PRESENTATIONAUTO)
				RunMode = RM_PRESENTATIONMANUAL;
		}
		ReplyMsg((struct Message *)Msg_RGEDial);
	}
}

/*****************************************************
*Func : Scan the PageTalk info record list until
*		a eventlist is found. Then send the eventlist
*		to the globalevent processor
*in   : PTList -> ptr to PageTalkList
*out  : -
*/
void SendLocalEvents( PTList)
struct List *PTList;
{
  PTHEADER	 	*CurPTH;
  PER 			*CurPER;
  GEDIALOGUE	*GD;

	CurPER = NULL;
	
	if(PTList)
	{
		for(CurPTH = (PTHEADER *)PTList->lh_Head;	
			CurPTH->ph_Node.ln_Succ != NULL;
			CurPTH = (PTHEADER *)CurPTH->ph_Node.ln_Succ)
		{
			if(CurPTH->ph_NodeType == TALK_LOCALEVENT)
			{
				CurPER = (PER *)CurPTH;
				break;
			}
		}
	}

	if((GD = GetFreeGEDial()) != NULL)
	{
		if(CurPER)
		{
#if _PRINTF
			printf("Sending new local events\n");
#endif
			GD->gd_Cmd = EDC_NEWLOCALEVENTS;
			GD->gd_Luggage.gl_LocEvList = CurPER->er_LocalEvents;
		}
		else
		{
#if _PRINTF
			printf("clearing new local events\n");
#endif
			GD->gd_Cmd = EDC_CLRLOCALEVENTS;
		}
		PutMsg(Port_PCtoGE,(struct Message *)GD);
	}	
}

/**************************************************************
*Func : The synchronizer has something to say
*in   : -
*out  : NO_ERROR 
*		or ERR_QUIT_REQUEST
*/
int SynchroTalks(void)
{
  PROCESSINFO	*PI;
  SYNCDIALOGUE 	*Msg_RSyncDial;
  int			PunchType;
  SNR			*PunchSNR;
  PROCDIALOGUE	*CurPD;	
  BOOL			doDelay=FALSE; //, obsoletePunch=FALSE;

#if _PRINTSER
PrintSer("SynchroTalks 1\n");
#endif

	// for every punch we send to the synchronizer we will now receive one message
	while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg(Port_StoPC)) != NULL)
	{
		if(Msg_RSyncDial->sd_Cmd == SDI_PUNCHCOL)
		{
			PunchType = Msg_RSyncDial->sd_Punch->sp_PunchType;
			PunchSNR = Msg_RSyncDial->sd_Punch->sp_SNR;
			if(PunchType == PT_DORUN)
			{
#if _PRINTSER
				PrintSer("Received PT_DORUN for page %d from synchro\n",PunchSNR->PageNr);
				PrintSer(" page %d ProcInfo = %x, state = %d\n",PunchSNR->PageNr,(int)PunchSNR->ProcInfo,
						(int)((PROCESSINFO *)PunchSNR->ProcInfo)->pi_State);
				PrintSer("SNRPath = %s\n",PunchSNR->objectPath);
#endif
				if(CurPD = GetFreeDial((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo))
				{
					CurPD->pd_ChildPI = NULL;
					CurPD->pd_Cmd = DCC_DORUNOBJ;
					CurPD->pd_Luggage.lu_SNR = PunchSNR;
					CurPD->pd_Msg.mn_ReplyPort = RepP_PtoC;

					if(PunchSNR->ParentSNR->nodeType == TALK_STARTPAR)
						PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
					else
					{
						PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
						SNR_TrackPlay = SNR_Play = PunchSNR;
						B_PlayObj = TRUE;
					}

					if(PunchSNR->nodeType == TALK_PAGE)
						SendLocalEvents(PunchSNR->list);
				
					doDelay=FALSE;
					if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS &&
						PunchSNR->duration==0 )
						doDelay=TRUE;


					if (	doDelay && (PunchSNR->nodeType == TALK_PAGE ||
							PunchSNR->nodeType == TALK_ANIM) &&
						 	(PunchSNR->numericalArgs[2] != 0) )
						Delay(5L);
				}
			}
			else
				if(PunchType == PT_DOHOLD)
				{
#if _PRINTF
					printf("Received PT_DOHOLD for page %d from synchro\n",PunchSNR->PageNr);
					printf(" page %d ProcInfo = %x, state = %d\n",PunchSNR->PageNr,(int)PunchSNR->ProcInfo,
							(int)((PROCESSINFO *)PunchSNR->ProcInfo)->pi_State);
#endif

					if(CurPD = GetFreeDial((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo))
					{
						CurPD->pd_ChildPI = NULL;
						CurPD->pd_Cmd = DCC_DOHOLDOBJ;
						CurPD->pd_Luggage.lu_SNR = PunchSNR;
						CurPD->pd_Msg.mn_ReplyPort = RepP_PtoC;
						if(PunchSNR->ParentSNR->nodeType == TALK_STARTPAR)
							PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
						else
							PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
					}	
				}
				else
					if(PunchType == PT_DOSTOP)
					{
						MakeCopyOfDelayedVars();
#if _PRINTF
						printf("Received PT_DOSTOP for page %d from synchro\n",PunchSNR->PageNr);

						printf(" page %d ProcInfo = %x, state = %d\n",PunchSNR->PageNr,(int)PunchSNR->ProcInfo,
								(int)((PROCESSINFO *)PunchSNR->ProcInfo)->pi_State);
#endif	
						// The New punches are send to the synchronizer as soon
						// as the guide has replied and the next object is ready.

						if(CurPD = GetFreeDial((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo))
						{
							CurPD->pd_ChildPI = NULL;
							CurPD->pd_Cmd = DCC_DOTERMOBJ;
							CurPD->pd_Luggage.lu_SNR = PunchSNR;
							CurPD->pd_Msg.mn_ReplyPort = RepP_PtoC;

							if(PunchSNR->ParentSNR->nodeType == TALK_STARTPAR)
								PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
							else
								PutMsg(((PROCESSINFO *)PunchSNR->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);

							SNR_TrackPlay = NULL;

							if(PunchSNR->numericalArgs[0] == ARGUMENT_CONTINUE)
							{
					 			for(PI = (PROCESSINFO *)PIList->lh_Head;
									(PROCESSINFO *)PI->pi_Node.ln_Succ;
									PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
								{
									if((PI->pi_Node.ln_Type == NT_MODULE) && !strnicmp(PI->pi_Name,HOST_TRANSITIONS,sizeof(HOST_TRANSITIONS)))
									{
										PI->pi_State = -1;
										Signal((struct Task *)PI->pi_Process, SIGF_ABORT);
									}
								}
							}	
						}
						else
							B_PunchObj = TRUE;
					}
#if _PRINTSER
					else
					{
		 				//obsoletePunch=TRUE;
						PrintSer("RECEIVED obsolete syncpunch\n");
					}

				PrintSer("freeing Punch at %08x\n",(int)Msg_RSyncDial->sd_Punch);
#endif
			FreeMem(Msg_RSyncDial->sd_Punch,sizeof(SYNCPUNCH));
		}
		else
		{
			if(Msg_RSyncDial->sd_Cmd == SDI_EXTERNALEXIT)
			{
				Msg_RSyncDial->sd_Cmd = SDC_EXTERNALEXIT;
				return(ERR_QUIT_REQUEST);
			}
		}

		//if ( !obsoletePunch )
		ReplyMsg((struct Message *)Msg_RSyncDial);

	}

	return(NO_ERROR);
}

/*****************************************************************************
*Func : A reply on a message send to the synchronizer
*in   : -
*out  : -
*/	
void GetSynchroReply( void)
{
  SYNCDIALOGUE *Msg_RSyncDial;
  int HH,MM,SS,FF;  
  ULONG FrameCode;

	while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg(RepP_PCtoS)) != NULL)
	{
		switch(Msg_RSyncDial->sd_Cmd)
		{
			case SDI_TIMECODEGET:
					if(PublicRexxContext != NULL)
					{
						HH = ((int)Msg_RSyncDial->sd_Punch >> 24) & 0xff;
						MM = ((int)Msg_RSyncDial->sd_Punch >> 16) & 0xff;
						SS = ((int)Msg_RSyncDial->sd_Punch >> 8) & 0xff;
						FF = (int)Msg_RSyncDial->sd_Punch & 0xff;
						FrameCode = FF + SS*100 + MM*10000 + HH*1000000;
						sprintf(PublicRexxReturn,"%d",FrameCode);	

						SetARexxLastError(PublicRexxContext, PublicMsg_Arexx,NULL);
						ReplyARexxMsg(PublicRexxContext,PublicMsg_Arexx, PublicRexxReturn, 0);
						PublicRexxContext = NULL;
						PublicMsg_Arexx = NULL;
					}
					break;

			case SDI_TIMECODEGET_ASC:
					if(PublicRexxContext != NULL)
					{
						HH = ((int)Msg_RSyncDial->sd_Punch >> 24) & 0xff;
						MM = ((int)Msg_RSyncDial->sd_Punch >> 16) & 0xff;
						SS = ((int)Msg_RSyncDial->sd_Punch >> 8) & 0xff;
						FF = (int)Msg_RSyncDial->sd_Punch & 0xff;
						sprintf(PublicRexxReturn,"%02d:%02d:%02d:%02d",HH,MM,SS,FF);	

						SetARexxLastError(PublicRexxContext, PublicMsg_Arexx,NULL);
						ReplyARexxMsg(PublicRexxContext,PublicMsg_Arexx, PublicRexxReturn, 0);
						PublicRexxContext = NULL;
						PublicMsg_Arexx = NULL;
					}
					break;

			default:
					break;
		}
		Msg_RSyncDial->sd_InUse = FALSE;
	}
	// no reply since this IS the reply 
}

/*****************************************************************************
*Func : A reply on a message send to the Global Event Processor
*in   : -
*out  : -
*/	
void GetGEReply( void)
{
  GEDIALOGUE *Msg_RGEDial;
  
	while( (Msg_RGEDial = (GEDIALOGUE *)GetMsg(RepP_PCtoGE)) != NULL)
		Msg_RGEDial->gd_InUse = FALSE;
	// no reply since this IS the reply 
}

/************************************************************
*Func : Push a SNR onto the stack
*in   : SNR_Push -> SNR to be pushed 
*		JumpType
*		HitSNR -> see minc:types.h for detailed description
*out  : -
*/
void PushToStack( SNRStack, SNR_Push, JumpType, HitSNR)
SNRSTACK *SNRStack;
SNR *SNR_Push;
ULONG JumpType;
SNR *HitSNR;
{
	if(SNRStack->sn_Ptr == SNRStack->sn_Upper)
	{
		// Inform user, Stack overflow
		// Sever error, quit script
		ProcessError(ERR_GOSUBSTACKOVERFLOW);
		B_Terminate = TRUE;
	}
	else
	{
		if(PeekStackSNR( SNRStack) != SNR_Push)
		{
			SNRStack->sn_Ptr->si_SNR = SNR_Push;
			SNRStack->sn_Ptr->si_JumpType = JumpType;
			SNRStack->sn_Ptr->si_HitSNR = HitSNR;
			
			SNRStack->sn_Ptr++;
		}
	}
}

/************************************************************
*Func : Pull a SNR from the stack
*		
*in   : -
*out  : Next SNR from the Stack
*/
SNR *PullFromStack( SNRStack)
SNRSTACK *SNRStack;
{
	if(SNRStack->sn_Ptr == SNRStack->sn_Lower)
		return(NULL);
	else
	{
		SNRStack->sn_Ptr--;
		return(SNRStack->sn_Ptr->si_SNR);
	}
}

/************************************************************
*Func : Peek the next SNR on the stack, return the jumptype
*in   : -
*out  : JumpType of SNR on stack
*		or -1 if the stack was empty
*/
ULONG PeekStackJumpType( SNRStack)
SNRSTACK *SNRStack;
{
  ULONG JumpType;

	if(SNRStack->sn_Ptr == SNRStack->sn_Lower)
		return(-1);
	else
	{
		SNRStack->sn_Ptr--;
		JumpType = SNRStack->sn_Ptr->si_JumpType;
		SNRStack->sn_Ptr++;
	}
	return(JumpType);
}

/************************************************************
*Func : Peek the next SNR on the stack, return the value of
*		HitSNR
*in   : -
*out  : JumpType of SNR on stack
*		or null if the stack was empty
*/
SNR *PeekStackHitSNR( SNRStack)
SNRSTACK *SNRStack;
{
  SNR *HitSNR;

	if(SNRStack->sn_Ptr == SNRStack->sn_Lower)
		return(NULL);
	else
	{
		SNRStack->sn_Ptr--;
		HitSNR = SNRStack->sn_Ptr->si_HitSNR;
		SNRStack->sn_Ptr++;
	}

	return(HitSNR);
}

/************************************************************
*Func : Peek the next SNR on the stack
*in   : -
*out  : Next SNR on stack
*/
SNR *PeekStackSNR( SNRStack)
SNRSTACK *SNRStack;
{
  SNR *snr;

	if(SNRStack->sn_Ptr == SNRStack->sn_Lower)
		return(NULL);
	else
	{
		SNRStack->sn_Ptr--;
		snr = SNRStack->sn_Ptr->si_SNR;
		SNRStack->sn_Ptr++;
	}

	return(snr);
}

/********************************************************************
*Func : Take the next object from a serial level
*in   : SNR_Guide -> ptr to guide from which the executable object must
*					 be taken.
*		Direction -> take previous or next object from list
*		B_StorePlaySNR -> if TRUE, the new object will be stored in the PLayTrackSNR
*
*out  : next object to be used
*/
SNR *TakeIndexedObject( SNR_Guide, Direction, B_StorePlaySNR)
SNR *SNR_Guide;
UWORD Direction;
BOOL B_StorePlaySNR;
{
  SNR *SNR_Current;

	// empty list check
	if(SNR_Guide->list->lh_TailPred == (struct Node *)SNR_Guide->list)
		return(NULL);

	if(B_StorePlaySNR)
		SNR_Current = (SNR *)SNR_Guide->PlayTrackSNR;
	else
		SNR_Current = (SNR *)SNR_Guide->LoadTrackSNR;

	if(Direction == TG_SPNEXTGOSUB)
	{
		// if there is a next object then take it
		if(SNR_Current && SNR_Current->node.ln_Succ->ln_Succ)
			SNR_Current = (SNR *)SNR_Current->node.ln_Succ;
		else	// else take the first one from the list
			SNR_Current = (SNR *)SNR_Guide->list->lh_Head;

#if _PRINTSER
		PrintSer("TIO Next object = %d\n",SNR_Current->PageNr);
#endif
	}

	if(Direction == TG_SPPREVGOSUB)
	{
		// if there is a previous object then take it
		if(SNR_Current && SNR_Current->node.ln_Pred->ln_Pred)
			SNR_Current = (SNR *)SNR_Current->node.ln_Pred;
		else	// else take the last one from the list
			SNR_Current = (SNR *)SNR_Guide->list->lh_TailPred;

#if _PRINTSER
		PrintSer("TIO Previous object = %d\n",SNR_Current->PageNr);
#endif
	}

	if(B_StorePlaySNR)
		SNR_Guide->PlayTrackSNR = (ULONG)SNR_Current;
	else
		SNR_Guide->LoadTrackSNR = (int)SNR_Current;

	return(SNR_Current);
}

/**********************************************************
*Func : check if an objects' time is outdated. 
*		if so then set its OBJ_OUTDATED FLAG
*in   : CurSNR -> SNR to be checked
*out  : -
*info :
*	Day bits check
*	Sunday = b0
*	Monday = b1
*	|
*	Saturday = b6
*/
void DoObjectDateCheck( CurSNR)
SNR *CurSNR;
{
  struct DateStamp CurDate;
  LONG CurMinutesSince1978, ObjMinutesSince1978Start,ObjMinutesSince1978End;

	if(ObjectDateCheck)
	{
		CurSNR->miscFlags &= ~OBJ_OUTDATED;

    	DateStamp(&CurDate);
		if( CurSNR->dayBits & (1<<(CurDate.ds_Days % 7)) )
		{
			if(CurSNR->startendMode == ARGUMENT_CYCLICAL)
			{
#if _PRINTF
				printf("CurDay = %d, startDay = %d, endDay = %d\n",CurDate.ds_Days,CurSNR->Start.HHMMSS.Days,CurSNR->End.HHMMSS.Days);
#endif
				if((CurSNR->Start.HHMMSS.Minutes == 0) && (CurSNR->End.HHMMSS.Minutes == 0))
				{
					if(!(
						  (CurSNR->Start.HHMMSS.Days <= CurDate.ds_Days) &&
						  (CurSNR->End.HHMMSS.Days >= CurDate.ds_Days)
					  ) )	
						CurSNR->miscFlags |= OBJ_OUTDATED;
				}
				else
				{
					if(!(
						  (CurSNR->Start.HHMMSS.Days <= CurDate.ds_Days) &&
						  (CurSNR->End.HHMMSS.Days >= CurDate.ds_Days) &&
						  (CurSNR->Start.HHMMSS.Minutes <= CurDate.ds_Minute) &&
						  (CurSNR->End.HHMMSS.Minutes >= CurDate.ds_Minute)
					  ) )	
						CurSNR->miscFlags |= OBJ_OUTDATED;
				}
			}

			if(CurSNR->startendMode == ARGUMENT_CONTINUOUS)
			{
				CurMinutesSince1978 = (CurDate.ds_Days*1440)+CurDate.ds_Minute;
				ObjMinutesSince1978Start = (CurSNR->Start.HHMMSS.Days*1440)+CurSNR->Start.HHMMSS.Minutes;
				ObjMinutesSince1978End = (CurSNR->End.HHMMSS.Days*1440)+CurSNR->End.HHMMSS.Minutes;
#if _PRINTF
				printf("CurMin = %d, MinStart = %d, MinEnd  %d\n",CurMinutesSince1978,ObjMinutesSince1978Start,ObjMinutesSince1978End);
#endif

				if(!(
					  (ObjMinutesSince1978Start <= CurMinutesSince1978) &&
					  (ObjMinutesSince1978End >= CurMinutesSince1978) 
				  ) )
					CurSNR->miscFlags |= OBJ_OUTDATED;
			}
	
		}
		else
			CurSNR->miscFlags |= OBJ_OUTDATED;

#if _PRINTF
		if(CurSNR->miscFlags & OBJ_OUTDATED)
			printf("Pagenr %d is outdated\n",CurSNR->PageNr);
		else
			printf("Pagenr %d is in date\n",CurSNR->PageNr);
#endif
	}
}
 
/*****************************************************
*Func : Follow the script and return the next to be
*		played object.
*in   : SNR_Current -> Current object
*		SNRStack -> The stack that must be used
*		B_BuildStack -> If TRUE, this function was called only
*						to find the next executable script item. No Gosub/Goto
*						checking should be done.
*						Every time a GOSUB is encountered
*						this follower will push the next object
*						(whatever it may be) onto the stack.
*						This way we prevent the follower from
*						keep digging whenever a number of GOSUB
*						cmds follow directly upon eachother.
*out  : Next object to be played
*/
SNR *ScriptForwardFollower(SNR_Current,SNRStack, B_BuildStack, B_NoParse, B_FromPlayer)
SNR *SNR_Current;
SNRSTACK *SNRStack;
BOOL B_BuildStack;
BOOL B_NoParse;
BOOL B_FromPlayer;
{
  SNR *SNR_Entry, *SNR_Lower, *SNR_Stack, *SNR_Hit;

	SNR_Entry = SNR_Current;

	if(!B_NoParse)
	{
		if(!B_BuildStack && (PeekStackJumpType(SNRStack) == JT_SINGLE))
		{
			SNR_Current = PullFromStack(SNRStack);
#if _PRINTF
			printf("SINGLE PAge on Stack, PageNr = %d\n",SNR_Current->PageNr);
#endif
		}
		else
		{
		 	if(SNR_Current == NULL)
			{
				SNR_Current = (SNR *)fromSNR; //SIR->allLists[1]->lh_Head;
			}
			else
			{
				// If there is a next object in this list then take it
				if(((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ)
				{
					SNR_Current = (SNR *)SNR_Current->node.ln_Succ;
#if _PRINTF
					printf("Logical Page nr = %d\n",SNR_Current->PageNr);
#endif
				}
				else
				{
					// We're about to jump to the next object in a lower
					// level. Before doing so let's see if there is a
					// an object on the Stack. If so then take that
					// object as our next object.

					// but first, lets check the hitsnr of the ontop stack item
					SNR_Stack = NULL;
					if( (SNR_Hit = PeekStackHitSNR(SNRStack)) == NULL)
						SNR_Stack = PullFromStack(SNRStack);

#if _PRINTF
					printf("1SNR_Hit = %x, %s\n",(int)SNR_Hit,SNR_Hit ? ((PROCESSINFO *)SNR_Hit->ProcInfo)->pi_Name: " ");
#endif
					if(SNR_Stack == NULL)
					{					
						// Step backwards through the levels to find a level
						// with a valid object.
						for(SNR_Lower = SNR_Current->ParentSNR;
							SNR_Lower != Guide1SNR;
							SNR_Lower = SNR_Lower->ParentSNR)
						{
#if _PRINTF
							printf("SNR_lower = %x, %s\n",(int)SNR_Lower,SNR_Lower ? ((PROCESSINFO *)SNR_Lower->ProcInfo)->pi_Name: " ");
#endif

							if(SNR_Lower == SNR_Hit)
							{
#if _PRINTF
								printf("while stepping back, a level with Hit snr was found\n"); 
#endif
								SNR_Current = PullFromStack(SNRStack);
								SNR_Lower = NULL;
								break;
							}
							// Has the lower level a succeeding object then take it
							if(
								SNR_Lower->node.ln_Succ &&
						  		((SNR *)SNR_Lower->node.ln_Succ)->node.ln_Succ
						  	  )
							{
								SNR_Current = (SNR *)SNR_Lower->node.ln_Succ;
								break;
							}
						}
						// If this is the main level, take the first object
						// from this list.
						if(SNR_Lower == Guide1SNR)
						{
							if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS && TempoType == TT_PLAY)
								SNR_Current = (SNR *)SIR->allLists[1]->lh_Head;
							else
								return(NULL);
						}
					}
					else
						SNR_Current = SNR_Stack;
				}
			}
		}
	}

	if(B_BuildStack)
	{
		// check if object is outdated during loading time)
		if(!B_FromPlayer)
			DoObjectDateCheck(SNR_Current);

		if(!(SNR_Current->miscFlags & OBJ_OUTDATED))
		{
#if _PRINTF
			printf("STACK CHECK -> NEXT OBJECT = %d\n",(int)SNR_Current);
#endif
			return(SNR_Current);
		}
	}

	while(TRUE)
	{
		if(SetSignal(0,0) & Sig_GEtoPC)
			return(SNR_Entry);

		if ( SNR_Current->nodeType == TALK_GOTO )
		{
			// goto, gosubs and conditional jumps are all part of TALK_GOTO
			switch(SNR_Current->numericalArgs[0])
			{
				case TG_GOTO:
#if _PRINTF
						printf("GOTO to page %d [%s] [%s]\n",((SNR *)SNR_Current->extraData)->PageNr,
											SNR_Current->objectName,SNR_Current->objectPath);
#endif
						if ( SNR_Current->extraData != NULL )
							SNR_Current = (SNR *)SNR_Current->extraData;
						break;
				case TG_GOSUB:
#if _PRINTF
						printf("GOSUB to page %d\n",((SNR *)SNR_Current->extraData)->PageNr);
#endif
						if ( SNR_Current->extraData != NULL )
						{
							if(((SNR *)SNR_Current->extraData)->nodeType == TALK_STARTSER)
								PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_SUBLIST,(SNR *)SNR_Current->extraData);
							else
							{
								PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_FULLLIST,SNR_Current->ParentSNR);
#if _PRINTF	
								printf("Parent snr pushed to stack = %x\n",(int)SNR_Current->ParentSNR);
#endif
							}	
							SNR_Current = (SNR *)SNR_Current->extraData;
						}
						break;
				case TG_SPNEXTGOSUB:
				case TG_SPPREVGOSUB:
#if _PRINTF
						printf("SINGLEPAGEGOSUB to page %d\n",((SNR *)SNR_Current->extraData)->PageNr);
#endif
						if ( SNR_Current->extraData != NULL )
						{
							if(((SNR *)SNR_Current->extraData)->nodeType == TALK_STARTSER)
							{
								// push next object from original list onto the stack
								PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_SINGLE,NULL);
								// now take the next/prev executable object in row in the serial level
								if( (SNR_Current = TakeIndexedObject((SNR *)SNR_Current->extraData,SNR_Current->numericalArgs[0],B_FromPlayer)) == NULL)
									SNR_Current = (SNR *)SNR_Current->extraData;
							}
							else
							{
								// a NEXTGOSUB to a label will be interpreted
								// as a normal GOSUB.
								PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_FULLLIST,NULL);
								SNR_Current = (SNR *)SNR_Current->extraData;
							}
						}
						break;
			case TG_CONDITIONJUMP:
/** VAR AREA **/
						{
							VCR *HitVCR;

							if(B_FromPlayer && ((HitVCR = CalculateVariablesOfSNR(SNR_Current->list)) != NULL))
							{
								PrintSer("CONDJUMP to SNR %x, jumpType = %x\n",(int)HitVCR->vcr_LabelSNR,(int)HitVCR->vcr_JumpType);
								if(HitVCR->vcr_LabelSNR != NULL)
								{
									if(HitVCR->vcr_JumpType == TG_GOTO)
										SNR_Current = HitVCR->vcr_LabelSNR;
									else
									{
										if(HitVCR->vcr_JumpType == TG_GOSUB)
										{	
											if(HitVCR->vcr_LabelSNR->nodeType == TALK_STARTSER)
												PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_SUBLIST,HitVCR->vcr_LabelSNR);
											else
												PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_FULLLIST,SNR_Current->ParentSNR);
											SNR_Current = HitVCR->vcr_LabelSNR;
										}
										else
										{
											if(
												HitVCR->vcr_JumpType == TG_SPPREVGOSUB ||
											   	HitVCR->vcr_JumpType == TG_SPNEXTGOSUB
											  )
											{	
												if(HitVCR->vcr_LabelSNR->nodeType == TALK_STARTSER)
												{
												  int JumpDir;

													if(HitVCR->vcr_JumpType == TG_SPPREVGOSUB)
														JumpDir = TG_SPPREVGOSUB;
													else
														JumpDir = TG_SPNEXTGOSUB;

													// push next object from original list onto the stack
													PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_SINGLE,NULL);
													// now take the next/prev executable object in row in the serial level
													if( (SNR_Current = TakeIndexedObject(HitVCR->vcr_LabelSNR,JumpDir,B_FromPlayer)) == NULL)
													 	SNR_Current = HitVCR->vcr_LabelSNR;
												}
												else
												{
													// a NEXTGOSUB to a label will be interpreted
													// as a normal GOSUB.
													PushToStack(SNRStack, ScriptForwardFollower(SNR_Current,SNRStack, TRUE,FALSE,B_FromPlayer),JT_FULLLIST,NULL);
													SNR_Current = HitVCR->vcr_LabelSNR;
												}
											}
										}
									}
								}
							}
							else
							{
								// If there is a next object in this list then take it
								if(((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ)
								{
									SNR_Current = (SNR *)SNR_Current->node.ln_Succ;
#if _PRINTF
									printf("ffSNR Current = %x\n",(int)SNR_Current);
#endif
								}	
								else
								{
									// We're about to jump to the next object in a lower
									// level. Before doing so let's see if there is a
									// an object on the Stack. If so then take that
									// object as our next object.
			
									// but first, lets check the hitsnr of the ontop stack item
									SNR_Stack = NULL;
									if( (SNR_Hit = PeekStackHitSNR(SNRStack)) == NULL)
										SNR_Stack = PullFromStack(SNRStack);
#if _PRINTF
									printf("3SNR_Hit = %x, %s\n",(int)SNR_Hit,SNR_Hit ? ((PROCESSINFO *)SNR_Hit->ProcInfo)->pi_Name: " ");
#endif
									if(SNR_Stack == NULL)
									{					
										// Step backwards through the levels to find a level
										// with a valid object	
										for(SNR_Lower = SNR_Current->ParentSNR;
												SNR_Lower != Guide1SNR;
												SNR_Lower = SNR_Lower->ParentSNR)
										{
#if _PRINTF
											printf("3SNR_lower = %x, %s\n",(int)SNR_Lower,SNR_Lower ? ((PROCESSINFO *)SNR_Lower->ProcInfo)->pi_Name: " ");
#endif	
											if(SNR_Lower == SNR_Hit)
											{
#if _PRINTF			
												printf("while stepping back, a level with Hit snr was found\n"); 
#endif
												SNR_Current = PullFromStack(SNRStack);
												SNR_Lower = NULL;
												break;
											}
											// Has the lower level a succeeding object then take it
											if(
													SNR_Lower->node.ln_Succ &&
						  						((SNR *)SNR_Lower->node.ln_Succ)->node.ln_Succ
							  		  	)
											{	
												SNR_Current = (SNR *)SNR_Lower->node.ln_Succ;
												break;
											}
										}
										// If this is the main level, take the first object
										// from this list.
										if(SNR_Lower == Guide1SNR)
										{
											if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS && TempoType == TT_PLAY)
												SNR_Current = (SNR *)SIR->allLists[1]->lh_Head;
											else
												return(NULL);
										}
									}
									else
										SNR_Current = SNR_Stack;
								}
							}
						}
						break;
/** VAR AREA END **/
				default :
						if ( SNR_Current->extraData != NULL )
							SNR_Current = (SNR *)SNR_Current->extraData;
						break;
			}
		}
		else
		{

			if(
				((SNR_Current->nodeType == TALK_ANIM) ||
				 (SNR_Current->nodeType == TALK_AREXX) ||
				 (SNR_Current->nodeType == TALK_DOS) ||
				 (SNR_Current->nodeType == TALK_PAGE) ||
				 (SNR_Current->nodeType == TALK_SOUND) ||
				 (SNR_Current->nodeType == TALK_USERAPPLIC) ||
				 (SNR_Current->nodeType == TALK_STARTPAR) ||
				 (SNR_Current->nodeType == TALK_STARTSER))
			    )
			{
				if(!B_FromPlayer)
					DoObjectDateCheck(SNR_Current);	

				if(SNR_Current->nodeType != TALK_STARTSER)
				{
			    	if(!(SNR_Current->miscFlags & OBJ_OUTDATED)) 	
					{
#if _PRINTF
						printf("SFF -> NEXT OBJECT, in date, = %d\n",(int)SNR_Current);
#endif
						return(SNR_Current);
					}
				}
			}

			if(SNR_Current->nodeType == TALK_STARTSER && !(SNR_Current->miscFlags & OBJ_OUTDATED))
			{
#if _PRINTF
				printf("list->h_Head = %x\n",(int)SNR_Current->list->lh_Head);
				printf("list->h_Head->ln_succ = %x\n",(int)((SNR *)SNR_Current->list->lh_Head)->node.ln_Succ);
#endif
				if(	
					SNR_Current->list->lh_Head &&
					((SNR *)SNR_Current->list->lh_Head)->node.ln_Succ
				  ) 
					SNR_Current = (SNR *)SNR_Current->list->lh_Head;
				else
				{
#if _PRINTF
					printf("stepping back\n");
					printf("ParentSNR = %x, Guide1SNR = %x\n",(int)SNR_Current->ParentSNR, (int)Guide1SNR);
#endif
					// We're about to jump to the next object in a lower
					// level. Before doing so let's see if there is a
					// an object on the Stack. If so then take that
					// object as our next object.

					// but first, lets check the hitsnr of the ontop stack item
					SNR_Stack = NULL;
					if( (SNR_Hit = PeekStackHitSNR(SNRStack)) == NULL)
						SNR_Stack = PullFromStack(SNRStack);

#if _PRINTF
					printf("2SNR_Hit = %x, %s\n",(int)SNR_Hit,SNR_Hit ? ((PROCESSINFO *)SNR_Hit->ProcInfo)->pi_Name: " ");
#endif
					if(SNR_Stack == NULL)
					{					
						// Step backwards through the levels to find a level
						// with a valid object.

						for(SNR_Lower = SNR_Current;
							SNR_Lower != Guide1SNR;
							SNR_Lower = SNR_Lower->ParentSNR)
						{
#if _PRINTF
							printf("2SNR_lower = %x, %s\n",(int)SNR_Lower,SNR_Lower ? ((PROCESSINFO *)SNR_Lower->ProcInfo)->pi_Name: " ");
#endif
							if(SNR_Lower == SNR_Hit)
							{
#if _PRINTF
								printf("while stepping back, a level with Hit snr was found\n");
#endif
								SNR_Current = PullFromStack(SNRStack);
								SNR_Lower = NULL;
								break;
							}
							// Has the lower level a succeeding object then take it
							if(
								SNR_Lower->node.ln_Succ &&
							  	((SNR *)SNR_Lower->node.ln_Succ)->node.ln_Succ
							  )
							{	
								SNR_Current = (SNR *)SNR_Lower->node.ln_Succ;
								break;
							}
						}
						// If this is the main level, take the first object
						// from this list.
						if(SNR_Lower == Guide1SNR)
						{
#if _PRINTF
							printf("Below main level\n");
#endif
							if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS && TempoType == TT_PLAY)
								SNR_Current = (SNR *)SIR->allLists[1]->lh_Head;
							else
								return(NULL);
						}
					}
					else
						SNR_Current = SNR_Stack;	
				}
			}
			else
			{
#if _PRINTF
				printf("ffSNR_Current.ln_Succ = %x\n",(int)SNR_Current->node.ln_Succ);
				printf("ffSNR_Current.ln_Succ.ln_succ = %x\n",(int)((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ);
#endif
				// If there is a next object in this list then take it
				if(((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ)
				{
					SNR_Current = (SNR *)SNR_Current->node.ln_Succ;
#if _PRINTF
					printf("ffSNR Current = %x\n",(int)SNR_Current);
#endif
				}	
				else
				{
					// We're about to jump to the next object in a lower
					// level. Before doing so let's see if there is a
					// an object on the Stack. If so then take that
					// object as our next object.
	
					// but first, lets check the hitsnr of the ontop stack item
					SNR_Stack = NULL;
					if( (SNR_Hit = PeekStackHitSNR(SNRStack)) == NULL)
						SNR_Stack = PullFromStack(SNRStack);
#if _PRINTF
					printf("3SNR_Hit = %x, %s\n",(int)SNR_Hit,SNR_Hit ? ((PROCESSINFO *)SNR_Hit->ProcInfo)->pi_Name: " ");
#endif
					if(SNR_Stack == NULL)
					{					
						// Step backwards through the levels to find a level
						// with a valid object	
						for(SNR_Lower = SNR_Current->ParentSNR;
							SNR_Lower != Guide1SNR;
							SNR_Lower = SNR_Lower->ParentSNR)
						{
#if _PRINTF
							printf("3SNR_lower = %x, %s\n",(int)SNR_Lower,SNR_Lower ? ((PROCESSINFO *)SNR_Lower->ProcInfo)->pi_Name: " ");
#endif
							if(SNR_Lower == SNR_Hit)
							{
#if _PRINTF			
								printf("while stepping back, a level with Hit snr was found\n"); 
#endif
								SNR_Current = PullFromStack(SNRStack);
								SNR_Lower = NULL;
								break;
							}
							// Has the lower level a succeeding object then take it
							if(
								SNR_Lower->node.ln_Succ &&
						  		((SNR *)SNR_Lower->node.ln_Succ)->node.ln_Succ
							  )
							{	
								SNR_Current = (SNR *)SNR_Lower->node.ln_Succ;
								break;
							}
						}
						// If this is the main level, take the first object
						// from this list.
						if(SNR_Lower == Guide1SNR)
						{
							if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS && TempoType == TT_PLAY)
								SNR_Current = (SNR *)SIR->allLists[1]->lh_Head;
							else
								return(NULL);
						}
					}
					else
						SNR_Current = SNR_Stack;
				}
			}
		}
	}
#if _PRINTF
	printf("WEIRD EXIT -> NEXT OBJECT = %d\n",(int)SNR_Current);
#endif
}


/*****************************************************
*Func : Follow the script backwards and return the next to be
*		played object.
*in   : SNR_Current -> Take next object starting from this object
*out  : Next object to be played
*/
SNR *ScriptReverseFollower( SNR_Current, B_FromPlayer)
SNR *SNR_Current;
BOOL B_FromPlayer;
{
  SNR *SNR_Lower;

 	if(SNR_Current == NULL)
		SNR_Current = (SNR *)SIR->allLists[1]->lh_TailPred;
	else
	{
		// If the previous object is not the list structure then take the object
		// if pred of pred is null then prev node is List structure
		if(((SNR *)SNR_Current->node.ln_Pred)->node.ln_Pred)
			SNR_Current = (SNR *)SNR_Current->node.ln_Pred;
		else
		{
			// Step backwards through the levels to find a level
			// with a valid object.
			for(SNR_Lower = SNR_Current->ParentSNR;
				SNR_Lower != Guide1SNR;
				SNR_Lower = SNR_Lower->ParentSNR)
			{
				// Has the lower level a succeeding object then take it
				if(((SNR *)SNR_Lower->node.ln_Pred)->node.ln_Pred)
				{	
					SNR_Current = (SNR *)SNR_Lower->node.ln_Pred;
					break;
				}
			}
			// If this is the main level, take the last object
			// from this list.
			if(SNR_Lower == Guide1SNR)
				SNR_Current = (SNR *)SIR->allLists[1]->lh_TailPred;
		}
	}

	while(TRUE)
	{
		switch(SNR_Current->nodeType)
		{
			case TALK_ANIM:
			case TALK_AREXX:
			case TALK_DOS:
			case TALK_PAGE:
			case TALK_SOUND:
			case TALK_USERAPPLIC:
			case TALK_STARTPAR:
			case TALK_STARTSER:
						if(SNR_Current->nodeType != TALK_STARTSER)
						{
							if(!B_FromPlayer)
								DoObjectDateCheck(SNR_Current);	
	
				    		if(!(SNR_Current->miscFlags & OBJ_OUTDATED)) 	
							{
#if _PRINTF
								printf("REVERSE LOOP -> NEXT OBJECT = %d\n",(int)SNR_Current);
#endif
								return(SNR_Current);
							}
						}
						if(SNR_Current->nodeType == TALK_STARTSER)
						{
#if _PRINTF
							printf("list->h_Head = %x\n",(int)SNR_Current->list->lh_Head);
							printf("list->h_TailPred = %x\n",(int)SNR_Current->list->lh_TailPred);
#endif

							// check if new list is not empty
							if(((SNR *)SNR_Current->list->lh_Head)->node.ln_Succ)
								SNR_Current = (SNR *)SNR_Current->list->lh_TailPred;
							else
							{
#if _PRINTF
								printf("stepping back\n");
								// Step backwards through the levels to find a level
								// with a valid object.
								printf("ParentSNR = %x, Guide1SNR = %x\n",(int)SNR_Current->ParentSNR, (int)Guide1SNR);
#endif
								for(SNR_Lower = SNR_Current;
									SNR_Lower != Guide1SNR;
									SNR_Lower = SNR_Lower->ParentSNR)
								{
									// Has the lower level a preceeding object then take it
#if _PRINTF
									printf("ParentSNR = %x, Guide1SNR = %x\n",(int)SNR_Lower->ParentSNR, (int)Guide1SNR);
									printf("SNR_Lower->node.ln_Succ = %x\n",(int)SNR_Lower->node.ln_Succ);
									printf("SNR_Lower->node.ln_Succ->node.ln_succ = %x\n",(int)((SNR *)SNR_Lower->node.ln_Succ)->node.ln_Succ);
#endif
									if(((SNR *)SNR_Lower->node.ln_Pred)->node.ln_Pred)
									{	
										SNR_Current = (SNR *)SNR_Lower->node.ln_Pred;
										break;
									}
								}
								// If this is the main level, take the first object
								// from this list.
								if(SNR_Lower == Guide1SNR)
								{
#if _PRINTF
									printf("Below main level\n");
#endif
									SNR_Current = (SNR *)SIR->allLists[1]->lh_TailPred;
								}
							}
						}
						else
						{
#if _PRINTF
							printf("SNR_Current.ln_Succ = %x\n",(int)SNR_Current->node.ln_Succ);
							printf("SNR_Current.ln_Succ.ln_succ = %x\n",(int)((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ);
#endif

							if(((SNR *)SNR_Current->node.ln_Pred)->node.ln_Pred)
								SNR_Current = (SNR *)SNR_Current->node.ln_Pred;
							else
							{
								// Step backwards through the levels to find a level
								// with a valid object.
								for(SNR_Lower = SNR_Current->ParentSNR;
									SNR_Lower != Guide1SNR;
									SNR_Lower = SNR_Lower->ParentSNR)
								{
									// Has the lower level a succeeding object then take it
									if(((SNR *)SNR_Lower->node.ln_Pred)->node.ln_Pred)
									{	
										SNR_Current = (SNR *)SNR_Lower->node.ln_Pred;
										break;
									}
								}
								// If this is the main level, take the last object
								// from this list.
								if(SNR_Lower == Guide1SNR)
									SNR_Current = (SNR *)SIR->allLists[1]->lh_TailPred;
							}
						}
						break;
			default:
#if _PRINTF
						printf("SNR_Current.ln_Succ = %x\n",(int)SNR_Current->node.ln_Succ);
						printf("SNR_Current.ln_Succ.ln_succ = %x\n",(int)((SNR *)SNR_Current->node.ln_Succ)->node.ln_Succ);
#endif

						if(((SNR *)SNR_Current->node.ln_Pred)->node.ln_Pred)
							SNR_Current = (SNR *)SNR_Current->node.ln_Pred;
						else
						{
							// Step backwards through the levels to find a level
							// with a valid object.
							for(SNR_Lower = SNR_Current->ParentSNR;
								SNR_Lower != Guide1SNR;
								SNR_Lower = SNR_Lower->ParentSNR)
							{
								// Has the lower level a succeeding object then take it
								if(((SNR *)SNR_Lower->node.ln_Pred)->node.ln_Pred)
								{	
									SNR_Current = (SNR *)SNR_Lower->node.ln_Pred;
									break;
								}
							}
							// If this is the main level, take the last object
							// from this list.
							if(SNR_Lower == Guide1SNR)
								SNR_Current = (SNR *)SIR->allLists[1]->lh_TailPred;
						}
						break;
		}
#if _PRINTF
		printf("Object = %d\n",(int)SNR_Current);
#endif
	}
}


/**************************************************************
*Func : Player -> Find out which object has to be played next.
*		This function gets its input from 3 different 
*		object followers.
*		1 - The script follower. Takes the next object from
*			the script.
*		2 - The GlobalEventProcessor. Takes an object from 
*			the label list after a key has been pressed.
*		3 - Objects from a PAGE object.
*in   : -
*out  : -
*/
void ObjectPlayer( void)
{
  SYNCDIALOGUE 	*Msg_RSyncDial;
  SYNCDIALOGUE *SD;
  PROCESSINFO *PI;
  PROCDIALOGUE *PD;
  SNR *SNR_Next;

	if(Port_PCtoS == NULL)
		return;

	if(B_PlayObj)
	{
#if _PRINTF
		printf("ObjectPlayer must play next object\n");
#endif
		if(SNR_GE)
			SNR_Script = SNR_GE;

		if(FollowDirection == FOLLOW_FORWARD)
			SNR_Next = ScriptForwardFollower(SNR_Script, &PlayStack, FALSE, B_NoPlayParse,TRUE);
		else
			if(FollowDirection == FOLLOW_REVERSE)
				SNR_Next = ScriptReverseFollower(SNR_Script,TRUE);
			else
				SNR_Next = NULL;

		B_NoPlayParse = FALSE;

#if _PRINTF
		printf("PLAYER Next pagenr = %d, out SFF\n",SNR_Next->PageNr);
#endif
		// SNR_Next now contains the next object to be played
		// Send the punches of this new object to the synchronizer
		SNR_Punch = SNR_Next;

		// EXCEPTION
		// If this SNR is from the GlobalEvent processor then we have to
		// remove the current active punches
		if(B_SNRGE && SNR_Play)
		{
			if(SD = GetFreeSyncDial())
			{
				SD->sd_Cmd = SDC_ABORTPUNCHES;
				PutMsg(Port_PCtoS,(struct Message *)SD);
			}

			// remove current available syncmessages
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg(Port_StoPC)) != NULL)
				FreeMem(Msg_RSyncDial->sd_Punch,sizeof(SYNCPUNCH));
#if _PRINTF
			printf("New SNR from GE, terminating old objs\n");
#endif
 			for(PI = (PROCESSINFO *)PIList->lh_Head;
				(PROCESSINFO *)PI->pi_Node.ln_Succ;
				PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
			{
#if _PRINTF
				printf("PI->pi_Name = [%s], type = %d\n",PI->pi_Name,PI->pi_Node.ln_Type);
#endif
				if((PI->pi_Node.ln_Type == NT_MODULE) && !strnicmp(PI->pi_Name,HOST_TRANSITIONS,sizeof(HOST_TRANSITIONS)))
				{
#if _PRINTF
					printf("Signalling HOST_TRANSITION\n");	
#endif
					PI->pi_State = -1;
					Signal((struct Task *)PI->pi_Process, SIGF_ABORT);
				}
				if(PI->pi_Node.ln_Type == NT_SERGUIDEPROC)
				{
					if(PD = GetFreeDial(PI))
					{
						PD->pd_ChildPI = NULL;
						PD->pd_Cmd = DCC_DOTERMALLOBJS;
						PD->pd_Luggage.lu_SNR = SNR_Next;		// if in our list, don't terminate this object
						PD->pd_Msg.mn_ReplyPort = RepP_PtoC;
						PutMsg(PI->pi_Port_PtoC,(struct Message *)PD);
					}
				}
			}
		
			SNR_TrackPlay = SNR_Play = NULL;
			B_PunchObj = TRUE;
		}
		B_SNRGE = FALSE;

		// Pass current Script snr onto the ScriptFollower
		SNR_Script = SNR_Next;
		B_PlayObj = FALSE;
		SNR_GE = NULL;
	}
}

/********************************************************
*Func : Take care of loading all modules 
*		which may be next in line.
*in   : -
*out  : -
*/
void ObjectLoader( void)
{
  SNR *SNR_Next;
  PROCDIALOGUE *CurPD;

	if(B_LoadObj)
	{
#if _PRINTF
		printf("ObjectLoader must load next object\n");
#endif
		// if there is an object from the global event processor then load it
		// immediately.
		if(SNR_GE)
			SNR_Load = SNR_GE;

		if(FollowDirection == FOLLOW_FORWARD)
			SNR_Next = ScriptForwardFollower(SNR_Load,&LoadStack,FALSE,B_NoLoadParse,FALSE);
		else
		{
			if(FollowDirection == FOLLOW_REVERSE)
				SNR_Next = ScriptReverseFollower(SNR_Load,FALSE);
			else
				SNR_Next = NULL;
		}
		B_NoLoadParse = FALSE;

#if _PRINTF
		printf("LOADER out SFF, pagenr to be loaded = %d\n\n",SNR_Next->PageNr);
#endif
		B_LoadObj = FALSE;

		// SNR_Next now contains the next object to be loaded
		// Send the object to its guide.
		if(SNR_Next)
		{
			if(SNR_Next->ProcInfo == NULL)
			{
				if(CurPD = GetFreeDial((PROCESSINFO *)SNR_Next->ParentSNR->ProcInfo))
				{
					CurPD->pd_ChildPI = NULL;
					CurPD->pd_Cmd = DCC_DOLOADOBJ;
					CurPD->pd_Luggage.lu_SNR = SNR_Next;
					CurPD->pd_Msg.mn_ReplyPort = RepP_PtoC;
					PutMsg(((PROCESSINFO *)SNR_Next->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);
#if _PRINTF
					printf("Requesting DCC_DOLOADOBJ for page %d\n",SNR_Next->PageNr);
#endif
				}
			}
			else
			{
#if _PRINTF
				printf("LOADPAGE -> Page %d , state %d already loaded\n",SNR_Next->PageNr,((PROCESSINFO *)SNR_Next->ProcInfo)->pi_State);
#endif
#if 0
				// The next object does already exist, 
				// Its guide must tell it to prepare itself for the next run.
				if(CurPD = GetFreeDial((PROCESSINFO *)SNR_Next->ParentSNR->ProcInfo))
				{
					CurPD->pd_ChildPI = NULL;
					CurPD->pd_Cmd = DCC_DOPREPOBJ;
					CurPD->pd_Luggage.lu_SNR = SNR_Next;
					CurPD->pd_Msg.mn_ReplyPort = RepP_PtoC;
					PutMsg(((PROCESSINFO *)SNR_Next->ParentSNR->ProcInfo)->pi_Port_PtoC,(struct Message *)CurPD);

#if _PRINTF
					printf("Requesting DCC_DOPREPOBJ for page %d\n",SNR_Next->PageNr);
#endif
				}
#endif
				B_LoadObj = TRUE;
			}
		}
		SNR_Load = SNR_Next;
	}
}

/*********************************************************
*Func : Send the Punches for a obj to the synchronizer
*in   : -
*out  : -
*/
void ObjectPuncher( void)
{
  SYNCPUNCH *FirstPunch, *SecPunch;
  SYNCDIALOGUE *SD;
  ULONG HH, MM, SS, FF;

	if(Port_PCtoS == NULL || !B_PunchObj)
		return;

#if _PRINTF
	printf("PUNCHPAGE -> Page %d , state %d\n",SNR_Punch->PageNr,SNR_Punch->ProcInfo ? ((PROCESSINFO *)SNR_Punch->ProcInfo)->pi_State:-1);
#endif

	if(SNR_Punch)
	{	
		if(
			SNR_Punch->ProcInfo &&
			(
 			  (((PROCESSINFO *)SNR_Punch->ProcInfo)->pi_State == ST_READY) ||
 			  (((PROCESSINFO *)SNR_Punch->ProcInfo)->pi_State == ST_RUNNING)
			)
	  	  )
		{
			// make a punch list and send it to the synchronizer
			if( (FirstPunch = (SYNCPUNCH *)AllocMem(sizeof(SYNCPUNCH),MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
			{	// startpunch (starts right away)
	

				FirstPunch->sp_MinNode.mln_Pred = NULL;
				FirstPunch->sp_SNR = SNR_Punch;
				FirstPunch->sp_PunchType = PT_DORUN;
	
				if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
					FirstPunch->sp_PunchInfo.pi_HMSTDuration = 0;	// start right now
				else
				{
					HH = SNR_Punch->Start.TimeCode.HH;
					MM = SNR_Punch->Start.TimeCode.MM;
					SS = SNR_Punch->Start.TimeCode.SS;
					FF = SNR_Punch->Start.TimeCode.FF;
					FirstPunch->sp_PunchInfo.pi_FCFastCheck = HH<<24|MM<<16|SS<<8|FF;
				}
				/* insert punches for a object at this place, for example
				   when an object gets an extra list with punches, they
				   may be parsed and linked at this place. AFTER the startpunch
				   and BEFORE the endpunch */	
	
				if(RunMode != RM_PRESENTATIONMANUAL)
				{
					if( (SecPunch = (SYNCPUNCH *)AllocMem(sizeof(SYNCPUNCH),MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
					{	// stoppunch

						FirstPunch->sp_MinNode.mln_Succ = (struct MinNode *)SecPunch;
						SecPunch->sp_MinNode.mln_Pred = (struct MinNode *)FirstPunch;
						SecPunch->sp_SNR = SNR_Punch;
						SecPunch->sp_PunchType = PT_DOSTOP;

						if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
							SecPunch->sp_PunchInfo.pi_HMSTDuration = SNR_Punch->duration;
						else
						{
							HH = SNR_Punch->End.TimeCode.HH;
							MM = SNR_Punch->End.TimeCode.MM;
							SS = SNR_Punch->End.TimeCode.SS;
							FF = SNR_Punch->End.TimeCode.FF;
							SecPunch->sp_PunchInfo.pi_FCFastCheck = HH<<24|MM<<16|SS<<8|FF;
						}
					}
					else
					{
						FreeMem(FirstPunch,sizeof(SYNCPUNCH));
						FirstPunch = NULL;
					}	
				}
		
				if(FirstPunch)
				{
					// If these punches are for a PARALLEL OBJECT then send the 
					// punches of the objects within the level as well
					if(SNR_Punch->nodeType == TALK_STARTPAR)
						PunchParObjects(SNR_Punch->list);
#if _PRINTF	
					printf("Sending punches of Page %d to Synchro\n",SNR_Punch->PageNr);
#endif
					if(SD = GetFreeSyncDial())
					{
						SD->sd_Cmd = SDC_NEWPUNCH;
						SD->sd_Punch = FirstPunch;
						PutMsg(Port_PCtoS,(struct Message *)SD);
					}
				}
			}
			B_PunchObj = FALSE;
			SNR_Punch = NULL;
		}
		else
			Signal((struct Task *)FindTask(NULL),Sig_CtoP);

    if (SNR_Punch!=NULL && SNR_Punch->miscFlags & OBJ_OUTDATED) 	
		{
#if _PRINTF	
			printf("Page %d to be punched became outdated, skipping it now\n",SNR_Punch->PageNr);
#endif
			SNR_Punch = NULL;
			B_PunchObj = TRUE;
			B_PlayObj = TRUE;
		}
	}
}


/*********************************************************
*Func : Send the Punches for objects within a parallel guide
*		to the synchronizer
*in   : PI -> First Object for which new punches have to be send
*out  : -
*/
void PunchParObjects( ParList)
struct List *ParList;
{
  SYNCDIALOGUE *SD;
  int Error;
  SYNCPUNCH *FirstPunch, *NextPunch, *PrevPunch, *CurPunch;
  SNR *CurSNR;
  ULONG HH, MM, SS, FF;

#if _PRINTF
	printf("Also sending punches of parallel objects\n");
#endif

	FirstPunch = NULL;
	PrevPunch = NULL;
	Error = FALSE;

	for(CurSNR = (SNR *)ParList->lh_Head;	
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
		// make a punch list and send it to the synchronizer
		if( (CurPunch = (SYNCPUNCH *)AllocMem(sizeof(SYNCPUNCH),MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
		{
			CurPunch->sp_MinNode.mln_Pred = (struct MinNode *)PrevPunch;
			if(PrevPunch != NULL)
				PrevPunch->sp_MinNode.mln_Succ = (struct MinNode *)CurPunch;
			else
				FirstPunch = CurPunch;
			CurPunch->sp_SNR = CurSNR;
			CurPunch->sp_PunchType = PT_DORUN;

			if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
			{
#if _PRINTF
				printf("Sending Startpunch duration %d of parobject %d\n",(int)CurSNR->Start.ParHMSTOffset,CurSNR->PageNr);
#endif
				CurPunch->sp_PunchInfo.pi_HMSTDuration = CurSNR->Start.ParHMSTOffset;
			}
			else
			{
				HH = CurSNR->Start.TimeCode.HH;
				MM = CurSNR->Start.TimeCode.MM;
				SS = CurSNR->Start.TimeCode.SS;
				FF = CurSNR->Start.TimeCode.FF;
				CurPunch->sp_PunchInfo.pi_FCFastCheck = HH<<24|MM<<16|SS<<8|FF;
			}

			PrevPunch = CurPunch;
		}
		else
			Error = TRUE;

		if( (CurPunch = (SYNCPUNCH *)AllocMem(sizeof(SYNCPUNCH),MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
		{
			CurPunch->sp_MinNode.mln_Pred = (struct MinNode *)PrevPunch;
			PrevPunch->sp_MinNode.mln_Succ = (struct MinNode *)CurPunch;
			CurPunch->sp_SNR = CurSNR;
			CurPunch->sp_PunchType = PT_DOSTOP;

			if( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
			{
#if _PRINTF
				printf("Sending endpunch duration %d of parobject %d\n",(int)CurSNR->End.ParHMSTOffset,CurSNR->PageNr);
#endif
				CurPunch->sp_PunchInfo.pi_HMSTDuration = CurSNR->End.ParHMSTOffset;
			}
			else
			{
				HH = CurSNR->End.TimeCode.HH;
				MM = CurSNR->End.TimeCode.MM;
				SS = CurSNR->End.TimeCode.SS;
				FF = CurSNR->End.TimeCode.FF;
				CurPunch->sp_PunchInfo.pi_FCFastCheck = HH<<24|MM<<16|SS<<8|FF;
			}

			PrevPunch = CurPunch;
		}
		else
			Error = TRUE;

		if(Error)
			break;
	}
	if(Error)
	{
		for(CurPunch = FirstPunch; 
			CurPunch != NULL; 
			CurPunch = NextPunch)
		{	
			NextPunch = (SYNCPUNCH *)CurPunch->sp_MinNode.mln_Succ;
			FreeMem(CurPunch,sizeof(SYNCPUNCH));
		}
	}	
	else
	{
		if(FirstPunch != NULL)
		{
			if( (SD = GetFreeSyncDial()) != NULL)
			{
				SD->sd_Cmd = SDC_NEWPUNCH;
				SD->sd_Punch = FirstPunch;
				PutMsg(Port_PCtoS,(struct Message *)SD);
			}
		}
	}
}

/*****************************************************
*Func : Main entry for script processing
*		First of all, a Global event task is set up
*		with the available eventlist.
*		After that the synchronizer is started to provide 
*		proper sync signals.
*		Then the first scriptobject (always a STARTSER)
*		is processed
*in   : ScriptInfo -> ptr to ScriptInfoRecord for script information
*		Guide1SNR -> Ptr to SNR of the first guide. The SNR of
*					 this guide is made by PlayScript().
*out  : NO_ERROR
*		else errorcode
*/
int pc_ProcessScript( ScriptInfo, Guide1)
struct ScriptInfoRecord *ScriptInfo;
SNR *Guide1;
{
  int Err, ChildError;
  int Error;
  int i;
  PROCESSINFO *PI;
  ULONG SigWait;
  BOOL standByDone=FALSE;
  int tempErr=-1;

	// Initialise state
	RunState = RS_INIT;

	Guide1SNR = Guide1;
	SIR = ScriptInfo;

	// Follow the script from the first object to the last object
	FollowDirection = FOLLOW_FORWARD;	

	if(FindObject(ScriptInfo->allLists[1],NULL) == NULL)
		return(ERR_EMPTYSCRIPT);

	if( (Error = InitProcessController(SIR)) != NO_ERROR)
		return(FreeProcessController(Error));

	if( (Error = ProcessInitializer(ScriptInfo)) != NO_ERROR)
	{
		ProcessDeInitializer();
		return(FreeProcessController(Error));
	}

	for(i = 0; i < DIAL_MAXSYNC; i++)
	{
		// set up standard SyncDialogue information 
		Msg_SyncDial[i]->sd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		Msg_SyncDial[i]->sd_Msg.mn_Length = sizeof(SYNCDIALOGUE);
		Msg_SyncDial[i]->sd_Msg.mn_ReplyPort = RepP_PCtoS;
		Msg_SyncDial[i]->sd_ProcInfo = (PROCESSINFO *)-1L;
	}

	for(i = 0; i < DIAL_MAXGE; i++)
	{
		// set up standard GEDialogue information 
		Msg_GEDial[i]->gd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		Msg_GEDial[i]->gd_Msg.mn_Length = sizeof(GEDIALOGUE);
		Msg_GEDial[i]->gd_Msg.mn_ReplyPort = RepP_PCtoGE;
	}

	SigR_WBStartup 	= 1 << RepP_WBStartup->mp_SigBit;
	Sig_CtoP 		= 1 << Port_CtoP->mp_SigBit;
	SigR_PtoC 		= 1 << RepP_PtoC->mp_SigBit;
 	Sig_GEtoPC 		= 1 << Port_GEtoPC->mp_SigBit;
	SigR_PCtoS		= 1 << RepP_PCtoS->mp_SigBit;
	Sig_StoPC		= 1 << Port_StoPC->mp_SigBit;
	SigR_PCtoGE		= 1 << RepP_PCtoGE->mp_SigBit;
	if(RexxContext)
		Sig_RXtoPC	= 1 << RexxContext->ARexxPort->mp_SigBit;
	else
		Sig_RXtoPC = 0;

#if _PRINTF
	printf("First SNR = %x\n",(int)ScriptInfo->allLists[1]->lh_Head);

	printf("SIGBIT SigR_WBStartup   = %08x\n",SigR_WBStartup);
	printf("SIGBIT SigR_PtoC        = %08x\n",SigR_PtoC);
	printf("SIGBIT Sig_CtoP         = %08x\n",Sig_CtoP);
	printf("SIGBIT Sig_GEtoPC       = %08x\n",Sig_GEtoPC);
	printf("SIGBIT SigR_PCtoS       = %08x\n",SigR_PCtoS);
	printf("SIGBIT Sig_RXtoPC       = %08x\n",Sig_RXtoPC);
	printf("SIGBIT SigR_PCtoGE      = %08x\n",SigR_PCtoGE);
#endif

	SNR_TrackPlay = NULL;
	SNR_Play = NULL;	
	SNR_Load = NULL;
	SNR_Script = NULL;
 	SNR_Punch = NULL;	
	SNR_GE = NULL;

	B_NoPlayParse = FALSE;
	B_NoLoadParse = FALSE;
	B_PlayObj = FALSE;
	B_PunchObj = FALSE;	
	B_Terminate = FALSE;
	B_SNRGE = FALSE;
	B_MemProblem = FALSE;

	if( (Err = AddGuide(MLMMULibBase,MLSystem,SegList,PIList,SIR,GT_SER,Guide1SNR,Port_CtoP,RepP_WBStartup,MainDirPath,PreLoadLevel)) == NO_ERROR)
	{
		if( (Err = AddGuides(ScriptInfo->allLists[1])) == NO_ERROR)
		{
			SigWait = Sig_RXtoPC|Sig_CtoP|SIGF_ABORT;
			// Main loop, continue this loop until all processes have removed themselves
			while( (struct List *)PIList->lh_TailPred != (struct List *)PIList)
			{

if (RunState == RS_REMOVE)
{
#if _PRINTSER
PrintSer("Remove TerminateChildren\n");
#endif
}

				SigRecvd = Wait(SigWait);

				if ( CPrefs.standBy && !standByDone )
				{
					standByDone=TRUE;
					WaitInPlayScreen();
				}

				/*
					The mainloop has three parts
					RS_INIT	 	- Init of guides and their children
					RS_CONTROL 	- Running the show
					RS_REMOVE 	- Termination of the guides and wait for removal
				*/

				if(RunState == RS_INIT)
				{
					if(SigRecvd & Sig_RXtoPC);
						ArexxTalks(RexxContext,SIR);

					if(SigRecvd & Sig_CtoP)
						ChildTalks();

					B_PlayObj = TRUE;
					B_PunchObj = TRUE;	

					RunState = RS_CONTROL;
					SigWait = Sig_RXtoPC|SigR_PtoC|Sig_CtoP|Sig_StoPC|
									  Sig_GEtoPC|SigR_PCtoS|SigR_PCtoGE|SIGF_ABORT;

					for(PI = (PROCESSINFO *)PIList->lh_Head; 
						(PROCESSINFO *)PI->pi_Node.ln_Succ;
						PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
					{
						if(PI->pi_Node.ln_Type == NT_SERGUIDEPROC && PI->pi_State != ST_READY)
						{
							B_PlayObj = FALSE;
							B_PunchObj = FALSE;
							RunState = RS_INIT;
							SigWait = Sig_RXtoPC|Sig_CtoP|SIGF_ABORT;
						}
						// if any of the modules terminate then go to RS_REMOVE
						if(PI->pi_Node.ln_Type == NT_SERGUIDEPROC && PI->pi_State == ST_TERM)
						{
							SigWait = SigR_WBStartup|SigR_PtoC|Sig_CtoP|Sig_StoPC|
									  Sig_RXtoPC|Sig_GEtoPC|SigR_PCtoS|SIGF_ABORT;
							RunState = RS_REMOVE;
							if(PI->pi_Arguments.ar_RetErr != NO_ERROR)
								Err = PI->pi_Arguments.ar_RetErr;
							break;
						}
					}		
					// Last memory check, if there is too little memory then free
					// some of it.
#if 0
					if((RunState == RS_CONTROL) && B_MemProblem)
					{
						if(MLMMU_AvailMem(MEMF_PUBLIC) < 50000)	
						{
							if(MLMMU_PleaseFree(100000) < 50000)
								MLMMU_SetMemReside(FALSE);
						}
					}
#endif
				}

				if(RunState == RS_CONTROL)
				{
					for(PI = (PROCESSINFO *)PIList->lh_Head;
						(PROCESSINFO *)PI->pi_Node.ln_Succ;
						PI = (PROCESSINFO *)PI->pi_Node.ln_Succ)
					{
						// if any of the guides/modules terminates then go to RS_REMOVE
						if(PI->pi_State == ST_TERM)
						{
							SigWait = Sig_RXtoPC|SigR_WBStartup|SigR_PtoC|SIGF_ABORT;
							RunState = RS_REMOVE;
							if(PI->pi_Arguments.ar_RetErr != NO_ERROR)
								Err = PI->pi_Arguments.ar_RetErr;
							break;
						}
					}

					if(SigRecvd & Sig_RXtoPC);
						ArexxTalks(RexxContext,SIR);

					if(SigRecvd & SigR_PCtoGE)
						GetGEReply();

					if(SigRecvd & Sig_GEtoPC)
						GETalks();	

					if(SigRecvd & Sig_StoPC)
						if(SynchroTalks() == ERR_QUIT_REQUEST)
						{
							B_Terminate = TRUE;
							Err = ERR_QUIT_REQUEST;
						}

					if(SigRecvd & SigR_PCtoS)
						GetSynchroReply();

					if(SigRecvd & SigR_PtoC)
						ChildReply();
		
					if(SigRecvd & Sig_CtoP)
						ChildTalks();

#if _PRINTF
// BTW doing this gives for the first page an e-hit!
PrintSer("PageNr of SNR_PLAY = %d, PageNr of SNR_LOAD = %d, B_Load = %s\n", SNR_Play->PageNr,SNR_Load->PageNr,B_LoadObj ? "TRUE":"FALSE");
#endif

					if(SNR_Play == SNR_Load)
					{	
						ObjectLoader();			
						B_LoadObj = TRUE;
					}

					if(B_SNRGE)
					{
						ObjectLoader();			
						ObjectPlayer();
						ObjectPuncher();
					}
					else
					{
						ObjectPlayer();
						ObjectPuncher();
						ObjectLoader();			
					}
#if _PRINTF
					printf("PageNr of SNR_PLAY = %d, PageNr of SNR_LOAD = %d, B_Load = %s\n",
							SNR_Play->PageNr,SNR_Load->PageNr,B_LoadObj ? "TRUE":"FALSE");
#endif

					if(B_Terminate)
					{
						SigWait = Sig_RXtoPC|SigR_WBStartup|SigR_PtoC|Sig_CtoP|
								  Sig_StoPC|Sig_GEtoPC|SigR_PCtoS|SIGF_ABORT;

						RunState = RS_REMOVE;
					}
				}

				if(RunState == RS_REMOVE)
				{
#if _PRINTSER
PrintSer("Remove ArexxTalks\n");
#endif
					if(SigRecvd & Sig_RXtoPC);
						ArexxTalks(RexxContext,SIR);

#if _PRINTSER
PrintSer("Remove ChildReply\n");
#endif
					if(SigRecvd & SigR_PtoC)
						ChildReply();

#if _PRINTSER
PrintSer("Remove GETalks\n");
#endif
					if(SigRecvd & Sig_GEtoPC)
						GETalks();	

#if _PRINTSER
PrintSer("Remove SynchroTalks\n");
#endif
					if(SigRecvd & Sig_StoPC)
						tempErr=SynchroTalks();	// THIS ONE'S A BAD BOY!
#if _PRINTSER
PrintSer("tempErr=%d\n",tempErr);
tempErr=-1;
#endif

#if _PRINTSER
PrintSer("Remove GetSynchroReply\n");
#endif
					if(SigRecvd & SigR_PCtoS)
						GetSynchroReply();
		
#if _PRINTSER
PrintSer("Remove ChildTalks\n");
#endif
					if(SigRecvd & Sig_CtoP)
						ChildTalks();

#if _PRINTSER
PrintSer("Remove RemoveChild\n");
#endif
					if(SigRecvd & SigR_WBStartup)
						if((ChildError = RemoveChild()) != NO_ERROR)
							Err = ChildError;

#if _PRINTSER
PrintSer("Remove TerminateChildren\n");
#endif
					SNR_TrackPlay = NULL;
					TerminateChildren();
				}
			}
		}
	}

#if _PRINTSER
PrintSer("Outside while\n");
#endif

	ProcessDeInitializer();

#if _PRINTSER
PrintSer("Before return\n");
#endif

	return(FreeProcessController(Err));
}

