	IFND	MEDIAPOINT_SCAREM_I
MEDIAPOINT_SCAREM_I	SET	1
**
**	$VER: mediapoint/pascal/scarem.i 00.038 (14.01.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

  STRUCTURE ScaleRemapInfo,0
	APTR	sri_SrcBitMap
	APTR	sri_DstBitMap
	ULONG	sri_SrcViewModes
	ULONG	sri_DstViewModes
	APTR	sri_SrcColorMap
	APTR	sri_DstColorMap
	UWORD	sri_SrcX		; source origin
	UWORD	sri_SrcY
	UWORD	sri_SrcWidth		; source size
	UWORD	sri_SrcHeight
	UWORD	sri_XSrcFactor		; scale factor denominators
	UWORD	sri_YSrcFactor
	UWORD	sri_DestX		; destination origin
	UWORD	sri_DestY
	UWORD	sri_DestWidth		; destination size result
	UWORD	sri_DestHeight
	UWORD	sri_XDestFactor		; scale factor numerators
	UWORD	sri_YDestFactor
	ULONG	sri_Flags		; see below for definitions
	UWORD	sri_DitherMode
	APTR	sri_DstMaskPlane
	UWORD	sri_TransparentColor
	LABEL	sri_SIZEOF


**	BITDEF	SCAREM,TRANSPARENT,0		**** OBSOLETE ****
	BITDEF	SCAREM,USE_AVERAGE,1
	BITDEF	SCAREM,OPAQUE,2

	ENDC	; MEDIALINK_SCAREM_I
