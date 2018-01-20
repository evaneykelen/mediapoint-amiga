#ifndef MEDIAPOINT_SCALE_H
#define MEDIAPOINT_SCALE_H


/*- scale.h -----------------------------------------------------------
  Includefile for definitions for the FastBitMapScale routine. This
  routine is written to support BitMap scale of HAM, EHB and normal
  pictures under both all versions of the kickstart ROM.
  All code written by Pascal Eeftinck, BOMB AudioVisual Entertainment

  $VER: mediapoint_scale.h 1.003 (20/08/93)
--------------------------------------------------------------------*/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif


#define SCALEF_CONVERT_TO_GRAY	1		/* enable conversion to grayscales */
#define SCALEF_CONVERT_HAM			2		/* if we convert, convert HAM too */
																		/* SCALE_CONVERT_TO_GRAY must be ON */
#define SCALEF_TRANSPARENT			4		/* enable transparent BGND color */
																		/* SCALE_CONVERT_TO_GRAY must be ON */
#define SCALEF_USE_AVERAGE			8		/* calculate grayscale average of skipped pixels */
																		/* SCALE_CONVERT_TO_GRAY must be ON */

#define SCALEB_CONVERT_TO_GRAY	0
#define SCALEB_CONVERT_HAM			1
#define SCALEB_TRANSPARENT			2
#define	SCALEB_USE_AVERAGE			3

struct FastBitScaleArgs {
	UWORD		fbsa_SrcX;
	UWORD		fbsa_SrcY;							/* source origin */
	UWORD		fbsa_SrcWidth;
	UWORD		fbsa_SrcHeight;					/* source size */
	UWORD		fbsa_XSrcFactor;
	UWORD		fbsa_YSrcFactor;				/* scale factor denominators */
	UWORD		fbsa_DestX;
	UWORD		fbsa_DestY;							/* destination origin */
	UWORD		fbsa_DestWidth;
	UWORD		fbsa_DestHeight;				/* destination size result */
	UWORD		fbsa_XDestFactor;
	UWORD		fbsa_YDestFactor;				/* scale factor numerators */
	struct BitMap *fbsa_SrcBitMap;	/* source BitMap */
	struct BitMap *fbsa_DestBitMap;	/* destination BitMap */
	ULONG		fbsa_Flags;							/* see above for definitions */
	struct ColorMap *fbsa_SrcColorMap;
	ULONG		fbsa_SrcViewModes;
};

extern void FastBitMapScale( struct FastBitScaleArgs * );


#endif /* MEDIAPOINT_SCALE_H */
