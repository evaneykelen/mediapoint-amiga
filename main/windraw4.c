#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct BitMap sizeBM;
extern struct RastPort sizeRP;
extern struct MsgPort *capsPort;
extern struct EventData CED;
extern int lastUndoableAction;
extern int lastUndoWindow;
extern struct Library *medialinkLibBase;

/**** globals ****/

WORD undoOffsetX, undoOffsetY;

/**** functions ****/

/******** MovePicture() ********/

void MovePicture(int wdwNr)
{
ULONG signals;
BOOL loop=TRUE, mouseMoved=FALSE, wasRemapped=FALSE;
WORD startX, startY, x=0, y=0, offsetX,offsetY, x1,y1,x2,y2;
struct IntuiMessage *message;
struct EditWindow *ew;
struct EditSupport *es;
struct TEInfo *TEI;
PLANEPTR mask_bm, ori_mask_bm;

	if (wdwNr==-1 || !(EditSupportList[wdwNr]->photoOpts & MOVE_PHOTO) )
		return;

	ew = EditWindowList[wdwNr];
	es = EditSupportList[wdwNr];

	GetWindowVars(ew,&x1,&y1,&x2,&y2);
	GetWindowVarsShadow(ew,&x1,&y1,&x2,&y2);

	UA_SetSprite(pageWindow,SPRITE_HAND);

	DrawAllHandles(LEAVE_ACTIVE);

	UA_SwitchMouseMoveOn(pageWindow);

	startX = pageWindow->MouseX;
	startY = MassageY(pageWindow->MouseY);

	offsetX = ew->PhotoOffsetX;
	offsetY = ew->PhotoOffsetY;

	undoOffsetX = offsetX;
	undoOffsetY = offsetY;
	lastUndoableAction = PAGE_UNDO_PICMOVE;
	lastUndoWindow = wdwNr;

	x = pageWindow->MouseX - startX;
	y = MassageY(pageWindow->MouseY);
	y = y - startY;
	x += offsetX;
	y += offsetY;

	if ( es->photoOpts & REMAP_PHOTO )
	{
		wasRemapped = TRUE;
		UnSetByteBit(&(es->photoOpts), REMAP_PHOTO);
	}

	mask_bm = es->mask_bm.Planes[0];
	ori_mask_bm = es->ori_mask_bm.Planes[0];
	es->mask_bm.Planes[0] = NULL;
	es->ori_mask_bm.Planes[0] = NULL;

	DrawSafeBox(pageWindow->RPort,ew->X-1,ew->Y-1,ew->Width+2,ew->Height+2);

	TEI = ew->TEI;
	ew->TEI = NULL;

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				CED.Code = message->Code;
				ReplyMsg((struct Message *)message);
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
				x = pageWindow->MouseX - startX;
				y = MassageY(pageWindow->MouseY);
				y = y - startY;
				x += offsetX;
				y += offsetY;
				ew->PhotoOffsetX = x;
				ew->PhotoOffsetY = y;
				RestoreBack(ew,es);
				DrawEditWindow(ew,es);
			}
		}
	}

	ew->TEI = TEI;

	if ( wasRemapped )
		SetByteBit(&(es->photoOpts), REMAP_PHOTO);

	es->mask_bm.Planes[0] = mask_bm;
	es->ori_mask_bm.Planes[0] = ori_mask_bm;

	DrawSafeBox(pageWindow->RPort,ew->X-1,ew->Y-1,ew->Width+2,ew->Height+2);

	if ( (ew->PhotoOffsetX * ew->PhotoOffsetY) != 0 )
	{
		RemovePicFromWindow(es, &es->ori_mask_bm);
		ClearBitMap(&es->ori_mask_bm);
		es->ori_mask_w = 0;
		es->ori_mask_h = 0;
		UnSetByteBit(&es->photoOpts,HAS_A_MASK);
	}

	RedrawAllOverlapWdwEasy(wdwNr,TRUE,TRUE);

	UA_SwitchMouseMoveOff(pageWindow);

	UA_SetSprite(pageWindow,SPRITE_NORMAL);

	DrawAllHandles(LEAVE_ACTIVE);
}

#if 0

/******** unclipWindow() ********/
/*
 * code lifted from RKM page 723
 *
 * Used to remove a clipping region installed by clipWindow(),
 * disposing of the installed region and reinstalling the region removed.
 *
 */

void unclipWindow(struct Window *win)
{
struct Region *old_region;

	/* Remove any old region by installing a NULL region,
	** then dispose of the old region if one was installed.
	*/

	if (NULL != (old_region = InstallClipRegion(win->WLayer,NULL)))
		DisposeRegion(old_region);
}

/******** clipWindow() ********/
/*
 * code lifted from RKM page 723
 *
 * Clip a window to a specified rectangle (given by upper left and lower
 * right corner). The removed region is returned so that it may be re-
 * installed later.
 *
 */

struct Region *clipWindow(struct Window *win, LONG minX, LONG minY,
													LONG maxX, LONG maxY)
{
struct Region *new_region;
struct Rectangle my_rectangle;

	my_rectangle.MinX = minX; //+2; 
	my_rectangle.MinY = minY; //+1;
	my_rectangle.MaxX = maxX; //-2;
	my_rectangle.MaxY = maxY; //-1;

	if (NULL != (new_region = NewRegion()))
	{
		if (FALSE == OrRectRegion(new_region, &my_rectangle))
		{
			DisposeRegion(new_region);
			new_region = NULL;
		}
	}

	/* Install the new region, and return any existing region.
	** If the above allocation and region processing failed, then
	** new_region will be NUL and no clip will be installed.
	*/

	return( InstallClipRegion(win->WLayer, new_region) );
}

#endif

/******** E O F ********/
