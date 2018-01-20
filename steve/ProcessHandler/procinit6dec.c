#include <workbench/startup.h>
#include <exec/types.h>
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
#include "external.h"

#define _PRINTF FALSE

extern MLSYSTEM	*MLSystem;	// MediaLink Global System structure
extern TEXT *dir_xapps;

STATIC void HasThisScriptMusicAndPages(struct ScriptInfoRecord *SIR, BOOL *music, BOOL *pages);

UWORD *RememberBuf0, *RememberBuf3;

/************************************************************
*Func : Initialise (allocate and setup) all global system
*		memory areas. Shared memory used by different XaPPs
*		as for WorkPage and WorkAnim are all allocated in here
*		ALL MLSystem vars MUST only be initialised by this
*		function.
*in   : -
*out  : NO_ERROR -> ok
*/

int ProcessInitializer( struct ScriptInfoRecord *SIR )
{
int	NrColors;
ULONG total, t, size, size3, left, min;
BOOL music, pages;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);

	total = AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024;

//PrintSer("total=%d\n",total);

	HasThisScriptMusicAndPages(SIR, &music, &pages);

//PrintSer("music=%d  page=%d\n",music,pages);

	if ( music )
		left = 512;
	else
		left = 256;

	if ( CPrefs.PalNtsc == PAL_MODE )
		min = 170;	// kB --> ~ 640*512*4
	else
		min = 135;	// kB --> ~ 640*400*4

	t = total - left;
	size = t / 3;
	if ( size < min )
	{
		t = total - 256;	// when there's no music (left==256) then this's done twice -
		size = t / 3;		// but that doesn't matter...
		if ( size < min )
		{
			t = total - 128;
			size = t / 3;
			if ( size < min )
			{
				t = total - 64;
				size = t / 3;
				if ( size < (min/2) )
				{
					CloseLibrary((struct Library *)GfxBase);
					return(ERR_NOGRAPHICSMEM);
				}
//				else
//					PrintSer("case 4\n");
			}
//			else
//				PrintSer("case 3\n");
		}
//		else
//			PrintSer("case 2\n");
	}
//	else
//		PrintSer("case 1\n");

	size = size * 1024;
	size3 = size;

//PrintSer("->1 size=%d  size3=%d\n",size,size3);

	if ( !pages )	// no docs with flying brushes --> shave buffer 3...
	{
		size3 = size3 / 4;	// make buffer3 1/4 the size of buffer 1 and 2 and
		size += size3;			// add what's left to buffer 1 and 2.
	}

//PrintSer("->2 size=%d  size3=%d\n",size,size3);

	NrColors = 32;
	if( (GfxBase->ChipRevBits0 & SETCHIPREV_AA) == SETCHIPREV_AA)
		NrColors = 256;

//PrintSer("NrColors %d\n",NrColors);

	// First two buffers are allocated as continuous memory

	MLSystem->ms_ScreenBufs[0].sb_Base = MLMMU_AllocMem((size*2)+32,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR,NULL);
	RememberBuf0 = MLSystem->ms_ScreenBufs[0].sb_Base;
//PrintSer("1 VOOR DE MASSAGE %x  ",RememberBuf0);
	if ( (ULONG)(MLSystem->ms_ScreenBufs[0].sb_Base) % 8 )
		MLSystem->ms_ScreenBufs[0].sb_Base += 4;
//PrintSer("NA DE MASSAGE %x\n",MLSystem->ms_ScreenBufs[0].sb_Base);
	MLSystem->ms_ScreenBufs[0].sb_Size		= size; 
	MLSystem->ms_ScreenBufs[0].sb_Viewed	= FALSE;
	MLSystem->ms_ScreenBufs[0].sb_InUse		= FALSE;
	MLSystem->ms_ScreenBufs[0].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	MLSystem->ms_ScreenBufs[1].sb_Base		= (UWORD*)((ULONG)MLSystem->ms_ScreenBufs[0].sb_Base + size);
	MLSystem->ms_ScreenBufs[1].sb_Size 		= size;
	MLSystem->ms_ScreenBufs[1].sb_Viewed 	= FALSE;
	MLSystem->ms_ScreenBufs[1].sb_InUse 	= FALSE;
	MLSystem->ms_ScreenBufs[1].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	MLSystem->ms_ScreenBufs[2].sb_Base 		= MLMMU_AllocMem(size3+32,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR,NULL);
	RememberBuf3 = MLSystem->ms_ScreenBufs[2].sb_Base;
//PrintSer("2 VOOR DE MASSAGE %x  ",RememberBuf3);
	if ( (ULONG)(MLSystem->ms_ScreenBufs[2].sb_Base) % 8 )
		MLSystem->ms_ScreenBufs[2].sb_Base += 4;
//PrintSer("NA DE MASSAGE %x\n",MLSystem->ms_ScreenBufs[2].sb_Base);
	MLSystem->ms_ScreenBufs[2].sb_Size 		= size3; 
	MLSystem->ms_ScreenBufs[2].sb_Viewed 	= FALSE;
	MLSystem->ms_ScreenBufs[2].sb_InUse 	= FALSE;
	MLSystem->ms_ScreenBufs[2].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	InitSemaphore(&MLSystem->ms_Sema_Transition);
	InitSemaphore(&MLSystem->ms_Sema_Music);

	CloseLibrary((struct Library *)GfxBase);

	if( RememberBuf0 == NULL || RememberBuf3 == NULL )
		return(ERR_NOGRAPHICSMEM);

	/**** fill ML system ****/

	MLSystem->monitorID = CPrefs.monitorID;

	MLSystem->miscFlags = 0L;

	if ( CPrefs.PalNtsc == PAL_MODE )
		MLSystem->miscFlags |= 0x00000001L;

	if( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS )
		MLSystem->miscFlags |= 0x00000002L;
	
	if ( CPrefs.scriptTiming==0 )	// 0 = normal, 1 = precise 
		MLSystem->miscFlags |= 0x00000004L;

	if ( CPrefs.playOptions==2 )	// 1=auto, 2=manual, 3=auto+manual
		MLSystem->miscFlags |= 0x00000008L;

	strcpy(MLSystem->xappPath, dir_xapps);

	return(NO_ERROR);
}

/*************************************************************
*Func : Free all fields of the MLSystem structure
*in   : -
*out  : -
*/

void ProcessDeInitializer( void)
{
  int i;

	for( i = 0; i < 3; i++) 
	{
 		if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort.ColorMap)
	 		FreeColorMap(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort.ColorMap);

    	FreeVPortCopLists(&MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort);

    	if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.LOFCprList)
		    FreeCprList(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.LOFCprList);    
    	if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.SHFCprList)
		    FreeCprList(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.SHFCprList);    

		FreeColorMap(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ColorMap);
	}

	MLMMU_FreeMem(RememberBuf0);
	MLMMU_FreeMem(RememberBuf3);
}

/******** TinyProcessInitializer(() ********/

int TinyProcessInitializer(void)
{
	InitSemaphore(&MLSystem->ms_Sema_Transition);
	InitSemaphore(&MLSystem->ms_Sema_Music);

	/**** fill ML system ****/

	MLSystem->monitorID = CPrefs.monitorID;

	MLSystem->miscFlags = 0L;

	if ( CPrefs.PalNtsc == PAL_MODE )
		MLSystem->miscFlags |= 0x00000001L;
	
	if ( CPrefs.scriptTiming==0 )	// 0 = normal, 1 = precise 
		MLSystem->miscFlags |= 0x00000004L;

	if ( CPrefs.playOptions==2 )	// 1=auto, 2=manual, 3=auto+manual
		MLSystem->miscFlags |= 0x00000008L;

	strcpy(MLSystem->xappPath, dir_xapps);

	return(NO_ERROR);
}

/******** TinyProcessDeInitializer(() ********/

void TinyProcessDeInitializer(void)
{
}

/******** HasThisScriptMusicAndPages() ********/

STATIC void HasThisScriptMusicAndPages(struct ScriptInfoRecord *SIR, BOOL *music, BOOL *pages)
{
int i;
SNRPTR this_node;
struct List *list;

	*music = FALSE;
	*pages = FALSE;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
					this_node->node.ln_Succ;
					this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if ( this_node->nodeType==TALK_SOUND )
						*music=TRUE;
					if ( this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 )
						*pages=TRUE;
					if ( *music && *pages )
						return;
				}
			}
		}
	}
	return;
}

/******** E O F ********/
