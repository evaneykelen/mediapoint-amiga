/********************************************************
*File : syncproc.c
*		All synchronizing is controlled from this
*		piece of coding
*		Messages are send to the guides
*		Non-resident/non-reentrant 
*
*		It is possible for workers to gain access to the synchronizer
*		but only whenever there has to be a precise synchro job. 
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

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE

UWORD *JOY2DAT = (UWORD *)0xbfe001;

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

	//KPrintF("FindSync ret null\n");
	return(NULL);
}


/********************************************************
*Func : internal clock hh:mm:ss:tt
*in   : -
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{
  struct Library 	*TimerBase;	// ptr to Timer device
  PROCESSINFO	*ThisPI;
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
 				SigR_Timer,		// Signal used by the timerdevice to signal synchro
				SigRecvd;		// Signals received

  struct MsgPort *Port_StoPC,
				 *Port_PCtoS,	// Port through which guides may send SyncDialogues.
  				 *RepP_Timer,	// Replyport for timer when signalling synchro task
				 *RepP_StoPC;	// Reply port to which guides must reply after 
								// receiving a SyncDialogue msg from the synchronizer
  SYNCGUIDE		*SyncGuide,		// temp var
				*NextSyncGuide;
  SYNCPUNCH		*SyncPunch;		// temp var 
  SYNCDIALOGUE	*Msg_RSyncDial,	// Received sync dialogue from a guide
				*Msg_SyncDial,	// dialogue sent to the guide
				*Msg_REMEMBER = 0;

  UWORD			AbortTimeOut = 0;

  struct timerequest
		 *Msg_TimerTemplate;	// used as a template for all timer requests

  TIMEREQUEST		*Msg_Timer,	// temp var, used when sending a timermsg to the timer device
					*Msg_RTimer;

  BOOL				RemoveOK, TermOK;
  int				SD_OutStanding;
  //int bug;
  SYNCDIALOGUE	Copy_Msg_RSyncDial;	// Received sync dialogue from a guide

//DEBUG

	ULONG	i_Alloced;
//	ULONG	tel = 0;
//	char	tt[200];


// END debug

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr( argc, argv)) == NULL)
		return;

	ThisPI->pi_Arguments.ar_RetErr = ERR_SYNCHRO;

	if(ThisPI->pi_Arguments.ar_Module.am_TempoType == TT_RECORD)
	{
		TempoEditor(ThisPI);
		return;
	}

	Port_StoPC = (struct MsgPort *)FindPort("Port_StoPC");

	// this port is used to get new SyncPunches from the guides
	// When a guide is set up it will search for this port.
	// Therefore it has to be a public port
	if( (Port_PCtoS = CreatePort("Port_Synchro",0)) == NULL)
		return;
	Sig_PCtoS =  1 << Port_PCtoS->mp_SigBit;

	// when sending a SDI_PUNCHCOL, the receiver may use this port
	// to reply to the synchronizer.
	// No need for this port to be public but since 1 port won't hurt the system..
	if( (RepP_StoPC = CreatePort("RepP_Synchro",0)) == NULL)
	{	
		DeletePort(Port_PCtoS);
		return;	
	}
	SigR_StoPC = 1 << RepP_StoPC->mp_SigBit;

	// Allocate a list for the SyncGuides
	if( (SGList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		DeletePort(RepP_StoPC);
		DeletePort(Port_PCtoS);
		return;	
	}
 	NewList(SGList);

	// Allocate a list for the TimeRequests
	if( (TRList = (struct List *)AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(Port_PCtoS);
		return;	
	}
 	NewList(TRList);

	// open the timer device, set up a msgport for communication
	if( (RepP_Timer = CreatePort("RepP_Timer",0)) == NULL)
	{
		FreeMem(TRList, sizeof(struct List));
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(Port_PCtoS);
		return;
	}
	// replysignal for timer device
	SigR_Timer = 1 << RepP_Timer->mp_SigBit;

	// make timer iorequest template	
	if( (Msg_TimerTemplate = (struct timerequest *)CreateExtIO(RepP_Timer,sizeof(struct timerequest))) == NULL)
	{
		DeletePort(RepP_Timer);
		FreeMem(TRList, sizeof(struct List));
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(Port_PCtoS);
		return;
	}
	Msg_TimerTemplate->tr_node.io_Command = TR_ADDREQUEST;
	
	if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)Msg_TimerTemplate,0))
	{
		DeleteExtIO( (struct IORequest *)Msg_TimerTemplate);
		DeletePort(RepP_Timer);
		FreeMem(TRList, sizeof(struct List));
		FreeMem(SGList, sizeof(struct List));
		DeletePort(RepP_StoPC);
		DeletePort(Port_PCtoS);
		return;
	}

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	TimerBase = (struct Library *)Msg_TimerTemplate->tr_node.io_Device;

	i_Alloced = 0;
	TermOK = RemoveOK = FALSE;
	SigRecvd = 0;
	SD_OutStanding = 0;
	while(!TermOK)
	{
		SigRecvd = Wait(SigR_Timer | Sig_PCtoS | SigR_StoPC | SIGF_ABORT);

		if(SigRecvd & SIGF_ABORT)
		{
			//if((*JOY2DAT & 0x8000) == 0x0 )
			//	KPrintF("SYNC Abort\n");
//bug=1;
			for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
				Msg_Timer->tr_Node.ln_Succ != NULL;
				Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
			{
				Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
				AbortIO((struct IORequest *)&Msg_Timer->tr_IOReq);
//if (bug>20) KPrintF("1!\n"); else bug++;
			}
			RemoveOK = TRUE;
		}
		
		// Signal from Proccont ?
		if(SigRecvd & Sig_PCtoS)
		{
			//if((*JOY2DAT & 0x8000) == 0x0 )
			//	KPrintF("SYNC PC\n");
//bug=1;

			// Get new punches
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)Port_PCtoS)) != NULL)
			{
				CopyMem(Msg_RSyncDial,&Copy_Msg_RSyncDial,sizeof(SYNCDIALOGUE));
				ReplyMsg((struct Message *)Msg_RSyncDial);

				// In case of new syncpunches, get them from the dialogue and send
				// them out to the timer device
				// Don't accept new punches while in Remove state
				if(!RemoveOK && Copy_Msg_RSyncDial.sd_Cmd == SDC_NEWPUNCH)
				{
					Copy_Msg_RSyncDial.sd_Cmd = SDI_PUNCHACCEPTED;	
					// see if this guide  has more syncpunches active

					if( (SyncGuide = FindSyncGuide(SGList,Copy_Msg_RSyncDial.sd_ProcInfo)) == NULL)
					{
						// allocate mem for the SyncGuide
						if( (SyncGuide = (SYNCGUIDE *)AllocMem(sizeof(SYNCGUIDE), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
							Copy_Msg_RSyncDial.sd_Cmd = SDI_PUNCHERROR;	// severe problems, reply an error 
						else
						{
#if _PRINTF
							i_Alloced += sizeof(SYNCGUIDE); 
							printf("1 allocated %d\n",sizeof(SYNCGUIDE));
#endif
						
							SyncGuide->sg_Node.ln_Type = NT_SYNCGUIDE; 
							SyncGuide->sg_Node.ln_Pri = 0; 
							SyncGuide->sg_Node.ln_Name = NULL; 
							SyncGuide->sg_NrPunches = 0;
							SyncGuide->sg_ProcInfo = Copy_Msg_RSyncDial.sd_ProcInfo;
							// insert the new syncguide into the SGList
							AddHead((struct List *)SGList,(struct Node *)SyncGuide);
						}
					}

					if(SyncGuide != NULL)
					{
						for(SyncPunch = Copy_Msg_RSyncDial.sd_Punch;
							SyncPunch != NULL;
							SyncPunch = (SYNCPUNCH *)SyncPunch->sp_MinNode.mln_Succ)
						{
							// not used, simply counts the number of punches filed
							SyncGuide->sg_NrPunches++;

							// allocate mem for the timerequest
							if( (Msg_Timer = (TIMEREQUEST *)AllocMem(sizeof(TIMEREQUEST), MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
							{
								// severe problems, reply an error 
								Copy_Msg_RSyncDial.sd_Cmd = SDI_PUNCHERROR;
							}
							else
							{
#if _PRINTF
//					i_Alloced += sizeof(TIMEREQUEST); 
					tel++;
					sprintf(tt,"a-%lx\n",Msg_Timer );
					KPrintF("%s",tt);
#endif
								// insert the new Timerequest into the TRList
								Msg_Timer->tr_Node.ln_Type = NT_TIMEREQUEST;
								Msg_Timer->tr_Node.ln_Pri = 0;
								Msg_Timer->tr_Node.ln_Name = NULL;
								AddHead((struct List *)TRList,(struct Node *)Msg_Timer);

//					sprintf(tt,"h-%lx\n",TRList->lh_Head );
//					KPrintF("%s",tt);
	
								// copy data from template, LatticeC will copy the entire structure
								Msg_Timer->tr_IOReq = *((struct IORequest *)&Msg_TimerTemplate->tr_node);

								// set secs
								Msg_Timer->tr_Time.tv_secs = 
									SyncPunch->sp_PunchInfo.pi_HMSTDuration / 10;
								// set tenth of secs, always add 10 usecs to prevent a system crash
								Msg_Timer->tr_Time.tv_micro = 
									((SyncPunch->sp_PunchInfo.pi_HMSTDuration % 10) * 100000) + 10;

								if((SyncPunch->sp_PunchType == PT_DOSTOP) &&
								   (SyncPunch->sp_PunchInfo.pi_HMSTDuration == 0))
									Msg_Timer->tr_Time.tv_micro = 50;

								Msg_Timer->tr_ProcInfo = Copy_Msg_RSyncDial.sd_ProcInfo;
								Msg_Timer->tr_Punch = SyncPunch;
								SendIO((struct IORequest *)&Msg_Timer->tr_IOReq);
							}
						}	
					} // if(SyncGuide != NULL)
				}
				else
				{
					// a guide wants to be removed from the SyncGuide chain
					if(Copy_Msg_RSyncDial.sd_Cmd == SDC_GUIDETERM)
					{
						if( (SyncGuide = FindSyncGuide(SGList,Copy_Msg_RSyncDial.sd_ProcInfo)) != NULL)
						{
							Remove((struct Node *)SyncGuide);
							FreeMem(SyncGuide, sizeof(SYNCGUIDE));
#if _PRINTF
							i_Alloced -= sizeof(SYNCGUIDE); 

							printf("1 Freed %d\n",sizeof(SYNCGUIDE));
#endif
						}
						Copy_Msg_RSyncDial.sd_Cmd = SDI_GUIDEREMOVED;
					}
					else
					{
						if(Copy_Msg_RSyncDial.sd_Cmd == SDC_ABORTPUNCHES)
						{
							Copy_Msg_RSyncDial.sd_Cmd = SDI_PUNCHESABORTED;
							for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
								Msg_Timer->tr_Node.ln_Succ != NULL;
								Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
							{
								Msg_Timer->tr_Punch->sp_PunchType = PT_NOPUNCH;
								AbortIO((struct IORequest *)&Msg_Timer->tr_IOReq);
							}
						}
						else
						{
							if(Copy_Msg_RSyncDial.sd_Cmd == SDC_SETTIMECODE)
								Copy_Msg_RSyncDial.sd_Cmd = SDI_TIMECODESET;
							else
							{
								if(Copy_Msg_RSyncDial.sd_Cmd == SDC_GETTIMECODE)
								{
									Copy_Msg_RSyncDial.sd_Cmd = SDI_TIMECODEGET;
									Copy_Msg_RSyncDial.sd_Punch->sp_PunchInfo.pi_HMSTDuration=-1;
								}
								else if(Copy_Msg_RSyncDial.sd_Cmd == SDC_GETTIMECODE_ASC)
								{
									Copy_Msg_RSyncDial.sd_Cmd = SDI_TIMECODEGET_ASC;
									Copy_Msg_RSyncDial.sd_Punch->sp_PunchInfo.pi_HMSTDuration=-1;
								}
							}
						}

					}
				}
				//ReplyMsg((struct Message *)&Copy_Msg_RSyncDial);
//if (bug>20) KPrintF("2!\n"); else bug++;
			}
		}

		if(SigRecvd & SigR_Timer)
		{
			//if((*JOY2DAT & 0x8000) == 0x0 )
			//	KPrintF("SYNC Timer\n");

//bug=1;
			// a timer event has occured, let's see what we've got
			while( (Msg_RTimer = (TIMEREQUEST *)GetMsg((struct MsgPort *)RepP_Timer)) != NULL)
			{
				Msg_RTimer = (TIMEREQUEST *)((ULONG)Msg_RTimer - (ULONG)sizeof(struct Node) - (ULONG)sizeof(UWORD));

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
						SD_OutStanding++;
#if _PRINTF
						i_Alloced += sizeof(SYNCDIALOGUE); 
						printf("3 allocated %d\n",sizeof(SYNCDIALOGUE));
#endif
						Msg_SyncDial->sd_Msg.mn_Node.ln_Type = NT_MESSAGE;
						Msg_SyncDial->sd_Msg.mn_Node.ln_Pri = 0;
						Msg_SyncDial->sd_Msg.mn_Node.ln_Name = NULL;
						Msg_SyncDial->sd_Msg.mn_ReplyPort = RepP_StoPC;
						Msg_SyncDial->sd_Cmd = SDI_PUNCHCOL;
						Msg_SyncDial->sd_Punch = SyncPunch;
						Msg_SyncDial->sd_TimeReq = Msg_RTimer;

//{ struct ScriptNodeRecord *SNR; char strstr[10]; SNR=SyncPunch->sp_SNR; sprintf(strstr,"%d\n",SNR->PageNr); KPrintF(strstr); }

						PutMsg(Port_StoPC,Msg_SyncDial);
						Remove((struct Node *)Msg_RTimer);

// DEBUG remember this so you can free it later

						if( Msg_REMEMBER == NULL )
							Msg_REMEMBER = Msg_SyncDial;

// END DEBUG

//						sprintf(tt,"f?-%lx\n",Msg_Timer );
//						KPrintF("%s",tt);

					}
					else		
					{
						//KPrintF("Shit hit the fan 1!\n");

						Remove((struct Node *)Msg_RTimer);
						FreeMem(Msg_RTimer,sizeof(TIMEREQUEST));
//					tel--;
//					sprintf(tt,"f1-%lx\n",Msg_Timer );
//					KPrintF("%s",tt);

#if _PRINTF
						i_Alloced -= sizeof(TIMEREQUEST); 
						printf("No memory for syncdial available, Freed %d\n",sizeof(TIMEREQUEST));
#endif
					}
				}
				//else
				//	KPrintF("Shit hit the fan 1!\n");
//if (bug>20) KPrintF("3!\n"); else bug++;
			}
		}

		if(SigRecvd & SigR_StoPC)
		{
			//if((*JOY2DAT & 0x8000) == 0x0 )
			//	KPrintF("SYNC sptoc\n");

//bug=1;
			// a guide replied to our dialogue, free its mem
			while( (Msg_RSyncDial = (SYNCDIALOGUE *)GetMsg((struct MsgPort *)RepP_StoPC)) != NULL)
			{
				SD_OutStanding--;

				FreeMem(Msg_RSyncDial->sd_TimeReq,sizeof(TIMEREQUEST));
//				tel--;
//				sprintf(tt,"f2-%lx\n",Msg_RSyncDial->sd_TimeReq );
//				KPrintF("%s",tt);
				FreeMem(Msg_RSyncDial,sizeof(SYNCDIALOGUE));

				if( Msg_RSyncDial == Msg_REMEMBER )
					Msg_REMEMBER = NULL;

//if (bug>20) KPrintF("4!\n"); else bug++;
			}
		}

		if(RemoveOK)
		{
			if(
				((struct List *)TRList->lh_TailPred == (struct List *)TRList) &&
				(SD_OutStanding == 0)
			  )
				TermOK = TRUE;
			else
			{
				if(AbortTimeOut++ == 100)
					TermOK = TRUE;
				else
				{
					Signal(&(ThisPI->pi_Process->pr_Task),SigR_Timer);
					Signal(Port_StoPC->mp_SigTask,1<<Port_StoPC->mp_SigBit);
				}
			}

#if _PRINTF
			printf("Outstanding punches = %d\n",SD_OutStanding);
			printf("TailPred = %x, TRList = %x\n",(int)TRList->lh_TailPred,(int)TRList);
#endif
		}
#if _PRINTF
		printf("Total used = %d\n",i_Alloced);
#endif
	}

	Msg_TimerTemplate->tr_time.tv_secs = 0;
	Msg_TimerTemplate->tr_time.tv_micro = 20;
	DoIO((struct IORequest *)Msg_TimerTemplate);

	SyncGuide = (SYNCGUIDE *)SGList->lh_Head;
	while( NextSyncGuide = (SYNCGUIDE *)SyncGuide->sg_Node.ln_Succ)
	{
		FreeMem(SyncGuide, sizeof(SYNCGUIDE));
#if _PRINTF
		printf("4 Freed %d\n",sizeof(SYNCGUIDE));
#endif
		SyncGuide = NextSyncGuide;
	}

	// Close the timer device
	CloseDevice( (struct IORequest *)Msg_TimerTemplate);
	DeleteExtIO( (struct IORequest *)Msg_TimerTemplate);

	DeletePort(RepP_Timer);

	FreeMem(TRList, sizeof(struct List));

	FreeMem(SGList, sizeof(struct List));

	DeletePort(RepP_StoPC);

	DeletePort(Port_PCtoS);

//	sprintf(tt,"Total tels %d\n",tel );
//	KPrintF("%s",tt );

//		sprintf(tt,"remember i s%lx\n",Msg_REMEMBER );
//		KPrintF("%s",tt );

	if( Msg_REMEMBER )
	{
		FreeMem(Msg_REMEMBER->sd_TimeReq,sizeof(TIMEREQUEST));
		FreeMem(Msg_REMEMBER,sizeof(SYNCDIALOGUE));
	}
}
