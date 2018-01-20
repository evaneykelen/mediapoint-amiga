	IFND	MEDIAPOINT_COLSEARCH_I
MEDIAPOINT_COLSEARCH_I	SET	1
**
**	$VER: mediapoint/pascal/colsearch.i 00.005 (18.01.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

  STRUCTURE ColorTableHeader,0
	ULONG	cth_Size		; amount of entries in table
	UWORD	cth_BaseRED
	UWORD	cth_BaseGRN
	UWORD	cth_BaseBLU
	UWORD	cth_DiffsRED		; private use
	UWORD	cth_DiffsGRN		; private use
	UWORD	cth_DiffsBLU		; private use
	UWORD	cth_DiffsREDHAM		; private use
	UWORD	cth_DiffsGRNHAM		; private use
	UWORD	cth_DiffsBLUHAM		; private use
	APTR	cth_ColorMap		; pointer to original colormap
	APTR	cth_ColorTable		; pointer to colortable data
	APTR	cth_ColorFunc		; function to find pen numbers
	UWORD	cth_MinDiff		; private use
	APTR	cth_DiffPtr		; private use
	
	LABEL	cth_SIZEOF

	ENDC	; MEDIALINK_COLSEARCH_I
