#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditSupport **Clipboard_SL;
extern struct EditSupport **Undo_SL;
extern struct Screen **DA_Screens;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct ColorMap *undoCM;
extern int lastUndoableAction;

/**** functions ****/

/******** ImportAScreen() ********/

void ImportAScreen(BOOL resizeIt, int remapIt, struct Screen *inScreen)
{
int i, slot;
BOOL palettesEqual;
struct Screen *grabScreen;

	if ( !inScreen )
	{
		/**** show screens to grab ****/

		i = ShowThumbScreens();
		if ( i == -1 )
			return;

		grabScreen = DA_Screens[i];
		if ( grabScreen==NULL )
			return; // s'thing went wrong
	}
	else
		grabScreen = inScreen;

	/**** find selected window or open new one ****/

	i = FirstActiveEditWindow();
	if (i==-1)	// no window selected, open one for our dearly beloved...
	{
		i = SearchEmptyWindow();
		if (i!=-1)
			i = OpenEditWindow(i,0,0,1,1);
	}
	if (i==-1)
		return;

	slot = i;

	/**** grab screen ****/

	if ( !PutScreenIntoWdw(grabScreen, slot, resizeIt, remapIt) )
	{
		// remove info from wdw that makes it a photo-carrying window
		EditSupportList[slot]->photoOpts = 0;
		EditSupportList[slot]->picPath[0] = '\0';
		EditWindowList[slot]->BackFillType = 0; // SOLID

		return;
	}

	/**** process colors ****/

	if (	EditSupportList[slot]->cm &&
				CompareCM(EditSupportList[slot]->cm, CPrefs.PageCM) )
		palettesEqual=TRUE;
	else
		palettesEqual=FALSE;

	if ( !palettesEqual && !remapIt )
	{
		if ( UA_OpenGenericWindow(	pageWindow, TRUE,TRUE, msgs[Msg_OK-1],msgs[Msg_Cancel-1],
																QUESTION_ICON, msgs[Msg_SpecialsUsePalette-1], TRUE, NULL ) )
		{
			if ( EditSupportList[slot]->cm )
				SetScreenToCM(pageScreen, EditSupportList[slot]->cm);
			lastUndoableAction=PAGE_UNDO_IMPORT;
		}
		else
			EditSupportList[slot]->photoOpts |= REMAP_PHOTO;
	}

	SyncAllColors(TRUE);

	/**** redraw windows ****/

	RedrawAllOverlapWdwEasy(slot,TRUE,TRUE);
}

/******** PutScreenIntoWdw() ********/

BOOL PutScreenIntoWdw(struct Screen *screen, int slot, BOOL resizeIt, int remapIt)
{
struct EditWindow *ew;
struct EditSupport *es;
UWORD x,y,w,h,px,py,pw,ph,ww,hh;
LONG needed, screen_size;
int numColors;

	ew = EditWindowList[slot];
	es = EditSupportList[slot];

	/**** store additional info ****/

	es->Active = TRUE;
	if (resizeIt)
		es->photoOpts = SIZE_PHOTO;
	else
		es->photoOpts = MOVE_PHOTO;
	es->picPath[0] = '\0';
	if (remapIt)
		es->photoOpts	|= REMAP_PHOTO;

	ew->PhotoOffsetX = 0;
	ew->PhotoOffsetY = 0;
	ew->BackFillType = 2; // TRANSP

	/**** grab screen ****/

	RemovePic24FromWindow(es,&es->ori_bm);
	RemovePicFromWindow(es,&es->scaled_bm);
	RemovePicFromWindow(es,&es->remapped_bm);
	RemovePicFromWindow(es,&es->mask_bm);
	RemovePicFromWindow(es,&es->ori_mask_bm);

	ClearBitMap24(&es->ori_bm);
	es->ori_w = 0;
	es->ori_h = 0;

	ClearBitMap(&es->scaled_bm);
	es->scaled_w = 0;
	es->scaled_h = 0;

	ClearBitMap(&es->remapped_bm);
	es->remapped_w = 0;
	es->remapped_h = 0;

	ClearBitMap(&es->mask_bm);
	es->mask_w = 0;
	es->mask_h = 0;

	ClearBitMap(&es->ori_mask_bm);
	es->ori_mask_w = 0;
	es->ori_mask_h = 0;

	/**** free colormap ****/

	if ( es->cm!=NULL )
		FreeColorMap(es->cm);
	es->cm = NULL;

	/**** alloc iff struct (if not yet done) ****/

	if ( es->iff == NULL )
	{
		es->iff = (struct IFF_FRAME *)AllocMem(	(LONG)sizeof(struct IFF_FRAME),
																						MEMF_ANY | MEMF_CLEAR);
		if (es->iff==NULL)
		{
			UA_WarnUser(144);
			return(FALSE);
		}
	}

	/**** make a fake iff struct ****/

	es->iff->BMH.w					= screen->Width;
	es->iff->BMH.h					= screen->Height;
	es->iff->BMH.x					= 0;
	es->iff->BMH.y					= 0;
	es->iff->BMH.nPlanes		= screen->RastPort.BitMap->Depth;
	es->iff->BMH.pageWidth	= CPrefs.PageScreenWidth;
	es->iff->BMH.pageHeight	= CPrefs.PageScreenHeight;
	es->iff->viewModes			= screen->ViewPort.Modes;

	/**** CHECK IF THIS PICTURE'S GONNA FIT ****/

	needed = RASSIZE(es->iff->BMH.w, es->iff->BMH.h) * es->iff->BMH.nPlanes;
	screen_size = RASSIZE(CPrefs.PageScreenWidth,CPrefs.PageScreenHeight)*
								CPrefs.PageScreenDepth;
	screen_size *= 2;

	if ( (AvailMem(MEMF_LARGEST)-needed) < screen_size )
	{
		CloseAllUndoWindows();
		CloseAllClipboardWindows();

		// second attempt

		if ( (AvailMem(MEMF_LARGEST)-needed) < screen_size )
		{
			Message(msgs[Msg_GraphicsMemError-1]);
			return(FALSE);
		}
	}

	/**** copy colormap ****/

	numColors = UA_GetNumberOfColorsInScreen(	(ULONG)es->iff->viewModes,
																						es->iff->BMH.nPlanes, CPrefs.AA_available);
	es->cm = GetColorMap(numColors);
	if (es->cm==NULL)
	{
		Message(msgs[Msg_NotEnoughMemory-1]);
		return(FALSE);
	}
	CopyCMtoCM(screen->ViewPort.ColorMap, es->cm);

	/**** create ori_bm ****/

	if ( !InitAndAllocFastBitMap24(&es->ori_bm, es->iff->BMH.nPlanes, es->iff->BMH.w, es->iff->BMH.h) )
	{
		UA_WarnUser(146);
		return(FALSE);
	}
	es->ori_w = es->iff->BMH.w;
	es->ori_h = es->iff->BMH.h;

	/**** copy bitmap (to fast mem) ****/

	BltBitMapFM(screen->RastPort.BitMap, 0,0, (struct BitMap *)&es->ori_bm, 0,0,
							es->iff->BMH.w, es->iff->BMH.h, 0xc0, 0xff, NULL);

	/**** conform picture to window size ****/

	ww = CPrefs.PageScreenWidth / 5;
	hh = CPrefs.PageScreenHeight / (MAXEDITWINDOWS/5);	

	if (ew->Width==2 && ew->Height==2)	// fresh window
	{
		px = ew->X;
		py = ew->Y;
		pw = ew->Width;
		ph = ew->Height;

		x = ew->X;
		y = ew->Y;
		w = ww;
		h = hh;

		ValidateBoundingBox(&x, &y, &w, &h);

		ew->X				= x;
		ew->Y				= y;
		ew->Width		= w;
		ew->Height	= h;

		CorrectEW(ew);
	}

	return(TRUE);
}

/******** E O F ********/
