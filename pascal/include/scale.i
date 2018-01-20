	IFND	MEDIAPOINT_SCALE_I
MEDIAPOINT_SCALE_I	SET	1
**
**	$VER: mediapoint/pascal/scale.i 01.003 (20.08.93)
**	Includes Release 39.108
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**


 STRUCTURE FastBitScaleArgs,0
	UWORD	fbsa_SrcX		; source origin
	UWORD	fbsa_SrcY
	UWORD	fbsa_SrcWidth		; source size
	UWORD	fbsa_SrcHeight
	UWORD	fbsa_XSrcFactor		; scale factor denominators
	UWORD	fbsa_YSrcFactor
	UWORD	fbsa_DestX		; destination origin
	UWORD	fbsa_DestY
	UWORD	fbsa_DestWidth		; destination size result
	UWORD	fbsa_DestHeight
	UWORD	fbsa_XDestFactor	; scale factor numerators
	UWORD	fbsa_YDestFactor
	APTR	fbsa_SrcBitMap		; source BitMap
	APTR	fbsa_DestBitMap		; destination BitMap
	ULONG	fbsa_Flags
	APTR	fbsa_SrcColorMap
	ULONG	fbsa_SrcViewModes
	LABEL	fbsa_SIZEOF


	BITDEF	SCALE,CONVERT_TO_GRAY,0
	BITDEF	SCALE,CONVERT_HAM,1
	BITDEF	SCALE,TRANSPARENT,2
	BITDEF	SCALE,USE_AVERAGE,3


	ENDC	; MEDIALINK_SCALE_I
