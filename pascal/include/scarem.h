#ifndef MEDIAPOINT_SCAREM_I
#define MEDIAPOINT_SCAREM_I

/*
**	$VER: mediapoint/pascal/scarem.h 00.038 (20.01.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
*/

/* ScaleRemapInfo */

struct ScaleRemapInfo {
	struct BitMap24	*SrcBitMap;
	struct BitMap24 *DstBitMap;
	ULONG	SrcViewModes;
	ULONG	DstViewModes;
	struct ColorMap *SrcColorMap;
	struct ColorMap *DstColorMap;
	UWORD	SrcX;						/* source origin */
	UWORD	SrcY;
	UWORD	SrcWidth;				/* source size */
	UWORD	SrcHeight;
	UWORD	XSrcFactor;			/* scale factor denominators */
	UWORD	YSrcFactor;
	UWORD	DestX;					/* destination origin */
	UWORD	DestY;
	UWORD	DestWidth;			/* destination size result */
	UWORD	DestHeight;
	UWORD	XDestFactor;		/* scale factor numerators */
	UWORD	YDestFactor;
	ULONG	Flags;					/* see below for definitions */
	UWORD	DitherMode;			/* see below for definitions */
	APTR DstMaskPlane;
	UWORD TransparentColor;
	};

/* Flag definitions */

//#define	SCAREMB_TRANSPARENT	0				// OBSOLETE
#define	SCAREMB_USE_AVERAGE	1
#define	SCAREMB_OPAQUE			2

//#define	SCAREMF_TRANSPARENT	(1<<SCAREMB_TRANSPARENT)		// OBSOLETE
#define	SCAREMF_USE_AVERAGE	(1<<SCAREMB_USE_AVERAGE)
#define	SCAREMF_OPAQUE			(1<<SCAREMB_OPAQUE)

/* Dithering definitions */

#define DITHER_OFF			0		/* No dithering */
#define	DITHER_FLOYD		1		/* Floyd-Steinberg dithering */
#define	DITHER_BURKES		2		/* Burkes dithering */
#define DITHER_ORDERED	3		/* Ordered dithering */
#define	DITHER_RANDOM		4		/* Random dithering */

// Note: the above list might as well be extended in the future.


/* Function prototype */

BOOL ScaleRemap( struct ScaleRemapInfo * );


#endif /* MEDIALINK_SCAREM_I */
