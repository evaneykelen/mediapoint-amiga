#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
#include <graphics/videocontrol.h>

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
STATIC void FillMLSystem(struct ScriptInfoRecord *SIR);

UWORD *RememberBuf0, *RememberBuf3;
struct TagItem ti[270];	// see also gl:control.c!!!!!!!!!!!!!!!!!!!!!!!
STATIC BOOL initedOK = FALSE;

/************************************************************
*Func : Initialise (allocate and setup) all global system
*		memory areas. Shared memory used by different XaPPs
*		as for WorkPage and WorkAnim are all allocated in here
*		ALL MLSystem vars MUST only be initialised by this
*		function.
*in   : -
*out  : NO_ERROR -> ok
*/

int ProcessInitializer( struct ScriptInfoRecord *SIR, BOOL initializers )
{
int	NrColors;
ULONG total, t, size, size3, left, min;
BOOL music, pages;

	if ( !initializers )
		return(NO_ERROR);

	initedOK = FALSE;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);

	total = AvailMem(MEMF_CHIP | MEMF_LARGEST) / 1024;

	// CHECK OUT IF THIS SCRIPT HAS MUSIC AND PAGES WITH CLIPS

	HasThisScriptMusicAndPages(SIR, &music, &pages);

	if ( music )
		left = 512;
	else
		left = 256;

	if ( CPrefs.PlayerPalNtsc == PAL_MODE )
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
			}
		}
	}

	size = size * 1024;
	size3 = size;

	if ( !pages )	// no docs with flying brushes --> shave buffer 3...
	{
		size3 = size3 / 4;		// make buffer3 1/4 the size of buffer 1 and 2 and
		size += size3;			// add what's left to buffer 1 and 2.
	}

	NrColors = 32;
	if( (GfxBase->ChipRevBits0 & SETCHIPREV_AA) == SETCHIPREV_AA)
		NrColors = 256;

	// First two buffers are allocated as continuous memory

	// BUFFER 0

	MLSystem->ms_ScreenBufs[0].sb_Base = MLMMU_AllocMem((size*2)+32,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR,NULL);
	RememberBuf0 = MLSystem->ms_ScreenBufs[0].sb_Base;
	if ( (ULONG)(MLSystem->ms_ScreenBufs[0].sb_Base) % 8 )
		MLSystem->ms_ScreenBufs[0].sb_Base += 4;
	MLSystem->ms_ScreenBufs[0].sb_Size		= size; 
	MLSystem->ms_ScreenBufs[0].sb_Viewed	= FALSE;
	MLSystem->ms_ScreenBufs[0].sb_InUse		= FALSE;
	MLSystem->ms_ScreenBufs[0].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	// BUFFER 1

	MLSystem->ms_ScreenBufs[1].sb_Base = (UWORD*)((ULONG)MLSystem->ms_ScreenBufs[0].sb_Base + size);
	MLSystem->ms_ScreenBufs[1].sb_Size 		= size;
	MLSystem->ms_ScreenBufs[1].sb_Viewed 	= FALSE;
	MLSystem->ms_ScreenBufs[1].sb_InUse 	= FALSE;
	MLSystem->ms_ScreenBufs[1].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	// BUFFER 2
	
	MLSystem->ms_ScreenBufs[2].sb_Base = MLMMU_AllocMem(size3+32,MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR,NULL);
	RememberBuf3 = MLSystem->ms_ScreenBufs[2].sb_Base;
	if ( (ULONG)(MLSystem->ms_ScreenBufs[2].sb_Base) % 8 )
		MLSystem->ms_ScreenBufs[2].sb_Base += 4;
	MLSystem->ms_ScreenBufs[2].sb_Size 		= size3; 
	MLSystem->ms_ScreenBufs[2].sb_Viewed 	= FALSE;
	MLSystem->ms_ScreenBufs[2].sb_InUse 	= FALSE;
	MLSystem->ms_ScreenBufs[2].sb_Display.dp_ColorMap = GetColorMap(NrColors);

	// SEMAPHORES

	InitSemaphore(&MLSystem->ms_Sema_Transition);
	InitSemaphore(&MLSystem->ms_Sema_Music);

	// VEXTRA AND VPEXTRA

	MLSystem->ms_ScreenBufs[0].sb_Display.dp_vextra = 0;
	MLSystem->ms_ScreenBufs[1].sb_Display.dp_vextra = 0;
	MLSystem->ms_ScreenBufs[2].sb_Display.dp_vextra = 0;

	MLSystem->ms_ScreenBufs[0].sb_Display.dp_vpextra = 0;
	MLSystem->ms_ScreenBufs[1].sb_Display.dp_vpextra = 0;
	MLSystem->ms_ScreenBufs[2].sb_Display.dp_vpextra = 0;

	CloseLibrary((struct Library *)GfxBase);

	if( RememberBuf0 == NULL || RememberBuf3 == NULL )
		return(ERR_NOGRAPHICSMEM);

	// INIT MLSYSTEM
	
	FillMLSystem(SIR);

	initedOK = TRUE;

	return(NO_ERROR);
}

/*************************************************************
*Func : Free all fields of the MLSystem structure
*in   : -
*out  : -
*/

void ProcessDeInitializer(void)
{
int i;

	if ( !initedOK )
		return;
	initedOK = FALSE;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);

	for( i=0; i<3; i++) 
	{
		MLSystem->ms_ScreenBufs[i].sb_Display.dp_vextra = GfxLookUp( &MLSystem->ms_ScreenBufs[i].sb_Display.dp_View );
		MLSystem->ms_ScreenBufs[i].sb_Display.dp_vpextra = GfxLookUp( &MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort );
 		if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort.ColorMap)
	 		FreeColorMap(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort.ColorMap);

	   	FreeVPortCopLists(&MLSystem->ms_ScreenBufs[i].sb_Display.dp_ViewPort);

	   	if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.LOFCprList)
		    FreeCprList(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.LOFCprList);    
	   	if(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.SHFCprList)
		    FreeCprList(MLSystem->ms_ScreenBufs[i].sb_Display.dp_View.SHFCprList);    

		FreeColorMap(MLSystem->ms_ScreenBufs[i].sb_Display.dp_ColorMap);

		if ( GfxBase )
		{
			if( MLSystem->ms_ScreenBufs[i].sb_Display.dp_vextra )
				GfxFree( MLSystem->ms_ScreenBufs[i].sb_Display.dp_vextra );
			if( MLSystem->ms_ScreenBufs[i].sb_Display.dp_vpextra )
				GfxFree( MLSystem->ms_ScreenBufs[i].sb_Display.dp_vpextra );
		}
	}

	WaitBlit();
	if ( GfxBase )
		CloseLibrary((struct Library *)GfxBase);

	MLMMU_FreeMem(RememberBuf0);
	MLMMU_FreeMem(RememberBuf3);
}

/******** TinyProcessInitializer(() ********/

int TinyProcessInitializer(void)
{
	InitSemaphore(&MLSystem->ms_Sema_Transition);
	InitSemaphore(&MLSystem->ms_Sema_Music);
	FillMLSystem(NULL);
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

					if ( this_node->nodeType==TALK_USERAPPLIC )
					{
						if ( !strcmpi(this_node->objectPath,"NewScript") )
							*pages=TRUE;
					}

					if ( *music && *pages )
						return;
				}
			}
		}
	}
}

/******** FillMLSystem() ********/

STATIC void FillMLSystem(struct ScriptInfoRecord *SIR)
{
	MLSystem->monitorID = CPrefs.playerMonitorID;

	MLSystem->miscFlags = 0L;

	if ( CPrefs.PlayerPalNtsc == PAL_MODE )
		MLSystem->miscFlags |= 0x00000001L;

	if ( SIR )
	{
		if( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS )
			MLSystem->miscFlags |= 0x00000002L;
	}

	if ( CPrefs.scriptTiming==0 )	// 0 = normal, 1 = precise 
		MLSystem->miscFlags |= 0x00000004L;

	if ( CPrefs.playOptions==2 )	// 1=auto, 2=manual, 3=auto+manual
		MLSystem->miscFlags |= 0x00000008L;

	MLSystem->miscFlags |= 0x00000010L;	// always laced is default

 	if ( CPrefs.objectPreLoading==10 || CPrefs.objectPreLoading==30 )
		MLSystem->miscFlags |= 0x00000040L;

	ti[0].ti_Tag  = VTAG_ATTACH_CM_SET;
	ti[0].ti_Data = NULL;
	ti[1].ti_Tag  = VTAG_VIEWPORTEXTRA_SET;
	ti[1].ti_Data = NULL;
	ti[2].ti_Tag  = VTAG_NORMAL_DISP_SET;
	ti[2].ti_Data = 0;
	ti[3].ti_Tag  = VTAG_BORDERBLANK_CLR;
	ti[3].ti_Data = NULL;
	ti[4].ti_Tag  = VTAG_CHROMAKEY_SET;
	ti[4].ti_Data = NULL;
	ti[5].ti_Tag  = VTAG_CHROMA_PEN_CLR;
	ti[5].ti_Data = 0;
	ti[6].ti_Tag  = VTAG_END_CM;
	ti[6].ti_Data = NULL;
	MLSystem->taglist = ti;

	strcpy(MLSystem->xappPath, dir_xapps);

	MLSystem->refreshRate = GetRefreshRate(MLSystem->monitorID);
}

/******** GetRefreshRate() ********/

int GetRefreshRate(ULONG ID)
{
	if ( (ID & MONITOR_ID_MASK) == NTSC_MONITOR_ID )
		return( 60 );
	else if ( (ID & MONITOR_ID_MASK) == PAL_MONITOR_ID )
		return( 50 );
	else if ( (ID & MONITOR_ID_MASK) == VGA_MONITOR_ID )
		return( 60 );
	else if ( (ID & MONITOR_ID_MASK) == EURO72_MONITOR_ID )
		return( 70 );
	else if ( (ID & MONITOR_ID_MASK) == EURO36_MONITOR_ID )
		return( 73 );
	else if ( (ID & MONITOR_ID_MASK) == SUPER72_MONITOR_ID )
		return( 72 );
	else if ( (ID & MONITOR_ID_MASK) == DBLNTSC_MONITOR_ID )
		return( 59 );
	else if ( (ID & MONITOR_ID_MASK) == DBLPAL_MONITOR_ID )
		return( 50 );
	else if ( (ID & MONITOR_ID_MASK) == A2024_MONITOR_ID )
		return( 50 );

	return( 50 );
}

/******** E O F ********/
