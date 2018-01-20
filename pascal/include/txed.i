	IFND	PASCAL_INCLUDE_TXED_I
PASCAL_INCLUDE_TXED_I	SET	1
**
**	$VER: medialink/pascal/include/txed.i 01.022 (23.02.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**


TEXTEDITSIZE	equ	2048

LEVEL_RANGE	equ	0
LEVEL_WINDOW	equ	1
LEVEL_FULL	equ	2

RECALC_NONE	equ	0
RECALC_FULL	equ	1
RECALC_LINE	equ	2
RECALC_DONE	equ	3
RECALC_FAST	equ	4

tec_SHIFTSIZE	equ	3	; shift << 3 to get tec_SIZE


  STRUCTURE TEChar,0
	APTR	tec_charFont
	UBYTE	tec_underlineColor
	UBYTE	tec_charStyle
	UBYTE	tec_charColor
	UBYTE	tec_charCode
	LABEL	tec_SIZE


	IFND	NO_TEXTEDIT_INFO

  STRUCTURE TEInfo,0
	STRUCT	tei_frameList,LH_SIZE
	STRUCT	tei_newText,tec_SIZE
	UWORD	tei_lineHeight
	UWORD	tei_baseLine
	UWORD	tei_selStart
	UWORD	tei_selEnd
	UWORD	tei_textLength		; max length is TEXTEDITSIZE
	APTR	tei_text		; pointer to actual text TEChar's
	STRUCT	tei_lineStarts,2*TEXTEDITSIZE
	UBYTE	tei_caretState		; used internally
	UBYTE	tei_recalLines		; used internally
	UWORD	tei_firstChanged	; used internally
	UWORD	tei_lastChanged		; used internally
	UBYTE	tei_fastUpdate		; used internally
	UBYTE	tei_oldUpdateMode	; used internally
	LABEL	tei_SIZE

	ENDC



	IFNE	tec_SIZE-8	; don't change unless code is changed too
	FAIL	"*** Code ASSUMES TEChar TWO LONGS in size and LONG aligned ***"
	ENDC

	IFNE	(1<<tec_SHIFTSIZE)-tec_SIZE
	FAIL	"*** Code ASSUMES TEChar TWO LONGS in size and LONG aligned ***"
	ENDC


	ENDC	; PASCAL_INCLUDE_TXED_I
