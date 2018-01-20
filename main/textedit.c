#define TEXTEDIT_PRIVATE

#include "pre.h"
#include "ctype.h"
#include "pascal:include/txedtools.h"

/**** defines ****/

#define KEYBUFSIZE 80
#define TPLCLICKED (IDCMP_GADGETHELP)	/* V39 IDCMP GadgetHelp Class Misused */

/**** function declarations ****/

STATIC VOID PerformRawKeyActions( struct TEInfo *, struct IntuiMessage * );
STATIC UWORD PerformMenuActions(struct TEInfo *teI, struct EditWindow *ew);
STATIC VOID ConvertEvent( struct IntuiMessage *localMsg );
STATIC void SetSelectedStyle(	struct EditWindow *, struct EditWindow *,
															BOOL *touchedList);

/**** externals ****/

extern struct EventData CED;
extern struct MsgPort *capsPort;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct MenuRecord **page_MR;
extern struct Window *pageWindow;
extern int selectedWell;
extern int HANDLESNIF;
extern struct TextFont *textFont;
extern struct Library *medialinkLibBase;
extern struct List *fontList;
extern UBYTE **msgs;
extern struct TEChar *scrapBuffer;
extern struct TEChar *undoBuffer;
extern struct RendezVousRecord rvrec;	// ADDED ERIK
extern BOOL fromTextEdit;

/**** static globals ****/

static int menu, item;
static ULONG prev_Seconds = 0L;
static ULONG prev_Micros  = 0L;
static UWORD oldx;
static BOOL posChanged, moved, reSelect;

struct BitMap24 textBM;
struct BitMap24 maskBM;
struct BitMap24 xtraBM;

static UWORD oldwidth, oldheight, width, height, depth;


/******** TextEdit() ********/

UWORD TextEdit(	struct EditWindow *ew )
	{
	ULONG signals;
	struct IntuiMessage *message;
	UWORD selectionStart, oldStart, lastX, lastY, firstX, firstY;
	struct TEInfo *teI;
	struct EditWindow *cew;
	UWORD oldpos = 65535;

	UWORD returncode = TE_UNKNOWN_ACTION;
	BOOL	loop = TRUE;
	BOOL	keyPressed = FALSE;

	reSelect = moved = FALSE;
	posChanged = TRUE;

	teI = ew->TEI;

	PaletteToBack();

	depth = CPrefs.PageScreenDepth;	//window->RPort->BitMap->Depth;
	DetermineBitMapSize( teI, &width, &height );

	if ( !AllocTextBitMaps( ew, width, height, depth ) )
		{
		return( TE_NOT_ENOUGH_CHIPMEM );
		}

	oldwidth = width;
	oldheight = height;

	DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
	DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
	if ( scrapBuffer )
		EnableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
	else
		DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
	DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
	DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);

 	cew = (struct EditWindow *) teI->frameList.lh_Head;

	if ( (cew != NULL) && (cew->undoBM == NULL) )
		{
		TESetSelect( teI, cew->FirstChar, cew->LastChar );
		TESetUpdateRange( cew, LEVEL_RANGE );
		}

	teI->fastUpdate = FALSE;

	teI->newText.charFont				= ew->charFont;
	teI->newText.charStyle			= ew->charStyle;
	teI->newText.charColor			= ew->charColor;
	// START - ERIK Monday 21-Feb-94 22:31:14
	teI->newText.underlineColor = ew->charColor;	//ew->underlineColor;
	// END - ERIK Monday 21-Feb-94 22:31:14

	lastX = firstX = CED.MouseX;
	lastY = firstY = CED.MouseY;
	selectionStart = TECaretPos( ew, lastX, lastY );
	TESetSelect( teI, selectionStart, selectionStart );
	UA_SwitchMouseMoveOn( pageWindow );

	TESetSelect( teI, selectionStart, selectionStart );
	TEActivate( teI );

	while( loop )
		{
		signals = Wait( SIGNALMASK );
		if ( signals & SIGNALMASK )
			{
			while ( message = (struct IntuiMessage *) GetMsg( capsPort ) )
				{
				ConvertEvent( message );
				keyPressed = FALSE;

				switch( CED.Class )
					{
					case IDCMP_MOUSEMOVE:
						if ( firstX != CED.MouseX || firstY != CED.MouseY )
							{
							moved = posChanged = TRUE;
							lastX = (CED.MouseX > ew->X) ? CED.MouseX : ew->X;
							lastY = (CED.MouseY > ew->Y) ? CED.MouseY : ew->Y;
							}
						break;

					case IDCMP_MOUSEBUTTONS:
						{
						if ( !(CED.Qualifier & (AMIGARIGHT|IEQUALIFIER_CONTROL)))
							{
							if ( CED.Code == SELECTDOWN )
								{
								lastX = firstX = (CED.MouseX > ew->X) ? CED.MouseX : ew->X;
								lastY = firstY = (CED.MouseY > ew->Y) ? CED.MouseY : ew->Y;
								oldStart = selectionStart;
								selectionStart = TECaretPos( ew, firstX, firstY );
								posChanged = TRUE;
								moved = FALSE;
								DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
								DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
								DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);

								if ( CED.extraClass == DBLCLICKED )
									{
									if ( ! isspace( teI->text[selectionStart].charCode ) )
										{
										UWORD pos = selectionStart;
											while ( selectionStart>0 && !isspace(
											teI->text[selectionStart-1].charCode) )
											--selectionStart;
										while ( pos<teI->textLength && !isspace(
											teI->text[pos+1].charCode) )
											++pos;
										TESetSelect( teI, selectionStart, pos+1 );
										}
									else
										TESetSelect( teI, selectionStart, selectionStart );
									}
								else if ( CED.extraClass == TPLCLICKED )
									{
									UWORD line = TEWhichLine( teI );
									oldStart = selectionStart = teI->lineStarts[line];
									TESetSelect( teI, selectionStart, teI->lineStarts[line+1]);
									}
								else
									{
									if ( (CED.Qualifier & IEQUALIFIER_LSHIFT) ||
										(CED.Qualifier & IEQUALIFIER_RSHIFT) )
										TESetSelect( teI, selectionStart, oldStart );
									else
										TESetSelect( teI, selectionStart, selectionStart );
									}
								UA_SwitchMouseMoveOn( pageWindow );
								}
							else if ( CED.Code == SELECTUP )
								{
								UA_SwitchMouseMoveOff( pageWindow );

								if ( teI->selStart != teI->selEnd )
									{
									EnableMenu(page_MR[EDIT_MENU], EDIT_CUT);
									EnableMenu(page_MR[EDIT_MENU], EDIT_COPY);
									EnableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
									}
								}
							}
						else	// rcommand and/or ctrl were pressed too
							{
							returncode = TE_UNKNOWN_ACTION;
							loop = FALSE;
							}
						}
						break;

					case IDCMP_RAWKEY:
						if (CED.Code==RAW_ESCAPE)
							loop=FALSE;
						else if (CED.Code==RAW_F7 )
							{
							if ( ew->antiAliasLevel!=0 )
								ew->antiAliasLevel = 0;
							else
								ew->antiAliasLevel = 3;
							TEDeactivate(teI);
							TESetUpdateRange( ew, LEVEL_WINDOW );
							TESetAntiAlias( ew, ew->antiAliasLevel );
							TEUpdate( teI, TRUE );
							}
						else
							{
							PerformRawKeyActions( teI, message );
							keyPressed = TRUE;
							if ( teI->selStart == teI->selEnd )
								{
								DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
								DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
								DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
								}
							}
						break;

					case IDCMP_MENUPICK:
						if (CED.menuNum == -1 && CED.itemNum == -1)
								loop = FALSE;
						else
							{
							returncode = PerformMenuActions( teI, ew );
							if ( returncode == TE_UNKNOWN_MENU_ITEM )
								loop = FALSE;
							}
						break;
					}
					ReplyMsg( (struct Message *) message );
				}

			if ( moved )
				{
				UWORD pos = TECaretPos( ew, lastX, lastY );
				if ( pos != oldpos )
					{
					if ( pos < selectionStart )
						TESetSelect( teI, selectionStart, pos );
					else
						TESetSelect( teI, selectionStart, pos+1 );
					if ( teI->selStart != teI->selEnd )
						{
						EnableMenu(page_MR[EDIT_MENU], EDIT_CUT);
						EnableMenu(page_MR[EDIT_MENU], EDIT_COPY);
						EnableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
						}
					}
				else
					reSelect = FALSE;
				moved = FALSE;
				oldpos = pos;
				}

			if ( teI->recalLines != RECALC_NONE )	//&& ticks >= 2 )
				{
				TEDeactivate( teI );
				DetermineBitMapSize( teI, &width, &height );
				if ( (width > oldwidth) || (height > oldheight) )
					{
					FreeTextBitMaps();
					if ( !AllocTextBitMaps( ew, width, height, depth ) )
						{
						return( TE_NOT_ENOUGH_CHIPMEM );
						}
					oldwidth = width;
					oldheight = height;
					}
				TEUpdate( teI, TRUE );
				if ( !keyPressed )
					TESetSelect( teI, teI->selStart, teI->selEnd );
				}
			if ( reSelect )
				{
				TESetSelect( teI, teI->selStart, teI->selEnd );
				reSelect = FALSE;
				}
			TEActivate( teI );
			}
		}

	UA_SwitchMouseMoveOff( pageWindow );

	SetPageEditMenuItems();

	ew->charFont				= teI->newText.charFont;
	ew->charStyle				= teI->newText.charStyle;
	ew->charColor				= teI->newText.charColor;
	ew->underlineColor	= teI->newText.underlineColor;

	TEDeactivate( teI );
	FreeTextBitMaps();

	return( returncode );
	}






/******** PerformRawKeyActions() ********/

STATIC VOID PerformRawKeyActions( struct TEInfo *teI, struct IntuiMessage *message )
	{
	UWORD pos, i, newx, newy, line;
	UBYTE code;
	LONG amount;
	struct EditWindow *cew;
  UWORD literal, ct;
	TEXT datecmd[20];
	TEXT fileName[SIZE_FILENAME], fullPath[SIZE_FULLPATH];

	STATIC struct InputEvent ievent = {NULL,IECLASS_RAWKEY,0,0};
	STATIC UBYTE keybuf[KEYBUFSIZE];

	ievent.ie_Code = message->Code;
	ievent.ie_Qualifier = message->Qualifier;
	ievent.ie_position.ie_addr = *((APTR*) message->IAddress);

	amount = RawKeyConvert( &ievent, keybuf, KEYBUFSIZE, NULL );
	if ( amount == -1 ) amount = KEYBUFSIZE;

	for ( i = 0; i < amount; i++ )
		{
		if ( keybuf[i] == 0x9B )
			{
			i++;
			if ( keybuf[i] >= '0' && keybuf[i] <= '9' )
				{
				while ( keybuf[i++] != '~' )
					;
				}
			else switch( keybuf[i++] )
				{
				case 'A':		// cursor up
					line = TEWhichLine( teI );
					if ( line > 0 )
						{
						TEReportPos( teI, &newx, &newy );
						if ( newx != 0xFFFF )
							{
							if ( newy != 0 )
								{
								cew = TEWhichBox( teI, teI->selStart );
								newx += cew->X;
								newy += cew->Y;
								oldx = newx = posChanged ? newx : oldx;
								pos = TECaretPos( cew, newx, newy - 1 );
								TESetSelect( teI, pos, pos );
								posChanged = FALSE;
								}
							else
								{
								cew = TEWhichBox( teI, teI->selStart );

								while ( cew )
									{
									cew = (struct EditWindow *) cew->frameNode.mln_Pred;
									if ( cew && cew->FirstChar < cew->LastChar)
										break;
									}

								if ( cew && cew->FirstChar < cew->LastChar )
									{
									oldx = newx = posChanged ? newx : oldx;
									cew = (struct EditWindow *) cew->frameNode.mln_Pred;
									TEDeactivate( teI );
									teI->selStart = teI->lineStarts[line-1];
									TEReportPos( teI, &newx, &newy );
									newx += cew->X;
									newy += cew->Y;
									pos = TECaretPos( cew, oldx, newy );
									TESetSelect( teI, pos, pos );
									posChanged = FALSE;
									}
								}
							}
						}
					break;

				case 'B':		// cursor down
					line = TEWhichLine( teI );
					if ( teI->lineStarts[line] != teI->lineStarts[line+1] )
						{
						TEReportPos( teI, &newx, &newy );
						if ( newx != 0xFFFF )
							{
							cew = TEWhichBox( teI, teI->selStart );
							newx += cew->X;
							newy += cew->Y;
							oldx = newx = posChanged ? newx : oldx;
							pos = TECaretPos( cew, newx, newy + TELineHeight( teI, cew ) );
							TESetSelect( teI, pos, pos );
							posChanged = FALSE;
							}
						}
					break;

				case 'D':		// cursor left
					TESetSelect( teI, teI->selStart-1, teI->selStart-1 );
					posChanged = TRUE;
					break;
				case 'C':		// cursor right
					TESetSelect( teI, teI->selStart+1, teI->selStart+1 );
					posChanged = TRUE;
					break;
				case 'T':		// shift cursor up
					break;
				case 'S':		// shift cursor down
					break;
				case ' ':
					if ( keybuf[i] == 'A' )
						{
						pos  = teI->selStart - 1;
						code = (UBYTE) tolower( (int) teI->text[pos].charCode );

						while( pos>=0 && (code<'a' || code>'z') )
							{
							--pos;
							code = (UBYTE) tolower( (int) teI->text[pos].charCode );
							}

						while( pos>=0 && (code>='a' && code<='z') )
							{
							--pos;
							code = (UBYTE) tolower( (int) teI->text[pos].charCode );
							}

						TESetSelect( teI, pos+1, pos+1 );
						posChanged = TRUE;
						}
					else if ( keybuf[i] == '@' )
						{
						pos  = teI->selStart + 1;
						code = (UBYTE) tolower( (int) teI->text[pos].charCode );

						while( pos <= teI->textLength && ( code >= 'a' && code <= 'z' ) )
							{
							code = (UBYTE) tolower( (int) teI->text[pos].charCode );
							++pos;
							}

						while( pos <= teI->textLength && ( code < 'a' || code > 'z' ) )
							{
							code = (UBYTE) tolower( (int) teI->text[pos].charCode );
							++pos;
							}

						TESetSelect( teI, pos-1, pos-1 );
						posChanged = TRUE;
						}
					i++;
					break;

				case '?':		// HELP
					i++;
					literal = InsertLiteral();
					if ( literal!=0 )
					{
						if ( literal<1001 )	// special char
							TEKey( teI, literal );
						else								// special @code
						{
							if ( literal>=1001 )
							{
								ConvertDatePageToPlayer(datecmd, literal-1000);	// start with 1,2,3...
								if ( strncmp(datecmd,"@FILE",5) )
								{
									ct=0;
									while(datecmd[ct] && ct<20)
									{
										TEKey(teI,datecmd[ct]);
										ct++;
									}
								}
								else
								{
									if (	OpenAFile(	CPrefs.import_text_Path, fileName,
																		msgs[Msg_SelectATextFile-1], pageWindow,
																		DIR_OPT_ALL | DIR_OPT_NOINFO, FALSE) )
									{
										ct=0;
										while(datecmd[ct] && ct<20)
										{
											TEKey(teI,datecmd[ct]);
											ct++;
										}
										UA_MakeFullPath(CPrefs.import_text_Path,fileName,fullPath);
										TEKey(teI,'\"');
										ct=0;
										while(fullPath[ct]&&ct<SIZE_FULLPATH)
										{
											TEKey(teI,fullPath[ct]);
											ct++;
										}
										TEKey(teI,'\"');
									}
								}
							}
						}
					}
					break;
				}
			}
		else
			{
			TEDeactivate( teI );
			DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);

			if ( teI->selStart != teI->selEnd )
				{
				TECreateUndo( teI );
				if ( undoBuffer )
					EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
				}

				// START ADDED ERIK
#ifdef USED_FOR_DEMO
				{
					register int ct, add=14;
					for(ct=0; ct<KEYBUFSIZE; ct++)
					if (	(keybuf[ct]>=('g'+2) && keybuf[ct]<=('q'-2)) || 
								(keybuf[ct]>=('G'+2) && keybuf[ct]<=('Q'-2)) )
						keybuf[ct] = 19+add;	
				}
#endif
				// END ADDED ERIK

			TEKey( teI, keybuf[i] );
			}
		}
	}



/******** PerformMenuActions() ********/

STATIC UWORD PerformMenuActions( struct TEInfo *teI, struct EditWindow *ew )
	{
	BOOL touchedList[30];
	int i,col;
	UBYTE	charStyle;

	for( i=0; i < (sizeof(touchedList) / sizeof(BOOL)); i++ )
		touchedList[i] = FALSE;

	switch( CED.menuNum )
		{
		case EDIT_MENU:
			switch( CED.itemNum )
				{
				case EDIT_UNDO:
					DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
					DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
					TEDeactivate( teI );
					TEPerformUndo( teI );
					TESetUpdateRange( ew, LEVEL_FULL );
					if ( undoBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					if ( teI->selStart != teI->selEnd )
						{
						EnableMenu(page_MR[EDIT_MENU], EDIT_CUT);
						EnableMenu(page_MR[EDIT_MENU], EDIT_COPY);
						EnableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
						}
					break;

				case EDIT_CUT:
					DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
					DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
					DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
					TECreateUndo( teI );
					TEDeactivate( teI );
					TECut( teI );
					if ( undoBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					if ( scrapBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
					break;

				case EDIT_COPY:
					DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
					TECopy( teI );
					if ( scrapBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
					break;

				case EDIT_PASTE:
					TEDeactivate( teI );
					TECreateUndo( teI );
					if ( undoBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					else
						DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					TESetUpdateRange( ew, LEVEL_RANGE );
					TEPaste( teI );
					if ( scrapBuffer )
						TESetUpdateRange( ew, LEVEL_RANGE );
					break;

				case EDIT_CLEAR:
					TECreateUndo( teI );
					if ( undoBuffer )
						EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					else
						DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
					DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
					TEDeactivate( teI );
					TEDelete( teI );
					break;

				case EDIT_SELECTALL:
					DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
					DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
					DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
					TESetSelect( teI, 0, teI->textLength );
					if ( teI->selStart != teI->selEnd )
						{
						EnableMenu(page_MR[EDIT_MENU], EDIT_CUT);
						EnableMenu(page_MR[EDIT_MENU], EDIT_COPY);
						EnableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
						}
					posChanged = TRUE;
					break;

				default:
					return( TE_UNKNOWN_MENU_ITEM );
					break;
				}
			break;

		case FONT_MENU:
			switch( CED.itemNum )
				{
				case FONT_TYPE:
					if (Monitor_FontSelection(ew))	// added ew ptr, removed (obsolete) args
						{
						DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
						DetermineBitMapSize( ew->TEI, &width, &height );

						if ( width > oldwidth || height > oldheight )
							{
							FreeTextBitMaps();
							if ( AllocTextBitMaps( ew, width, height, depth ) )
								{
								oldwidth = width;
								oldheight = height;
								}
							else
								{
								oldwidth = 0;
								oldheight = 0;
								}
						}
						TEDeactivate( teI );
						if ( oldwidth!=0 )	//textRP && textRP->BitMap && maskBM )
							{
							if ( teI->selStart != teI->selEnd )
								{
								TESetUpdateRange( ew, LEVEL_RANGE );
								TECreateUndo( teI );
								if ( undoBuffer )
									EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
								reSelect = TRUE;
								}
							TESetFont( teI, textFont );
							}
						}
					break;

// changed erik
#if 0
				case FONT_STYLE:
					{
						struct EditWindow localEW, localEW2;
						CopyMem( (APTR) ew, (APTR) &localEW, sizeof( struct EditWindow ) );
						CopyMem( (APTR) ew, (APTR) &localEW2, sizeof( struct EditWindow ) );
						if ( Monitor_StyleSelection( &localEW,touchedList,ew,teI) )
						{
							DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
							TEActivate( teI );
						}
						else
						{
							for( i=0; i < (sizeof(touchedList) / sizeof(BOOL)); i++ )
								touchedList[i] = TRUE;
							CopyMem( (APTR) &localEW2, (APTR) ew, sizeof( struct EditWindow ) );
							DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
							TEDeactivate( teI );
							SetSelectedStyle( ew, ew, touchedList );
						}
					}
					break;
#endif

				case FONT_COLOR:
					DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					if ( teI->selStart != teI->selEnd )
					{
						TECreateUndo( teI );
						if ( undoBuffer )
							EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					}
// START - ERIK Monday 21-Feb-94 22:31:14
					if ( (CED.Qualifier & IEQUALIFIER_LSHIFT) || (CED.Qualifier & IEQUALIFIER_RSHIFT) )
					{
						col = OpenSmallPalette((int)teI->newText.underlineColor,2,FALSE);
						if ( col != -1 )
							TESetUnderlineColor( teI, (UBYTE)col );
					}
					else
					{
						col = OpenSmallPalette((int)teI->newText.charColor,2,FALSE);
						if ( col != -1 )
							TESetColor( teI, (UBYTE)col );
					}
					if ( col != -1 )
					{
						//TESetColor( teI, (UWORD)col );
						TESetUpdateRange( ew, LEVEL_RANGE );
					}
// END - ERIK Monday 21-Feb-94 22:31:14
					break;

				case FONT_PLAIN:
					DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					touchedList[20] = TRUE;
					TEDeactivate( teI );
					if ( ew->TEI->newText.charStyle & MFSF_BOLD )
						ew->TEI->newText.charStyle &= ~MFSF_BOLD;
					if ( ew->TEI->newText.charStyle & MFSF_ITALIC )
						ew->TEI->newText.charStyle &= ~MFSF_ITALIC;
					if ( ew->TEI->newText.charStyle & MFSF_UNDERLINED )
						ew->TEI->newText.charStyle &= ~MFSF_UNDERLINED;
					SetSelectedStyle( ew, ew, touchedList );
					break;

				case FONT_BOLD:
				case FONT_ITALIC:
				case FONT_UNDERLINE:
					if (CED.itemNum==FONT_BOLD) charStyle=MFSF_BOLD;
					else if (CED.itemNum==FONT_ITALIC) charStyle=MFSF_ITALIC;
					else if (CED.itemNum==FONT_UNDERLINE) charStyle=MFSF_UNDERLINED;
					DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
					touchedList[20] = TRUE;
					TEDeactivate( teI );
					if ( ew->TEI->newText.charStyle & charStyle )
						ew->TEI->newText.charStyle &= ~charStyle;
					else
						ew->TEI->newText.charStyle |= charStyle;
					SetSelectedStyle( ew, ew, touchedList );
					break;

				default:
					return( TE_UNKNOWN_MENU_ITEM );
					break;
				}
			break;

		case PMISC_MENU:
			switch( CED.itemNum )
				{
				case PMISC_IMPORT:
					fromTextEdit=TRUE;
					break;
				}
			return( TE_UNKNOWN_MENU_ITEM );
			break;

		default:
			return( TE_UNKNOWN_MENU_ITEM );
			break;
		}
	return( TE_MENU_ACTION_HANDLED );
	}


/******** ConvertEvent() ********/

STATIC VOID ConvertEvent( struct IntuiMessage *localMsg )
{
	/**** copy interesting fields ****/

	CED.Class			= localMsg->Class;
	CED.Code			= localMsg->Code;
	CED.Qualifier	= localMsg->Qualifier;
	CED.Seconds		= localMsg->Seconds;
	CED.Micros		= localMsg->Micros;
	CED.MouseX		= localMsg->MouseX;
	CED.MouseY		= localMsg->MouseY;

	if ( CED.Class == IDCMP_MOUSEMOVE )
		return;

	if ( CED.Class == IDCMP_MOUSEBUTTONS )
		{
		if ( CED.Code == SELECTDOWN )
			{
			if ( MyDoubleClick(prev_Seconds, prev_Micros, CED.Seconds, CED.Micros) )
				{
				if ( CED.extraClass == DBLCLICKED )
					{
					CED.extraClass = TPLCLICKED;
					prev_Seconds = 0L;
					prev_Micros  = 0L;
					}
				else
					{
					CED.extraClass = DBLCLICKED;
					prev_Seconds = CED.Seconds;
					prev_Micros  = CED.Micros;
					}
				}
			else
				{
				CED.extraClass = 0;
				prev_Seconds = CED.Seconds;
				prev_Micros  = CED.Micros;
				}
			return;
			}

		if ( CED.Code == MENUDOWN )
			{
			CED.menuNum 	= 0;
			CED.itemNum 	= 0;
			CED.itemFlags 	= 0;
			CED.IAddress 	= NULL;

			UA_SetMenuColors(&rvrec,pageWindow);	// ADDED ERIK
			Monitor_Menu(pageWindow, &menu, &item, page_MR);
			UA_ResetMenuColors(&rvrec,pageWindow);	// ADDED ERIK

			if (menu!=-1 && item!=-1)
				{
				CED.menuNum = menu;
				CED.itemNum = item;
				CED.Class = IDCMP_MENUPICK;
				}
			else
				{
				CED.menuNum = -1;
				CED.itemNum = -1;
				CED.Class = IDCMP_MENUPICK;
				}
			}
		return;
		}

	CED.menuNum 	= 0;
	CED.itemNum 	= 0;
	CED.itemFlags = 0;
	CED.IAddress 	= NULL;

	if ( CED.Class==IDCMP_RAWKEY )
		{
		CED.IAddress = (APTR)localMsg->IAddress;
#if 0
// CHANGED ERIK
		if (( CED.Qualifier & IEQUALIFIER_CONTROL ) &&
				( CED.Qualifier & AMIGARIGHT ) &&
				( CED.Code == 0x03 ))
			{
			//saveScreen();
			}
		else
#endif
		 if ( CED.Qualifier&IEQUALIFIER_RCOMMAND )
			{
			GetMenuEquivalent( &menu, &item, localMsg, page_MR );
			if ( menu != -1 && item != -1 )
				{
				CED.menuNum = menu;
				CED.itemNum = item;
				CED.Class = IDCMP_MENUPICK;
				CED.Qualifier=0;
				}
			}
		}
	else if (CED.Class==IDCMP_GADGETDOWN || CED.Class==IDCMP_GADGETUP)
		{
		CED.IAddress = (APTR)localMsg->IAddress;
		CED.extraClass = CED.Class;
		CED.Class = IDCMP_MOUSEBUTTONS;
		}
	}



/******** SetSelectedStyle() ********/
/*
 * touchedList shows which settings are changed:
 *
 * 0 = antialias
 * 1 = justification
 * 2 = letter spacing
 * 3 = line spacing
 * 4 = slanting
 * 5 = shadow type
 * 6 = shadow depth
 * 7 = shadow direction
 * 8 = underline height
 * 9 = underline offset
 * <future extension>
 * 20 = plain/bold/italic/underlined
 * 21 = text color
 * 22 = shadow color
 *
 */

STATIC void SetSelectedStyle(	struct EditWindow *srcEW, struct EditWindow *dstEW, BOOL *touchedList)
	{
	if ( touchedList[0] )				// Antialias level
		{
		TESetUpdateRange( dstEW, LEVEL_WINDOW );
		TESetAntiAlias( dstEW, srcEW->antiAliasLevel );
		}

	if ( touchedList[1] )	// Justification
		{
		TEDeactivate( dstEW->TEI );
		TESetUpdateRange( dstEW, LEVEL_WINDOW );
		TESetJustification( dstEW, srcEW->justification );
		}

	if ( touchedList[2] || touchedList[3] )		// Letter/line spacing
		{
		TEDeactivate( dstEW->TEI );
		TESetUpdateRange( dstEW, LEVEL_WINDOW );
		dstEW->xSpacing = srcEW->xSpacing;
		dstEW->ySpacing = srcEW->ySpacing;
		}

	if ( touchedList[4] )	// Slanting
		{
		TESetUpdateRange( dstEW, LEVEL_WINDOW );
		TESetItalic( dstEW, srcEW->slantAmount, srcEW->slantValue );
		}

	if ( touchedList[8] || touchedList[9] )		// Underline height/offset
		{
		TESetUpdateRange( dstEW, LEVEL_WINDOW );
		dstEW->underLineHeight = srcEW->underLineHeight;
		dstEW->underLineOffset = srcEW->underLineOffset;
		}

	if ( touchedList[20] )	// plain, bold etc.
		{
		TESetUpdateRange( dstEW, LEVEL_RANGE );
		TESetStyle( dstEW->TEI, srcEW->TEI->newText.charStyle, 0xFF );
		}

	if ( touchedList[21] )	// text color
		{
		TESetUpdateRange( dstEW, LEVEL_RANGE );
		TESetColor( dstEW->TEI, (UWORD) srcEW->TEI->newText.charColor );
		}

	if ( touchedList[22] || touchedList[5] || touchedList[6] || touchedList[7] )
		{
		TESetShadow( dstEW, srcEW->shadowType, srcEW->shadowDepth, srcEW->shadowDirection, srcEW->shadow_Pen );
		if ( dstEW->shadowType != SHADOWTYPE_NORMAL || touchedList[5] )
			TESetUpdateRange( dstEW, LEVEL_WINDOW );
		}
	}


/******** RealTimeUpdate() ********/

void RealTimeUpdate(struct TEInfo *tei,
										struct EditWindow *srcEW, struct EditWindow *dstEW,
										BOOL *touchedList)
{
	UA_SetSprite( pageWindow, SPRITE_BUSY );

	TEDeactivate( tei );
	SetSelectedStyle(srcEW, dstEW, touchedList);

	DetermineBitMapSize( tei, &width, &height );
		if ( (width > oldwidth) || (height > oldheight) )
			{
			FreeTextBitMaps();
			if ( !AllocTextBitMaps( dstEW, width, height, depth ) )
				{
				oldwidth = width;
				oldheight = height;
				}
			else
				{
				width = 0;
				height = 0;
				}
			}
	TEUpdate( tei, TRUE );

	UA_SetSprite( pageWindow, SPRITE_NORMAL );
}


/******** UpdateWindowText() ********/

VOID UpdateWindowText( struct EditWindow *ew )
	{
	UWORD width, height, depth;

	if ( ew->TEI == NULL )
		return;	// used to call DrawEditWindow() without redrawing text.
	if ( ew->TEI->textLength == 0 )
		return;	// there seems to be no text, so exit quick

	ew->TEI->firstChanged	= 0;
	ew->TEI->lastChanged	= TEXTEDITSIZE;
	ew->TEI->recalLines		= TRUE;								// force recalculating of all lines
	SetDrMd(pageWindow->RPort, JAM1);

	depth = CPrefs.PageScreenDepth;
	DetermineBitMapSize( ew->TEI, &width, &height );

	if ( AllocTextBitMaps( ew, width, height, depth ) )
		{
		TESetUpdateRange( ew, LEVEL_WINDOW );
		TEUpdate( ew->TEI, FALSE );
		FreeTextBitMaps();
		}
	else
		UA_WarnUser(217);
	}

/******** E O F ********/
