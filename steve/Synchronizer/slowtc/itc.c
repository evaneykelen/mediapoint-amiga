#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <exec/execbase.h> 
#include <devices/timer.h>
#include <devices/serial.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <resources/cia.h>

#include <workbench/startup.h>
#include <clib/exec_protos.h>
#include <clib/cia_protos.h>
#include <clib/misc_protos.h>

#include <pragmas/misc_pragmas.h>
#include <resources/misc.h>

#include <stdlib.h>
#include <stdio.h>

#include "nb:pre.h"

#include "minc:types.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "external.h"

#define _MAIN_ON FALSE

extern struct Custom custom;

struct Library *MiscBase;
struct Library *BattMemBase;
struct Library *ciasbase;
UWORD OldICRMask;

// Data field for both the TC interrupt
SYNCDATA	 		ITCSyncData;

// Interrupt for TimeCode generation
struct Interrupt 	ITCInt;
struct Interrupt 	*Int_OwnerTimerB;

// Serial control
struct MsgPort		*Port_Serial;	// Port for use by Serial Device

/**************************************************************
*Func : Pulse the framecntr to add another frame to itself
*		actual checking is done by the vert blank itc
*in   : -
*out  : -
*/
void PulseFrameCntr( void)
{
  ULONG CurFrame;

	CurFrame = ITCSyncData.sd_FCFastCheck;
	// actual frame cntr
	CurFrame += 0x00000001;
	if(((UBYTE)(CurFrame & 0x000000ff)) == ITCSyncData.sd_TimeRate)
	{
		CurFrame &= 0xffffff00;
		CurFrame += 0x00000100;
		if((CurFrame & 0x0000ff00) == (60<<8))
		{
			CurFrame += 196<<8;
			if((CurFrame & 0x00ff0000) == (60<<16))
			{
				CurFrame += 196<<16;
				if((CurFrame & 0xff000000) == (24<<24))
					CurFrame &= 0x00ffffff;
			}
		}
	}
	ITCSyncData.sd_FCFastCheck = CurFrame;
}

/***********************************************************
*Func : interrupt handler for TimerB ints
*in   : Data -> ptr to ITCSyncData struct
*out  : -
*/
int __asm ITCIntHandler( register __a1 SYNCDATA *SD)
{
  ULONG CurFrame;
  struct List *TRList;
  TIMEREQUEST *Msg_Timer;
  BOOL B_SendSignal;

	SD->sd_B_Field = !SD->sd_B_Field;

	if(SD->sd_B_Field)
		return(0);

	if(!SD->sd_B_Count)
		return(0);

	TRList = SD->sd_TRList;
	CurFrame = SD->sd_FCFastCheck;

	if(SD->sd_B_DoCheck)
	{
		B_SendSignal = FALSE;
		// Do frame checking
		for(Msg_Timer = (TIMEREQUEST *)TRList->lh_Head;
			Msg_Timer->tr_Node.ln_Succ != NULL;
			Msg_Timer = (TIMEREQUEST *)Msg_Timer->tr_Node.ln_Succ)
		{
			if(Msg_Timer->tr_ColState == CS_WAIT)
			{	
				if(Msg_Timer->tr_Punch->sp_PunchInfo.pi_FCFastCheck <= CurFrame)
				{
					Msg_Timer->tr_ColState = CS_FRAMECOL;
					B_SendSignal = TRUE;
				}	
				if(Msg_Timer->tr_Punch->sp_PunchType == PT_NOPUNCH)
				{
					Msg_Timer->tr_ColState = CS_FRAMECOL;
					B_SendSignal = TRUE;
				}
			}
		}
		if(B_SendSignal)
			Signal(SD->sd_Task,SD->sd_Sig_TCItoS);
	}

	SD->sd_FrameNr++;

	// if Filled then there is a serial output request
	if(SD->sd_B_TimeCodeOut)
	{	
		switch(SD->sd_FrameNr & 0x03)
		{
			case 0:		// Frames	001xxxxx
				SD->sd_Custom->serdat = (UWORD)(*(((UBYTE *)&CurFrame)+3)) | 0x0120;
				break;
			case 1:		// Seconds  01xxxxxx
				SD->sd_Custom->serdat = (UWORD)(*(((UBYTE *)&CurFrame)+2)) | 0x0140;
				break;
			case 2:		// Minutes  10xxxxxx
				SD->sd_Custom->serdat = (UWORD)(*(((UBYTE *)&CurFrame)+1)) | 0x0180;
				break;
			case 3:		// Hours    110xxxxx
				SD->sd_Custom->serdat = (UWORD)(*((UBYTE *)&CurFrame)) | 0x01c0;
				break;
		}
	}
	return(0);
}

/*************************************************
*func : add an interruptserver to the vblnk
*		of CIAB
*in   : SIR -> information on TimeCode format
*		Sig_ITCItoS -> signal used to reach the synchronizer
*out  : TRUE -> ok
*		FALSE -> Error
*/
int	InitITCInt( SIR, Sig_ITCItoS)
struct ScriptInfoRecord *SIR;
ULONG Sig_ITCItoS;
{
  ULONG HH,MM,SS,FF;

	ITCSyncData.sd_B_TimeCodeOut = FALSE;

#if !_MAIN_ON 
	if(SIR->timeCodeOut)
#endif
	{
		if((MiscBase = OpenResource(MISCNAME)) == NULL)
			return(FALSE);

		ITCSyncData.sd_B_TimeCodeOut = TRUE;
		ITCSyncData.sd_Custom = &custom;
		
		if(AllocMiscResource(MR_SERIALPORT,"MediaLink_TimeCode"))
			return(FALSE);
		if(AllocMiscResource(MR_SERIALBITS,"MediaLink_TimeCode"))
		{
			FreeMiscResource(MR_SERIALPORT);
			return(FALSE);
		}
		
		custom.serper = 0x0071;		//midi speed
//		custom.serper = 0x0175;		//9600 baud
	}

	ITCInt.is_Node.ln_Type = NT_INTERRUPT;	
	ITCInt.is_Node.ln_Pri = 100;	
	ITCInt.is_Node.ln_Name = "Int_ITC";
	ITCInt.is_Data = (APTR)&ITCSyncData;
	ITCInt.is_Code = (APTR)ITCIntHandler;	

	ITCSyncData.sd_B_Field = TRUE;			// start with even field
	ITCSyncData.sd_Task = FindTask(NULL);
	ITCSyncData.sd_Sig_TCItoS = Sig_ITCItoS;
	ITCSyncData.sd_B_Count = FALSE;			// don't count frames yet
	ITCSyncData.sd_FrameNr = 0;				// start at frame 0, counts vblnks 

#if _MAIN_ON 
	ITCSyncData.sd_TimeRate = 25;
	ITCSyncData.sd_FCFastCheck = 0;
#else
	switch(SIR->timeCodeRate)
	{
		case TIMERATE_25FPS:
			    ITCSyncData.sd_TimeRate = 25;
				break;
		case TIMERATE_30FPS:
			    ITCSyncData.sd_TimeRate = 30;
				break;
		default:
			    ITCSyncData.sd_TimeRate = 25;
				break;
	}

	HH = (SIR->Offset.TimeCode.HH & 0xff) << 24;
	MM = (SIR->Offset.TimeCode.MM & 0xff) << 16;
	SS = (SIR->Offset.TimeCode.SS & 0xff) << 8;
	FF = SIR->Offset.TimeCode.FF & 0xff;
	ITCSyncData.sd_FCFastCheck = HH|MM|SS|FF;
#endif
    AddIntServer(INTB_VERTB,&ITCInt);

 	return(TRUE);
}

/**************************************************
*Func : Free the TimeCode generator
*in   : -
*out  : -
*/
void FreeITCInt( SIR)
struct ScriptInfoRecord *SIR;
{
	// make sure previous byte was sent
	Delay(1);
	if(SIR->timeCodeOut)
		custom.serdat = 0x01e0;			// Send 'EXIT' command to other modules

	RemIntServer(INTB_VERTB,&ITCInt);
	
#if !_MAIN_ON 
	if(SIR->timeCodeOut)
#endif
	{
		FreeMiscResource(MR_SERIALBITS);
		FreeMiscResource(MR_SERIALPORT);
	}
}

#if _MAIN_ON
void main( argc, argv)
int argc;
char **argv;
{
  int TermOK;
  ULONG SigRecvd;

	if(!InitITCInt(NULL, 1<<AllocSignal(-1)))
		return;

	ITCSyncData.sd_B_Count = TRUE;		// don't count frames yet
	TermOK = FALSE;
	while(!TermOK)
	{
		SigRecvd = Wait(SIGBREAKF_CTRL_E|ITCSyncData.sd_Sig_TCItoS);

		if(SigRecvd & ITCSyncData.sd_Sig_TCItoS)
			printf("%08x\n",ITCSyncData.sd_FCFastCheck);

		if(SigRecvd & SIGBREAKF_CTRL_E)
			TermOK = TRUE;
	}

	FreeITCInt(NULL);
	return;
}

#endif
