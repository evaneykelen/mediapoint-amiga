#ifndef MEDIAPOINT_MISCTOOLS_H
#define MEDIAPOINT_MISCTOOLS_H

/*
**	$VER: mediapoint/pascal/misctools.h 00.013 (7.12.93)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
*/


/**** for determination if bitmap = bitmap24 ****/

#define MAGIC_COOKIE_BM24 0x6D416743


/**** 24 bits BitMap ****/

struct BitMap24 {
	UWORD	Modulo;					// BytesPerRow renamed for 'new' approach, e.g. interleaved bm.
	UWORD	Rows;
	UBYTE	Flags;
	UBYTE	Depth;
	UWORD	pad;
	PLANEPTR Planes[24];
	ULONG	rasSize;				// for free'ing memory
	UWORD	PixelWidth;			// actual width in pixels, e.g. for writing iff bitmap
	ULONG ViewModes;
	ULONG	PaletteSize;		// amount of ULONG's below that make the palette, not CMAPSIZE!
	ULONG *ColorTable;		// bits: 31:24 clear! 23:16 = RED, 15:8 = GRN, 7:0 = BLU
	UWORD	Flags24;
	ULONG	MagicCookie;		// initialize to MAGIC_COOKIE_BM24 when using embedded BitMap24.
	};

// Note: BitMap is interleaved when Planes[1]-Planes[0] == Modulo. In that case, the
// amount of bytes that make up one row of a plane is thus Modulo divided by Depth.
// ** this is the case in the graphics.library bitmap's too **


/**** definitions ****/

#define	BM24A_HEIGHT			0		/* GetBitMapAttr24 request types */
#define	BM24A_WIDTH				1
#define	BM24A_DEPTH				2
#define	BM24A_FLAGS				3
#define BM24A_TYPE				4

#define	BM24F_NONE				0
#define	BM24F_CLEAR				1
#define	BM24F_INTERLEAVED	2		/* Planes are stored interleaved (only for normal bitmaps) */
#define	BM24F_CHUNKY			4		/* Data is stored as chunky UBYTES */
#define	BM24F_FASTMEM			8		/* BitMap data MIGHT be in Fast Memory, not blitable */
#define	BM24F_TRUE_RGB		16	/* BitMap is 24 planes or 3 planes chunky RGB

/**** returned by GetBitMapAttr( TYPE ) ****/

#define	BM24T_NORMAL		0			/* 1 to 8 planes, 2^Depth colors */
#define	BM24T_EHB				1			/* 6 planes, 32 colors, 32 half-bright colors */
#define	BM24T_HAM6			2			/* 6 planes, 4096 colors Hold and Modify */
#define	BM24T_HAM8			3			/* 8 planes, ~256000 colors Hold and Modify */
#define	BM24T_24BIT			4			/* 24 planes, TRUE COLOR 16.7 million colors. No CMAP */
#define	BM24T_RGB				5			/* 3 planes, CHUNKY RGB TRUE COLOR. No CMAP */

/* Note: The upper 4 bitmap-types may contain just 1 plane. Data is then stored as one
   plane CHUNKY data. This can be determined using GBMA24( BM24A_FLAGS ). Note that
   bm24->Depth still reflects the amount of data needed for the bitmap. */



/***** Function ProtoTypes *****/

extern VOID __asm SetRGBCM(	register __a0 struct ColorMap *,
				register __d0 WORD,			// color number
				register __d1 ULONG,		// 32 bit RED
				register __d2 ULONG,		// 32 bit GRN
				register __d3 ULONG );	// 32 bit BLU

extern VOID __asm GetRGB(	register __a0 struct ColorMap *,
				register __d0 WORD,				// color number
				register __a1 ULONG * );	// ptr to 3 longwords to store in

extern struct BitMap24 *AllocBitMap24( UBYTE depth, UWORD width, UWORD height, ULONG memtype,
	ULONG flags );

extern BOOL AllocPlanes24( struct BitMap24 *bitmap, UBYTE depth, UWORD width, UWORD height,
	ULONG memtype, ULONG flags );

extern VOID FreeBitMap24( struct BitMap24 *bitmap );

extern VOID FreePlanes24( struct BitMap24 *bitmap );

extern ULONG __asm GetBitMapAttr24(
				register __a0 struct BitMap24 *,	// BitMap
				register __d1 ULONG );						// BM24 Attribute

extern VOID * __asm AllocRememberMP(
				register __d0 ULONG,							// byteSize
				register __d1 ULONG );						// attributes

extern VOID __asm FreeRememberMP(
				register __a1 );									// memoryBlock

extern VOID __asm PlanarToChunky(
				register __a0 struct BitMap24 *,	// BitMap
				register __a1 UBYTE *,						// chunkyData
				register __d1 UWORD,							// srcY
				register __d6 UWORD,							// width ( should be bytes * 8 )
				register __d7 UWORD );						// height

extern VOID __asm PlanarToChunky24(
				register __a0 struct BitMap24 *,	// BitMap
				register __a1 UBYTE *,						// RGBData
				register __d1 UWORD,							// srcY
				register __d6 UWORD,							// width ( should be bytes * 8 )
				register __d7 UWORD );						// height

extern VOID __asm ChunkyToPlanar(
				register __a0 UBYTE *,						// chunkyBytes
				register __a1 struct BitMap24 *,	// dstBitMap
				register __d1 UWORD,							// dstY
				register __d6 UWORD,							// width ( should be bytes * 8 )
				register __d7 UWORD );						// height

extern VOID __asm MakeColorTable(
				register __a0 struct ColorMap *,
				register __a1 ULONG *,						// table to store in
				register __d0 ULONG,							// PaletteSize
				register __d1 ULONG );						// ViewModes

extern VOID __asm GetPaletteSize(
				register __a0 struct BitMap24 *,
				register __d1 ULONG );						// ViewModes


#endif /* MEDIALINK_MISCTOOLS_H */
