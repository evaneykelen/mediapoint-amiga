	IFND	MEDIAPOINT_TTXT_I
MEDIAPOINT_TTXT_I	SET	1
**
**	$VER: mediapoint/pascal/include/ttxt.i 00.011 (09.03.93)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

	IFND	EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC


NO_TEXTEDIT_INFO	SET	1


  STRUCTURE TDInfo,0
	APTR	tdi_chipMem		; pointer to alloc'ed gfxmem
	ULONG	tdi_chipSize		; size of alloc'ed gfxmem
	APTR	tdi_dstBitMap		; render to this bitmap
	UWORD	tdi_dstClear		; above bitmap is zero'd (T/F)
	UWORD	tdi_renderLine		; render line # (0-xx)
	UWORD	tdi_dstXPos		; ew->X or 0 if bitmap is the window
	UWORD	tdi_dstYPos		; (ew->Y or 0) + ew->TopMargin + lineheights
	WORD	tdi_resultHeight	; set by function itself

	WORD	tdi_resultWidth		; internal use
	UWORD	tdi_lineHeight		; internal use
	UWORD	tdi_baseLine		; internal use

	APTR	tdi_textData		; points to struct TEChar
	APTR	tdi_editWindow		; ptr to editwindow
	APTR	tdi_maskBitMap	; ptr to mask-bitmap
	ULONG	tdi_viewModes
	APTR	tdi_colorMap

	APTR	tdi_SysBase
	APTR	tdi_GfxBase

	LABEL	tdi_SIZE



	ENDC	; MEDIAPOINT_TTXT_Im
