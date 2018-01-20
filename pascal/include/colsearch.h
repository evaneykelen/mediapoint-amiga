#ifndef MEDIAPOINT_COLSEARCH_H
#define MEDIAPOINT_COLSEARCH_H

/*
**	$VER: mediapoint/pascal/include/colsearch.h 00.018 (18.01.94)
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
*/


struct ColorTableHeader {
	ULONG	Size;									/* amount of entries in table */
	UWORD	BaseRED;
	UWORD	BaseGRN;
	UWORD	BaseBLU;
	UWORD	DiffsRED;							/* private use */
	UWORD	DiffsGRN;							/* private use */
	UWORD	DiffsBLU;							/* private use */
	UWORD	DiffsREDHAM;					/* private use */
	UWORD	DiffsGRNHAM;					/* private use */
	UWORD	DiffsBLUHAM;					/* private use */
	struct ColorMap *ColorMap;
	VOID	*ColorTable;					/* pointer to colortable data */
	VOID	*ColorFunc;						/* function to find pen numbers */
	UWORD	MinDiff;							/* private use */
	APTR	DiffPtr;							/* private use */
	};

extern VOID AllocateTable( VOID );
extern VOID InvalidateTable( VOID );
extern VOID ReleaseTable( VOID );

extern VOID __asm CMapToUWORDS(
				register __a0 struct ColorTableHeader *,
				register __d1 ULONG );										/* ViewModes */

#endif /* MEDIALINK_COLSEARCH_H */
