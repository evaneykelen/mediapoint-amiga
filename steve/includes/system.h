
#ifndef MINC_SYSTEM_H
#define  MINC_SYSTEM_H

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

#ifndef GRAPHICS_RASTPORT_H
#include <graphics/rastport.h>
#endif

/**********************************************************
*Desc : Information on a View system for use by
*		applications other than the originator.
*/

typedef struct
{
  struct View 			dp_View;
  struct ViewPort 		dp_ViewPort; 		    
  struct RasInfo 		dp_RasInfo;
  struct RastPort 		dp_RastPort;
  struct BitMap 		dp_BitMap;
  struct ColorMap 		*dp_ColorMap;
  struct ViewPortExtra	*dp_vpextra;	// NEW 5-aug-93
  struct ViewExtra		*dp_vextra;  	// NEW 5-aug-93
} DISPLAY;

/************************************************************
*Desc : These buffers are allocated by the ProcessInitialiser
*		and may be used by any application that needs to have
*		access to the screen
*/

typedef struct
{
  UWORD    	*sb_Base;	// Where is the buffer located in memory
  ULONG		sb_Size;	// size in bytes of the buffer
  BOOL 		sb_Viewed;	// if TRUE, this buffer is current shown on screen
  BOOL 		sb_InUse;	// if TRUE, this buffer is currently used by an application
  DISPLAY 	sb_Display;	// Info on the display that uses this buffer
} SCREENBUFFER;

#define MAXSCREENBUFS 3

/************************************************************
*Desc : MediaLink system structure with data which may be used 
*		by any application at any time. 
*/
typedef struct
{
  SCREENBUFFER ms_ScreenBufs[MAXSCREENBUFS]; // Chipmem buffers free for use
  ULONG monitorID;	// DEFAULT_MONITOR_ID, PAL_MONITOR_ID etc.
  ULONG miscFlags;	// bit 0: 1=PAL				0=NTSC				0x00000001
					// bit 1: 1=TIMECODE		0=HHMMSST			0x00000002
					// bit 2: 1=NORMAL			0=PRECISE			0x00000004
					// bit 3: 1=MANUAL			0=AUTOMATIC			0x00000008
					// bit 4: 1=ALWAYS LACED	0=SOMETIMES LACED	0x00000010
					// bit 5: 1=GENLOCKED		0=NOT GENLOCKED		0x00000020
					// bit 6: 1=MEMF_STAY   	0=DON'T MEMF_STAY	0x00000040
	ULONG extend[8];	// future use
  struct TagItem *taglist;
  struct List *VIList;	// list of variable declarations
  struct SignalSemaphore ms_Sema_Transition, ms_Sema_Music;
  TEXT xappPath[150];
  UWORD refreshRate;
} MLSYSTEM;

#endif // MINC_SYSTEM_H
