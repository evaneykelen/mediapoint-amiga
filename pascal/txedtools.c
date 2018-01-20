#define TEXTEDIT_PRIVATE

#include "nb:pre.h"
#include "pascal:include/txedtools.h"


/**** externals ****/

extern struct BitMap24 textBM;
extern struct BitMap24 maskBM;
extern struct BitMap24 xtraBM;

extern struct List *fontList;
extern struct BitMap sharedBM;

struct TEChar *scrapBuffer = NULL;
struct TEChar *undoBuffer = NULL;



/**** local support functions ****/

STATIC VOID freeUndo( VOID );
STATIC VOID freeScrap( VOID );
STATIC VOID createScrap( struct TEInfo *TEI );
STATIC VOID teSetUpdateRange( struct TEInfo *TEI, UWORD start, UWORD end );

/**** external fast machine code funcs from txed.a ****/

VOID teDelSelection( struct TEInfo *TEI );



/**** tool functions ****/

VOID TESetUpdateRange( struct EditWindow *EW, UBYTE level )
	{
	register UWORD start, end;

	switch( level )
		{
		case LEVEL_FULL:
			{
			start = EW->FirstChar;
			end = EW->TEI->textLength;
			break;
			}
		case LEVEL_WINDOW:
			{
			start = EW->FirstChar;
			end = EW->LastChar;
			break;
			}
		case LEVEL_RANGE:
		default:
			{
			start = EW->TEI->selStart;
			end = EW->TEI->selEnd;
			break;
			}
		}
	if ( start != end )
		teSetUpdateRange( EW->TEI, start, end );
	}


UWORD TELineHeight( struct TEInfo *TEI, struct EditWindow *EW )
	{
	return((UWORD)( TEI->lineHeight + EW->ySpacing + TEExtraSpace( EW ) ));
	}


VOID TEDelete( struct TEInfo *TEI )
	{
	teDelSelection( TEI );
	}


VOID TEInitInfo( struct TEInfo *TEI )
	{
	UWORD i;
	UBYTE *p;

	p = (UBYTE *) TEI;
	for( i=0; i<sizeof( struct TEInfo ); i++ )
		{
		p[i] = 0;
		}

	TEI->caretState = FALSE;
	TEI->recalLines = RECALC_FULL;	
	TEI->lastChanged = TEXTEDITSIZE;
	}


VOID TEInitWindow( struct EditWindow *EW, struct RastPort *RP )
	{
	EW->FirstChar = 0;
	EW->LastChar = 0;
	EW->justification = JUSTIFICATION_LEFT;
	EW->antiAliasLevel = ANTIALIAS_NONE;
	EW->shadowType = SHADOWTYPE_NORMAL;
	EW->shadowDirection = LIGHTSOURCE_SE;
	EW->shadowDepth = 0;
	EW->shadow_Pen = 1;
	EW->slantAmount = 2;
	EW->slantValue = 1;
	EW->xSpacing = 0;
	EW->ySpacing = 0;
	EW->underLineHeight = 1;
	EW->underLineOffset = 0;
	EW->rastPort = RP;
	}


VOID TERemoveScrap( VOID )
	{
	freeUndo();
	freeScrap();
	}


VOID TECut( struct TEInfo *TEI )
	{
	if ( TEI->selStart != TEI->selEnd )
		{
		createScrap( TEI );
		teDelSelection( TEI );
		}
	}


VOID TECopy( struct TEInfo *TEI )
	{
	if ( TEI->selStart != TEI->selEnd )
		{
		createScrap( TEI );
		}
	}


VOID TECreateUndo( struct TEInfo *TEI )
	{
	ULONG *ptr, size;
	UWORD *wptr;

	freeUndo();
	size = (TEI->textLength+1) * sizeof( struct TEChar );
	if ( ptr = (VOID *) AllocMem( size + 3*sizeof(ULONG), MEMF_ANY ) )
		{
		undoBuffer = (struct TEChar *) &ptr[3];
		ptr[0] = size;
		ptr[1] = (ULONG) TEI->textLength;
		wptr = (UWORD *) (&ptr[2]);
		wptr[0] = TEI->selStart;
		wptr[1] = TEI->selEnd;
		CopyMemQuick( TEI->text, undoBuffer, size );
		}
	}


VOID TEPaste( struct TEInfo *TEI )
	{
	ULONG size, *ptr;
	UWORD cnt;
	register ULONG *sptr, *dptr;

	if ( scrapBuffer )
		{
		teDelSelection( TEI );

		ptr = (ULONG *) scrapBuffer;
		ptr -= 1;
		size = ptr[0];
		if ( ((size/sizeof(struct TEChar))+TEI->textLength) <= (TEXTEDITSIZE-3) )
			{
			sptr = (VOID *) &( TEI->text[TEI->textLength+1] );
			dptr = (VOID *) (((UBYTE *)sptr)+size);

			for ( cnt = 0; cnt < ((TEI->textLength-TEI->selStart)+1); cnt++ )
				{
				(*(--dptr)) = (*(--sptr));
				(*(--dptr)) = (*(--sptr));
				}

			teSetUpdateRange( TEI, TEI->selStart, TEI->selStart+(size/sizeof(struct TEChar)) );

			CopyMemQuick( scrapBuffer, &(TEI->text[TEI->selStart]), size );

			TEI->selStart += size/sizeof(struct TEChar);
			TEI->selEnd = TEI->selStart;
			TEI->textLength += size/sizeof(struct TEChar);
			}
		}
	}


VOID TEPerformUndo( struct TEInfo *TEI )
	{
	ULONG *ptr, size;
	UWORD *wptr;

	if ( undoBuffer )
		{
		ptr = (ULONG *) undoBuffer;
		ptr -= 3;
		size = ptr[0];
		CopyMemQuick( undoBuffer, TEI->text, size );
		TEI->textLength = (UWORD) ptr[1];
		wptr = (UWORD *) (&ptr[2]);
		TEI->selStart = wptr[0];
		TEI->selEnd = wptr[1];
		freeUndo();
		}
	}


STATIC VOID freeUndo( VOID )
	{
	if ( undoBuffer )
		{
		ULONG *ptr = (ULONG *) undoBuffer;
		ptr -= 3;
		FreeMem( ptr, ptr[0]+3*sizeof(ULONG) );
		undoBuffer = NULL;
		}
	}



VOID TESetJustification( struct EditWindow *EW, UBYTE justification )
	{
	if ( EW )
		{
		EW->justification = justification;
		teSetUpdateRange( EW->TEI, EW->FirstChar, EW->LastChar );
		}
	}


VOID TESetItalic( struct EditWindow *EW, UBYTE slantAmt, BYTE slantVal )
	{
	if ( EW )
		{
		EW->slantAmount = (UWORD) slantAmt;
		EW->slantValue = (WORD) slantVal;
		teSetUpdateRange( EW->TEI, EW->FirstChar, EW->LastChar );
		}
	}


VOID TESetShadow( struct EditWindow *EW, UBYTE type, UBYTE depth,
	UBYTE direction, UBYTE pen )
	{
	if ( EW )
		{
		EW->shadowType = type;
		EW->shadowDepth = depth;
		EW->shadowDirection = direction;
		EW->shadow_Pen = pen;
		teSetUpdateRange( EW->TEI, EW->FirstChar, EW->LastChar );
		}
	}


VOID TESetAntiAlias( struct EditWindow *EW, UBYTE level )
	{
	if ( EW )
		{
		EW->antiAliasLevel = level;
		teSetUpdateRange( EW->TEI, EW->FirstChar, EW->LastChar );
		}
	}


VOID TESetFont( struct TEInfo *TEI, struct TextFont *Font )
	{
	UWORD i;
	TEI->newText.charFont = Font;
	for ( i = TEI->selStart; i < TEI->selEnd; i++ )
		TEI->text[i].charFont = Font;
	}


VOID TESetStyle( struct TEInfo *TEI, UBYTE style, UBYTE stylemask )
	{
	UWORD i;

	style &= stylemask;
	stylemask = ~stylemask;

	TEI->newText.charStyle &= stylemask;
	TEI->newText.charStyle |= style;

	for ( i = TEI->selStart; i < TEI->selEnd; i++ )
		{
		TEI->text[i].charStyle &= stylemask;
		TEI->text[i].charStyle |= style;
		}
	}


VOID TESetColor( struct TEInfo *TEI, UBYTE color )
	{
	UWORD i;
	TEI->newText.charColor = color;
	for ( i = TEI->selStart; i < TEI->selEnd; i++ )
		{
		TEI->text[i].charColor = color;
		TEI->text[i].underlineColor = color;
		}
	}


VOID TESetUnderlineColor( struct TEInfo *TEI, UBYTE color )
	{
	UWORD i;
	for ( i = TEI->selStart; i < TEI->selEnd; i++ )
		TEI->text[i].underlineColor = color;
	}






STATIC VOID freeScrap( VOID )
	{
	if ( scrapBuffer )
		{
		ULONG *ptr = (ULONG *) scrapBuffer;
		ptr -= 1;
		FreeMem( ptr, ptr[0]+sizeof(ULONG) );
		scrapBuffer = NULL;
		}
	}


STATIC VOID teSetUpdateRange( struct TEInfo *TEI, UWORD start, UWORD end )
	{
	if ( start < TEI->firstChanged )
		TEI->firstChanged = start;
	if ( end+1 > TEI->lastChanged )
		TEI->lastChanged = end+1;
	TEI->recalLines = RECALC_FULL;
	}


STATIC VOID createScrap( struct TEInfo *TEI )
	{
	ULONG *ptr;

	freeScrap();
	if ( ptr = (VOID *) AllocMem( ((TEI->selEnd - TEI->selStart) << 3) + sizeof(ULONG), MEMF_ANY ) )
		{
		scrapBuffer = (struct TEChar *) &(ptr[1]);
		ptr[0] = (ULONG)((TEI->selEnd-TEI->selStart)<<3);
		CopyMemQuick( &(TEI->text[TEI->selStart]), scrapBuffer, (TEI->selEnd-TEI->selStart)<<3 );
		}
	else
		scrapBuffer = NULL;
	}




/******** DetermineBitMapSize() ********/

VOID DetermineBitMapSize( struct TEInfo *teI, UWORD *width, UWORD *height )
	{
	struct FontListRecord *FLR;
	struct EditWindow *EW;
	UWORD extraSpace = 0;
	UWORD ySpace = 0;

	*width = *height = 0;

	FLR = (struct FontListRecord *) fontList->lh_Head;

	while( FLR->node.ln_Succ )
		{
		if ( FLR->fontSize > *height )
			*height = FLR->fontSize;
		FLR = (struct FontListRecord *) FLR->node.ln_Succ;
		}


	EW = (struct EditWindow *) teI->frameList.lh_Head;

	extraSpace = TEExtraSpace( EW );
	*width = EW->Width;
	ySpace = (UWORD) (( EW->ySpacing >= 0) ? EW->ySpacing : 0 );

	*width  += (16+*height);						// 2 for aa, *height for italics
	*height += (4+ySpace+extraSpace);		// 2 for antialiasing
	}
	


/******** AllocTextBitMaps() ********/

BOOL AllocTextBitMaps( struct EditWindow *ew, UWORD width, UWORD height, UWORD depth )
	{
	textBM.MagicCookie = MAGIC_COOKIE_BM24;
	maskBM.MagicCookie = MAGIC_COOKIE_BM24;
	xtraBM.MagicCookie = MAGIC_COOKIE_BM24;

	if ( AllocPlanes24( &textBM, depth, width, height, MEMF_ANY|MEMF_PUBLIC, BM24F_CHUNKY ) )
		{
		if ( AllocPlanes24( &maskBM, depth, width, height, MEMF_ANY|MEMF_PUBLIC, BM24F_CHUNKY ) )
			{
			if ( AllocPlanes24( &xtraBM, depth, width, height, MEMF_CHIP, BM24F_NONE ) )
				return( TRUE );
			}
		}
	FreeTextBitMaps();
	return( FALSE );
	}


/******** FreeTextBitMaps() ********/

VOID FreeTextBitMaps( VOID )
	{
	WaitBlit();
	FreePlanes24( &xtraBM );
	FreePlanes24( &maskBM );
	FreePlanes24( &textBM );
	}


/******** E O F ********/
