#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern struct NewWindow NewWindowStructure;
extern struct EditWindow undoEW;
extern struct EditSupport undoES;
extern int selectedWell;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditWindow **Clipboard_WL;
extern struct EditSupport **Clipboard_SL;
extern struct EditWindow **Undo_WL;
extern struct EditSupport **Undo_SL;
extern int lastUndoableAction;
extern int lastUndoWindow;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern struct ColorMap *undoCM;

/****** local functions *****/

STATIC int compare( RGB *p, RGB *q );
STATIC void sort_pal( RGB *pal, int entries );
STATIC void xchg_rgb( RGB *pal, int m, int n );
STATIC void copy_pal( RGB *sp, RGB *dp, int entries );
STATIC void contrast_pal( RGB *pal, int entries , int level, BOOL AA );

STATIC BOOL NotYetStored(int hole, int r, int g, int b);

/***** external functions ****/

extern void reduce_palette( RGB *pal, int npal, int nsub );
extern void smooth_colormap( RGB *img, int nimg, RGB *dev, int ndev );

/**** functions ****/

/******** OptimizePalette() ********/

BOOL OptimizePalette(BOOL smooth)
{
int i, j, neededSpace=0, hole=0, r,g,b, numEntries, numPageColors;
RGB *colorSpace, *dest;

	SetSpriteOfActWdw(SPRITE_BUSY);

	// undoCM is used by RemapWindows() (see function below) because it:
	// 1: looks for the pen e.g. a border has
	// 2: looks what rgb values in undoCM the pen has
	// 3: looks in PageCM what pen corresponds best with the former pen
	CopyCMtoCM(CPrefs.PageCM,undoCM);

	// count number of windows with pictures

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] &&
					EditSupportList[i]->cm )
		{
			if (	EditSupportList[i]->photoOpts & MOVE_PHOTO ||
						EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				neededSpace++;
			}
		}
	}
	if ( backES.cm && (backES.photoOpts & MOVE_PHOTO || backES.photoOpts & SIZE_PHOTO) )
		neededSpace++;

	// allocate color space (probably more than needed)

	if ( neededSpace==0 )
	{
		// START REMAP WINDOWS
		RemapWindows(1);	// start filling from well 1
		SyncAllColors(FALSE);
		RefreshPalette();
		SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
		PaletteToFront();
		// END REMAP WINDOWS

		SetSpriteOfActWdw(SPRITE_NORMAL);
		return( TRUE );
	}
	
	colorSpace = (RGB *)AllocMem((sizeof(RGB)*256)*neededSpace, MEMF_ANY | MEMF_CLEAR);
	if ( !colorSpace )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		return( FALSE );
	}

	// fill color space with all colors

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( 	EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] &&
					EditSupportList[i]->cm )
		{
			if (	EditSupportList[i]->photoOpts & MOVE_PHOTO ||
						EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				numEntries = UA_GetNumberOfColorsInScreen(EditSupportList[i]->iff->viewModes,
																									EditSupportList[i]->iff->BMH.nPlanes,
																									TRUE);
				for(j=0; j<numEntries; j++)
				{
					GetColorComponents(EditSupportList[i]->cm, j, &r, &g, &b);
					colorSpace[hole].red = r;
					colorSpace[hole].green = g;
					colorSpace[hole].blue = b;
					colorSpace[hole].dist = 0;
					hole++;

					if ( EditSupportList[i]->iff->viewModes & EXTRAHALFBRITE_KEY )
					{
						GetColorComponents(EditSupportList[i]->cm, j, &r, &g, &b);
						colorSpace[hole].red = r/2;
						colorSpace[hole].green = g/2;
						colorSpace[hole].blue = b/2;
						colorSpace[hole].dist = 0;
						hole++;
					}
				}
			}
		}
	}
	if ( backES.cm && (backES.photoOpts & MOVE_PHOTO || backES.photoOpts & SIZE_PHOTO) )
	{
		numEntries = UA_GetNumberOfColorsInScreen(backES.iff->viewModes,
																							backES.iff->BMH.nPlanes,
																							TRUE);
		for(j=0; j<numEntries; j++)
		{
			GetColorComponents(backES.cm, j, &r, &g, &b);
			colorSpace[hole].red = r;
			colorSpace[hole].green = g;
			colorSpace[hole].blue = b;
			colorSpace[hole].dist = 0;
			hole++;

			if ( backES.iff->viewModes & EXTRAHALFBRITE_KEY )
			{
				GetColorComponents(backES.cm, j, &r, &g, &b);
				colorSpace[hole].red = r/2;
				colorSpace[hole].green = g/2;
				colorSpace[hole].blue = b/2;
				colorSpace[hole].dist = 0;
				hole++;
			}
		}
	}

	// allocate space for destination colors

	dest = (RGB *)AllocMem((sizeof(RGB)*256)*neededSpace, MEMF_ANY | MEMF_CLEAR);
	if ( !dest )
	{
		FreeMem(colorSpace, (sizeof(RGB)*256)*neededSpace);
		SetSpriteOfActWdw(SPRITE_NORMAL);
		return( FALSE );
	}

	// optimize palette

	numPageColors = UA_GetNumberOfColorsInScreen(	CPrefs.PageScreenModes,
																								CPrefs.PageScreenDepth,
																								TRUE );

	copy_pal(colorSpace, dest, 256*neededSpace);

	if( (numPageColors-1) < hole )
		reduce_palette(colorSpace, hole, numPageColors-1);

	if ( smooth )
	{
		smooth_colormap(dest, hole, colorSpace, numPageColors-1 );
		contrast_pal( colorSpace, numPageColors-1, 16, CPrefs.AA_available);
		sort_pal( colorSpace, numPageColors-1 );
	}

	// put optimized palette into page colormap

	for(i=1; i<numPageColors; i++)
		SetColorComponents(	pageScreen->ViewPort.ColorMap,
												i,
												colorSpace[i-1].red,
												colorSpace[i-1].green,
												colorSpace[i-1].blue );

	// START REMAP WINDOWS
	RemapWindows(hole);	// start filling from well 'hole'
	// END REMAP WINDOWS

	SyncAllColors(FALSE);
	RefreshPalette();
	SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
	PaletteToFront();

	// free color palette

	FreeMem(dest, (sizeof(RGB)*256)*neededSpace);
	FreeMem(colorSpace, (sizeof(RGB)*256)*neededSpace);

	SetSpriteOfActWdw(SPRITE_NORMAL);

	return(TRUE);
}

//
// compare
//
STATIC int compare( RGB *p, RGB *q )
{
	return p->dist > q->dist ? -1 : p->dist > q->dist ? 1 : 0;
}

//
// Sort palette with the distance to zero 
//
STATIC void sort_pal( RGB *pal, int entries )
{
int i;

	for( i = 0; i < entries; i++ )
		pal[i].dist = (int)pal[i].red * (int)pal[i].red +
									(int)pal[i].green * (int)pal[i].green +
									(int)pal[i].blue * (int)pal[i].blue;

	qsort( pal, entries, sizeof( RGB ), compare );

	//xchg_rgb( pal, 0, entries-1 );

/*
	if ( entries>16 )
	{
		xchg_rgb( pal, 17, entries-4 );
		xchg_rgb( pal, 18, entries-3 );
		xchg_rgb( pal, 19, entries-2 );
	}
*/
}

/******** xchg_rgb() ********/
/*
 * swap two palette entries
 *
 */

STATIC void xchg_rgb( RGB *pal, int m, int n )
{
RGB tmp;

	tmp.red = pal[m].red;
	tmp.green = pal[m].green;
	tmp.blue = pal[m].blue;
	tmp.dist = pal[m].dist;
	pal[m].red = pal[n].red;
	pal[m].green = pal[n].green;
	pal[m].blue = pal[n].blue;
	pal[m].dist = pal[n].dist;
	pal[n].red = tmp.red;
	pal[n].green = tmp.green;
	pal[n].blue = tmp.blue;
	pal[n].dist = tmp.dist;
}

STATIC void contrast_pal( RGB *pal, int entries , int level, BOOL AA )
{
	int i;
	int low,high;
	int delta;
	UBYTE ctab[256];

	if( level > 120 )
		level = 120;
	low = level;
	high = 255 - level;

	delta = 255;
	delta = ( delta << 4 ) / ( high - low ) ;
	for( i = 0; i < 256; i++ )
	{
		if( i < low )
			ctab[ i ] = 0;
		else
			if( i > high )
				ctab[i] = 255;
			else
				ctab[i] = ((i - low )*delta)>>4;
	}

	if( AA )
	{
		for( i = 0; i < entries; i++ )
		{
			pal[i].red = ctab[(UBYTE)pal[i].red];
			pal[i].green = ctab[(UBYTE)pal[i].green];
			pal[i].blue = ctab[(UBYTE)pal[i].blue];
		}
	}
	else
		for( i = 0; i < entries; i++ )
		{
			pal[i].red = ctab[(pal[i].red<<4) | pal[i].red]>>4;
			pal[i].green = ctab[(pal[i].green<<4) | pal[i].green]>>4;
			pal[i].blue = ctab[(pal[i].blue<<4) | pal[i].blue]>>4;
		}
}

/******** copy_pal() ********/

STATIC void copy_pal( RGB *sp, RGB *dp, int entries )
{
	int i;
	for( i = 0; i < entries; i++ )
	{
		dp[i].red = sp[i].red;
		dp[i].green = sp[i].green;
		dp[i].blue = sp[i].blue;
	}
}

/******** RemapWindows() ********/

void RemapWindows(int emptyFromHere)
{
int i, npc, hole, j;
struct ViewPort *vp;
struct BitMap *bm;
ULONG r,g,b;
UBYTE bit;

	vp = &(pageScreen->ViewPort);
	bm = pageScreen->RastPort.BitMap;
	npc = UA_GetNumberOfColorsInScreen(vp->Modes, bm->Depth, CPrefs.AA_available);

	hole=emptyFromHere;

	if ( hole>npc )
		hole=npc;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			//===========================
			// BACKFILLCOLOR
			//===========================
			if ( EditWindowList[i]->BackFillType!=2 )	// solid or pattern
			{
				GetColorComponents(undoCM, EditWindowList[i]->BackFillColor, (int *)&r,(int *)&g,(int *)&b);			
				if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
				{
					SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
					EditWindowList[i]->BackFillColor = hole;
					hole++;
				}
				else
				{
					if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
					TurnSmallIntoLarge(&r,&g,&b);
					EditWindowList[i]->BackFillColor = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
				}
			}

			//===========================
			// BORDERCOLORS
			//===========================
			bit=1;
			for(j=0; j<4; j++)
			{
				if ( EditWindowList[i]->Border & bit )
				{
					GetColorComponents(undoCM, EditWindowList[i]->BorderColor[j], (int *)&r,(int *)&g,(int *)&b);			
					if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
					{
						SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
						EditWindowList[i]->BorderColor[j] = hole;
						hole++;
					}
					else
					{
						if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
						TurnSmallIntoLarge(&r,&g,&b);
						EditWindowList[i]->BorderColor[j] = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
					}
				}
				bit <<= 1;	
			}

			//===========================
			// INDIVIDUAL TEXT COLOR
			//===========================
			if ( EditWindowList[i]->TEI )
			{
				for(j=0; j<EditWindowList[i]->TEI->textLength; j++)
				{
					if ( EditWindowList[i]->TEI->text[j].charCode != '\0' )
					{
						GetColorComponents(undoCM, EditWindowList[i]->TEI->text[j].charColor, (int *)&r,(int *)&g,(int *)&b);			
						if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
						{
							SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
							EditWindowList[i]->TEI->text[j].charColor = hole;
							hole++;
						}
						else
						{
							if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
							TurnSmallIntoLarge(&r,&g,&b);
							EditWindowList[i]->TEI->text[j].charColor = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
						}
					}
				}
			}

			//===========================
			// FONT SHADOW COLOR
			//===========================
			if ( EditWindowList[i]->shadowType!=0 )
			{
				GetColorComponents(undoCM, EditWindowList[i]->shadow_Pen, (int *)&r,(int *)&g,(int *)&b);			
				if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
				{
					SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
					EditWindowList[i]->shadow_Pen = hole;
					hole++;
				}
				else
				{
					if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
					TurnSmallIntoLarge(&r,&g,&b);
					EditWindowList[i]->shadow_Pen = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
				}
			}

			//===========================
			// WINDOW SHADOW PEN
			//===========================

			if ( EditWindowList[i]->wdw_shadowDirection!=0 )
			{
				GetColorComponents(undoCM, EditWindowList[i]->wdw_shadowPen, (int *)&r,(int *)&g,(int *)&b);			
				if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
				{
					SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
					EditWindowList[i]->wdw_shadowPen = hole;
					hole++;
				}
				else
				{
					if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
					TurnSmallIntoLarge(&r,&g,&b);
					EditWindowList[i]->wdw_shadowPen = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
				}
			}

			//===========================
			// INITIAL FONT COLOR
			//===========================
			GetColorComponents(undoCM, EditWindowList[i]->charColor, (int *)&r,(int *)&g,(int *)&b);			
			if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
			{
				SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
				EditWindowList[i]->charColor = hole;
				hole++;
			}
			else
			{
				if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
				TurnSmallIntoLarge(&r,&g,&b);
				EditWindowList[i]->charColor = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
			}

			//===========================
			// CRAWL COLOR
			//===========================

			if ( EditWindowList[i]->crawl_fontName[0]!='\0' ) // this window carries crawl data
			{
				GetColorComponents(undoCM, EditWindowList[i]->crawl_color, (int *)&r,(int *)&g,(int *)&b);			
				if ( (hole<=(npc-2)) && NotYetStored(hole,r,g,b) )	// npc-2 is eg 254
				{
					SetColorComponents(pageScreen->ViewPort.ColorMap,hole,r,g,b);
					EditWindowList[i]->crawl_color = hole;
					hole++;
				}
				else
				{
					if ( !CPrefs.AA_available ) { r |= (r<<4); g |= (g<<4); b |= (b<<4); }
					TurnSmallIntoLarge(&r,&g,&b);
					EditWindowList[i]->crawl_color = UA_MyFindColor(pageScreen->ViewPort.ColorMap,r,g,b,hole-1,hole-1,TRUE);
				}
			}
		}
	}
}

/******** NotYetStored() ********/
/*
 * search if triplets are already stored up to hole (exclusive hole)
 *
 */

STATIC BOOL NotYetStored(int hole, int r, int g, int b)
{
int i;
int rr,gg,bb;

	for(i=0; i<hole; i++)
	{
		GetColorComponents(pageScreen->ViewPort.ColorMap, i, &rr,&gg,&bb);			
		if ( rr==r && gg==g && bb==b )
			return(FALSE);
	}

	return(TRUE);
}

/******** E O F ********/
