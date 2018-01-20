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

extern struct Library *MiscBase;
extern struct Library *BattMemBase;
extern UWORD OldICRMask;

// Data field for both the TC interrupt
SYNCDATA	 		ETCSyncData;
RBFDATA	 			RBFIntData;

// Interrupt for TimeCode generation
struct Interrupt 	RBFInt;
struct Interrupt	*RBFOwner;
BOOL				RBFEnabled;	// state of interrupt bits before take over


/***********************************************************
*Func : interrupt handler for Serial Interrupts
*		Read 
*in   : Data -> ptr to ETCSyncData struct
*out  : -
*/
void __asm RBFIntHandler( register __a1 RBFDATA *RD)
{
  UBYTE SerData;
  UBYTE *CurFrame; 
  SYNCDATA *SD;
  int lSerData;

	SD = RD->rd_SyncData;

	SerData = (UBYTE)(SD->sd_Custom->serdatr & 0x00ff);
	CurFrame = (UBYTE *)&SD->sd_FCFastCheck;

	// Frame part header identifier
	if(SerData == 0xf1)
	{
		RD->rd_DataExpected = DE_QUARTERFRAME;
	}
	else
	{
		if(SerData == 0xf0)
		{
			RD->rd_DataExpected = DE_FULLFRAME;
			RD->rd_BufIndex = 0;
		}
		else	
		{
			if(RD->rd_DataExpected == DE_QUARTERFRAME)
			{
				lSerData = (SerData & 0xf0) >> 4;

				if(lSerData == 0)
					RD->rd_BufLength = 0;

				RD->rd_BufLength++;

				RD->rd_Buffer[lSerData] = SerData;

				// received last quarter frame and total received quarter frames is 8
				if((lSerData == 7) && (RD->rd_BufLength == 8))
				{
					*(CurFrame+3) = (RD->rd_Buffer[0] & 0x0f) | ((RD->rd_Buffer[1] & 0x01) << 4);
					*(CurFrame+2) = (RD->rd_Buffer[2] & 0x0f) | ((RD->rd_Buffer[3] & 0x03) << 4);
					*(CurFrame+1) = (RD->rd_Buffer[4] & 0x0f) | ((RD->rd_Buffer[5] & 0x03) << 4);
					*CurFrame =     (RD->rd_Buffer[6] & 0x0f) | ((RD->rd_Buffer[7] & 0x01) << 4);

					RD->rd_TimeCodeType = RD->rd_Buffer[7] & 0x06;
					switch(RD->rd_TimeCodeType)
					{
						case 0x02:
					    		SD->sd_TimeRate = 25;
								break;
						case 0x06:
					    		SD->sd_TimeRate = 30;
								break;
					}
					Signal(SD->sd_Task,SD->sd_Sig_TCItoS);
				}	
			}
			else
			{
				if(RD->rd_DataExpected == DE_FULLFRAME)
				{
					// check on real time and non-real time messages only	
					if( (RD->rd_BufIndex == 0) && ((SerData & 0xfe) != 0x7e))
						RD->rd_DataExpected = DE_NONE;
					else
					{
						RD->rd_Buffer[RD->rd_BufIndex++] = SerData;
						if(SerData == 0xf7)
						{
							// check for real time msg
							if(RD->rd_Buffer[0] == 0x7f)
							{
								switch(RD->rd_Buffer[3])
								{
									case 1:	// full midi tim code message
											*(CurFrame+3) = RD->rd_Buffer[7] & 0x1f;
											*(CurFrame+2) = RD->rd_Buffer[6] & 0x3f;
											*(CurFrame+1) = RD->rd_Buffer[5] & 0x3f;
											*CurFrame     = RD->rd_Buffer[4] & 0x1f;
											RD->rd_TimeCodeType = (RD->rd_Buffer[4] & 0x60) >> 4;
											if(RD->rd_TimeCodeType == 0x02)
					    						SD->sd_TimeRate = 25;
											else
												if(RD->rd_TimeCodeType == 0x06)
						    						SD->sd_TimeRate = 30;
											break;
									default :
											break;
								}
							}
							else
							{	
								// check for non-real time msg
								if(RD->rd_Buffer[0] == 0x7e)
								{
									switch(RD->rd_Buffer[3])
									{
										case 0:	// special
												if((RD->rd_Buffer[9] == 4) && (RD->rd_Buffer[10] == 0))
												{
													SD->sd_SpecCmd = SCC_EXIT;
													Signal(SD->sd_Task,SD->sd_Sig_TCItoS);
												}
												break;
										default:
												break;
									}
								}	
							}
						}
					}
				}
			}
		}
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
		
	if(AllocMiscResource(MR_SERIALPORT,"MediaLink_MidiCodeRec"))
		return(FALSE);

	if(AllocMiscResource(MR_SERIALBITS,"MediaLink_MidiCodeRec"))
	{
		FreeMiscResource(MR_SERIALPORT);
		return(FALSE);
	}
		
	custom.serper = 0x0071;		//midi speed
//	custom.serper = 0x0175;		//9600 baud

	RBFInt.is_Node.ln_Type = NT_INTERRUPT;	
	RBFInt.is_Node.ln_Pri = 0;
	RBFInt.is_Node.ln_Name = "Int_ETC";
	RBFInt.is_Data = (APTR)&RBFIntData;
	RBFInt.is_Code = (APTR)RBFIntHandler;

	ETCSyncData.sd_B_Field = TRUE;			// start with even field
	ETCSyncData.sd_Task = FindTask(NULL);
	ETCSyncData.sd_Sig_TCItoS = Sig_ETCItoS;
	ETCSyncData.sd_B_Count = TRUE;			// receive frames right away
	ETCSyncData.sd_SpecCmd = SCC_NOCMD;

	ETCSyncData.sd_TimeRate = 0;
	ETCSyncData.sd_FCFastCheck = 0;

	RBFIntData.rd_SyncData = &ETCSyncData;
	RBFIntData.rd_DataExpected = DE_NONE;	// don't expect anything yet
	RBFIntData.rd_BufLength = 0;

	// save state of RBF int and disable it
	RBFEnabled = custom.intenar & INTF_RBF ? TRUE: FALSE;
	custom.intena = INTF_RBF;
    RBFOwner = SetIntVector(INTB_RBF,&RBFInt);
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

	ETCSyncData.sd_B_Count = TRUE;

	TermOK = FALSE;
	while(!TermOK)
	{
		SigRecvd = Wait(SIGBREAKF_CTRL_E|ETCSyncData.sd_Sig_TCItoS);

		if(SigRecvd & ETCSyncData.sd_Sig_TCItoS)
			printf("%08x fps = %d, SpecCmd = %d\n",ETCSyncData.sd_FCFastCheck,(int)ETCSyncData.sd_TimeRate,(int)ETCSyncData.sd_SpecCmd);

		if(SigRecvd & SIGBREAKF_CTRL_E)
			TermOK = TRUE;
	}

	FreeETCInt(NULL);
	return;
}

#endif
