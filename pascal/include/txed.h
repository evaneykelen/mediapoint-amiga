#ifndef MEDIAPOINT_TEXTEDIT_H
#define MEDIAPOINT_TEXTEDIT_H

/*
Supporting definitions for the MediaPoint text-edit & display functions...

$VER: mediapoint/pascal/include/textedit.h 1.017 (25.FEB.94)
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

/**** defines ****/

#define RECALC_NONE	0
#define	RECALC_FULL	1
#define	RECALC_LINE	2
#define	RECALC_DONE	3
//#define RECALC_FAST	4

// defines for Update level

#define	LEVEL_RANGE		0
#define	LEVEL_WINDOW	1
#define	LEVEL_FULL		2


struct TEChar {
	struct TextFont *charFont;			// TextFont structure for char
	UBYTE underlineColor;						// color for underline for this char
	UBYTE	charStyle;								// defined in textedit.h
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


VOID TEUpdate( TEPT, BOOL redrawBgnd );
VOID TESetSelect( TEPT, UWORD selstart, UWORD selend );

VOID	TEKey( TEPT, UWORD );
VOID	TEActivate( TEPT );
VOID	TEDeactivate( TEPT );
UWORD TECaretPos( EWPT, UWORD, UWORD );
VOID	TEReportPos( TEPT, UWORD *, UWORD * );
UWORD	TEWhichLine( TEPT );

/**** register or stack parameters ****/

struct EditWindow *TEWhichBox( TEPT, UWORD );


#endif /* MEDIAPOINT_TEXTEDIT_H */
