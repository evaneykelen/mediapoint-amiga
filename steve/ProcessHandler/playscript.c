#include <workbench/startup.h>
#include <exec/types.h>
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

/**** externals ****/

int PageCntr,SerGuideCntr;
ULONG *SFC, *EFC;
BOOL B_FrameCodeErr;

/******************************************************
*Func : add a Backlist ptr into each level of the list
*		pointing to the list owner
*in   : List -> this current list
*		BackSNR -> Ptr to owner SNR of this list
*out  : -
*/
void MakeBackList( SIR, CurList, ParentSNR, PageCntr, SerGuideCntr)
struct ScriptInfoRecord *SIR;
struct List *CurList;
SNR *ParentSNR;
int *PageCntr, *SerGuideCntr;
{
  SNR *CurSNR;

	for(CurSNR = (SNR *)CurList->lh_Head;	
		CurSNR->node.ln_Succ != NULL;
		CurSNR = (SNR *)CurSNR->node.ln_Succ)
	{
		CurSNR->ProcInfo = NULL;
		CurSNR->miscFlags &= ~OBJ_OUTDATED;		
		switch(CurSNR->nodeType)
		{
			// standard XaPP
			case TALK_ANIM:
			case TALK_AREXX:
			case TALK_DOS:
			case TALK_VARS:	// NEW
			case TALK_PAGE:
			case TALK_SOUND:
			case TALK_USERAPPLIC:
				if(RunMode == RM_TIMECODE)
				{
					SFC = (ULONG *)&CurSNR->Start.TimeCode.HH;
					EFC = (ULONG *)&CurSNR->End.TimeCode.HH;
					if(*SFC >= *EFC)
						B_FrameCodeErr = TRUE;
				}
				CurSNR->PageNr = (*PageCntr)++;
				CurSNR->ParentSNR = ParentSNR;
				break;
			case TALK_STARTPAR:
				if(RunMode == RM_TIMECODE)
				{
					SFC = (ULONG *)&CurSNR->Start.TimeCode.HH;
					EFC = (ULONG *)&CurSNR->End.TimeCode.HH;
					if(*SFC >= *EFC)
						B_FrameCodeErr = TRUE;
				}
				CurSNR->PageNr = (*PageCntr)++;	
				CurSNR->ParentSNR = ParentSNR;
				MakeBackList(SIR, CurSNR->list,CurSNR,PageCntr,SerGuideCntr);
				break;
			case TALK_STARTSER:
				CurSNR->PlayTrackSNR = 0L;		// used by SPNEXTGOSUBS and SPPREVGOSUBS
				CurSNR->LoadTrackSNR = 0L;		// used by SPNEXTGOSUBS and SPPREVGOSUBS
				CurSNR->PageNr = (*SerGuideCntr)++;
				CurSNR->ParentSNR = ParentSNR;
				MakeBackList(SIR, CurSNR->list,CurSNR,PageCntr,SerGuideCntr);
				break;
			case TALK_GOTO:
				CurSNR->ParentSNR = ParentSNR;
				CurSNR->extraData = (UBYTE *)FindLabel(SIR, CurSNR->objectName);
				break;
			default:
				CurSNR->ParentSNR = ParentSNR;
				break;
		}
	}
}

/******** playScript() ********/

BOOL playScript(struct ScriptInfoRecord *SIR, UBYTE playMode, BOOL initializers, BOOL mlsystem, BOOL deInit)
{
  struct List *MTList;
  int Err=0;
  SNR *Guide1SNR;

	MaxMemSize 		= CPrefs.maxMemSize;
	SNRStackSize 	= CPrefs.gosubStackSize;	// Size of SNR stack as used for GOSUBs

	// START PRELOAD

	if ( CPrefs.objectPreLoading==10 || CPrefs.objectPreLoading==20 )
		PreLoadLevel = 1;	// none
	else
		PreLoadLevel = 5;	// all

	// END PRELOAD

	if(playMode == 1)
		TempoType = TT_PLAY;
	else
		TempoType = TT_RECORD;

	switch(CPrefs.playOptions)
	{
		case 1:
			RunMode	= RM_PRESENTATIONAUTO;
			break;
		case 2:
			RunMode = RM_PRESENTATIONMANUAL;
			break;
		case 3:
			RunMode = RM_INTERACTIVE;
			break;
	}

	ObjectDateCheck = FALSE;

	// With timecode, RunMode must always be RM_TIMECODE

	if( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS)
		RunMode = RM_TIMECODE;
	else
	{
		// Use of Date check should always result in low preload level
		ObjectDateCheck = CPrefs.showDays;

		//if ( ObjectDateCheck )	// Wednesday 06-Jul-94 14:37:10 NEW
		//	PreLoadLevel = 1;		// Wednesday 06-Jul-94 14:37:10 NEW		
	}

	for(Guide1SNR = (SNR *)((SNR *)SIR->allLists[0]->lh_Head)->node.ln_Succ;	
		Guide1SNR->node.ln_Succ != NULL;
		Guide1SNR = (SNR *)Guide1SNR->node.ln_Succ)
		if(Guide1SNR->nodeType == TALK_STARTSER)
			break;

	PageCntr 	 	= 1;
	SerGuideCntr	= 1;

	Guide1SNR->PageNr = SerGuideCntr;
	Guide1SNR->PlayTrackSNR = NULL;
	Guide1SNR->LoadTrackSNR = NULL;

	SerGuideCntr++;

	Guide1SNR->list = SIR->allLists[1];
	Guide1SNR->ParentSNR = Guide1SNR;

	B_FrameCodeErr = FALSE;
	MakeBackList(SIR, Guide1SNR->list,Guide1SNR,&PageCntr,&SerGuideCntr);

	if(!B_FrameCodeErr || (TempoType == TT_RECORD))
	{
		/* execute the scriptobjects */
		PageCntr--;
		do
		{
			Err = pc_ProcessScript(SIR,Guide1SNR,initializers,mlsystem,deInit);
		}
		while(Err == ERR_QUIT_REQUEST);
	}
	else
		Err = ERR_ILLEGALTIMECODE;
	
	ProcessError(Err);

	MTList = MLMMU_GetMemList();

	if ( CPrefs.bufferOptions==1 )
	{
		MLMMU_FlushMem(MEMF_ALL);
		MLMMU_ReallyFlushMem(MEMF_ALL);
	}

	return(TRUE);
}

/******** E O F ********/
