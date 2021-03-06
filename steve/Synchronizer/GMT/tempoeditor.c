/********************************************************
*File : synchroigmtte.c
*		Unlike the normal synchroigmt this synchro
*		sets the current timevalue of an SNR when
*/

#include "nb:pre.h"

#include <resources/battmem.h>
#include <resources/cia.h>
#include <resources/ciabase.h>
#include <hardware/cia.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "minc:types.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:ge.h"
#include "minc:sync.h"
#include "external.h"

#define ed_Sig_ItoS ed_Sig_ItoGE

#define _PRINTF FALSE

extern void *Int_IEServer();

#define STARTCUROBJ 	0x10		
#define STOPCUROBJ 		0x20
#define STARTPAROBJ 	0x30		// Low nibble is used as F-key identifier 0 = F1
#define STOPPAROBJ 		0x40

/*********************************************************
*Func : Return a Punch to its owner
*in   : SGList -> Ptr to guide list
*		Msg_Timer -> As from the timer device, only this time
*					 it is generated by ourself
*		RepP_StoPC -> to which port should the PC reply
*		Port_StoPC -> to get through to the proccont
*		SD_OutStanding -> ptr to outstanding syncdialogue cntr
*out  : -
*/
void ReturnPunch( SGList, Msg_Timer, RepP_StoPC, Port_StoPC, SD_OutStanding)
struct List *SGList;
TIMEREQUEST *Msg_Timer;
struct MsgPort *RepP_StoPC;
struct MsgPort *Port_StoPC;
int *SD_OutStanding;
{
  SYNCDIALOGUE	*Msg_SyncDial;	// dialogue sent to the guide
  SYNCGUIDE		*SyncGuide;		// temp var

	if( (SyncGuide = FindSyncGuide(SGList, Msg_Timer->tr_ProcInfo)) != NULL)
	{
		SyncGuide->sg_NrPunches--;
		if( (Msg_SyncDial = (SYNCDIALOGUE *)AllocMem(sizeof(SYNCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
		{
#if _PRINTF
			printf("alloc %d\n",sizeof(SYNCDIALOGUE));
#endif

			(*SD_OutStanding)++;
			Msg_SyncDial->sd_Msg.mn_Node.ln_Type = NT_MESSAGE;
			Msg_SyncDial->sd_Msg.mn_Node.ln_Pri = 0;
			Msg_SyncDial->sd_Msg.mn_Node.ln_Name = NULL;
			Msg_SyncDial->sd_Msg.mn_ReplyPort = RepP_StoPC;
			Msg_SyncDial->sd_Cmd = SDI_PUNCHCOL;
			Msg_SyncDial->sd_Punch = Msg_Timer->tr_Punch;
			Msg_SyncDial->sd_TimeReq = Msg_Timer;
			PutMsg(Port_StoPC,Msg_SyncDial);
			Remove((struct Node *)Msg_Timer);
		}
		else		
		{
			Remove((struct Node *)Msg_Timer);
			FreeMem(Msg_Timer,sizeof(TIMEREQUEST));
#if _PRINTF
		printf("Freed %d\n",sizeof(TIMEREQUEST));
#endif
		}
	}
}


/********************************************************
*Func : TempoEditor
*in   : -
*out  : -
*/
void TempoEditor( ThisPI)
PROCESSINFO	*ThisPI;
{
  struct Library 	*TimerBase;		// ptr to Timer device

  struct List 	*TRList;		// List of TIMEREQUESTs send to the timer device
								// and which are being processed.
  struct List	*SGList;		// List of guides that have filed a sync request
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
  ULONG			SigR_StoPC,		// signal used to get reply from guides.
				Sig_PCtoS,		// signal used to indicate a SyncDial msg from a guide.
				SigRecvd;		// Signals received

  struct MsgPort *Port_StoPC,
				 *Port_PCtoS,	// Port through which guides may send SyncDialogues.
  				 *RepP_Timer,	// Replyport for timer when signalling synchro task
				 *RepP_StoPC;	// Reply port to which guides must reply after 
								// receiving a SyncDialogue msg from the synchronizer
  SYNCGUIDE		*SyncGuide,		// temp var
				*NextSyncGuide;
  SYNCPUNCH		*SyncPunch;		// temp var 
  SYNCDIALOGUE	*Msg_RSyncDial;	// Received sync dialogue from a guide

  struct timeval TimeValStore;
  struct timerequest
				 *Msg_TimerReqRead;		// used to read the timevalue from the timerdevice

  TIMEREQUEST		*Msg_Timer;	// temp var, used when sending a timermsg to the timer device
  BOOL				RemoveOK, TermOK;

  struct MsgPort	*Port_IDtoIEI;	// Port for input device to InputEventInterrupt
  int				SigNum_ItoS;
  EVENTDATA 		SEventData; 	// Data field for both the Int_SyncEProc and 
									// the SEventTask
  struct IOStdReq 	*InputRequestBlock;
  struct Interrupt 	Int_SyncEProc;
  int 				StopFKey, StartFKey;
  int 				SD_OutStanding;
  int				TimerDeviceErr,
					InputDeviceErr;
  BOOL				B_ProcessKey;
	
	Port_PCtoS = NULL;
	RepP_StoPC = NULL;
	SGList = NULL;
	TRList = NULL;
	RepP_Timer = NULL;
	InputRequestBlock = NULL;
	Msg_TimerReqRead = NULL;
	SigNum_ItoS = -1;
	Port_IDtoIEI = NULL;
	TimerDeviceErr = InputDeviceErr = 0;

	ThisPI->pi_Arguments.ar_RetErr = ERR_SYNCHRO;

	if( 
		((Port_StoPC = (struct MsgPort *)FindPort("Port_StoPC")) == NULL) ||
		((Port_PCtoS = CreatePort("Port_Synchro",0)) == NULL) ||
		((RepP_StoPC = CreatePort("RepP_Synchro",0)) == NULL) ||
		((SGList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((TRList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((RepP_Timer = CreatePort(0,0)) == NULL) ||
		((Msg_TimerReqRead = (struct timerequest *)CreateExtIO(RepP_Timer,sizeof(struct timerequest))) == NULL) ||
		((TimerDeviceErr = (BOOL)OpenDevice(TIMERNAME,UNIT_MICROHZ,(struct IORequest *)Msg_TimerReqRead,0)) != 0) ||
		((SigNum_ItoS = AllocSignal(-1)) == -1) ||
		((Port_IDtoIEI = CreatePort("Port_IDtoSyncIEI",0)) == NULL) ||
		((InputRequestBlock = (struct IOStdReq *)CreateStdIO(Port_IDtoIEI)) == NULL) ||
		((InputDeviceErr = OpenDevice("input.device",0,InputRequestBlock,0)) != 0)
	  )
		goto End;

 	NewList(SGList);
 	NewList(TRList);

	Sig_PCtoS =  1 << Port_PCtoS->mp_SigBit;
	SigR_StoPC = 1 << RepP_StoPC->mp_SigBit;
	SEventData.ed_Sig_ItoS = 1 << SigNum_ItoS;
	SEventData.ed_Task = ThisPI->pi_Process;

    Int_SyncEProc.is_Data = (APTR)&SEventData;
    Int_SyncEProc.is_Code = (void *)Int_IEServer;
    Int_SyncEProc.is_Node.ln_Pri = 60;
    Int_SyncEProc.is_Node.ln_Name = "Int_SyncEProc";
 
    InputRequestBlock->io_Command = IND_ADDHANDLER;
    InputRequestBlock->io_Data = (APTR)&Int_SyncEProc;
    DoIO(InputRequestBlock);

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	TimerBase = (struct Library *)Msg_TimerReqRead->tr_node.io_Device;

	SD_OutStanding = 0;
	StopFKey = 1;
	StartFKey = 1;
	TermOK = RemoveOK = FALSE;
	SigRecvd = 0;
	B_ProcessKey = TRUE;
	while(!TermOK)
	{
		SigRecvd = Wait(SEventData.ed_Sig_ItoS | Sig_PCtoS | SigR_StoPC | SIGF_ABORT);

		if(SigRecvd & SIGF_ABORT)
		{
			for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
				Msg_Timer->tr_Node.ln_Succ != NULL;
				Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
			{
				Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
				ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
			}
			RemoveOK = TRUE;
		}
		
		// Signal from Proccont ?
		if(SigRecvd & Sig_PCtoS)
		{
			// Get new punches
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)Port_PCtoS)) != NULL)
			{
				// First Reset the Timer to count the new elapse from this time off
	 			Msg_TimerReqRead->tr_node.io_Command = TR_GETSYSTIME;
				DoIO((struct IORequest *)Msg_TimerReqRead);
				TimeValStore.tv_micro = Msg_TimerReqRead->tr_time.tv_micro;
				TimeValStore.tv_secs = Msg_TimerReqRead->tr_time.tv_secs;
#if _PRINTF
				printf("R1 s%d m%d\n",TimeValStore.tv_secs,TimeValStore.tv_micro);
#endif

				// In case of new syncpunches, get them from the dialogue and send
				// them out to the timer device.
				// Don't accept new punches while in Remove state.
				if(!RemoveOK && Msg_RSyncDial->sd_Cmd == SDC_NEWPUNCH)
				{
					Msg_RSyncDial->sd_Cmd = SDI_PUNCHACCEPTED;	
					// see if this guide  has more syncpunches active

					if( (SyncGuide = FindSyncGuide(SGList,Msg_RSyncDial->sd_ProcInfo)) == NULL)
					{
						// allocate mem for the SyncGuide
						if( (SyncGuide = (SYNCGUIDE *)AllocMem(sizeof(SYNCGUIDE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
							Msg_RSyncDial->sd_Cmd = SDI_PUNCHERROR;	// severe problems, reply an error 
						else
						{
#if _PRINTF
							printf("alloc %d\n",sizeof(SYNCGUIDE));
#endif
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
#if _PRINTF
								printf("alloc %d\n",sizeof(TIMEREQUEST));
#endif
								// insert the new Timerequest into the TRList
								Msg_Timer->tr_Node.ln_Type = NT_TIMEREQUEST;
								Msg_Timer->tr_Node.ln_Pri = 0;
								Msg_Timer->tr_Node.ln_Name = NULL;
	
								Msg_Timer->tr_ProcInfo = Msg_RSyncDial->sd_ProcInfo;
								Msg_Timer->tr_Punch = SyncPunch;
								AddHead((struct List *)TRList,(struct Node *)Msg_Timer);
	
								// Start punches of objects that are not a part of
								// a PARALLEL event should be returned right now.
								// Objects belonging to a PARALLEL events are 
								// put under Functions-keys.
								if(SyncPunch->sp_SNR->ParentSNR->nodeType == TALK_STARTPAR)
								{
									// Assign a Function key to the punch
									if(SyncPunch->sp_PunchType == PT_DORUN)
										SyncPunch->sp_FKey = StartFKey++;
									if(SyncPunch->sp_PunchType == PT_DOSTOP)
										SyncPunch->sp_FKey = StopFKey++;
								}	
								else
								{
									if(SyncPunch->sp_PunchType == PT_DORUN)
									{
#if _PRINTF
										printf("Returning startpunch\n");	
#endif
										ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
									}
								}	
							}
						}	
					} // if(SyncGuide != NULL)
				}
				else
				{
					// a guide wants to be removed from the SyncGuide chain
					if(Msg_RSyncDial->sd_Cmd == SDC_GUIDETERM)
					{
						Msg_RSyncDial->sd_Cmd = SDI_GUIDEREMOVED;
						if( (SyncGuide = FindSyncGuide(SGList,Msg_RSyncDial->sd_ProcInfo)) != NULL)
						{
							Remove((struct Node *)SyncGuide);
							FreeMem(SyncGuide, sizeof(SYNCGUIDE));
#if _PRINTF
							printf("Freed %d\n",sizeof(SYNCGUIDE));
#endif
						}
					}
					else
					{
						if(Msg_RSyncDial->sd_Cmd == SDC_ABORTPUNCHES)
						{
							Msg_RSyncDial->sd_Cmd = SDI_PUNCHESABORTED;
							for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
								Msg_Timer->tr_Node.ln_Succ != NULL;
								Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
							{
								Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
								ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
							}
						}
						else
						{
							if(Msg_RSyncDial->sd_Cmd == SDC_SETTIMECODE)
								Msg_RSyncDial->sd_Cmd = SDI_TIMECODESET;
							else
							{
								if(Msg_RSyncDial->sd_Cmd == SDC_GETTIMECODE)
								{
									Msg_RSyncDial->sd_Cmd = SDI_TIMECODEGET;
									Msg_RSyncDial->sd_Punch->sp_PunchInfo.pi_HMSTDuration=-1;
								}
								else if(Msg_RSyncDial->sd_Cmd == SDC_GETTIMECODE_ASC)
								{
									Msg_RSyncDial->sd_Cmd = SDI_TIMECODEGET_ASC;
									Msg_RSyncDial->sd_Punch->sp_PunchInfo.pi_HMSTDuration=-1;
								}
							}
						}

					}
				}
				ReplyMsg((struct Message *)Msg_RSyncDial);
			}
		}

		// If A key was hit then find out for what punch the key is for
		if(B_ProcessKey)
		{
			if(SigRecvd & SEventData.ed_Sig_ItoS)
			{
				// First Get the system time
 				Msg_TimerReqRead->tr_node.io_Command = TR_GETSYSTIME;
				DoIO((struct IORequest *)Msg_TimerReqRead);

#if _PRINTF
				printf("R1 s%d m%d\n",Msg_TimerReqRead->tr_time.tv_secs,Msg_TimerReqRead->tr_time.tv_micro);
#endif
				
				SubTime(&Msg_TimerReqRead->tr_time,&TimeValStore);

#if _PRINTF
				printf("R1 s%d m%d\n",Msg_TimerReqRead->tr_time.tv_secs,Msg_TimerReqRead->tr_time.tv_micro);
#endif

				if(SEventData.ed_Cmd == STOPCUROBJ)
				{
					StopFKey = 1;
					StartFKey = 1;

					// Return all STOP/START punches
					for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
						Msg_Timer->tr_Node.ln_Succ != NULL;
						Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
					{
						if(Msg_Timer->tr_Punch->sp_PunchType == PT_DOSTOP)
						{
							if(Msg_Timer->tr_Punch->sp_SNR->ParentSNR->nodeType == TALK_STARTSER)
								Msg_Timer->tr_Punch->sp_SNR->duration = (Msg_TimerReqRead->tr_time.tv_secs*10) + (Msg_TimerReqRead->tr_time.tv_micro/100000);
							else
								Msg_Timer->tr_Punch->sp_SNR->End.ParHMSTOffset = (Msg_TimerReqRead->tr_time.tv_secs*10) + (Msg_TimerReqRead->tr_time.tv_micro/100000);
						}

						if(Msg_Timer->tr_Punch->sp_PunchType == PT_DORUN)
						{
							// Make sure the object for the punch doesn't start
							Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
//							if(Msg_Timer->tr_Punch->sp_SNR->ParentSNR->nodeType == TALK_STARTPAR)
//								Msg_Timer->tr_Punch->sp_SNR->Start.ParHMSTOffset = (Msg_TimerReqRead->tr_time.tv_secs*10) + (Msg_TimerReqRead->tr_time.tv_micro/100000);
						}
						ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
					}
				}
				if((SEventData.ed_Cmd & STARTPAROBJ) == STARTPAROBJ)
				{
					// Return all STOP/START punches
					for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
						Msg_Timer->tr_Node.ln_Succ != NULL;
						Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
					{
						if(Msg_Timer->tr_Punch->sp_PunchType == PT_DORUN)
						{
							if( (Msg_Timer->tr_Punch->sp_FKey - 1) == (SEventData.ed_Cmd & 0x0f))
							{
								Msg_Timer->tr_Punch->sp_SNR->Start.ParHMSTOffset = (Msg_TimerReqRead->tr_time.tv_secs*10) + (Msg_TimerReqRead->tr_time.tv_micro/100000);
								ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
							}
						}
					}
				}

				if((SEventData.ed_Cmd & STOPPAROBJ) == STOPPAROBJ)
				{
					// Return all STOP/START punches
					for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
						Msg_Timer->tr_Node.ln_Succ != NULL;
						Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
					{
						if(Msg_Timer->tr_Punch->sp_PunchType == PT_DOSTOP)
						{
							if( (Msg_Timer->tr_Punch->sp_FKey - 1) == (SEventData.ed_Cmd & 0x0f))
							{
								Msg_Timer->tr_Punch->sp_SNR->End.ParHMSTOffset = (Msg_TimerReqRead->tr_time.tv_secs*10) + (Msg_TimerReqRead->tr_time.tv_micro/100000);
								ReturnPunch(SGList,Msg_Timer,RepP_StoPC,Port_StoPC,&SD_OutStanding);
							}
						}
					}
				}
				B_ProcessKey = FALSE;
			}
		}

		if(SigRecvd & SigR_StoPC)
		{
			B_ProcessKey = TRUE;
			// a guide replied to our dialogue, free its mem
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)RepP_StoPC)) != NULL)
			{
				SD_OutStanding--;
				FreeMem(Msg_RSyncDial->sd_TimeReq,sizeof(TIMEREQUEST));
#if _PRINTF
				printf("Freed %d\n",sizeof(TIMEREQUEST));
#endif
				FreeMem(Msg_RSyncDial,sizeof(SYNCDIALOGUE));
#if _PRINTF
				printf("Freed %d\n",sizeof(SYNCDIALOGUE));
#endif
			}
		}

		if(RemoveOK)
			if(
				((struct List *)TRList->lh_TailPred == (struct List *)TRList) &&
				(SD_OutStanding == 0)
			  )
				TermOK = TRUE;
	}

	SyncGuide = (SYNCGUIDE *)SGList->lh_Head;
	while( NextSyncGuide = (SYNCGUIDE *)SyncGuide->sg_Node.ln_Succ)
	{
		FreeMem(SyncGuide, sizeof(SYNCGUIDE));
#if _PRINTF
		printf("Freed %d\n",sizeof(SYNCGUIDE));
#endif
		SyncGuide = NextSyncGuide;
	}

End:
	if(Msg_TimerReqRead != NULL)
	{
		if(TimerDeviceErr == 0)
			CloseDevice( (struct IORequest *)Msg_TimerReqRead);
		DeleteExtIO( (struct IORequest *)Msg_TimerReqRead);
	}

	if(InputRequestBlock != NULL)
	{
		InputRequestBlock->io_Command = IND_REMHANDLER;
    	InputRequestBlock->io_Data = (APTR)&Int_SyncEProc;
	   	DoIO(InputRequestBlock);
		if(InputDeviceErr == 0)
	    	CloseDevice(InputRequestBlock);
		DeleteStdIO(InputRequestBlock);
	}

	if(SigNum_ItoS != -1)
		FreeSignal(SigNum_ItoS);
	if(Port_IDtoIEI != NULL)
		DeletePort(Port_IDtoIEI);
	if(RepP_Timer != NULL)
		DeletePort(RepP_Timer);
	if(TRList != NULL)
		FreeMem(TRList, sizeof(struct List));
	if(SGList != NULL)
		FreeMem(SGList, sizeof(struct List));
	if(RepP_StoPC != NULL)
		DeletePort(RepP_StoPC);
	if(Port_PCtoS != NULL)
		DeletePort(Port_PCtoS);

}

/***************************************************
*Func : Handle the inputevent stream
*in   : InEvent -> ptr to an event
*		SEventData -> ptr to EVENTDATA struct
*out  : InEvent
*/
struct InputEvent *Int_IEHandler( InEvent, SEventData)
struct InputEvent *InEvent;
EVENTDATA *SEventData;
{
  UWORD KeyCode;
  UWORD Qual;

	switch(InEvent->ie_Class)
	{
	    case IECLASS_RAWKEY:
	    	KeyCode = (UWORD)InEvent->ie_Code;
			Qual = InEvent->ie_Qualifier;

			// check special preprogrammed keys
			switch(KeyCode)
			{
				case 0x4e:	//crsr right
						InEvent->ie_Class = IECLASS_NULL;
						SEventData->ed_Cmd = STOPCUROBJ;
						Signal(SEventData->ed_Task,SEventData->ed_Sig_ItoS);
						break;
			}
			if(KeyCode >= 0x50 && KeyCode <= 0x59)
			{
				InEvent->ie_Class = IECLASS_NULL;
				if(Qual & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT|IEQUALIFIER_CAPSLOCK))
					SEventData->ed_Cmd = STOPPAROBJ|(KeyCode & 0x0f);
				else
					SEventData->ed_Cmd = STARTPAROBJ|(KeyCode & 0x0f);

				Signal(SEventData->ed_Task,SEventData->ed_Sig_ItoS);
			}
			break;
	    case IECLASS_RAWMOUSE:
		    KeyCode = (UWORD)InEvent->ie_Code;
			switch(KeyCode)
			{
				case IECODE_RBUTTON:
						InEvent->ie_Class = IECLASS_NULL;
						SEventData->ed_Cmd = STOPCUROBJ;
						Signal(SEventData->ed_Task,SEventData->ed_Sig_ItoS);
						break;
			}
			break;
		default:
			break;
	}

	return(InEvent);
}
