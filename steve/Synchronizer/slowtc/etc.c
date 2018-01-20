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
#include "external.h"

#define _MAIN_ON FALSE

extern struct Custom custom;

extern struct Library *MiscBase;
extern struct Library *BattMemBase;
extern UWORD OldICRMask;

// Data field for both the TC interrupt
SYNCDATA	 		ETCSyncData;

// Interrupt for TimeCode generation
struct Interrupt 	ETCInt;
struct Interrupt	*RBFOwner;
BOOL				RBFEnabled;	// state of interrupt bits before take over


/***********************************************************
*Func : interrupt handler for Serial Interrupts
*in   : Data -> ptr to ETCSyncData struct
*out  : -
*/
void __asm ETCIntHandler( register __a1 SYNCDATA *SD)
{
  UBYTE SerData;
  UBYTE *CurFrame; 

	SerData = (UBYTE)(SD->sd_Custom->serdatr & 0x00ff);
	CurFrame = (UBYTE *)&SD->sd_FCFastCheck;

	if(SerData == 0xe0)
	{
		SD->sd_SpecCmd = SCC_EXIT;
		Signal(SD->sd_Task,SD->sd_Sig_TCItoS);
	}
	else
	{
		switch(SerData & 0xc0)
		{	
			case 0x00:
					SD->sd_TimeRate = 1;	
					*(CurFrame+3) = SerData & 0x1f;
					break;
			case 0x40:
					*(CurFrame+2) = SerData & 0x3f;
					SD->sd_TimeRate++;	
					break;
			case 0x80:
					*(CurFrame+1) = SerData & 0x3f;
					SD->sd_TimeRate++;	
					break;
			case 0xc0:
					*CurFrame = SerData & 0x1f;
					SD->sd_TimeRate++;
					break;
		}
		if(SD->sd_TimeRate == 4)
			Signal(SD->sd_Task,SD->sd_Sig_TCItoS);
	}

	SD->sd_Custom->intreq = INTF_RBF;
}
/*************************************************
*func : add an interrupthandler to the RBF
*		of CIAB
*in   : SIR -> information on TimeCode format
*		Sig_ITCItoS -> signal used to reach the synchronizer
*out  : TRUE -> ok
*		FALSE -> Error
*/
int	InitETCInt( SIR, Sig_ETCItoS)
struct ScriptInfoRecord *SIR;
ULONG Sig_ETCItoS;
{
	if((MiscBase = OpenResource(MISCNAME)) == NULL)
		return(FALSE);

	ETCSyncData.sd_B_TimeCodeOut = TRUE;
	ETCSyncData.sd_Custom = &custom;
		
	if(AllocMiscResource(MR_SERIALPORT,"MediaLink_TimeCodeRec"))
		return(FALSE);

	if(AllocMiscResource(MR_SERIALBITS,"MediaLink_TimeCodeRec"))
	{
		FreeMiscResource(MR_SERIALPORT);
		return(FALSE);
	}
		
	custom.serper = 0x0071;		//midi speed
//	custom.serper = 0x0175;		//9600 baud

	ETCInt.is_Node.ln_Type = NT_INTERRUPT;	
	ETCInt.is_Node.ln_Pri = 0;
	ETCInt.is_Node.ln_Name = "Int_ETC";
	ETCInt.is_Data = (APTR)&ETCSyncData;
	ETCInt.is_Code = (APTR)ETCIntHandler;

	ETCSyncData.sd_B_Field = TRUE;			// start with even field
	ETCSyncData.sd_Task = FindTask(NULL);
	ETCSyncData.sd_Sig_TCItoS = Sig_ETCItoS;
	ETCSyncData.sd_B_Count = TRUE;			// receive frames right away
	ETCSyncData.sd_SpecCmd = SCC_NOCMD;

	ETCSyncData.sd_TimeRate = 0;
	ETCSyncData.sd_FCFastCheck = 0;

	// save state of RBF int and disable it
	RBFEnabled = custom.intenar & INTF_RBF ? TRUE: FALSE;
	custom.intena = INTF_RBF;

    RBFOwner = SetIntVector(INTB_RBF,&ETCInt);

	custom.intena = INTF_SETCLR|INTF_RBF;

 	return(TRUE);
}

/**************************************************
*Func : Free the TimeCode receiver
*in   : -
*out  : -
*/
void FreeETCInt( SIR)
struct ScriptInfoRecord *SIR;
{
	custom.intena = INTF_RBF;
    SetIntVector(INTB_RBF,RBFOwner);
	if(RBFEnabled)
		custom.intena = INTF_SETCLR|INTF_RBF;

	FreeMiscResource(MR_SERIALBITS);
	FreeMiscResource(MR_SERIALPORT);
}

#if _MAIN_ON
void main( argc, argv)
int argc;
char **argv;
{
  int TermOK;
  ULONG SigRecvd;

	if(!InitETCInt(NULL, 1<<AllocSignal(-1)))
		return;

	TermOK = FALSE;
	while(!TermOK)
	{
		SigRecvd = Wait(SIGBREAKF_CTRL_E|ETCSyncData.sd_Sig_TCItoS);

		if(SigRecvd & ETCSyncData.sd_Sig_TCItoS)
			printf("%08x\n",ETCSyncData.sd_FCFastCheck);

		if(SigRecvd & SIGBREAKF_CTRL_E)
			TermOK = TRUE;
	}

	FreeETCInt(NULL);
	return;
}

#endif
