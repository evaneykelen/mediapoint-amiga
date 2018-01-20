#ifndef MEDIAPOINT_TXEDTOOLS_H
#define MEDIAPOINT_TXEDTOOLS_H

/*
Supporting definitions for the MediaPoint text functions...

$VER: mediapoint/pascal/include/txedtools.h 0.013 (23.FEB.94)
*/


VOID TEInitInfo( TEPT );
VOID TEInitWindow( EWPT, struct RastPort * );

VOID TESetUpdateRange( EWPT, UBYTE level );

VOID TESetFont( TEPT, struct TextFont * );
VOID TESetStyle( TEPT, UBYTE style, UBYTE stylemask );
VOID TESetColor( TEPT, UBYTE color );
VOID TESetUnderlineColor( TEPT, UBYTE color );
VOID TESetItalic( EWPT, UBYTE slantAmount, BYTE slantValue );
VOID TESetShadow( EWPT, UBYTE type, UBYTE depth, UBYTE direction, UBYTE pen );
VOID TESetAntiAlias( EWPT, UBYTE level );
VOID TESetJustification( EWPT, UBYTE justmode );

VOID TERemoveScrap( VOID );

VOID TECut( TEPT );
VOID TECopy( TEPT );
VOID TEPaste( TEPT );
VOID TEDelete(	TEPT );

VOID TECreateUndo( TEPT );
VOID TEPerformUndo( TEPT );

UWORD TELineHeight( TEPT, EWPT );

// Supporting routines for TEUpdate()

VOID DetermineBitMapSize( TEPT, UWORD *, UWORD * );
BOOL AllocTextBitMaps( struct EditWindow *, UWORD, UWORD, UWORD );
VOID FreeTextBitMaps( VOID );

#endif /* MEDIAPOINT_TXEDTOOLS_H */
