#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
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
extern int HANDLESNIF;

/**** functions ****/

/******** CreateEditWindow() ********/

BOOL CreateEditWindow(void)
{
ULONG signals;
BOOL loop=TRUE, drawIt=FALSE, mouseMoved=FALSE;
WORD start_x, start_y, end_x, end_y;
int slot;
struct IntuiMessage *message;
struct Window *aWin;

	/**** first find an empty editwindow slot ****/

	slot = SearchEmptyWindow();
	if (slot==-1)
		return(FALSE);	// no more windows can be opened: not a s erious error

	aWin = GetActiEditWin();
	if (aWin!=NULL)
		UA_SwitchMouseMoveOn(aWin);

	/**** draw first box (i.e. only a dot) ****/

	start_x	= pageWindow->MouseX;
	start_y	= MassageY(pageWindow->MouseY);
	end_x		= start_x;
	end_y		= start_y;
	DrawMarquee(start_x, start_y, &end_x, &end_y);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class		= message->Class;
				CED.Code		= message->Code;
				CED.MouseX	= pageWindow->MouseX;
				CED.MouseY	= MassageY(pageWindow->MouseY);

				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
						{
							drawIt=TRUE;
							loop=FALSE;
						}
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}
			if (mouseMoved)
				DrawMarquee(start_x, start_y, &end_x, &end_y);
		}
	}

	if (aWin!=NULL)
		UA_SwitchMouseMoveOff(aWin);

	/**** swap co-ordinates if necessary ****/

	if (end_x < start_x)
		swapWORDS(&start_x, &end_x);

	if (end_y < start_y)
		swapWORDS(&start_y, &end_y);

	/**** cancel opening of window if dimensions are too small ****/

	if ( (end_x-start_x) < MINWINWIDTH)
		drawIt = FALSE;

	/**** cancel opening of window if dimensions are too small ****/

	else if ( (end_y-start_y) < MINWINHEIGHT)
		drawIt = FALSE;

	/****  remove marquee ****/

	DrawMarqueeBox(pageWindow->RPort, start_x, start_y, end_x, end_y);

	if(drawIt)
	{
		DrawAllHandles(MAKE_INACTIVE);
		slot = OpenEditWindow(slot, start_x, start_y, end_x, end_y);
		if (slot != -1)
		{
			DrawEditWindow(EditWindowList[slot], EditSupportList[slot]);
			EditSupportList[slot]->Active = TRUE;
			DrawHandles(EditWindowList[slot]->X,
									EditWindowList[slot]->Y,
									EditWindowList[slot]->X + EditWindowList[slot]->Width - 1,
									EditWindowList[slot]->Y + EditWindowList[slot]->Height - 1);
		}
	}

	return(drawIt);
} 

/******** SearchEmptyWindow() ********/
/*
 *  return empty slot number or -1
 */

int SearchEmptyWindow(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i]==NULL )
			return(i);

	return(-1);
}

/******** DrawHandles() ********/

void DrawHandles(WORD x1, WORD y1, WORD x2, WORD y2)
{
WORD mx,my,lx,ty,rx,by;
struct RastPort *rp;

	rp = pageWindow->RPort;

	mx = x1 + (x2-x1)/2;
	my = y1 + (y2-y1)/2;

	SetDrMd(rp, JAM2 | COMPLEMENT);
	SafeSetWriteMask(rp, 0x3);

	if (x1<(HANDLESNIF-1))
		lx=x1;
	else
		lx=(HANDLESNIF-1);

	if (y1<(HANDLESNIF-1))
		ty=y1;
	else
		ty=(HANDLESNIF-1);

	if (x2>=CPrefs.PageScreenWidth-HANDLESNIF)
		rx=CPrefs.PageScreenWidth-x2-1;
	else
		rx=(HANDLESNIF-1);

	if (y2>=CPrefs.PageScreenHeight-HANDLESNIF)
		by=CPrefs.PageScreenHeight-y2-1;
	else
		by=(HANDLESNIF-1);

	/**** the 3 is half of MINWINWIDTH and MINWINHEIGHT ****/

	RectFill(rp, x1-lx, y1-ty, x1+(HANDLESNIF-1), y1+(HANDLESNIF-1));
	RectFill(rp, mx-(HANDLESNIF-1), y1-ty, mx+(HANDLESNIF-1), y1+(HANDLESNIF-1));
	RectFill(rp, x2-(HANDLESNIF-1), y1-ty, x2+rx, y1+(HANDLESNIF-1));
	RectFill(rp, x2-(HANDLESNIF-1), my-(HANDLESNIF-1), x2+rx, my+(HANDLESNIF-1));
	RectFill(rp, x2-(HANDLESNIF-1), y2-(HANDLESNIF-1), x2+rx, y2+by);
	RectFill(rp, mx-(HANDLESNIF-1), y2-(HANDLESNIF-1), mx+(HANDLESNIF-1), y2+by);
	RectFill(rp, x1-lx, y2-(HANDLESNIF-1), x1+(HANDLESNIF-1), y2+by);
	RectFill(rp, x1-lx, my-(HANDLESNIF-1), x1+(HANDLESNIF-1), my+(HANDLESNIF-1));

	DrawMarqueeBox(rp, x1, y1, x2, y2);

	SetDrMd(rp, JAM1);
	SafeSetWriteMask(rp, 0xff);
}

/******** DrawAllHandles() ********/
/*
 *  If Flag is MAKE_INACTIVE (TRUE), windows will be made inactive
 *  other value is LEAVE_ACTIVE (FALSE)
 */

void DrawAllHandles(BOOL Flag)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] != NULL)
		{
			if (EditSupportList[i]->Active)
				DrawHandles(EditWindowList[i]->X,
										EditWindowList[i]->Y,
										EditWindowList[i]->X + EditWindowList[i]->Width - 1,
										EditWindowList[i]->Y + EditWindowList[i]->Height - 1);
			if (Flag)
				EditSupportList[i]->Active = FALSE;
		}
	}
}

/******** DrawMarquee() ********/

void DrawMarquee(WORD start_x, WORD start_y, WORD *end_x, WORD *end_y)
{
int mouseX,mouseY;

	mouseX = pageWindow->MouseX;
	mouseY = MassageY(pageWindow->MouseY);
	DrawMarqueeBox(pageWindow->RPort, start_x, start_y, *end_x, *end_y);
	DrawMarqueeBox(pageWindow->RPort, start_x, start_y, mouseX, mouseY);
	*end_x = mouseX;
	*end_y = mouseY;
}

/******** DrawMarqueeBox() ********/

void DrawMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2)
{
	SetAPen(rp, 1L);
	SetDrMd(rp, JAM2|COMPLEMENT);
	SafeSetWriteMask(rp, 0x3);

	if (x2<x1)
		swapWORDS(&x1, &x2);

	if (y2<y1)
		swapWORDS(&y1, &y2);

	Move(rp, (LONG)x1, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y1);

	SetDrMd(rp, JAM1);
	SafeSetWriteMask(rp, 0xff);
}

/******** DrawFatMarqueeBox() ********/

void DrawFatMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2)
{
	SetAPen(rp, 1L);
	SetDrMd(rp, JAM2|COMPLEMENT);
	SafeSetWriteMask(rp, 0x3);

	if (x2<x1)
		swapWORDS(&x1, &x2);

	if (y2<y1)
		swapWORDS(&y1, &y2);

	Move(rp, (LONG)x1, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y1);

	Move(rp, (LONG)x1+1, (LONG)y1+1);
	Draw(rp, (LONG)x2-1, (LONG)y1+1);
	Draw(rp, (LONG)x2-1, (LONG)y2-1);
	Draw(rp, (LONG)x1+1, (LONG)y2-1);
	Draw(rp, (LONG)x1+1, (LONG)y1+1);

	SetDrMd(rp, JAM1);
	SafeSetWriteMask(rp, 0xff);
}

/******** DrawEditWindow() ********/

void DrawEditWindow(struct EditWindow *ew, struct EditSupport *es)
{
BOOL realloc=FALSE;

	if ( !es->restore_bm.Planes[0] )
		realloc=TRUE;
	else if ( ew->Width+16 > es->restore_w || ew->Height > es->restore_h )
		realloc=TRUE;

	if ( realloc )
	{
		if ( es->restore_bm.Planes[0] )
		{
			FreeFastBitMap( &es->restore_bm );
			es->restore_w = 0;
			es->restore_h = 0;
		}

		if ( !InitAndAllocBitMap(&es->restore_bm, CPrefs.PageScreenDepth, ew->Width+16, ew->Height, MEMF_ANY) )
		{
			UA_WarnUser(-1);
			return;
		}
		es->restore_w = ew->Width+16;
		es->restore_h = ew->Height;
	}

	BltBitMapFM(	pageWindow->RPort->BitMap,
								ew->X+pageWindow->LeftEdge, ew->Y+pageWindow->TopEdge,
								&es->restore_bm,
								0,0, ew->Width,ew->Height, 0xc0,0xff,NULL);

	if ( ew->BackFillType == 0 )	// solid
	{
		RenderWindowInterior(pageWindow->RPort, ew);
	}
	else if ( ew->BackFillType == 1 )	// pattern
	{
		if ( !(es->photoOpts & MOVE_PHOTO) && !(es->photoOpts & SIZE_PHOTO) )
			RenderWindowInterior(pageWindow->RPort, ew);
	}

	RenderPhotoAndOrDoTransp(ew,es,pageWindow->RPort);

	RenderWindowBorders(pageWindow->RPort, ew);

	if ( ew->TEI )
		UpdateWindowText(ew);
}

/******** OpenEditWindow() ********/
/*
 * see also ptread, backwin and config for other window inits
 *
 *
 */

int OpenEditWindow(	int slot, WORD start_x, WORD start_y,
										WORD end_x, WORD end_y )
{
struct EditWindow *ew;
struct EditSupport *es;
BOOL fresh;
struct EditWindow localEW;
int i;
LONG pen,pen2;
struct ViewPort *vp;
int numPageColors;
struct BitMap *bm;
ULONG r,g,b,r2,g2,b2,l;

	/**** open window ****/

	if ( start_x==0 && start_y==0 && end_x==0 && end_y==0 )
	{
		fresh=FALSE;

		ew = EditWindowList[slot];
		es = EditSupportList[slot];
		CopyMem(ew, &localEW, sizeof(struct EditWindow));

		TEInitWindow(ew, pageWindow->RPort);
	}
	else	// allocate window
	{
		fresh = TRUE;

		ew = (struct EditWindow *)AllocMem(sizeof(struct EditWindow), MEMF_ANY | MEMF_CLEAR);
		if (ew == NULL)
		{
			UA_WarnUser(131);
			return(-1);
		}
		EditWindowList[slot] = ew;

		es = (struct EditSupport *)AllocMem(sizeof(struct EditSupport), MEMF_ANY | MEMF_CLEAR);
		if (es == NULL)
		{
			UA_WarnUser(132);
			return(-1);
		}
		EditSupportList[slot] = es;

		if (end_x<start_x)
			swapWORDS(&start_x, &end_x);

		if (end_y<start_y)
			swapWORDS(&start_y, &end_y);

		/**** init all EditWindow vars ****/

		ew->X 								= start_x;
		ew->Y 								= start_y;
		ew->Width 						= end_x-start_x+1;
		ew->Height 						= end_y-start_y+1;
		if ( ew->Width!=2 )	// import width
			ValidateBoundingBox(&ew->X, &ew->Y, &ew->Width, &ew->Height);	// NEW
		ew->TopMargin					= DEFAULT_TM;
		ew->BottomMargin			= DEFAULT_BM;
		ew->LeftMargin				= DEFAULT_LM;
		ew->RightMargin				= DEFAULT_RM;
		ew->Border						= DEFAULT_BORDER;
		ew->BorderColor[0]		= DEFAULT_BCOLOR;
		ew->BorderColor[1]		= DEFAULT_BCOLOR;
		ew->BorderColor[2]		= DEFAULT_BCOLOR;
		ew->BorderColor[3]		= DEFAULT_BCOLOR;
		ew->BorderWidth				= DEFAULT_BWIDTH;
		ew->BackFillType			= DEFAULT_BFTYPE;
		ew->BackFillColor			= selectedWell;
		ew->PhotoOffsetX			= 0;
		ew->PhotoOffsetY			= 0;
		ew->patternNum				= DEFAULT_PATTERN;
		ew->DrawSeqNum				= GetNewDrawSeqNum_2(ew);

		ew->charFont					= largeFont;
		ew->underlineColor		= 2;
		ew->charStyle					= 0;
		ew->charColor					= 2;

		ew->flags							= 0;

		for(i=0; i<3; i++)
		{
			ew->in1[i]					= -1;
			ew->in2[i]					= -1;
			ew->in3[i]					= -1;
			ew->out1[i]					= -1;
			ew->out2[i]					= -1;
			ew->out3[i]					= -1;
			ew->inDelay[i]			= 0;
			ew->outDelay[i]			= 0;
		}

		ew->antiAliasLevel		= 0;
		ew->justification			= 0;
		ew->xSpacing					= 0;
		ew->ySpacing					= 0;
		ew->slantAmount				= 2;
		ew->slantValue				= 1;
		ew->underLineHeight		= 1;
		ew->underLineOffset		= 0;
		ew->shadowDepth				= 0;
		ew->shadow_Pen				= 0;
		ew->shadowType				= 0;
		ew->shadowDirection		= 0;

		ew->wdw_shadowDepth		= 4;
		ew->wdw_shadowDirection = 0;
		ew->wdw_shadowPen			= 1;

		ew->crawl_fontName[0]	= '\0';
		ew->crawl_fontSize		= 0;
		ew->crawl_speed				= 0;
		ew->crawl_flags				= 0;
		ew->crawl_text				= NULL;
		ew->crawl_length			= 0;
		ew->crawl_color				= 1;

		ew->bx								= -1;
		ew->by								= -1;
		ew->bwidth						= -1;
		ew->bheight						= -1;
		ew->jumpType					= 0;
		ew->renderType				= RENDERTYPE_INVERT;
		ew->audioCue					= 0;
		ew->keyCode						= -1;	
		ew->rawkeyCode				= -1;
		ew->buttonName[0]			= '\0';
		ew->assignment[0]			= '\0';

		ew->animIsAnim				= FALSE;
		ew->animLoops					= 0;
		ew->animSpeed					= 0;
		ew->animFromDisk			= 0;
		ew->animAddFrames			= 0;

		/**** init all EditSupport vars ****/

		es->Active						= FALSE;
		es->iff								= NULL;
		es->photoOpts					= 0;
		es->picPath[0]				= '\0';
		es->cm								= NULL;

		ClearBitMap24(&es->ori_bm);
		es->ori_w							= 0;
		es->ori_h							= 0;

		ClearBitMap(&es->scaled_bm);
		es->scaled_w					= 0;
		es->scaled_h					= 0;

		ClearBitMap(&es->remapped_bm);
		es->remapped_w				= 0;
		es->remapped_h				= 0;

		ClearBitMap(&es->restore_bm);
		es->restore_w					= 0;
		es->restore_h					= 0;

		ClearBitMap(&es->mask_bm);
		es->mask_w						= 0;
		es->mask_h						= 0;

		ClearBitMap(&es->ori_mask_bm);
		es->ori_mask_w				= 0;
		es->ori_mask_h				= 0;

		es->ditherMode				= DITHER_FLOYD;
	}

	if ( !ew->TEI )
	{
		/**** text editor stuff ****/

		ew->TEI = (struct TEInfo *)AllocMem(sizeof(struct TEInfo), MEMF_ANY | MEMF_CLEAR);
		if ( ew->TEI == NULL )
		{
			UA_WarnUser(134);
			return(-1);
		}
		
		TEInitInfo(ew->TEI);

		TEInitWindow(ew, pageWindow->RPort);

		NewList((struct List *)&(ew->TEI->frameList));
		AddTail((struct List *)&(ew->TEI->frameList), (struct Node *)ew);

		ew->TEI->text = (struct TEChar *)AllocMem(
																				sizeof(struct TEChar)*TEXTEDITSIZE,
																				MEMF_ANY | MEMF_CLEAR);
		if ( ew->TEI->text==NULL )
		{
			UA_WarnUser(135);
			return(-1);
		}

		ew->TEI->text[0].charCode = 0;
		ew->TEI->textLength = 0;

		TESetUpdateRange( ew, LEVEL_FULL );

		TESetSelect(ew->TEI, 0, ew->TEI->textLength);	// selection range = hele tekst
		TESetFont(ew->TEI, largeFont);								// zet het font voor de selectie
		TESetColor(ew->TEI, 2);												// zet de kleur voor de selectie
		TESetStyle(ew->TEI, 0, 0xFF);									// zet de style voor de selectie
		TESetSelect(ew->TEI, 0, 0);										// herstel cursor op linksboven

		TESetFont(ew->TEI, largeFont);								// zet het default font
		TESetColor(ew->TEI, 2);												// zet de default kleur
		TESetStyle(ew->TEI, 0, 0xFF);									// zet de default style
	}

	/**** if not freshly allocated but already in existance, the text edit ****/
	/**** initing may have destroyed some values. Here they are put back   ****/

	if (!fresh)
	{
		ew->antiAliasLevel		= localEW.antiAliasLevel;
		ew->justification			= localEW.justification;
		ew->xSpacing					= localEW.xSpacing;
		ew->ySpacing					= localEW.ySpacing;
		ew->slantAmount				= localEW.slantAmount;
		ew->slantValue				= localEW.slantValue;
		ew->underLineHeight		= localEW.underLineHeight;
		ew->underLineOffset		= localEW.underLineOffset;
		ew->shadowDepth				= localEW.shadowDepth;
		ew->shadow_Pen				= localEW.shadow_Pen;
		ew->shadowType				= localEW.shadowType;
		ew->shadowDirection		= localEW.shadowDirection;
	}	

	//slot = SortEditWindowLists(slot);
	//ValidateMargins(slot);

	/**** NEW! put brightest color in text pen and darkest in shadow pen ****/
	/**** ALSO set most recently loaded font ****/

	if ( fresh )
	{
		vp = &(pageScreen->ViewPort);
		bm = pageScreen->RastPort.BitMap;
		numPageColors = UA_GetNumberOfColorsInScreen(vp->Modes, bm->Depth, CPrefs.AA_available);
	
		GetColorComponents(CPrefs.PageCM, ew->BackFillColor, (int *)&r, (int *)&g, (int *)&b);
		if ( !CPrefs.AA_available )
		{
			r |= (r<<4);
			g |= (g<<4);
			b |= (b<<4);
		}
		r2 = r;
		g2 = g;
		b2 = b;
		TurnSmallIntoLarge(&r, &g, &b);

		r = ~r;
		g = ~g;
		b = ~b;
		pen = UA_MyFindColor(	CPrefs.PageCM, r, g, b, numPageColors-1, numPageColors-1, TRUE );

		if (pen==0 || pen==ew->BackFillColor)	// avoid two the same colors
			pen++;
		if (pen>(numPageColors-1))
			pen=1;

		if ( pen != -1 )
		{
			TESetSelect(ew->TEI, 0, ew->TEI->textLength);	// selection range = hele tekst
			TESetFont(ew->TEI, textFont);									// zet het font voor de selectie
			TESetColor(ew->TEI, 2);												// zet de kleur voor de selectie
			TESetSelect(ew->TEI, 0, 0);										// herstel cursor op linksboven

			TESetFont(ew->TEI, textFont);									// zet het default font
			TESetColor(ew->TEI, 2);												// zet de default kleur
			TESetStyle(ew->TEI, 0, 0xFF);									// zet de default style

			ew->charFont	= textFont;
			ew->charColor = pen;
		}

		r = r2;	// back fill color
		g = g2;
		b = b2;

		r = 255-r;
		g = 255-g;
		b = 255-b;

		if ( r > r2 )
		{
			l = r-r2;
			l = l / 2; 
			r = r2 + l;
		}
		else
		{
			l = r2-r;
			l = l / 2; 
			r = r + l;
		}

		if ( g > g2 )
		{
			l = g-g2;
			l = l / 2; 
			g = g2 + l;
		}
		else
		{
			l = g2-g;
			l = l / 2; 
			g = g + l;
		}

		if ( b > b2 )
		{
			l = b-b2;
			l = l / 2; 
			b = b2 + l;
		}
		else
		{
			l = b2-b;
			l = l / 2; 
			b = b + l;
		}
 
		pen2 = pen;

		TurnSmallIntoLarge(&r, &g, &b);
		pen = UA_MyFindColor(	CPrefs.PageCM, r, g, b, numPageColors-1, numPageColors-1, TRUE );
		if ( pen != -1 )
			ew->shadow_Pen = pen;

		if (pen==pen2)	// avoid two the same colors
			pen++;
		if (pen>(numPageColors-1))
			pen=0;

		ew->shadow_Pen = pen;
	}

	return(slot);
}

/******** CloseEditWindow() ********/
/*
 * Don't forget to zero EditWindowList and EditSupportList !
 *
 */

void CloseEditWindow(struct EditWindow *ew, struct EditSupport *es)
{
struct EditWindow *oldEW;

	oldEW = ew;

	/**** free window bitmaps ****/

	RemovePic24FromWindow(es,&es->ori_bm);
	RemovePicFromWindow(es,&es->scaled_bm);
	RemovePicFromWindow(es,&es->remapped_bm);
	RemovePicFromWindow(es,&es->restore_bm);
	RemovePicFromWindow(es,&es->mask_bm);
	RemovePicFromWindow(es,&es->ori_mask_bm);

	/**** free iff struct ****/

	if ( es->iff )
		FreeMem(es->iff, sizeof(struct IFF_FRAME));

	/**** free colormap ****/

	if ( es->cm )
		FreeColorMap(es->cm);

	/**** free text and TEI ****/

	if ( ew->TEI )
	{
		if ( ew->TEI->text )
			FreeMem(ew->TEI->text, sizeof(struct TEChar)*TEXTEDITSIZE );
		FreeMem(ew->TEI, sizeof(struct TEInfo) );
	}

	/**** free crawl ****/

	if ( ew->crawl_text )
		FreeMem(ew->crawl_text, ew->crawl_length);

	/**** rest ****/

	if ( ew )
		FreeMem(ew, sizeof(struct EditWindow));

	if ( es )
		FreeMem(es, sizeof(struct EditSupport));
}

/******** CloseAllEditWindows() ********/
/*
 * Closes EditWindowList, Clipboard_WL and Undo_WL windows.
 *
 */

void CloseAllEditWindows(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] != NULL)
		{
			CloseEditWindow(EditWindowList[i], EditSupportList[i]);
			EditWindowList[i] = NULL;
			EditSupportList[i] = NULL;
		}
	}

	CloseAllClipboardWindows();

	CloseAllUndoWindows();
}

/******** CloseAllClipboardWindows() ********/

void CloseAllClipboardWindows(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (Clipboard_WL[i] != NULL)
		{
			CloseEditWindow(Clipboard_WL[i], Clipboard_SL[i]);
			Clipboard_WL[i] = NULL;
			Clipboard_SL[i] = NULL;
		}
	}
}

/******** CloseAllUndoWindows() ********/

void CloseAllUndoWindows(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (Undo_WL[i] != NULL)
		{
			CloseEditWindow(Undo_WL[i], Undo_SL[i]);
			Undo_WL[i] = NULL;
			Undo_SL[i] = NULL;
		}
	}
}

/******** GetNewDrawSeqNum() ********/

int GetNewDrawSeqNum(void)
{
int i, highest;

	highest=-1;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i]!=NULL)
		{
			if (EditWindowList[i]->DrawSeqNum > highest)
				highest = EditWindowList[i]->DrawSeqNum;
		}
	}
	
	if (highest==-1)
		highest=1;
	else
		highest++;

	return(highest);
}

/******** GetNewDrawSeqNum_2() ********/

int GetNewDrawSeqNum_2(struct EditWindow *ew)
{
int i, highest;

	highest=-1;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i]!=NULL && EditWindowList[i]!=ew)
		{
			if (EditWindowList[i]->DrawSeqNum > highest)
				highest = EditWindowList[i]->DrawSeqNum;
		}
	}
	
	if (highest==-1)
		highest=1;
	else
		highest++;

	return(highest);
}

/******** DetermineClickEvent() ********/

int DetermineClickEvent(int *hitWdwNr, BOOL drawHandles)
{
int i,j;
WORD x1,x2,y1,y2,mx,my;
int mouseX,mouseY;
BOOL reverse=FALSE, flag, shifted;

	*hitWdwNr=-1;

	if ( CED.Qualifier & IEQUALIFIER_CONTROL )
		return(DO_EW_OPENWDW);

	mouseX = pageWindow->MouseX;
	mouseY = MassageY(pageWindow->MouseY);

	if ( CED.Qualifier&IEQUALIFIER_LALT || CED.Qualifier&IEQUALIFIER_RALT )
		reverse=TRUE;

	if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
		shifted=TRUE;
	else
		shifted=FALSE;

	/**** first check IF a window has been hit ****/

	for(j=0; j<2; j++)
	{
		if (!reverse)
			i=MAXEDITWINDOWS-1;
		else
			i=0;

		while(1)
		{
			if ( reverse )
			{
				if ( j==0 )	// first run we only look to DE-activated windows
				{
					if ( shifted || ( EditSupportList[i] && !EditSupportList[i]->Active ) )
						flag = TRUE;
					else
						flag = FALSE;
				}
				else
					flag = TRUE;	// second run we accept all windows, also activated ones
			}
			else
			{
				if ( j==0 )	// first run we only look to activated windows
				{
					if ( EditSupportList[i] && EditSupportList[i]->Active )
						flag = TRUE;
					else
						flag = FALSE;
				}
				else
					flag = TRUE;	// second run we accept all windows, also deactivated ones
			}

			if (	flag && EditWindowList[i]!=NULL &&
						(mouseX > EditWindowList[i]->X - HANDLESNIF) &&
						(mouseX < (EditWindowList[i]->X + EditWindowList[i]->Width + HANDLESNIF)) &&
						(mouseY > EditWindowList[i]->Y - HANDLESNIF) &&
						(mouseY < (EditWindowList[i]->Y + EditWindowList[i]->Height + HANDLESNIF)) )
			{
				/**** found one: now check WHERE window has been hit ****/

				*hitWdwNr=i;

				x1 = EditWindowList[i]->X;
				y1 = EditWindowList[i]->Y;
				x2 = EditWindowList[i]->X + EditWindowList[i]->Width;
				y2 = EditWindowList[i]->Y + EditWindowList[i]->Height;
				mx = x1 + (x2-x1)/2;			
				my = y1 + (y2-y1)/2;			

				/**** if not selecting multiple, deselect all windows ****/

				if ( drawHandles && !shifted )
					DrawAllHandles(MAKE_INACTIVE);
	
				/**** if this window is inactive, make it active ****/

				if ( shifted && CED.Qualifier&IEQUALIFIER_LALT )
					;
				else
				{
					if (!EditSupportList[i]->Active)
					{
						if ( drawHandles )
						{
							DrawHandles(x1,y1,x2-1,y2-1);
							EditSupportList[i]->Active = TRUE;
						}
					}
					else
					{
						if ( drawHandles )
						{
							DrawHandles(x1,y1,x2-1,y2-1);
							EditSupportList[i]->Active = FALSE;
						}
					}
				}

				if (mouseX > x1+HANDLESNIF && mouseX < x2-HANDLESNIF &&
						mouseY > y1+HANDLESNIF && mouseY < y2-HANDLESNIF )
				{
					/**** shift-alt click deactivates window ****/

					if ( shifted && CED.Qualifier&IEQUALIFIER_LALT )
					{
						if (EditSupportList[i]->Active)
						{
							if ( drawHandles )
							{
								DrawHandles(x1,y1,x2-1,y2-1);
								EditSupportList[i]->Active = FALSE;
							}
						}
						else
						{
							if ( drawHandles )
							{
								DrawHandles(x1,y1,x2-1,y2-1);
								EditSupportList[i]->Active = TRUE;
							}
						}
					}
					return(DO_EW_SELECTED);
				}
				else if ( !(EditWindowList[i]->flags & EW_LOCKED) )
				{
					if (	mouseX > x1-HANDLESNIF && mouseX < x1+HANDLESNIF &&
								mouseY > y1-HANDLESNIF && mouseY < y1+HANDLESNIF )
						return(DO_EW_SIZE1);
					else if (	mouseX > mx-HANDLESNIF && mouseX < mx+HANDLESNIF &&
										mouseY > y1-HANDLESNIF && mouseY < y1+HANDLESNIF )
						return(DO_EW_SIZE2);
					else if ( mouseX > x2-HANDLESNIF && mouseX < x2+HANDLESNIF &&
										mouseY > y1-HANDLESNIF && mouseY < y1+HANDLESNIF )
						return(DO_EW_SIZE3);
					else if (	mouseX > x2-HANDLESNIF && mouseX < x2+HANDLESNIF &&
										mouseY > my-HANDLESNIF && mouseY < my+HANDLESNIF )
						return(DO_EW_SIZE4);
					else if (	mouseX > x2-HANDLESNIF && mouseX < x2+HANDLESNIF &&
										mouseY > y2-HANDLESNIF && mouseY < y2+HANDLESNIF )
						return(DO_EW_SIZE5);
					else if (	mouseX > mx-HANDLESNIF && mouseX < mx+HANDLESNIF &&
										mouseY > y2-HANDLESNIF && mouseY < y2+HANDLESNIF )
						return(DO_EW_SIZE6);
					else if (	mouseX > x1-HANDLESNIF && mouseX < x1+HANDLESNIF &&
										mouseY > y2-HANDLESNIF && mouseY < y2+HANDLESNIF )
						return(DO_EW_SIZE7);
					else if (	mouseX > x1-HANDLESNIF && mouseX < x1+HANDLESNIF &&
										mouseY > my-HANDLESNIF && mouseY < my+HANDLESNIF )
						return(DO_EW_SIZE8);
					else			
						return(DO_EW_DRAG);
				}
				goto openwdw;	//break;						
			}

			if (!reverse)
				i--;
			else
				i++;

			if (i<0 || i==MAXEDITWINDOWS)
				break;
		}
	}

openwdw:

	/**** nothing hit: go open edit window ****/

	return(DO_EW_OPENWDW);
}

/******** SizeEditWindow() ********/

void SizeEditWindow(int action, int lastHit)
{
WORD prevX1, prevY1, prevX2, prevY2, oriW, oriH;
BOOL loop=TRUE;
ULONG signals;
BOOL mouseMoved=FALSE;
struct IntuiMessage *message;
int restrain=0;	/* 1 is not, 2 is vertical, 3 is horizontal */
float f;
struct Window *aWin;

	/**** init vars ****/

	prevX1 = EditWindowList[lastHit]->X;
	prevY1 = EditWindowList[lastHit]->Y;
	prevX2 = prevX1 + EditWindowList[lastHit]->Width;
	prevY2 = prevY1 + EditWindowList[lastHit]->Height;
	oriW = EditWindowList[lastHit]->Width;
	oriH = EditWindowList[lastHit]->Height;

	/**** copy window to undo buffer ****/

	CopyMem(EditWindowList[lastHit],  &undoEW, sizeof(struct EditWindow));
	CopyMem(EditSupportList[lastHit], &undoES, sizeof(struct EditSupport));
	lastUndoableAction = PAGE_UNDO_RESIZE;
	lastUndoWindow=lastHit;

	/**** sizing can only be done on one window at a time						****/
	/**** so: deselect all windows, remove all handles and reselect ****/
	/**** window to be sized																				****/	

	DrawAllHandles(LEAVE_ACTIVE);

	aWin = GetActiEditWin();
	if (aWin!=NULL)
		UA_SwitchMouseMoveOn(aWin);

	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[lastHit]->X,
									EditWindowList[lastHit]->Y,
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);

	/**** size window ****/

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class			= message->Class;
				CED.Code 			= message->Code;
				CED.MouseX		= pageWindow->MouseX;
				CED.MouseY		= MassageY(pageWindow->MouseY);
				CED.Qualifier	= message->Qualifier;
				ReplyMsg((struct Message *)message);

				if (	restrain==0 &&
							(CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT) )
					restrain=2;
				else if (restrain==0)
					restrain=1;

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}
			if (mouseMoved)
			{
				if (restrain==2)
				{
					if ( action==DO_EW_SIZE1 )	/* size top-left */
					{
						if (CPrefs.PageScreenWidth > CPrefs.PageScreenHeight)
						{
							f = (float)(prevX1-CED.MouseX) * (float)oriH;
							f = f / (float)oriW;
							CED.MouseY = prevY1 - (WORD)f;
						}
						else
						{
							f = (float)(prevY1-CED.MouseY) * (float)oriW;
							f = f / (float)oriH;
							CED.MouseX = prevX1 - (WORD)f;
						}
					}
					else if (action==DO_EW_SIZE3 )	/* size top-right */
					{
						if (CPrefs.PageScreenWidth > CPrefs.PageScreenHeight)
						{
							f = (float)(CED.MouseX-prevX1) * (float)oriH;
							f = f / (float)oriW;
							CED.MouseY = prevY2 - (WORD)f - 2;
						}
						else
						{
							f = (float)(CED.MouseY-prevY1) * (float)oriW;
							f = f / (float)oriH;
							CED.MouseX = prevX2 - (WORD)f - 1;
						}
					}
					else if ( action==DO_EW_SIZE5 )	/* size bottom-right */
					{
						if (CPrefs.PageScreenWidth > CPrefs.PageScreenHeight)
						{
							f = (float)(CED.MouseX-prevX1) * (float)oriH;
							f = f / (float)oriW;
							CED.MouseY = (WORD)f + prevY1 + 1;
						}
						else
						{
							f = (float)(CED.MouseY-prevY1) * (float)oriW;
							f = f / (float)oriH;
							CED.MouseX = (WORD)f + prevX1;
						}
					}
					else if ( action==DO_EW_SIZE7 )	/* size bottom-left */
					{
						if (CPrefs.PageScreenWidth > CPrefs.PageScreenHeight)
						{
							f = (float)(prevX2 - CED.MouseX) * oriH;
							f = f / (float)oriW;
							CED.MouseY = (WORD)f + prevY1 - 1;
						}
						else
						{
							f = (float)(prevY2 - CED.MouseY) * oriW;
							f = f / (float)oriH;
							CED.MouseX = (WORD)f + prevX1;
						}
					}
					if (CED.MouseX <= 0)
						CED.MouseX = 0;
					if (CED.MouseY <= 0)
						CED.MouseY = 0;
					if (CED.MouseX >= CPrefs.PageScreenWidth)
						CED.MouseX = CPrefs.PageScreenWidth-1;
					if (CED.MouseY >= CPrefs.PageScreenHeight)
						CED.MouseY = CPrefs.PageScreenHeight-1;
				}
				SizeActiveEditWindow(action, lastHit, CED.MouseX, CED.MouseY);
			}
		}
	}

	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[lastHit]->X,
									EditWindowList[lastHit]->Y,
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);

	/**** do post-sizing stuff ****/

	if (aWin!=NULL)
		UA_SwitchMouseMoveOff(aWin);

	EditSupportList[lastHit]->Active = TRUE;

 	/**** really size the window ****/

	CorrectEW(EditWindowList[lastHit]);

	if (	prevX1!=EditWindowList[lastHit]->X || prevY1!=EditWindowList[lastHit]->Y ||
				oriW!=EditWindowList[lastHit]->Width || oriH!=EditWindowList[lastHit]->Height )
	{
		RedrawAllOverlapWdw(prevX1, prevY1, oriW, oriH,
												EditWindowList[lastHit]->X,
												EditWindowList[lastHit]->Y,
												EditWindowList[lastHit]->Width,
												EditWindowList[lastHit]->Height,lastHit,TRUE,TRUE);
	}

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** SizeActiveEditWindow() ********/

void SizeActiveEditWindow(int action, int lastHit, int mouseX, int mouseY)
{
WORD x2, y2;

	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[lastHit]->X, EditWindowList[lastHit]->Y,
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);

	switch(action)
	{
		case DO_EW_SIZE1:
			x2 = EditWindowList[lastHit]->X + EditWindowList[lastHit]->Width;
			y2 = EditWindowList[lastHit]->Y + EditWindowList[lastHit]->Height;
			if (mouseX > x2-MINWINWIDTH)
				EditWindowList[lastHit]->X = x2-MINWINWIDTH;
			else
				EditWindowList[lastHit]->X = mouseX;
			if (mouseY > y2-MINWINHEIGHT)
				EditWindowList[lastHit]->Y = y2-MINWINHEIGHT;
			else
				EditWindowList[lastHit]->Y = mouseY;
			EditWindowList[lastHit]->Width	= x2 - EditWindowList[lastHit]->X;
			EditWindowList[lastHit]->Height = y2 - EditWindowList[lastHit]->Y;
			break;

		case DO_EW_SIZE2:
			y2 = EditWindowList[lastHit]->Y + EditWindowList[lastHit]->Height;
			if (mouseY > y2-MINWINHEIGHT)
				EditWindowList[lastHit]->Y = y2-MINWINHEIGHT;
			else
				EditWindowList[lastHit]->Y = mouseY;
			EditWindowList[lastHit]->Height = y2 - EditWindowList[lastHit]->Y;
			break;

		case DO_EW_SIZE3:
			y2 = EditWindowList[lastHit]->Y + EditWindowList[lastHit]->Height;
			if (mouseX < EditWindowList[lastHit]->X+MINWINWIDTH)
				EditWindowList[lastHit]->Width = MINWINWIDTH;
			else
				EditWindowList[lastHit]->Width = mouseX-EditWindowList[lastHit]->X+1;
			if (mouseY > y2-MINWINHEIGHT)
				EditWindowList[lastHit]->Y = y2-MINWINHEIGHT;
			else
				EditWindowList[lastHit]->Y = mouseY;
			EditWindowList[lastHit]->Height = y2 - EditWindowList[lastHit]->Y;
			break;

		case DO_EW_SIZE4:
			if (mouseX < EditWindowList[lastHit]->X+MINWINWIDTH)
				EditWindowList[lastHit]->Width = MINWINWIDTH;
			else
				EditWindowList[lastHit]->Width = mouseX-EditWindowList[lastHit]->X+1;
			break;

		case DO_EW_SIZE5:
			EditWindowList[lastHit]->Width = mouseX-EditWindowList[lastHit]->X+1;
			if (EditWindowList[lastHit]->Width < MINWINWIDTH)
				EditWindowList[lastHit]->Width = MINWINWIDTH;
			EditWindowList[lastHit]->Height = mouseY-EditWindowList[lastHit]->Y+1;
			if (EditWindowList[lastHit]->Height < MINWINHEIGHT)
				EditWindowList[lastHit]->Height = MINWINHEIGHT;
			break;

		case DO_EW_SIZE6:
			EditWindowList[lastHit]->Height = mouseY-EditWindowList[lastHit]->Y+1;
			if (EditWindowList[lastHit]->Height < MINWINHEIGHT)
				EditWindowList[lastHit]->Height = MINWINHEIGHT;			
			break;

		case DO_EW_SIZE7:
			x2 = EditWindowList[lastHit]->X + EditWindowList[lastHit]->Width;
			if (mouseX > x2-MINWINWIDTH)
				EditWindowList[lastHit]->X = x2-MINWINWIDTH;
			else
				EditWindowList[lastHit]->X = mouseX;
			if (mouseY < EditWindowList[lastHit]->Y+MINWINHEIGHT)
				EditWindowList[lastHit]->Height = MINWINHEIGHT;
			else
				EditWindowList[lastHit]->Height = mouseY-EditWindowList[lastHit]->Y+1;
			EditWindowList[lastHit]->Width = x2-EditWindowList[lastHit]->X;
			break;

		case DO_EW_SIZE8:
			x2 = EditWindowList[lastHit]->X + EditWindowList[lastHit]->Width;
			if (mouseX > EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-MINWINWIDTH)
				EditWindowList[lastHit]->X = x2-MINWINWIDTH;
			else
				EditWindowList[lastHit]->X = mouseX;
			EditWindowList[lastHit]->Width = x2-EditWindowList[lastHit]->X;
			break;
	}

	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[lastHit]->X, EditWindowList[lastHit]->Y,
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);
}

/******** DragEditWindow() ********/

void DragEditWindow(int action, int lastHit)
{
WORD prevX, prevY, prevW, prevH, DiffX, DiffY, oriCX, oriCY;
BOOL loop=TRUE;
ULONG signals;
BOOL mouseMoved=FALSE;
struct IntuiMessage *message;
int restrain=0;	/* 1 is not, 2 is vertical, 3 is horizontal */
struct Window *aWin;

	/**** init vars ****/

	oriCX = pageWindow->MouseX;
	oriCY = MassageY(pageWindow->MouseY);

	DiffX = oriCX - EditWindowList[lastHit]->X;
	DiffY = oriCY - EditWindowList[lastHit]->Y;

	prevX = EditWindowList[lastHit]->X;
	prevY = EditWindowList[lastHit]->Y;
	prevW = EditWindowList[lastHit]->Width;
	prevH = EditWindowList[lastHit]->Height;

	/**** copy window to undo buffer ****/

	CopyMem(EditWindowList[lastHit],  &undoEW, sizeof(struct EditWindow));
	CopyMem(EditSupportList[lastHit], &undoES, sizeof(struct EditSupport));
	lastUndoableAction = PAGE_UNDO_MOVE;
	lastUndoWindow=lastHit;

	/**** dragging can only be done on one window at a time					****/
	/**** so: deselect all windows, remove all handles and reselect ****/
	/**** window to be dragged																			****/	

	//DrawAllHandles(LEAVE_ACTIVE);

	aWin = GetActiEditWin();
	if (aWin!=NULL)
		UA_SwitchMouseMoveOn(aWin);

	MakeMovePic(lastHit);

	RestoreBack( EditWindowList[lastHit], EditSupportList[lastHit] );

	DrawMovePic(EditWindowList[lastHit],
							EditWindowList[lastHit]->X, EditWindowList[lastHit]->Y);

	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[lastHit]->X, EditWindowList[lastHit]->Y,
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);

	/**** drag window ****/

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class			= message->Class;
				CED.Code			= message->Code;
				CED.MouseX		= pageWindow->MouseX;
				CED.MouseY		= MassageY(pageWindow->MouseY);
				CED.Qualifier	= message->Qualifier;
				ReplyMsg((struct Message *)message);

				if ( CED.Class==IDCMP_RAWKEY )
				{
					if (	CED.Code==0x60 || CED.Code==0x61 ||
								CED.Code==0xe0 || CED.Code==0xe1 ) // shift pressed down or come up
					{
						oriCX = pageWindow->MouseX;
						oriCY = MassageY(pageWindow->MouseY);
						restrain=0;
					}
				}
				else
				{
					if (	restrain==0 &&
								(CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT) )
					{
						if ( AbsWORD(oriCX, CED.MouseX) < AbsWORD(oriCY, CED.MouseY) )
							restrain=2;
						else
							restrain=3;
					}
					else if (restrain==0)
						restrain=1;
				}

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}
			if (mouseMoved)
			{
				if (restrain==2)
					CED.MouseX=oriCX;
				else if (restrain==3)
					CED.MouseY=oriCY;
				MoveEditWindow(	CED.MouseX, CED.MouseY, DiffX, DiffY,
												(struct EditWindow *)EditWindowList[lastHit],
												(struct EditSupport *)EditSupportList[lastHit] );
			}
		}
	}

	DrawMovePic(EditWindowList[lastHit],
							EditWindowList[lastHit]->X, EditWindowList[lastHit]->Y);

	DrawMarqueeBox( pageWindow->RPort,
									EditWindowList[lastHit]->X,EditWindowList[lastHit]->Y, 
									EditWindowList[lastHit]->X+EditWindowList[lastHit]->Width-1,
									EditWindowList[lastHit]->Y+EditWindowList[lastHit]->Height-1);

	/**** do post-dragging stuff ****/

	if (aWin!=NULL)
		UA_SwitchMouseMoveOff(aWin);

	EditSupportList[lastHit]->Active = TRUE;

 	/**** really move the window ****/

#if 0
	// NEW
	ValidateBoundingBox(&EditWindowList[lastHit]->X, &EditWindowList[lastHit]->Y,
											&EditWindowList[lastHit]->Width, &EditWindowList[lastHit]->Height);
#endif
	CorrectEW(EditWindowList[lastHit]);

	RedrawAllOverlapWdw(	prevX, prevY, prevW, prevH,
												EditWindowList[lastHit]->X,
												EditWindowList[lastHit]->Y,
												EditWindowList[lastHit]->Width,
												EditWindowList[lastHit]->Height, lastHit, TRUE, TRUE );

	//DrawAllHandles(LEAVE_ACTIVE);
}

/******** MoveEditWindow() ********/

void MoveEditWindow(int mouseX, int mouseY, WORD DiffX, WORD DiffY,
										struct EditWindow *ew, struct EditSupport *es)
{
	DrawMovePic(ew, ew->X, ew->Y);

	DrawMarqueeBox(pageWindow->RPort, ew->X, ew->Y, ew->X+ew->Width-1, ew->Y+ew->Height-1);

	ew->X = mouseX - DiffX;
	ew->Y = mouseY - DiffY;

	if (ew->X < 0)
		ew->X = 0;
	if (ew->Y < 0)
		ew->Y = 0;
	if ( (ew->X + ew->Width) >= CPrefs.PageScreenWidth)
		ew->X = CPrefs.PageScreenWidth-ew->Width;
	if ( (ew->Y + ew->Height) >= CPrefs.PageScreenHeight)
		ew->Y = CPrefs.PageScreenHeight-ew->Height;

	DrawMovePic(ew, ew->X, ew->Y);

	DrawMarqueeBox(pageWindow->RPort, ew->X, ew->Y, ew->X+ew->Width-1, ew->Y+ew->Height-1);
}

/******** GetActiEditWin() ********/

struct Window *GetActiEditWin(void)
{
	Forbid();

	if ( pageWindow->Flags & WFLG_WINDOWACTIVE )
	{
		Permit();
		return(pageWindow);
	}

	Permit();

	return(NULL);
}

/******** DrawAllWindows() ********/

void DrawAllWindows(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] != NULL )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);
}

/******** SortEditWindowLists() ********/

int SortEditWindowLists(int slot)
{
struct EditWindow *EWL[ MAXEDITWINDOWS ], *slotEW;
struct EditSupport *ESL[ MAXEDITWINDOWS ];
int i,j,dsn,newSlot,pos;

	newSlot = slot;

	slotEW = EditWindowList[slot];

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		EWL[i] = NULL;
		ESL[i] = NULL;
	}

	dsn=1;
	pos=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		for(j=0; j<MAXEDITWINDOWS; j++)
		{
			if ( EditWindowList[j]!=NULL && EditWindowList[j]->DrawSeqNum==dsn )
			{		
				EWL[ pos ]	= EditWindowList[j];
				ESL[ pos ]	= EditSupportList[j];
				pos++;
			}
		}
		dsn++;
	}				

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		EditWindowList[i] = NULL;
		EditSupportList[i] = NULL;
	}

	dsn = 1;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EWL[i] != NULL )
		{
			EditWindowList[i]	= EWL[i]; 
			EditWindowList[i]->DrawSeqNum = dsn;
			dsn++;
			EditSupportList[i] = ESL[i];
			if ( slotEW == EditWindowList[i] )
				newSlot = i;
		}
	}

	return(newSlot);
}

/******** RemovePicFromWindow() ********/
/*
 * ONLY if picture is nowhere else used, remove it.
 *
 */

void RemovePicFromWindow(struct EditSupport *es, struct BitMap *bm)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		/**** EditSupportList ****/

		if ( EditSupportList[i] && EditSupportList[i] != es )
		{
			if ( EditSupportList[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}

		/**** Clipboard_SL ****/

		if ( Clipboard_SL[i] && Clipboard_SL[i] != es )
		{
			if ( Clipboard_SL[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}

		/**** Undo_SL ****/

		if ( Undo_SL[i] && Undo_SL[i] != es )
		{
			if ( Undo_SL[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}
	}

	/**** no other users found ****/

	if ( bm->Planes[0] )
	{
		WaitBlit();	// just in case that old lazy blitter is still chewing...
		//FreeMem(bm->Planes[0], RASSIZE(bm->BytesPerRow*8,bm->Rows)*bm->Depth);
		FreeFastBitMap( bm );
	}

	ClearBitMap(bm);
}

/******** RemovePic24FromWindow() ********/
/*
 * ONLY if picture is nowhere else used, remove it.
 *
 */

void RemovePic24FromWindow(struct EditSupport *es, struct BitMap24 *bm)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		/**** EditSupportList ****/

		if ( EditSupportList[i] && EditSupportList[i] != es )
		{
			if ( EditSupportList[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( EditSupportList[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}

		/**** Clipboard_SL ****/

		if ( Clipboard_SL[i] && Clipboard_SL[i] != es )
		{
			if ( Clipboard_SL[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Clipboard_SL[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}

		/**** Undo_SL ****/

		if ( Undo_SL[i] && Undo_SL[i] != es )
		{
			if ( Undo_SL[i]->ori_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->scaled_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->remapped_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use

			if ( Undo_SL[i]->ori_mask_bm.Planes[0] == bm->Planes[0] )
				return;	// pic in use
		}
	}

	/**** no other users found ****/

	if ( bm->Planes[0] )
	{
		WaitBlit();	// just in case that old lazy blitter is still chewing...
		FreeFastBitMap24(bm);
	}

	ClearBitMap24(bm);
}

/******** E O F ********/
