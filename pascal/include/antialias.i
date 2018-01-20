	IFND	MEDIAPOINT_ANTIALIAS_I
MEDIAPOINT_ANTIALIAS_I	SET	1
**
**	$VER: mediapoint/pascal/include/antialias.i 01.007 (02.07.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

ANTIALIAS_NONE	EQU	0
ANTIALIAS_LOW	EQU	1
ANTIALIAS_MED	EQU	2	; might have no other effect than low
ANTIALIAS_HIGH	EQU	3	; might have no other effect than low

  STRUCTURE AntiAliasInfo,0
	APTR	aa_ColorConversion
	APTR	aa_SrcBitMap
	APTR	aa_MskBitMap
	UWORD	aa_Width
	UWORD	aa_Height
	UBYTE	aa_Level
	UBYTE	aa_Pad0
	ULONG	aa_ViewModes
	ULONG	aa_PaletteSize
	LABEL	aa_SIZEOF

	ENDC	; MEDIAPOINT_ANTIALIAS_I
