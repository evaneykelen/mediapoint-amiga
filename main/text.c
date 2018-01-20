#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Screen **DA_Screens;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct BitMap sharedBM;
extern struct RastPort sharedRP;
extern struct MenuRecord **page_MR;
extern struct Screen *pageScreen;

/**** static globals ****/

static struct BitMap textUndoBM;
static struct RastPort textUndoRP;

/**** functions ****/

/******** ImportAText() ********/

void ImportAText(void)
{
BOOL retval;
TEXT filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
struct EditWindow *ew;
struct EditSupport *es;
int i, textLength, wdwNr;
	
	/**** show file requester ****/

	retval = OpenAFile(	CPrefs.import_text_Path, filename,
											msgs[Msg_SelectATextFile-1], pageWindow,
											DIR_OPT_ALL | DIR_OPT_NOINFO, FALSE);
	if (!retval)
		return;

	UA_SetSprite(pageWindow,SPRITE_BUSY);

	UA_MakeFullPath(CPrefs.import_text_Path, filename, fullPath);

	/**** find window ****/

	i = FirstActiveEditWindow();
	if (i==-1)	// no window selected, open one for our dearly beloved...
	{
		i = SearchEmptyWindow();
		if (i!=-1)
			i = OpenEditWindow(i, 0, 0, pageWindow->Width-1, pageWindow->Height-1);
	}
	if (i==-1)
	{
		UA_SetSprite(pageWindow,SPRITE_NORMAL);
		return;
	}
	ew = EditWindowList[i];
	es = EditSupportList[i];
	wdwNr = i;

	/**** parse text file ****/

	if ( ParseText(CPrefs.import_text_Path, filename, ew, es, &textLength) )
		ew->TEI->textLength = textLength;

	TESetSelect(ew->TEI, 0, 0);						// herstel cursor op linksboven
	TESetUpdateRange( ew, LEVEL_FULL );
	RedrawAllOverlapWdwEasy(wdwNr,TRUE,TRUE);
	es->Active = TRUE;	// NEW, import.c draws borders again
	UA_SetSprite(pageWindow,SPRITE_NORMAL);
}

/******** ProcessTextEdit() ********/

BOOL ProcessTextEdit(int hitWdw)
{
struct EditWindow *ew;
struct EditSupport *es;
int i,overlaps,add;
struct BitMap fakeBM;
struct RastPort fakeRP;
struct TEInfo *TEI;
WORD oldx,oldy;

	//Message("Sorry, no text editting today.");
	//return(FALSE);

	//PaletteToBack();

	UA_SetSprite(pageWindow,SPRITE_BUSY);

	/**** set menus ****/

	SetPageEditMenuItems();
	EnableMenu(page_MR[FONT_MENU], FONT_TYPE);
	//EnableMenu(page_MR[FONT_MENU], FONT_STYLE);
	EnableMenu(page_MR[FONT_MENU], FONT_COLOR);
	EnableMenu(page_MR[FONT_MENU], FONT_PLAIN);
	EnableMenu(page_MR[FONT_MENU], FONT_BOLD);
	EnableMenu(page_MR[FONT_MENU], FONT_ITALIC);
	EnableMenu(page_MR[FONT_MENU], FONT_UNDERLINE);

	ew = EditWindowList[hitWdw];
	es = EditSupportList[hitWdw];

	/**** draw border outside window ****/

	DrawAllHandles(LEAVE_ACTIVE);
	DrawSafeBox(pageWindow->RPort, ew->X-1, ew->Y-1, ew->Width+2, ew->Height+2);

	/**** wipe window ****/

	overlaps=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditWindowList[i] != ew )
		{
			if (	EditWindowList[i]->DrawSeqNum > ew->DrawSeqNum &&
						BoxInsideBox( EditWindowList[i], ew ) )
				overlaps++;
		}
	}

	if ( ew->TEI->textLength==0 && overlaps==0 )	// first time case (no text yet)
		;	// do nothing
	else
	{
		// If there's a picture then render the window again without text

		if ( overlaps!=0 || es->photoOpts & SIZE_PHOTO || es->photoOpts & MOVE_PHOTO )
		{
			RestoreBack(ew,es);
			TEI = ew->TEI;
			ew->TEI = NULL;
			DrawEditWindow(ew,es);
			ew->TEI = TEI;
		}
	}

	/**** copy window WITHOUT text to undo buffer ****/

	if ( ew->Height+(ew->Y%8) > sharedBM.Rows )
		add = 0;
	else
		add = ew->Y%8;

	InitBitMap(&fakeBM,CPrefs.PageScreenDepth,ew->Width+ew->X%16,ew->Height+add);
	for(i=0; i<8; i++)
		fakeBM.Planes[i] = sharedBM.Planes[i];
	InitRastPort(&fakeRP);
	fakeRP.BitMap=&fakeBM;

	ew->undoBM = &fakeBM;

	// If there's a picture then copy the window without text to the shared BM

	if ( overlaps!=0 || es->photoOpts & SIZE_PHOTO || es->photoOpts & MOVE_PHOTO )
	{
		ClipBlit(pageWindow->RPort,ew->X,ew->Y,&fakeRP,0,0,ew->Width,ew->Height,0xc0);
		WaitBlit();
	}
	else
	{
		// If there's NO picture then render the window again directly in the shared BM

		// Restore background in off screen area

		if ( es->restore_bm.Planes[0] )
			BltBitMapFM((struct BitMap *)&es->restore_bm,
									0, 0,
									&fakeBM,
									0, add, es->restore_w, es->restore_h,
									0xc0, 0xff, NULL);

		// Render window in off screen area

		oldx = ew->X;
		oldy = ew->Y;
		ew->X = ew->X%16;
		ew->Y = add;
		RenderWindowIandB(&fakeRP,ew);
		ew->X = oldx;
		ew->Y = oldy;

		// Shift the bitmap in off screen area to the right position

		ScrollRaster(&fakeRP, ew->X%16,add, 0,0,ew->Width-1+ew->X%16,ew->Height-1+add);

//BltBitMap(&fakeBM,0,0,pageScreen->RastPort.BitMap,0,0,ew->Width,ew->Height,0xc0,0xff,NULL);
//WaitBlit();
	}

	/**** show text again ****/

	if ( overlaps!=0 || es->photoOpts & SIZE_PHOTO || es->photoOpts & MOVE_PHOTO )
		UpdateWindowText( ew );

	/**** edit text ****/

	UA_SetSprite(pageWindow,SPRITE_NORMAL);

	TextEdit( ew ); 
	ew->undoBM = NULL;

	UA_SetSprite(pageWindow,SPRITE_BUSY);

	/**** redraw windows ****/

	DrawSafeBox(pageWindow->RPort, ew->X-1, ew->Y-1, ew->Width+2, ew->Height+2);

	if ( overlaps!=0 )
		RedrawAllOverlapWdwEasy(hitWdw,TRUE,TRUE);

	DrawAllHandles(LEAVE_ACTIVE);

	/**** set menus ****/

	DisableMenu(page_MR[FONT_MENU], FONT_TYPE);
	//DisableMenu(page_MR[FONT_MENU], FONT_STYLE);
	DisableMenu(page_MR[FONT_MENU], FONT_COLOR);
	DisableMenu(page_MR[FONT_MENU], FONT_PLAIN);
	DisableMenu(page_MR[FONT_MENU], FONT_BOLD);
	DisableMenu(page_MR[FONT_MENU], FONT_ITALIC);
	DisableMenu(page_MR[FONT_MENU], FONT_UNDERLINE);

	UA_SetSprite(pageWindow,SPRITE_NORMAL);

	return(TRUE);
}

/******** DrawSafeBox() ********/

void DrawSafeBox(struct RastPort *rp, WORD x, WORD y, WORD w, WORD h)
{
int x1,y1,x2,y2;

	SetAPen(rp, 1L);
	SetDrMd(rp, JAM2|COMPLEMENT);
	SafeSetWriteMask(rp, 0x3);

	/**** draw left line ****/

	if (x >= 0)
	{
		y1 = y;
		if ( y1<0 )
			y1=0;
		y2 = y+h-1;
		if ( y2 > (CPrefs.PageScreenHeight-1) )
			y2=(CPrefs.PageScreenHeight-1);
		Move(rp,x,y1); Draw(rp,x,y2);
	}

	/**** draw right line ****/

	if ( x+w-1 < CPrefs.PageScreenWidth )
	{
		y1 = y;
		if ( y1<0 )
			y1=0;
		y2 = y+h-1;
		if ( y2 > (CPrefs.PageScreenHeight-1) )
			y2=CPrefs.PageScreenHeight-1;
		Move(rp,x+w-1,y1); Draw(rp,x+w-1,y2);
	}

	/**** draw top line ****/

	if ( y >= 0 )
	{
		x1 = x;
		if ( x1<0 )
			x1=0;
		x2 = x+w-1;
		if ( x2 > (CPrefs.PageScreenWidth-1) )
			x2=CPrefs.PageScreenWidth-1;
		Move(rp,x1,y); Draw(rp,x2,y);
	}

	/**** draw bottom line ****/

	if ( y+h-1 < CPrefs.PageScreenHeight )
	{
		x1 = x;
		if ( x1<0 )
			x1=0;
		x2 = x+w-1;
		if ( x2 > (CPrefs.PageScreenWidth-1) )
			x2=CPrefs.PageScreenWidth-1;
		Move(rp,x1,y+h-1); Draw(rp,x2,y+h-1);
	}

	SetDrMd(rp, JAM2);
	SafeSetWriteMask(rp, 0xff);
}

/******** E O F ********/
