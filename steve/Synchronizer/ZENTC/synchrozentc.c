/********************************************************
*File : synchromltc.c
*		All synchronizing is controlled from this
*		piece of coding.
*		Messages are send to the guides
*		resident/non-reentrant 
*
*		It is possible for workers to gain access to the synchronizer
*		but only whenever there has to be a precise synchro job. 
*/


#include <proto/cia.h>
#include <proto/battmem.h>
#include <resources/battmem.h>
#include <resources/cia.h>
#include <resources/ciabase.h>
#include <hardware/cia.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "nb:pre.h"
#include "minc:types.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:external.h"
#include "timecode.h"
#include "external.h"

#define _PRINTF FALSE

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

extern SYNCDATA ITCSyncData;		// internal generated timecode
extern SYNCDATA ETCSyncData;		// external generated timecode

struct ScriptInfoRecord *SIR;
PROCESSINFO	*ThisPI;

struct List 	*TRList;		// List of TIMECODEs which have to be checked
								// every time a timecode comes in
struct List		*SGList;		// List of guides that have filed a sync request
								// when SyncCollision has to be send, the PI of
								// that guide is first searched for in this list
								// If it isn't available then the SYNCCOL will not
								// be send to the guide but will simply be removed
								// This check is needed because of the fact that
								// a guide may have terminated itself BEFORE it has
								// received all SYNCCOL infos. Else it would be 
								// possible to send SYNCCOLS to non-existing guides
								// which will lead to system crashes etc. 
								// type : NT_SYNCGUIDE

ULONG			SigR_StoZen,	// signal from Zen
				SigR_StoPC,		// signal used to get reply from guides.
				Sig_PCtoS,		// signal used to indicate a SyncDial msg from a guide.
 				Sig_TCItoS,		// The timecode interrupts sends us information
				Sig_ETCtoS,		// External timecode received
				SigRecvd;		// Signals received
int 			SigNum_ETCtoS,
				SigNum_TCItoS;
struct MsgPort  *Port_StoZen,	// port through which the synchro may talk to zen
				*RepP_StoZen,	// reply port through which Zen gives us info
				*Port_StoPC,
				*Port_PCtoS,	// Port through which guides may send SyncDialogues.
			 	*RepP_StoPC;	// Reply port to which guides must reply after 
								// receiving a SyncDialogue msg from the synchronizer
SYNCGUIDE		*SyncGuide,		// temp var
				*NextSyncGuide;
SYNCPUNCH		*SyncPunch;		// temp var 
SYNCDIALOGUE	*Msg_RSyncDial,	// Received sync dialogue from a guide
				*Msg_SyncDial;	// dialogue sent to the guide
TIMEREQUEST		*Msg_Timer,
				*Msg_RTimer;
struct TCMsg  	 Msg_ZenDial;	// message to Zen

BOOL			RemoveOK, 
				TermOK;
BOOL			B_PunchReceived,
				B_ZenReceived;

UBYTE 			*ZenCode;


/************************************************************
*Func : Find a SyncGuide in the SGList
*in   : SGList -> Ptr to SGList
*		PI -> ptr to processinfo of guide
*out  : ptr to SyncGuide
*		NULL -> PI not in list
*/
SYNCGUIDE *FindSyncGuide( SGList, PI)
struct List *SGList;
PROCESSINFO *PI;
{
  SYNCGUIDE *CurSG;
	
	for(CurSG = (SYNCGUIDE *)SGList->lh_Head;
		CurSG->sg_Node.ln_Succ != NULL;
		CurSG = (SYNCGUIDE *)CurSG->sg_Node.ln_Succ)
		if(PI == CurSG->sg_ProcInfo)
			return(CurSG);

	return(NULL);
}

/********************************************************
*Func : internal midi time code hh:mm:ss:ff
*in   : -
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{
  UBYTE i;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr( argc, argv)) == NULL)
		return;

	SIR = ThisPI->pi_Arguments.ar_Module.am_SIR;
	ThisPI->pi_Arguments.ar_RetErr = ERR_SYNCHRO;

	Port_StoZen = (struct MsgPort *)FindPort("TimeCodePort");

	if(ThisPI->pi_Arguments.ar_Module.am_TempoType == TT_RECORD)
	{
		TempoEditor();
		return;
	}

	Port_StoPC = (struct MsgPort *)FindPort("Port_StoPC");

	if( (Port_PCtoS = CreatePort("Port_Synchro",0)) == NULL)
		return;
	Sig_PCtoS = 1 << Port_PCtoS->mp_SigBit;

	if( (RepP_StoZen = CreatePort(NULL,0)) == NULL)
	{
		DeletePort(Port_PCtoS);
		return;
	}
	SigR_StoZen = 1 << RepP_StoZen->mp_SigBit;

	if( (RepP_StoPC = CreatePort(0,0)) == NULL)
	{	
		DeletePort(RepP_StoZen);
		DeletePort(Port_PCtoS);
		return;	
	}
	SigR_StoPC = 1 << RepP_StoPC->mp_SigBit;

	// Allocate a list for the SyncGuides
	if( (SGList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		DeletePort(RepP_StoPC);
		DeletePort(RepP_StoZen);
		DeletePort(Port_PCtoS);
		return;	
	}
 	NewList(SGList);

	// Allocate a list for the TimeCodes
	if( (TRList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(RepP_StoZen);
		DeletePort(Port_PCtoS);
		return;	
	}
 	NewList(TRList);

	SigNum_ETCtoS = AllocSignal(-1);
	Sig_ETCtoS = 1<<SigNum_ETCtoS;
	if(SIR->timeCodeSource == TIMESOURCE_EXTERNAL)
	{
		if(!InitETCInt(SIR, Sig_ETCtoS))
		{
			FreeSignal(SigNum_ETCtoS);
			FreeMem(TRList, sizeof(struct List));
			FreeMem(SGList, sizeof(struct List));
			DeletePort(RepP_StoPC);
			DeletePort(RepP_StoZen);
			DeletePort(Port_PCtoS);
			return;
		}
	}

	SigNum_TCItoS = AllocSignal(-1);
	Sig_TCItoS = 1<<SigNum_TCItoS;
	if(!InitITCInt(SIR, Sig_TCItoS))
	{
		if(SIR->timeCodeSource == TIMESOURCE_EXTERNAL)
			FreeETCInt(SIR);

		FreeSignal(SigNum_ETCtoS);
		FreeSignal(SigNum_TCItoS);

		FreeMem(TRList, sizeof(struct List));
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(RepP_StoZen);
		DeletePort(Port_PCtoS);
		return;
	}

	// Send message to Zen to get the current timecode
	Msg_ZenDial.Msg.mn_ReplyPort = RepP_StoZen; /* Set up our request */
	Msg_ZenDial.Msg.mn_Length = sizeof(Msg_ZenDial);
	Msg_ZenDial.Class = TCNORM; /* ask for status */ 
	Msg_ZenDial.Code  = 0;

	ITCSyncData.sd_B_DoCheck = TRUE;
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;

	ITCSyncData.sd_TRList = TRList;

	RemoveOK = TermOK = FALSE;

	if(Port_StoZen)
		PutMsg(Port_StoZen,&Msg_ZenDial);   

	B_PunchReceived = FALSE;
	// in case of Zen not available, start synchronizing on first punch
	B_ZenReceived = !(BOOL)Port_StoZen;
	ITCSyncData.sd_GenerateInternally = !(BOOL)Port_StoZen;

	SigRecvd = 0;
	while(!TermOK)
	{
		SigRecvd = Wait(SigR_StoZen|Sig_TCItoS|Sig_PCtoS|SigR_StoPC|Sig_ETCtoS|SIGF_ABORT);

		if(SigRecvd & SigR_StoZen)
		{
			B_ZenReceived = TRUE;
			if(B_PunchReceived)
				ITCSyncData.sd_B_Count = TRUE;

			ZenCode = (UBYTE*)&Msg_ZenDial.TCode.Frame;
			// BCD to hex conversion
			for(i = 0; i < 4;i++)
				ZenCode[i] = ((ZenCode[i]>>4) * 10) + (ZenCode[i] & 0x0f);

			ITCSyncData.sd_FCFastCheck = Msg_ZenDial.TCode.Frame;
#if _PRINTF
			printf("TCode = %08x\n",Msg_ZenDial.TCode.Frame);
#endif
			PutMsg(Port_StoZen,&Msg_ZenDial);   
		}

		if(SigRecvd & SIGF_ABORT)
		{
			for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
				Msg_Timer->tr_Node.ln_Succ != NULL;
				Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
				Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
			RemoveOK = TRUE;
		}

		// New frame received from the external frame code reader
		// Used if external sourcing is used
		if(SigRecvd & Sig_ETCtoS)
		{
			if(ETCSyncData.sd_SpecCmd == SCC_EXIT)
			{
				if( (Msg_SyncDial = (SYNCDIALOGUE *)AllocMem(sizeof(SYNCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
				{
					Msg_SyncDial->sd_Msg.mn_Node.ln_Type = NT_MESSAGE;
					Msg_SyncDial->sd_Msg.mn_ReplyPort = RepP_StoPC;
					Msg_SyncDial->sd_Cmd = SDI_EXTERNALEXIT;
					Msg_SyncDial->sd_Punch = NULL;
					Msg_SyncDial->sd_TimeReq = NULL;	// needed from removal
					PutMsg(Port_StoPC,Msg_SyncDial);
				}
			}
			else
			{
				Disable();
				ITCSyncData.sd_FCFastCheck = ETCSyncData.sd_FCFastCheck;
				ITCSyncData.sd_B_Count = TRUE;		// First full frame starts system
				Enable();
			}
		}	

		if(SigRecvd & Sig_TCItoS)
		{
			for(Msg_RTimer = (TIMEREQUEST *)TRList->lh_Head;
				Msg_RTimer->tr_Node.ln_Succ != NULL;
				Msg_RTimer = (TIMEREQUEST *)Msg_RTimer->tr_Node.ln_Succ)
			{
				if(Msg_RTimer->tr_ColState == CS_FRAMECOL)
				{
					Msg_RTimer->tr_ColState = CS_TRANSMITTED;
					// take the SyncPunch from the message.ln_name and send it to its
					// original guide.
					SyncPunch = Msg_RTimer->tr_Punch;
					// if the guide to which this msg has to be send doesn't exist then
					// don't send it anymore.
					if( (SyncGuide = FindSyncGuide(SGList, Msg_RTimer->tr_ProcInfo)) != NULL)
					{
						SyncGuide->sg_NrPunches--;
						// allocate mem for the syncdialogue, this mem will be freed when
						// a reply has been received from the guide that received this message.
						if( (Msg_SyncDial = (SYNCDIALOGUE *)AllocMem(sizeof(SYNCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
						{
							Msg_SyncDial->sd_Msg.mn_Node.ln_Type = NT_MESSAGE;
							Msg_SyncDial->sd_Msg.mn_Node.ln_Pri = 0;
							Msg_SyncDial->sd_Msg.mn_Node.ln_Name = NULL;
							Msg_SyncDial->sd_Msg.mn_ReplyPort = RepP_StoPC;
							Msg_SyncDial->sd_Cmd = SDI_PUNCHCOL;
							Msg_SyncDial->sd_Punch = SyncPunch;
							Msg_SyncDial->sd_TimeReq = Msg_RTimer;	// needed from removal
							PutMsg(Port_StoPC,Msg_SyncDial);
						}
					}
				}
			}
		}

		// Signal from Guide ?
		if(SigRecvd & Sig_PCtoS)
		{
			// Get new punches
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)Port_PCtoS)) != NULL)
			{
				// In case of new syncpunches, get them from the dialogue and send
				// them out to the ITC interrupt.
				if(Msg_RSyncDial->sd_Cmd == SDC_NEWPUNCH)
				{
					Msg_RSyncDial->sd_Cmd = SDI_PUNCHACCEPTED;
					// see if this guide has more syncpunches active

					if((SyncGuide = FindSyncGuide(SGList,Msg_RSyncDial->sd_ProcInfo)) == NULL)
					{
						// allocate mem for the SyncGuide
						if( (SyncGuide = (SYNCGUIDE *)AllocMem(sizeof(SYNCGUIDE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
							Msg_RSyncDial->sd_Cmd = SDI_PUNCHERROR;	// severe problems, reply an error
						else
						{
							SyncGuide->sg_Node.ln_Type = NT_SYNCGUIDE; 
							SyncGuide->sg_Node.ln_Pri = 0; 
							SyncGuide->sg_Node.ln_Name = NULL; 
							SyncGuide->sg_NrPunches = 0;
							SyncGuide->sg_ProcInfo = Msg_RSyncDial->sd_ProcInfo;
							// insert the new syncguide into the SGList
							AddHead((struct List *)SGList,(struct Node *)SyncGuide);
						}
					}

					if(SyncGuide != NULL)
					{
						for(SyncPunch = Msg_RSyncDial->sd_Punch;
							SyncPunch != NULL;
							SyncPunch = (SYNCPUNCH *)SyncPunch->sp_MinNode.mln_Succ)
						{
							// not used, simply counts the number of punches filed
							SyncGuide->sg_NrPunches++;
							// allocate mem for the timerequest
							if( (Msg_Timer = (TIMEREQUEST *)AllocMem(sizeof(TIMEREQUEST), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
							{
								// severe problems, reply an error 
								Msg_RSyncDial->sd_Cmd = SDI_PUNCHERROR;
							}
							else
							{
								// insert the new Timerequest into the TRList
								Msg_Timer->tr_ColState = CS_WAIT;
								Msg_Timer->tr_Node.ln_Type = NT_TIMEREQUEST;
								Msg_Timer->tr_ProcInfo = Msg_RSyncDial->sd_ProcInfo;
								Msg_Timer->tr_Punch = SyncPunch;
								Disable();
								AddHead((struct List *)TRList,(struct Node *)Msg_Timer);
								Enable();
							}
						}	
					} // if(SyncGuide != NULL)
					// trigger timecode generator only if Zen is available and a syncpunch has been received
					if(SIR->timeCodeSource == TIMESOURCE_INTERNAL)
					{
						B_PunchReceived = TRUE;
						if(B_ZenReceived)
							ITCSyncData.sd_B_Count = TRUE;
					}	
				}
				else
				{
					// a guide wants to be removed from the SyncGuide chain
					if(Msg_RSyncDial->sd_Cmd == SDC_GUIDETERM)
					{
						if( (SyncGuide = FindSyncGuide(SGList,Msg_RSyncDial->sd_ProcInfo)) != NULL)
						{
							Remove((struct Node *)SyncGuide);
							FreeMem(SyncGuide, sizeof(SYNCGUIDE));
						}
						Msg_RSyncDial->sd_Cmd = SDI_GUIDEREMOVED;
					}
					else
					{
						if(Msg_RSyncDial->sd_Cmd == SDC_ABORTPUNCHES)
						{
							// this message normally comes from the processcontroller
							// We need to abort all outstanding messages
							Msg_RSyncDial->sd_Cmd = SDI_PUNCHESABORTED;
							// Abort all outstanding Timer messages
							Disable();
							for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
								Msg_Timer->tr_Node.ln_Succ != NULL;
								Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
								Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
							Enable();
						}
						else
						{
							if(Msg_RSyncDial->sd_Cmd == SDC_SETTIMECODE)
							{
								Msg_RSyncDial->sd_Cmd = SDI_TIMECODESET;
								ITCSyncData.sd_FCFastCheck = (ULONG)Msg_RSyncDial->sd_Punch;
							}
							else
							{
								if(Msg_RSyncDial->sd_Cmd == SDC_GETTIMECODE)
								{
									Msg_RSyncDial->sd_Cmd = SDI_TIMECODEGET;
									Msg_RSyncDial->sd_Punch = (SYNCPUNCH *)ITCSyncData.sd_FCFastCheck;
								}
								else if(Msg_RSyncDial->sd_Cmd == SDC_GETTIMECODE_ASC)
								{
									Msg_RSyncDial->sd_Cmd = SDI_TIMECODEGET_ASC;
									Msg_RSyncDial->sd_Punch = (SYNCPUNCH *)ITCSyncData.sd_FCFastCheck;
								}
							}
						}
					}
				}
				ReplyMsg((struct Message *)Msg_RSyncDial);
			}
		}

		if(SigRecvd & SigR_StoPC)
		{
			// a guide replied to our dialogue, free its mem
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)RepP_StoPC)) != NULL)
			{
				Disable();
				Remove((struct Node *)Msg_RSyncDial->sd_TimeReq);

				if(Msg_RSyncDial->sd_TimeReq != NULL)
					FreeMem(Msg_RSyncDial->sd_TimeReq,sizeof(TIMEREQUEST));

				FreeMem(Msg_RSyncDial,sizeof(SYNCDIALOGUE));
				Enable();
			}
		}

		if(RemoveOK)
			if((struct List *)TRList->lh_TailPred == (struct List *)TRList)
				TermOK = TRUE;
	}

	FreeITCInt(SIR);
	if(SIR->timeCodeSource == TIMESOURCE_EXTERNAL)
		FreeETCInt(SIR);

	if(Port_StoZen)
	{
		// get reply on last message to Zen 
		WaitPort(RepP_StoZen);
		GetMsg(RepP_StoZen);
	}

	FreeSignal(SigNum_ETCtoS);
	FreeSignal(SigNum_TCItoS);

	SyncGuide = (SYNCGUIDE *)SGList->lh_Head;
	while( NextSyncGuide = (SYNCGUIDE *)SyncGuide->sg_Node.ln_Succ)
	{
		FreeMem(SyncGuide, sizeof(SYNCGUIDE));
#if _PRINTF
		printf("Freed %d\n",sizeof(SYNCGUIDE));
#endif
		SyncGuide = NextSyncGuide;
	}

	FreeMem(TRList, sizeof(struct List));
	FreeMem(SGList, sizeof(struct List));
	DeletePort(RepP_StoPC);
	DeletePort(RepP_StoZen);
	DeletePort(Port_PCtoS);
}

