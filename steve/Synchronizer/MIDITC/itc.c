/********************************************
*Desc : Internal midi timecode generator
*		Timecode is send out to the standard
*		RS232 port.
*		A full timeframe is send in 4 vblnks == 2 frames
*		each timeframe contains 4 bytes
*		Let's say the current timecode is 01:37:52:16 = 01:25:34:10 Hex
* FRAME 0
*					   v-> indicates low nibble frame nr	
*		VBLNK 0 -> 	F1 00		(Low nibble of frame nr)
*				   	F1 11		(high nibble of frame nr
*					   ^-> indicates high nibble frame nr
*					   v-> indicates low nibble seconds nr	
*		VBLNK 1 -> 	F1 24		(Low nibble of seconds nr)
*				   	F1 33		(high nibble of seconds nr
*					   ^-> indicates high nibble seconds nr
* FRAME 1
*					   v-> indicates low nibble minutes nr	
*		VBLNK 2 -> 	F1 45		(Low nibble of minutes nr)
*				   	F1 52		(high nibble of minutes nr
*					   ^-> indicates high nibble minutes nr
*					   v-> indicates low nibble hours nr
*		VBLNK 3 -> 	F1 61		(Low nibble of hours nr)
*				   	F1 76		(high nibble of hours nr
*					   ^-> indicates high nibble hours nr
*		The hours low and high nibble need some more information						
*		A full hours byte has the following bit definitions: x yy zzzzz
*		in which 'x' is undefined is is always set to 0
*		'yy' contain the time code type -> 0 = 24 fps (not supported)
*										   1 = 25 fps
*                                          2 = 30 fps non drop (not supported)
*                                          3 = 30 fps 		
*	   	'zzzzz' finally contains the hours
*/

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
#include <resources/misc.h>

#include <workbench/startup.h>
#include <clib/exec_protos.h>
#include <clib/cia_protos.h>
#include <clib/misc_protos.h>

#include <pragmas/misc_pragmas.h>

#include <stdlib.h>
#include <stdio.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "types.h"
#include "external.h"

#define _MAIN_ON FALSE

extern struct Custom custom;

struct Library *MiscBase;
struct Library *BattMemBase;
struct Library *ciasbase;

// Data field for both the TC interrupt
SYNCDATA	 		ITCSyncData;
TBEDATA	 			TBEIntData;

// Interrupt for TimeCode generation
struct Interrupt 	ITCInt;
struct Interrupt 	TBEInt, *TBE_OwnerInt;

BOOL 				TBE_OwnerFlag;

// Serial control
struct MsgPort		*Port_Serial;	// Port for use by Serial Device

/*********************************************************
*Func : Send a byte
*in   : TD-> ptr to TBEDATA structure
*out  : -
*/
void TransmitByte( TBEDATA *TD)
{
	if(TD->td_BufIndex < TD->td_BufLength)
		TD->td_SyncData->sd_Custom->serdat = (UWORD)TD->td_Buffer[TD->td_BufIndex++] | 0x0100;
}
/***********************************************************
*Func : interrupt handler for serial transmit buffer empty ints
*in   : Data -> ptr to TBESyncData struct
*out  : -
*/
void __asm TBEIntHandler( register __a1 TBEDATA *TD)
{
	TransmitByte(TD);
	TD->td_SyncData->sd_Custom->intreq = INTF_TBE;
}

/***********************************************************
*Func : interrupt handler for vertb ints
*in   : Data -> ptr to ITCSyncData struct
*out  : -
*/
int __asm ITCIntHandler( register __a1 TBEDATA *TD)
{
  ULONG CurFrame;
  struct List *TRList;
  TIMEREQUEST *Msg_Timer;
  BOOL B_SendSignal;
  SYNCDATA *SD;

//	*(UWORD *)0xdff180 = 0x0f0;

	SD = TD->td_SyncData;

	SD->sd_B_Field = !SD->sd_B_Field;

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
	}
	if(B_SendSignal)
		Signal(SD->sd_Task,SD->sd_Sig_TCItoS);

	if(!SD->sd_B_Field)
	{
		// actual frame cntr
		CurFrame += 0x00000001;
		if(((UBYTE)(CurFrame & 0x000000ff)) == SD->sd_TimeRate)
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
		SD->sd_FCFastCheck = CurFrame;
	}

	SD->sd_FrameNr++;
	// if Filled then there is a serial output request
	if(SD->sd_B_TimeCodeOut)
	{	
		switch(SD->sd_FrameNr & 0x03)
		{
			case 0:		// Frames	
				TD->td_Buffer[0] = TD->td_Buffer[2] = 0xf1;
				TD->td_Buffer[1] = ((*(((UBYTE *)&CurFrame)+3)) & 0x0f);
				TD->td_Buffer[3] = ((*(((UBYTE *)&CurFrame)+3)) >> 4) | 0x10;
				TD->td_BufIndex = 0;
				TD->td_BufLength = 4;	// 4 bytes to send
				TransmitByte(TD);		// send first byte
				break;
			case 1:		// Seconds  
				TD->td_Buffer[0] = TD->td_Buffer[2] = 0xf1;
				TD->td_Buffer[1] = ((*(((UBYTE *)&CurFrame)+2)) & 0x0f) | 0x20;
				TD->td_Buffer[3] = ((*(((UBYTE *)&CurFrame)+2)) >> 4) | 0x30;
				TD->td_BufIndex = 0;
				TD->td_BufLength = 4;	// 4 bytes to send
				TransmitByte(TD);		// send first byte
				break;
			case 2:		// Minutes  
				TD->td_Buffer[0] = TD->td_Buffer[2] = 0xf1;
				TD->td_Buffer[1] = ((*(((UBYTE *)&CurFrame)+1)) & 0x0f) | 0x40;
				TD->td_Buffer[3] = ((*(((UBYTE *)&CurFrame)+1)) >> 4) | 0x50;
				TD->td_BufIndex = 0;
				TD->td_BufLength = 4;	// 4 bytes to send
				TransmitByte(TD);		// send first byte
				break;
			case 3:		// Hours   
				TD->td_Buffer[0] = TD->td_Buffer[2] = 0xf1;
				TD->td_Buffer[1] = ((*((UBYTE *)&CurFrame)) & 0x0f) | 0x60;
				TD->td_Buffer[3] = ((*((UBYTE *)&CurFrame)) >> 4) | 0x70 | TD->td_TimeCodeType;
				TD->td_BufIndex = 0;
				TD->td_BufLength = 4;	// 4 bytes to send
				TransmitByte(TD);		// send first byte
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
  int lMaxWait;
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
		
		if(AllocMiscResource(MR_SERIALPORT,"MediaLink_MidiCode"))
			return(FALSE);

		if(AllocMiscResource(MR_SERIALBITS,"MediaLink_MidiCode"))
		{
			FreeMiscResource(MR_SERIALPORT);
			return(FALSE);
		}

		custom.serper = 0x0071;		//midi speed
//		custom.serper = 0x0175;		//9600 baud
	}

	TBEInt.is_Node.ln_Type = NT_INTERRUPT;	
	TBEInt.is_Node.ln_Pri = 0;
	TBEInt.is_Node.ln_Name = "Int_TBE";
	TBEInt.is_Data = (APTR)&TBEIntData;
	TBEInt.is_Code = (void *)TBEIntHandler;	

	TBEIntData.td_SyncData = &ITCSyncData;
	TBEIntData.td_BufIndex = 0;
	TBEIntData.td_BufLength = 0;

	ITCInt.is_Node.ln_Type = NT_INTERRUPT;	
	ITCInt.is_Node.ln_Pri = 0;	
	ITCInt.is_Node.ln_Name = "Int_ITC";
	ITCInt.is_Data = (APTR)&TBEIntData;
	ITCInt.is_Code = (void *)ITCIntHandler;	

	ITCSyncData.sd_B_Field = TRUE;			// start with even field
	ITCSyncData.sd_Task = FindTask(NULL);
	ITCSyncData.sd_Sig_TCItoS = Sig_ITCItoS;
	ITCSyncData.sd_B_Count = FALSE;			// don't count frames yet
	ITCSyncData.sd_FrameNr = 0;

#if _MAIN_ON 
	ITCSyncData.sd_TimeRate = 25;
	ITCSyncData.sd_FCFastCheck = 0;
	TBEIntData.td_TimeCodeType = 0x02;
#else
	switch(SIR->timeCodeRate)
	{
		case TIMERATE_25FPS:
				TBEIntData.td_TimeCodeType = 0x02;
			    ITCSyncData.sd_TimeRate = 25;
				break;
		case TIMERATE_30FPS:
			    ITCSyncData.sd_TimeRate = 30;
				TBEIntData.td_TimeCodeType = 0x06;
				break;
		default:
			    ITCSyncData.sd_TimeRate = 25;
				TBEIntData.td_TimeCodeType = 0x02;
				break;
	}

	HH = (SIR->Offset.TimeCode.HH & 0xff) << 24;
	MM = (SIR->Offset.TimeCode.MM & 0xff) << 16;
	SS = (SIR->Offset.TimeCode.SS & 0xff) << 8;
	FF =  SIR->Offset.TimeCode.FF & 0xff;
	ITCSyncData.sd_FCFastCheck = HH|MM|SS|FF;
#endif

#if !_MAIN_ON 
	if(ITCSyncData.sd_B_TimeCodeOut)
#endif
	{
		TBE_OwnerFlag = custom.intenar & INTF_TBE ? TRUE: FALSE;
    	TBE_OwnerInt = SetIntVector(INTB_TBE,&TBEInt);
		custom.intena = INTF_SETCLR | INTF_TBE;

		// Set up full frame msg for system init
		TBEIntData.td_Buffer[0] = 0xf0;	// real time universal system exclusive header
		TBEIntData.td_Buffer[1] = 0x7f;
		TBEIntData.td_Buffer[2] = 0x7f;	// message intended for entire system
		TBEIntData.td_Buffer[3] = 0x01;	// midi time code
		TBEIntData.td_Buffer[4] = 0x01;	// full time code msg
#if _MAIN_ON
		TBEIntData.td_Buffer[5] = 0;
		TBEIntData.td_Buffer[6] = 0;
		TBEIntData.td_Buffer[7] = 0;
		TBEIntData.td_Buffer[8] = 0;
#else
		TBEIntData.td_Buffer[5] = (SIR->Offset.TimeCode.HH & 0x1f) | (TBEIntData.td_TimeCodeType << 4);
		TBEIntData.td_Buffer[6] = SIR->Offset.TimeCode.MM & 0x3f;
		TBEIntData.td_Buffer[7] = SIR->Offset.TimeCode.SS & 0x3f;
		TBEIntData.td_Buffer[8] = SIR->Offset.TimeCode.FF & 0x1f;
#endif
		TBEIntData.td_Buffer[9] = 0xf7;	// EOX
		TBEIntData.td_BufLength = 10;
		TBEIntData.td_BufIndex = 0;
		TransmitByte(&TBEIntData);

		// make sure previous datastring was sent
		lMaxWait = 10;
		while(lMaxWait-- && (TBEIntData.td_BufIndex < TBEIntData.td_BufLength))
			Delay(1);
	}

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
  int lMaxWait;

	RemIntServer(INTB_VERTB,&ITCInt);

	// make sure previous datastring was sent
	lMaxWait = 10;
	while(lMaxWait-- && (TBEIntData.td_BufIndex < TBEIntData.td_BufLength))
		Delay(1);

#if !_MAIN_ON 
	if(SIR->timeCodeOut)
#endif
	{
		TBE_OwnerFlag = custom.intenar & INTF_TBE ? TRUE: FALSE;
    	TBE_OwnerInt = SetIntVector(INTB_TBE,&TBEInt);
		custom.intena = INTF_SETCLR | INTF_TBE;

		// Set up full frame msg for system init
		TBEIntData.td_Buffer[0] = 0xf0;	// non-real time universal system exclusive header
		TBEIntData.td_Buffer[1] = 0x7e;	
		TBEIntData.td_Buffer[2] = 0x7f;	// message intended for entire system
		TBEIntData.td_Buffer[3] = 0x04;	// midi time code
		TBEIntData.td_Buffer[4] = 0x00;	// special set up (see [10] and [11])
#if _MAIN_ON
		TBEIntData.td_Buffer[5] = 0;
		TBEIntData.td_Buffer[6] = 0;
		TBEIntData.td_Buffer[7] = 0;
		TBEIntData.td_Buffer[8] = 0;
#else
		TBEIntData.td_Buffer[5] = (SIR->Offset.TimeCode.HH & 0x1f) | (TBEIntData.td_TimeCodeType << 4);
		TBEIntData.td_Buffer[6] = SIR->Offset.TimeCode.MM & 0x3f;
		TBEIntData.td_Buffer[7] = SIR->Offset.TimeCode.SS & 0x3f;
		TBEIntData.td_Buffer[8] = SIR->Offset.TimeCode.FF & 0x1f;
#endif
		TBEIntData.td_Buffer[9] = 0;
		TBEIntData.td_Buffer[10] = 4;	// system stop at above timecode
		TBEIntData.td_Buffer[11] = 0;
		TBEIntData.td_Buffer[12] = 0xf7;	// EOX
		TBEIntData.td_BufLength = 13;
		TBEIntData.td_BufIndex = 0;
		TransmitByte(&TBEIntData);

		// make sure previous datastring was sent
		lMaxWait = 10;
		while(lMaxWait-- && (TBEIntData.td_BufIndex < TBEIntData.td_BufLength))
			Delay(1);

		// Make the other devices think this is the new time code, 
		// since they have received the stop cmd just before they will
		// stop afer receiving this matching framecode 
		TBEIntData.td_Buffer[0] = 0xf0;	// real time universal system exclusive header
		TBEIntData.td_Buffer[1] = 0x7f;
		TBEIntData.td_Buffer[2] = 0x7f;	// message intended for entire system
		TBEIntData.td_Buffer[3] = 0x01;	// midi time code
		TBEIntData.td_Buffer[4] = 0x01;	// full time code msg
#if _MAIN_ON
		TBEIntData.td_Buffer[5] = 0;
		TBEIntData.td_Buffer[6] = 0;
		TBEIntData.td_Buffer[7] = 0;
		TBEIntData.td_Buffer[8] = 0;
#else
		TBEIntData.td_Buffer[5] = (SIR->Offset.TimeCode.HH & 0x1f) | (TBEIntData.td_TimeCodeType << 4);
		TBEIntData.td_Buffer[6] = SIR->Offset.TimeCode.MM & 0x3f;
		TBEIntData.td_Buffer[7] = SIR->Offset.TimeCode.SS & 0x3f;
		TBEIntData.td_Buffer[8] = SIR->Offset.TimeCode.FF & 0x1f;
#endif
		TBEIntData.td_Buffer[9] = 0xf7;	// EOX
		TBEIntData.td_BufLength = 10;
		TBEIntData.td_BufIndex = 0;
		TransmitByte(&TBEIntData);

		lMaxWait = 10;
		while(lMaxWait-- && (TBEIntData.td_BufIndex < TBEIntData.td_BufLength))
			Delay(1);

		custom.intena = INTF_TBE;
		SetIntVector(INTB_TBE,TBE_OwnerInt);
		if(TBE_OwnerFlag)
			custom.intena = INTF_SETCLR | INTF_TBE;

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
	{
		FreeITCInt(NULL);
		return;
	}

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
