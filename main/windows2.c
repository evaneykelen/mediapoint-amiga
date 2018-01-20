#include "nb:pre.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditWindow **Clipboard_WL;
extern struct EditSupport **Clipboard_SL;
extern struct EditWindow **Undo_WL;
extern struct EditSupport **Undo_SL;
extern int lastUndoableAction;
extern int lastUndoWindow;
extern struct CapsPrefs CPrefs;
extern struct TextFont *largeFont;
extern struct EditWindow undoEW;
extern struct EditSupport undoES;
extern struct ColorMap *undoCM;
extern struct Screen *pageScreen;
extern WORD undoOffsetX, undoOffsetY;

/**** functions ****/

/******** NumActiveEditWindows() ********/

int NumActiveEditWindows(void)
{
int i,num=0;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditSupportList[i] && EditSupportList[i]->Active )
			num++;

	return(num);
}

/******** NumEditWindows() ********/

int NumEditWindows(void)
{
int i,num=0;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditSupportList[i] )
			num++;

	return(num);
}

/******** FirstActiveEditWindow() ********/

int FirstActiveEditWindow(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditSupportList[i] && EditSupportList[i]->Active )
			return(i);

	return(-1);
}

/******** CutActiveWindows() ********/
/*
 * - close all clipboard windows
 * - copy pointers of active edit windows to clipboard
 * - close all physical windows
 *
 */

void CutActiveWindows(void)
{
int i,j,wdw,num,dsn;
//BOOL list[MAXEDITWINDOWS];

	DrawAllHandles(LEAVE_ACTIVE);

	/**** first close all current clipboard windows ****/

	CloseAllClipboardWindows();

	/**** remove all windows from page screen ****/

	num = NumActiveEditWindows();
	wdw = FirstActiveEditWindow();
	dsn = EditWindowList[wdw]->DrawSeqNum;

	if (num==1)	// optimized redraw
	{
		RedrawAllOverlapWdw(EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height,
												EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height,wdw,FALSE,TRUE);
	}
	else
	{
		for(i=MAXEDITWINDOWS-1; i>=wdw; i--)
		{
			if ( EditSupportList[i] )
			{
				RestoreBack(EditWindowList[i], EditSupportList[i]);
				if ( EditSupportList[i]->restore_bm.Planes[0] )
				{
					FreeFastBitMap( &EditSupportList[i]->restore_bm );
					EditSupportList[i]->restore_w = 0;
					EditSupportList[i]->restore_h = 0;
				}
			}
		}
	}

	/**** I redirect the pointers to the active windows to the clipboard	****/
	/**** thereby bypassing the need to free the editwindow memory. Only	****/
	/**** the physical windows need to be closed.													****/

	j=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditSupportList[i] != NULL && EditSupportList[i]->Active)
		{
			/**** copy the pointers ****/

			Clipboard_WL[j]	= EditWindowList[i];
			Clipboard_SL[j]	= EditSupportList[i];
			j++;

			/**** zero the free editwindow entry ****/

			EditWindowList[i] = NULL;
			EditSupportList[i] = NULL;
		}
	}

	if ( num>1 )
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] && EditWindowList[i]->DrawSeqNum >= dsn )
				DrawEditWindow(EditWindowList[i],EditSupportList[i]);
	}

	SortEditWindowLists(0);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** CopyActiveWindows() ********/
/*
 * - close all clipboard windows
 * - allocate memory in clipboard to hold all active windows
 * - copy active edit window structures to the clipboard
 *
 */

void CopyActiveWindows(void)
{
int i,j;

	CloseAllClipboardWindows();

	j=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditSupportList[i] && EditSupportList[i]->Active )
		{
			CopyAWindow(i,j,FALSE);
			j++;
		}
	}
}

/******** CopyAWindow() ********/

void CopyAWindow(int wdw, int to_wdw, BOOL closeClipboard)
{
int numColors;

	/**** first close all current clipboard windows ****/

	if ( closeClipboard )
		CloseAllClipboardWindows();

	/**** allocate memory for window ****/

	Clipboard_WL[to_wdw] = (struct EditWindow *)AllocMem(sizeof(struct EditWindow), MEMF_ANY | MEMF_CLEAR);
	if ( !Clipboard_WL[to_wdw] )
	{
		UA_WarnUser(136);
		return;
	}

	Clipboard_SL[to_wdw] = (struct EditSupport *)AllocMem(sizeof(struct EditSupport), MEMF_ANY | MEMF_CLEAR);
	if ( !Clipboard_SL[to_wdw] )
	{
		UA_WarnUser(137);
		return;
	}

	/**** copy structures ****/

	if ( EditSupportList[wdw] && EditSupportList[wdw]->Active )
	{
		CopyMem(EditWindowList[wdw],  Clipboard_WL[to_wdw], sizeof(struct EditWindow));
		CopyMem(EditSupportList[wdw], Clipboard_SL[to_wdw], sizeof(struct EditSupport));

		ClearBitMap(&Clipboard_SL[to_wdw]->scaled_bm);		// don't share this one
		ClearBitMap(&Clipboard_SL[to_wdw]->remapped_bm);	// don't share this one
		ClearBitMap(&Clipboard_SL[to_wdw]->restore_bm);		// don't share this one
		ClearBitMap(&Clipboard_SL[to_wdw]->mask_bm);			// don't share this one

		if ( EditSupportList[wdw]->iff )
		{
			Clipboard_SL[to_wdw]->iff = (struct IFF_FRAME *)
											AllocMem((LONG)sizeof(struct IFF_FRAME), MEMF_ANY | MEMF_CLEAR);
			if ( !Clipboard_SL[to_wdw]->iff )
				UA_WarnUser(140);
			else
				CopyMem(EditSupportList[wdw]->iff, Clipboard_SL[to_wdw]->iff, sizeof(struct IFF_FRAME));
		}

		if ( EditSupportList[wdw]->cm )
		{
			/**** give this window its own color map ****/
			numColors = EditSupportList[wdw]->cm->Count;
			if ( numColors>0 )
			{
 				Clipboard_SL[to_wdw]->cm = GetColorMap(numColors);
				if ( Clipboard_SL[to_wdw]->cm )
					CopyCMtoCM(EditSupportList[wdw]->cm, Clipboard_SL[to_wdw]->cm);
			}
		}

		AddTEI(Clipboard_WL[to_wdw], EditWindowList[wdw]);

		AddCrawl(Clipboard_WL[to_wdw], EditWindowList[wdw]);
	}
}

/******** PasteActiveWindows() ********/
/*
 * - search empty slot in edit window list
 * - allocate memory for windows to paste
 * - alter left-corner and de-activate
 * - open physical window
 *
 */

void PasteActiveWindows(WORD x, WORD y)
{
int i, slot, numColors;
BOOL list[MAXEDITWINDOWS];

	for(i=0; i<MAXEDITWINDOWS; i++)
		list[i] = FALSE;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (Clipboard_WL[i] != NULL)
		{
			/**** search a place for window to paste ****/

			slot = SearchEmptyWindow();
			if (slot==-1)
				return; /* no more windows can be opened: not a s erious error */

			/**** allocate memory for window to paste ****/ 

			EditWindowList[slot] = (struct EditWindow *)AllocMem(sizeof(struct EditWindow), MEMF_ANY | MEMF_CLEAR);
			if (EditWindowList[slot] == NULL)
			{
				UA_WarnUser(138);
				return;
			}

			EditSupportList[slot] = (struct EditSupport *)AllocMem(sizeof(struct EditSupport), MEMF_ANY | MEMF_CLEAR);
			if (EditSupportList[slot] == NULL)
			{
				UA_WarnUser(139);
				return;
			}

			/**** copy structures ****/

			CopyMem(Clipboard_WL[i], EditWindowList[slot],  sizeof(struct EditWindow));
			CopyMem(Clipboard_SL[i], EditSupportList[slot], sizeof(struct EditSupport));

			/**** alter some parameters ****/

			ClearBitMap(&EditSupportList[slot]->scaled_bm);		// don't share this one
			ClearBitMap(&EditSupportList[slot]->remapped_bm);	// don't share this one
			ClearBitMap(&EditSupportList[slot]->restore_bm);	// don't share this one
			ClearBitMap(&EditSupportList[slot]->mask_bm);			// don't share this one

			EditWindowList[slot]->X = x;
			EditWindowList[slot]->Y = y;
			EditWindowList[slot]->DrawSeqNum = GetNewDrawSeqNum_2(EditWindowList[slot]);

			EditSupportList[slot]->Active = TRUE;

			/**** allocate (if needed) new bm and iff structures ****/

			if (EditSupportList[slot]->iff )	// pasted wdw carries a picture
			{
				EditSupportList[slot]->iff = (struct IFF_FRAME *)
						AllocMem((LONG)sizeof(struct IFF_FRAME), MEMF_ANY | MEMF_CLEAR);
				if ( !EditSupportList[slot]->iff )
					UA_WarnUser(140);
				else
					CopyMem(Clipboard_SL[i]->iff, EditSupportList[slot]->iff, sizeof(struct IFF_FRAME));
			}

			if (EditSupportList[slot]->cm != NULL)
			{
				/**** give this window its own color map ****/
				numColors = Clipboard_SL[i]->cm->Count;
				if (numColors>0)
				{
	 				EditSupportList[slot]->cm = GetColorMap(numColors);
					if ( EditSupportList[slot]->cm )
						CopyCMtoCM(Clipboard_SL[i]->cm, EditSupportList[slot]->cm);
				}
			}

			/**** open physical window ****/

/*
			ValidateBoundingBox(&EditWindowList[slot]->X,
													&EditWindowList[slot]->Y,
													&EditWindowList[slot]->Width,
													&EditWindowList[slot]->Height);
*/
			CorrectEW(EditWindowList[slot]);

			AddTEI(EditWindowList[slot],Clipboard_WL[i]);

			AddCrawl(EditWindowList[slot],Clipboard_WL[i]);

			slot = OpenEditWindow(slot, 0,0,0,0);

			//DrawEditWindow(EditWindowList[slot], EditSupportList[slot]);

			list[slot] = TRUE;
		}
	}

	//DoRedrawAllEasy( list );
	RedrawAllOverlapWdwList(list);
}					

/******** SelectAllWindows() ********/

void SelectAllWindows(void)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] != NULL && !EditSupportList[i]->Active )
		{
			DrawHandles(EditWindowList[i]->X,
									EditWindowList[i]->Y,
									EditWindowList[i]->X + EditWindowList[i]->Width - 1,
									EditWindowList[i]->Y + EditWindowList[i]->Height - 1);
			EditSupportList[i]->Active = TRUE;
		}
	}
}

/******** ClearActiveWindows() ********/
/*
 * - close all undo windows
 * - copy pointers of active edit windows to undo list
 * - close all physical windows
 *
 */

void ClearActiveWindows(void)
{
int i,j,wdw,num,dsn;
//BOOL list[MAXEDITWINDOWS];

	DrawAllHandles(LEAVE_ACTIVE);

	/**** first close all current undo windows ****/

	CloseAllUndoWindows();

	/**** remove all windows from page screen ****/

	num = NumActiveEditWindows();
	wdw = FirstActiveEditWindow();
	dsn = EditWindowList[wdw]->DrawSeqNum;

	if (num==1)	// optimized redraw
	{
		RedrawAllOverlapWdw(EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height,
												EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height,wdw,FALSE,TRUE);
	}
	else
	{
		for(i=MAXEDITWINDOWS-1; i>=wdw; i--)
		{
			if ( EditSupportList[i] )
			{
				RestoreBack(EditWindowList[i], EditSupportList[i]);
				if ( EditSupportList[i]->restore_bm.Planes[0] )
				{
					FreeFastBitMap( &EditSupportList[i]->restore_bm );
					EditSupportList[i]->restore_w = 0;
					EditSupportList[i]->restore_h = 0;
				}
			}
		}
	}

	/**** I redirect the pointers to the active windows to the undo list	****/
	/**** thereby bypassing the need to free the editwindow memory. Only	****/
	/**** the physical windows need to be closed.													****/

	j=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditSupportList[i] != NULL && EditSupportList[i]->Active)
		{
			lastUndoableAction = PAGE_UNDO_CLEAR;

			/**** copy the pointers ****/

			Undo_WL[j] = EditWindowList[i];
			Undo_SL[j] = EditSupportList[i];
			j++;

			/**** zero the free editwindow entry ****/

			EditWindowList[i] = NULL;
			EditSupportList[i] = NULL;
		}
	}

	if ( num>1 )
	{
/*
		for(i=0; i<MAXEDITWINDOWS; i++)
			list[i] = FALSE;

		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] && EditWindowList[i]->DrawSeqNum >= dsn )
				list[i] = FALSE;

		RedrawAllOverlapWdwList(list);
*/
		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] && EditWindowList[i]->DrawSeqNum >= dsn )
				DrawEditWindow(EditWindowList[i],EditSupportList[i]);
	}

	SortEditWindowLists(0);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** UndoEditWindowModification() ********/

void UndoEditWindowModification(void)
{
	if ( lastUndoableAction==PAGE_UNDO_RESIZE || lastUndoableAction==PAGE_UNDO_MOVE )
	{
		DrawAllHandles(LEAVE_ACTIVE);

		//if ( lastUndoableAction==PAGE_UNDO_RESIZE )
		//	ValidateMargins( lastUndoWindow );

		lastUndoableAction = 0;

		RedrawAllOverlapWdw(EditWindowList[lastUndoWindow]->X,
												EditWindowList[lastUndoWindow]->Y,
												EditWindowList[lastUndoWindow]->Width,
												EditWindowList[lastUndoWindow]->Height,
												undoEW.X, undoEW.Y, undoEW.Width, undoEW.Height,
												lastUndoWindow, TRUE,TRUE);

		DrawAllHandles(LEAVE_ACTIVE);
	}
	else if (lastUndoableAction==PAGE_UNDO_CLEAR)
	{
		lastUndoableAction = 0;
		PasteUndoWindows();
	}
	else if (lastUndoableAction==PAGE_UNDO_PICMOVE)
	{
		lastUndoableAction = 0;

		DrawAllHandles(LEAVE_ACTIVE);

		EditWindowList[lastUndoWindow]->PhotoOffsetX = undoOffsetX;
		EditWindowList[lastUndoWindow]->PhotoOffsetY = undoOffsetY;

		RedrawAllOverlapWdw(EditWindowList[lastUndoWindow]->X,
												EditWindowList[lastUndoWindow]->Y,
												EditWindowList[lastUndoWindow]->Width,
												EditWindowList[lastUndoWindow]->Height,
												EditWindowList[lastUndoWindow]->X,
												EditWindowList[lastUndoWindow]->Y,
												EditWindowList[lastUndoWindow]->Width,
												EditWindowList[lastUndoWindow]->Height,
												lastUndoWindow,TRUE,TRUE);

		DrawAllHandles(LEAVE_ACTIVE);
	}
	else if (lastUndoableAction==PAGE_UNDO_IMPORT)
	{
		lastUndoableAction = 0;
		CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
		SetScreenToCM(pageScreen,pageScreen->ViewPort.ColorMap);
		SyncAllColors(TRUE);
	}

	SetPageEditMenuItems();
}

/******** PasteUndoWindows() ********/
/*
 * - search empty slot in edit window list
 * - copy undo pointers to empty slots
 * - open physical window
 *
 */

void PasteUndoWindows(void)
{
int i, slot;
//BOOL list[MAXEDITWINDOWS];

//	for(i=0; i<MAXEDITWINDOWS; i++)
//		list[i] = FALSE;

	DrawAllHandles(LEAVE_ACTIVE);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (Undo_WL[i] != NULL)
		{
			/**** search a place for window to paste ****/

			slot = SearchEmptyWindow();
			if (slot==-1)
			{
				DrawAllHandles(LEAVE_ACTIVE);
				return; /* no more windows can be opened: not a s erious error */
			}

			/**** copy structures ****/

			EditWindowList[slot] = Undo_WL[i];
			EditSupportList[slot]	= Undo_SL[i];

			Undo_WL[i] = NULL;
			Undo_SL[i] = NULL;

			/**** alter some parameters ****/

			EditWindowList[slot]->DrawSeqNum = GetNewDrawSeqNum_2(EditWindowList[slot]);
			EditSupportList[slot]->Active = TRUE;

			/**** open physical window ****/

			slot = OpenEditWindow(slot, 0,0,0,0);

			//list[slot] = TRUE;

			DrawEditWindow(EditWindowList[slot], EditSupportList[slot]);
		}
	}

	//DoRedrawAllEasy( list );

	DrawAllHandles(LEAVE_ACTIVE);
}					

/******** ValidateBoundingBox() ********/

void ValidateBoundingBox(WORD *x, WORD *y, WORD *w, WORD *h)
{
	if (*x < 0)
		*x=0;

	if (*y < 0)
		*y=0;

	if (*w < MINWINWIDTH)
		*w = MINWINWIDTH;

	if (*h < MINWINWIDTH)
		*h = MINWINWIDTH;

	if ( *x  > (CPrefs.PageScreenWidth-1-MINWINWIDTH) )
	{
		*x = (CPrefs.PageScreenWidth-1) - *w;
		if (*x < 0)
			*x=0;
	}

	if ( *y > (CPrefs.PageScreenHeight-1-MINWINHEIGHT) ) 
	{
		*y = (CPrefs.PageScreenHeight-1) - *h;
		if (*y < 0)
			*y=0;
	}

	if ( (*x + *w) > CPrefs.PageScreenWidth)
		*w = CPrefs.PageScreenWidth - *x;

	if ( (*y + *h) > CPrefs.PageScreenHeight)
		*h = CPrefs.PageScreenHeight - *y;
}

/******** GetWindowVars() ********/
/*
 * Get interior co-ordinates.
 *
 */

void GetWindowVars(struct EditWindow *ew, WORD *x1, WORD *y1, WORD *x2, WORD *y2)
{
int border_top, border_bottom, border_left, border_right;
ULONG bitValue;

	bitValue = (ULONG)ew->Border;

	if ( TestBit(bitValue, BORDER_TOP) )
		border_top = ew->BorderWidth;
	else
		border_top = 0;

	if ( TestBit(bitValue, BORDER_BOTTOM) )
		border_bottom = ew->BorderWidth;
	else
		border_bottom = 0;

	if ( TestBit(bitValue, BORDER_LEFT) )
		border_left = ew->BorderWidth;
	else
		border_left = 0;

	if ( TestBit(bitValue, BORDER_RIGHT) )
		border_right = ew->BorderWidth;
	else
		border_right = 0;

	*x1 = border_left;
	*y1 = border_top;
	*x2 = ew->Width - border_right - 1;
	*y2 = ew->Height - border_bottom - 1;
}

/******** GetWindowVarsShadow() ********/
/*
 * Get interior co-ordinates. Extra trimmed for shadow.
 *
 */

void GetWindowVarsShadow(struct EditWindow *ew, WORD *x1, WORD *y1, WORD *x2, WORD *y2)
{
WORD sd, sd_lr;	// left+right

	sd = ew->wdw_shadowDepth;
	//if ( ew->wdw_shadowDepth > (ew->Height/2) )
	//	sd = 0;
	//else if ( ew->wdw_shadowDepth > (ew->Width/2) )
	//	sd = 0;

	sd_lr = sd;
	if ( !(CPrefs.PageScreenModes & LACE) )
		sd_lr *= 2;
	
	if ( ew->wdw_shadowDirection==1 )
	{
		*x2 = *x2 - sd_lr;
		*y2 = *y2 - sd;
	}
	else if ( ew->wdw_shadowDirection==2 )
	{
		*x1 = *x1 + sd_lr;
		*y2 = *y2 - sd;
	}
	else if ( ew->wdw_shadowDirection==3 )
	{
		*x1 = *x1 + sd_lr;
		*y1 = *y1 + sd;
	}
	else if ( ew->wdw_shadowDirection==4 )
	{
		*x2 = *x2 - sd_lr;
		*y1 = *y1 + sd;
	}
}

/******** AddTEI() ********/

void AddTEI(struct EditWindow *NEWew, struct EditWindow *OLDew)
{
int i;

	NEWew->TEI = (struct TEInfo *)AllocMem(sizeof(struct TEInfo), MEMF_ANY | MEMF_CLEAR);
	if ( NEWew->TEI )
	{
		CopyMem(OLDew->TEI, NEWew->TEI, sizeof(struct TEInfo));		

		NEWew->TEI->frameList.lh_Head=NULL;
		NEWew->TEI->frameList.lh_Tail=NULL;
		NEWew->TEI->frameList.lh_TailPred=NULL;
		NEWew->TEI->frameList.lh_Type=NULL;
		NEWew->TEI->frameList.l_pad=NULL;

		NewList((struct List *)&(NEWew->TEI->frameList));
		AddTail((struct List *)&(NEWew->TEI->frameList), (struct Node *)NEWew);
		NEWew->TEI->text = (struct TEChar *)AllocMem(	sizeof(struct TEChar)*TEXTEDITSIZE,
																									MEMF_ANY | MEMF_CLEAR	);
		if ( NEWew->TEI->text )
		{
			CopyMem(OLDew->TEI->text, NEWew->TEI->text, sizeof(struct TEChar)*TEXTEDITSIZE);
			for(i=0; i<TEXTEDITSIZE; i++)
				NEWew->TEI->lineStarts[i] = OLDew->TEI->lineStarts[i];
		}
	}
}

/******** AddCrawl() ********/

void AddCrawl(struct EditWindow *NEWew, struct EditWindow *OLDew)
{
	if ( OLDew->crawl_text )
	{
		NEWew->crawl_text = (UBYTE *)AllocMem(OLDew->crawl_length, MEMF_ANY | MEMF_CLEAR);
		if ( NEWew->crawl_text )
		{
			CopyMem(OLDew->crawl_text, NEWew->crawl_text, OLDew->crawl_length);
			strcpy(NEWew->crawl_fontName, OLDew->crawl_fontName);
			NEWew->crawl_fontSize = OLDew->crawl_fontSize;
			NEWew->crawl_speed		= OLDew->crawl_speed;
			NEWew->crawl_flags		= OLDew->crawl_flags;
			NEWew->crawl_length		= OLDew->crawl_length;
		}
	}
}

/******** CorrectEW() ********/
/*
 * This function is called by ptread, sizewindow etc.
 *
 */

void CorrectEW(struct EditWindow *ew)
{
ULONG bitValue;
WORD sd, sd_lr, x1,y1,x2,y2, bw, ow,oh;

	bitValue = (ULONG)ew->Border;
	bw = ew->BorderWidth;
	sd = ew->wdw_shadowDepth;
	sd_lr = sd;
	if ( !(CPrefs.PageScreenModes & LACE) )
		sd_lr *= 2;

	// Check if windows is not TOO SMALL

	GetWindowVars(ew,&x1,&y1,&x2,&y2);
	GetWindowVarsShadow(ew,&x1,&y1,&x2,&y2);

	if ( (x2-x1+1) < MINWINWIDTH )
	{
		ew->Width = MINWINWIDTH;
		if ( TestBit(bitValue, BORDER_LEFT) )
			ew->Width += bw; 		
		if ( TestBit(bitValue, BORDER_RIGHT) )
			ew->Width += bw; 		
		if ( ew->wdw_shadowDirection!=0 )
			ew->Width += sd_lr;	
	}

	if ( (y2-y1+1) < MINWINHEIGHT )
	{
		ew->Height = MINWINHEIGHT;
		if ( TestBit(bitValue, BORDER_TOP) )
			ew->Height += bw; 		
		if ( TestBit(bitValue, BORDER_BOTTOM) )
			ew->Height += bw; 		
		if ( ew->wdw_shadowDirection!=0 )
			ew->Height += sd;	
	}

	// Objective #1: Try to keep the width and height as faithful as possible

	if ( ew->Width > CPrefs.PageScreenWidth )
		ew->Width = CPrefs.PageScreenWidth;

	if ( (ew->X + ew->Width) > CPrefs.PageScreenWidth )		// 0+640
		ew->X = CPrefs.PageScreenWidth - ew->Width;					// 640-640

	if ( ew->Height > CPrefs.PageScreenHeight )
		ew->Height = CPrefs.PageScreenHeight;

	if ( (ew->Y + ew->Height) > CPrefs.PageScreenHeight )
		ew->Y = CPrefs.PageScreenHeight - ew->Height;

	// Check the margins

	if ( (ew->TopMargin + ew->BottomMargin) > ew->Height )
	{
		ew->TopMargin = 0;
		ew->BottomMargin = 0;
	}

	if ( (ew->LeftMargin + ew->RightMargin) > ew->Width )
	{
		ew->LeftMargin = 0;
		ew->RightMargin = 0;
	}





#if 0
ULONG bitValue;
WORD sd, sd_lr, x1,y1,x2,y2, bw, ow,oh;

	bitValue = (ULONG)ew->Border;
	bw = ew->BorderWidth;

	sd = ew->wdw_shadowDepth;
	sd_lr = sd;
	if ( !(CPrefs.PageScreenModes & LACE) )
		sd_lr *= 2;

	GetWindowVars(ew,&x1,&y1,&x2,&y2);
	GetWindowVarsShadow(ew,&x1,&y1,&x2,&y2);

	ow = ew->Width;
	oh = ew->Height;	

	if ( (x2-x1+1) < MINWINWIDTH )
	{
		ew->Width = MINWINWIDTH;
		if ( TestBit(bitValue, BORDER_LEFT) )
			ew->Width += bw; 		
		if ( TestBit(bitValue, BORDER_RIGHT) )
			ew->Width += bw; 		
		if ( ew->wdw_shadowDirection!=0 )
			ew->Width += sd_lr;	
	}

	if ( (y2-y1+1) < MINWINHEIGHT )
	{
		ew->Height = MINWINHEIGHT;
		if ( TestBit(bitValue, BORDER_TOP) )
			ew->Height += bw; 		
		if ( TestBit(bitValue, BORDER_BOTTOM) )
			ew->Height += bw; 		
		if ( ew->wdw_shadowDirection!=0 )
			ew->Height += sd;	
	}

	if ( ew->wdw_shadowDirection!=0 )
	{
		if ( sd_lr > (x2-x1+1) )
			ew->Width += (sd_lr-(x2-x1+1));
		if ( sd > (y2-y1+1) )
			ew->Height += (sd-(y2-y1+1));
	}

	ew->X -= (ew->Width-ow);
	ew->Y -= (ew->Height-oh);
	
	if ( ew->X < 0 )
		ew->X=0;

	if ( ew->Y < 0 )
		ew->Y=0;

	if ( ew->Width > CPrefs.PageScreenWidth )
		ew->Width = CPrefs.PageScreenWidth;

	if ( ew->Height > CPrefs.PageScreenHeight )
		ew->Height = CPrefs.PageScreenHeight;

	if ( (ew->X + ew->Width) > CPrefs.PageScreenWidth)		// 0+640
		ew->X = CPrefs.PageScreenWidth - ew->X;							// 640-640

	if ( (ew->Y + ew->Height) > CPrefs.PageScreenHeight)
		ew->Y = CPrefs.PageScreenHeight - ew->Y;

	if ( ew->X < 0 )
		ew->X=0;

	if ( ew->Y < 0 )
		ew->Y=0;

	if ( (ew->X + ew->Width) > CPrefs.PageScreenWidth )
	{
		ew->Width = CPrefs.PageScreenWidth - ew->X;	// eg 640-630 -> 10
		if ( (ew->X + ew->Width) > CPrefs.PageScreenWidth )
			ew->X = 0;
	}

	if ( (ew->Y + ew->Height) > CPrefs.PageScreenHeight )
	{
		ew->Height = CPrefs.PageScreenHeight - ew->Y;
		if ( (ew->Y + ew->Height) > CPrefs.PageScreenHeight )
			ew->Y = 0;
	}

	// Check the margins

	if ( (ew->TopMargin + ew->BottomMargin) > ew->Height )
	{
		ew->TopMargin = 0;
		ew->BottomMargin = 0;
	}
	if ( (ew->LeftMargin + ew->RightMargin) > ew->Width )
	{
		ew->LeftMargin = 0;
		ew->RightMargin = 0;
	}

	if ( ew->TopMargin < 0 )
		ew->TopMargin=0;

	if ( ew->RightMargin < 0 )
		ew->RightMargin=0;

	if ( ew->BottomMargin < 0 )
		ew->BottomMargin=0;

	if ( ew->LeftMargin < 0 )
		ew->LeftMargin=0;
#endif


}

/******** E O F ********/
