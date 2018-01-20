#ifndef MEDIAPOINT_TEXTEDIT_H
#define MEDIAPOINT_TEXTEDIT_H

/*
Supporting definitions for the MediaPoint text-edit & display functions...

$VER: mediapoint_textedit.h 1.008 (29/07/93)
*/


#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#ifndef GRAPHICS_RASTPORT_H
#include <graphics/rastport.h>
#endif

#ifndef GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif


#define TEXTEDITSIZE 2048


// defines for TextEdit returncode (almost obsolete)

#define	TE_UNKNOWN_ACTION				0
#define	TE_UNKNOWN_MENU_ITEM		1
#define	TE_CLOSED_WINDOW				2
#define	TE_NOT_ENOUGH_CHIPMEM		3
#define	TE_MENU_ACTION_HANDLED	4		// not returned, used internally


// defines for Update level

#define	LEVEL_RANGE		0
#define	LEVEL_WINDOW	1
#define	LEVEL_FULL		2


struct TEChar {
	struct TextFont *charFont;			// TextFont structure for char
	UWORD	charStyle;								// defined in textedit.h
	UBYTE	charColor;								// pen color number
	UBYTE	charCode;									// ASCII code for character
	};

struct TEInfo {
	struct List frameList;					// linked list of textframes
	struct TEChar newText;					// fresh text has this style
	UWORD	lineHeight;								// used internally
	UWORD	baseLine;									// used internally
	UWORD	selStart, selEnd;					// used internally
	UWORD	textLength;								// amount of characters in text
	struct TEChar *text;						// pointer to begin of text
	UWORD	lineStarts[TEXTEDITSIZE];	// used internally
	UBYTE	caretState;								// used internally
	UBYTE	recalLines;								// used internally
	UWORD	firstChanged;							// used internally
	UWORD	lastChanged;							// used internally
	UBYTE	fastUpdate;								// used internally
	UBYTE	oldUpdateMode;						// used internally
	};


// typedefs for better (shorter) layout

typedef struct EditWindow * EWPT;
typedef struct TEChar * CHPT;
typedef struct TEInfo * TEPT;


VOID TEInitInfo( TEPT );
VOID TEInitWindow( EWPT, struct RastPort * );

VOID TESetSelect( TEPT, UWORD selstart, UWORD selend );

VOID TESetFont( TEPT, struct TextFont * );
VOID TESetStyle( TEPT, UWORD style, UWORD stylemask );
VOID TESetColor( TEPT, UWORD color );

VOID TESetItalic( EWPT, UBYTE slantAmount, BYTE slantValue );
VOID TESetShadow( EWPT, UBYTE type, UBYTE depth, UBYTE direction, UBYTE pen );
VOID TESetAntiAlias( EWPT, UBYTE level );
VOID TESetJustification( EWPT, UBYTE justmode );

VOID TEUpdate( TEPT, struct RastPort *, struct BitMap *, struct BitMap *,
	struct RastPort *, BOOL redrawBgnd );

VOID TESetUpdateRange( EWPT, UWORD level );

VOID TERemoveScrap( VOID );


#ifdef TEXTEDIT_PRIVATE			// functions for TextEdit only

	VOID	TEKey( TEPT, UWORD );

	VOID	TECut( TEPT );
	VOID	TECopy( TEPT );
	VOID	TEPaste( TEPT );
	VOID	TEDelete(	TEPT );

	VOID	TECreateUndo( TEPT );
	VOID	TEPerformUndo( TEPT );

	VOID	TEActivate( TEPT );
	VOID	TEDeactivate( TEPT );

	UWORD TECaretPos( EWPT, UWORD, UWORD );
	VOID	TEReportPos( TEPT, UWORD *, UWORD * );
	UWORD	TEWhichLine( TEPT );
	UWORD TELineHeight( TEPT, EWPT );
	struct EditWindow *TEWhichBox( TEPT, UWORD );

#endif /* TEXTEDIT_PRIVATE */


// Supporting routines for TEUpdate()

VOID DetermineBitMapSize( TEPT, UWORD *, UWORD * );
BOOL AllocTextBitMap( struct EditWindow *, UWORD, UWORD, UWORD );
VOID FreeTextBitMap( UWORD, UWORD );

#endif /* MEDIAPOINT_TEXTEDIT_H */
