#ifndef MEDIAPOINT_TEXTTOOLS_H
#define MEDIAPOINT_TEXTTOOLS_H

/*
Supporting definitions for the MediaPoint text functions...

$VER: mediapoint/pascal/include/texttools.h 0.009 (15/12/93)
*/


VOID TEInitInfo( TEPT );
VOID TEInitWindow( EWPT, struct RastPort * );

VOID TESetUpdateRange( EWPT, UBYTE level );

VOID TESetFont( TEPT, struct TextFont * );
VOID TESetStyle( TEPT, UWORD style, UWORD stylemask );
VOID TESetColor( TEPT, UWORD color );
VOID TESetItalic( EWPT, UBYTE slantAmount, BYTE slantValue );
VOID TESetShadow( EWPT, UBYTE type, UBYTE depth, UBYTE direction, UBYTE pen );
VOID TESetAntiAlias( EWPT, UBYTE level );
VOID TESetJustification( EWPT, UBYTE justmode );

VOID TERemoveScrap( VOID );


#ifdef TEXTEDIT_PRIVATE			// functions for TextEdit only

	VOID	TECut( TEPT );
	VOID	TECopy( TEPT );
	VOID	TEPaste( TEPT );
	VOID	TEDelete(	TEPT );

	VOID	TECreateUndo( TEPT );
	VOID	TEPerformUndo( TEPT );

	UWORD TELineHeight( TEPT, EWPT );

#endif /* TEXTEDIT_PRIVATE */

// Supporting routines for TEUpdate()

VOID DetermineBitMapSize( TEPT, UWORD *, UWORD * );
BOOL AllocTextBitMaps( struct EditWindow *, UWORD, UWORD, UWORD );
VOID FreeTextBitMaps( VOID );

#endif /* MEDIAPOINT_TEXTTOOLS_H */
