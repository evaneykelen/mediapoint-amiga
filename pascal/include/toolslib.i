	IFND	MEDIAPOINT_TOOLSLIB_I
MEDIAPOINT_TOOLSLIB_I	SET	1
**
**	$VER: mediapoint/pascal/include/toolslib.i 01.001 (24.FEB.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

MAGIC_COOKIE_BM24	EQU	$6D416743	;'mAgC'

  STRUCTURE BitMap24,0
	UWORD	bm24_Modulo
	UWORD	bm24_Rows
	UBYTE	bm24_Flags
	UBYTE	bm24_Depth
	UWORD	bm24_pad
	STRUCT	bm24_Planes,24*4
	ULONG	bm24_rasSize		; for free'ing memory
	UWORD	bm24_PixelWidth		; actual width in pixels, e.g. for writing iff bitmap
	ULONG	bm24_ViewModes
	ULONG	bm24_PaletteSize	; amount of ULONG's that make palette, not CMAPSIZE!
	APTR	bm24_ColorTable		; bits: 31:24 clear! 23:16 = RED, 15:8 = GRN, 7:0 = BLU
	UWORD	bm24_Flags24
	ULONG	bm24_MagicCookie
	LABEL	bm24_SIZEOF


BM24A_HEIGHT	EQU	0		; GetBitMapAttr24 request types
BM24A_WIDTH	EQU	1
BM24A_DEPTH	EQU	2
BM24A_FLAGS	EQU	3
BM24A_TYPE	EQU	4


BM24F_NONE	EQU	0
	BITDEF	BM24,CLEAR,0
	BITDEF	BM24,INTERLEAVED,1
	BITDEF	BM24,CHUNKY,2
	BITDEF	BM24,FASTMEM,3
	BITDEF	BM24,TRUE_RGB,4

	BITDEF	BM24,SET_RASSIZE,15	; used internally

BM24T_NORMAL	EQU	0	; 1 to 8 planes, 2^Depth colors
BM24T_EHB	EQU	1	; 6 planes, 32 colors, 32 half-bright colors
BM24T_HAM6	EQU	2	; 6 planes, 4096 colors Hold and Modify
BM24T_HAM8	EQU	3	; 8 planes, 256000 colors Hold and Modify
BM24T_24BIT	EQU	4	; 24 planes, TRUE COLOR 16.7 million colors. No CMAP
BM24T_RGB	EQU	5	; 3 planes, CHUNKY RGB TRUE COLOR. No CMAP

* Note: The upper 4 bitmap-types may contain just 1 plane (depth = 1). Data is then
* stored as one plane CHUNKY data. This can be determined using GBMA24( BM24A_TYPE )


	ENDC	; MEDIALINK_TOOLSLIB_I
