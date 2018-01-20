	IFND	MEDIAPOINT_MACROS_I
MEDIAPOINT_MACROS_I	SET	1
**
**	$VER: mediapoint/pascal/macros.i 01.003 (3.12.93)
**
**	Contains coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

	IFND	GRAPHICS_GFX_I
	INCLUDE	"graphics/gfx.i"
	ENDC


_1stParam	equ	8		; defines for parameters on stack
_2ndParam	equ	12		; when a C function is called.
_3rdParam	equ	16
_4thParam	equ	20
_5thParam	equ	24
_6thParam	equ	28

TRUE		equ	-1
FALSE		equ	0


IsInterleaved	MACRO		; An bitmap, An scratch
	movea.l	bm_Planes+4(\1),\2
	suba.l	bm_Planes+0(\1),\2
	cmpa.w	bm_BytesPerRow(\1),\2
	ENDM
; bls TRUE bhi FALSE


	ENDC	; MEDIAPOINT_MACROS_I
