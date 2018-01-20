	IFND	MEDIAPOINT_DITHER_I
MEDIAPOINT_DITHER_I	SET	1
**
**	$VER: mediapoint/pascal/dither.i 00.007 (20.01.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

	IFND	MEDIAPOINT_COLSEARCH_I
	INCLUDE	"colsearch.i"
	ENDC

  STRUCTURE DitherInfo,0
	APTR	di_UpperErrors
	APTR	di_LowerErrors
	APTR	di_RGBupper
	APTR	di_PenDest
	APTR	di_DitherFuncEven
	APTR	di_DitherFuncOdd
	UWORD	di_ErrorModulo
	UWORD	di_LineNumber
	UWORD	di_LineWidth
	UWORD	di_DitherMode		; see below for definitions
	STRUCT	di_ColorHdr,cth_SIZEOF
	LABEL	di_SIZEOF

DITHER_OFF	EQU	0		; no dithering
DITHER_FLOYD	EQU	1		; Floyd-Steinberg dithering
DITHER_BURKES	EQU	2		; Burkes dithering
DITHER_ORDERED	EQU	3		; Ordered dithering
DITHER_RANDOM	EQU	4		; Random dithering
					; Note that the list above may be extended later.

	ENDC	; MEDIALINK_DITHER_I
