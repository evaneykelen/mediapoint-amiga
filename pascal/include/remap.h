#ifndef MEDIAPOINT_REMAP_H
#define MEDIAPOINT_REMAP_H

/************************************************************************

 definitions for FastBitMapRemap function

 $VER: remap.h 01.002 (19/05/93)

************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif


struct FastRemapInfo {
	struct BitMap *SrcBitMap;
	struct BitMap *DstBitMap;
	struct ColorMap *SrcColorMap;
	struct ColorMap *DstColorMap;
	ULONG  SrcViewModes;
	ULONG  DstViewModes;
	UWORD  Flags;
	};

#define	REMAPB_TRANSPARENT	0

#define	REMAPF_TRANSPARENT	(1<<REMAPB_TRANSPARENT)


extern BOOL FastBitMapRemap( struct FastRemapInfo * );


#endif /* MEDIAPOINT_REMAP_H */
