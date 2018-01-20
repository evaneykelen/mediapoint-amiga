#include "nb:pre.h"

/**** externals ****/

extern ULONG allocFlags;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct BitMap sizeBM;
extern struct RastPort sizeRP;
extern UWORD chip patterns_nl[];
extern UWORD chip patterns_l[];
extern struct BitMap sharedBM;
extern struct RastPort sharedRP;
extern struct BitMap transpBM;
extern struct RastPort transpRP;
extern struct Library *medialinkLibBase;
extern struct EventData CED;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern UBYTE **msgs;   

/**** functions ****/

/******** RenderPhotoAndOrDoTransp() ********/

void RenderPhotoAndOrDoTransp(struct EditWindow *ew, struct EditSupport *es,
															struct RastPort *destRP)
{
WORD dummy;

	if ( ew->animIsAnim )
	{
		if ( es->ori_bm.Planes[0] )
		{
			if ( es->photoOpts & REMAP_PHOTO || es->photoOpts & SIZE_PHOTO )
			{
				UnSetByteBit(&es->photoOpts, REMAP_PHOTO);
				UnSetByteBit(&es->photoOpts, SIZE_PHOTO);
				SetByteBit(&es->photoOpts, MOVE_PHOTO);

				RemovePicFromWindow(es,&es->scaled_bm);
				ClearBitMap(&es->scaled_bm);
				es->scaled_w = 0;
				es->scaled_h = 0;

				RemovePicFromWindow(es,&es->remapped_bm);
				ClearBitMap(&es->remapped_bm);
				es->remapped_w = 0;
				es->remapped_h = 0;

				RemovePicFromWindow(es,&es->mask_bm);
				ClearBitMap(&es->mask_bm);
				es->mask_w = 0;
				es->mask_h = 0;

				if ( allocFlags & PAGESCREEN_FLAG )
					Message( msgs[Msg_AnimBrNoScaleRemap-1] );
			}
		}
	}

	if ( es->photoOpts & MOVE_PHOTO )	// put photo on (transp) window
		PutMovePicture(	ew, es, &dummy, &dummy, &dummy, &dummy,
										FALSE, &dummy, &dummy, &dummy, &dummy, &dummy,
										destRP, ew->X, ew->Y );
	else if ( es->photoOpts & SIZE_PHOTO )	// put sized photo on (transp) window
		PutSizePicture(ew,es,FALSE,&dummy,&dummy, destRP, ew->X, ew->Y );
}

/******** doTrans() ********/

void doTrans(	struct EditWindow *ew, struct EditSupport *es,
							WORD x, WORD y, WORD w, WORD h, WORD offX,
							struct RastPort *destRP, WORD toX, WORD toY )
{
	SetAPen(&transpRP,0L);
	SetDrMd(&transpRP,JAM1);
	RectFill(&transpRP,0,0,w-1+16,h-1);
	WaitBlit();

	if ( ew->BackFillType==1 )	// PATTERN
	{
		if ( CPrefs.PageScreenModes & LACE )
		{
			SetAfPt(destRP, patterns_l + (ew->patternNum*18), 4);	// 2^4=16*2=32 bytes
		}
		else
		{
			SetAfPt(destRP, patterns_nl + (ew->patternNum*9), 3);	// 2^3=8*2=16 bytes
		}
		SetAPen(destRP, ew->BackFillColor);
		SetDrMd(destRP, JAM1);
		RectFill(destRP, ew->X, ew->Y, ew->X+ew->Width-1, ew->Y+ew->Height-1);
		WaitBlit();
		SetAfPt(destRP, NULL, 0);
	}

	if ( es->mask_bm.Planes[0] )	// mask is supplied by scale/remap functions
		ClipBlitTransparent(destRP, x,y,w,h,offX, &es->mask_bm);
	else
		ClipBlitTransparent(destRP, x,y,w,h,offX, NULL);
}

/******** BoxInsideBox() ********/

BOOL BoxInsideBox(struct EditWindow *ew1, struct EditWindow *ew2)
{
WORD ax2,ay2,bx2,by2;

	if (ew1==NULL || ew2==NULL)
		return(FALSE);

	ax2 = ew1->X + ew1->Width - 1;
	ay2 = ew1->Y + ew1->Height - 1;

	bx2 = ew2->X + ew2->Width - 1;
	by2 = ew2->Y + ew2->Height - 1;

	/**** first look if one of four corners of box 1 lie within box 2 ****/

	if ( PointInsideBox(ew1->X, ew1->Y, ew2) )
		return(TRUE);

	if ( PointInsideBox(ax2, ew1->Y, ew2) )
		return(TRUE);

	if ( PointInsideBox(ax2, ay2, ew2) )
		return(TRUE);

	if ( PointInsideBox(ew1->X, ay2, ew2) )
		return(TRUE);

	/**** then look if one of four corners of box 2 lie within box 1 ****/

	if ( PointInsideBox(ew2->X, ew2->Y, ew1) )
		return(TRUE);

	if ( PointInsideBox(bx2, ew2->Y, ew1) )
		return(TRUE);

	if ( PointInsideBox(bx2, by2, ew1) )
		return(TRUE);

	if ( PointInsideBox(ew2->X, by2, ew1) )
		return(TRUE);

	/**** then look if left row of box 1 lies within box 2 ****/

	if ( ew1->X >= ew2->X && ew1->X <= bx2 && ew1->Y <= ew2->Y && ay2 >= by2 )
		return(TRUE);

	/**** then look if right row of box 1 lies within box 2 ****/

	if ( ax2 >= ew2->X && ax2 <= bx2 && ew1->Y <= ew2->Y && ay2 >= by2 )
		return(TRUE);

	/**** then look if top row of box 1 lies within box 2 ****/

	if ( ew1->Y >= ew2->Y && ew1->Y <= by2 && ew1->X <= ew2->X && ax2 >= bx2 )
		return(TRUE);

	/**** then look if bottom row of box 1 lies within box 2 ****/

	if ( ay2 >= ew2->Y && ay2 <= by2 && ew1->X <= ew2->X && ax2 >= bx2 )
		return(TRUE);

	return(FALSE);
}

/******** PointInsideBox() ********/

BOOL PointInsideBox(int x, int y, struct EditWindow *ew)
{
	if (ew == NULL)
		return(FALSE);

	if (	x >= ew->X && x < (ew->X+ew->Width) &&
				y >= ew->Y && y < (ew->Y+ew->Height) )
		return(TRUE);

	return(FALSE);
}

/******** AllocateSharedBM() ********/

BOOL AllocateSharedBM(WORD w, WORD h, WORD d)
{
int i;

	WaitBlit();

	FreeSharedBM();

	/**** shared bm ****/

	InitBitMap(&sharedBM, d, w, h);

	for(i=0; i<d; i++)
	{
		sharedBM.Planes[i] = (PLANEPTR)AllocRaster(w,h);
		if ( !sharedBM.Planes[i] )
		{
			UA_WarnUser(231);
			return(FALSE);
		}
	}

	InitRastPort(&sharedRP);
	sharedRP.BitMap = &sharedBM;

	Move(&sharedRP,0,0);
	SetRast(&sharedRP, 0L);
	WaitBlit();

	/**** transp bm ****/

	InitBitMap(&transpBM, d, w, h);	// THE 'd' THING THIS IS IMPORTANT!

	transpBM.Planes[0] = (PLANEPTR)AllocRaster(w,h);
	if ( !transpBM.Planes[0] )
	{
		UA_WarnUser(231);
		return(FALSE);
	}

	for(i=1; i<d; i++)
		transpBM.Planes[i] = transpBM.Planes[0];

	InitRastPort(&transpRP);
	transpRP.BitMap = &transpBM;

	Move(&transpRP,0,0);
	SetRast(&transpRP, 0L);
	WaitBlit();

	return(TRUE);
}

/******** FreeSharedBM() ********/

void FreeSharedBM(void)
{
int i;

	WaitBlit();

	if ( sharedBM.Planes[0] )
	{
		for(i=0; i<sharedBM.Depth; i++)
			if ( sharedBM.Planes[i] )
				FreeRaster(sharedBM.Planes[i], sharedBM.BytesPerRow*8, sharedBM.Rows);

		if ( transpBM.Planes[0] )
			FreeRaster(transpBM.Planes[0], transpBM.BytesPerRow*8, transpBM.Rows);
	}

	for(i=0; i<8; i++)
	{
		sharedBM.Planes[i] = NULL;
		transpBM.Planes[i] = NULL;
	}
}

/******** PutSizePicture() ********/

void PutSizePicture(struct EditWindow *ew, struct EditSupport *es,
										BOOL forClip, WORD *clipW, WORD *clipH,
										struct RastPort *destRP, WORD toX, WORD toY )
{
BOOL realloc=TRUE,scale=FALSE;
struct ScaleRemapInfo SRI;
struct BitMap *scaleBM = NULL;
WORD x1,y1,x2,y2,w,h;

	GetWindowVars(ew,&x1,&y1,&x2,&y2);
	GetWindowVarsShadow(ew,&x1,&y1,&x2,&y2);
	w = x2-x1+1;
	h = y2-y1+1;

	/**** try to get pre-cooked bitmap pointers ****/

	if ( es->scaled_bm.Planes[0] )	// there is a scaled bitmap
	{
		if ( es->scaled_w != w || es->scaled_h != h )
		{
			realloc = TRUE;		// scaled no longer valid
			scale = TRUE;			// do a scale
			FreeFastBitMap(&es->scaled_bm);
			FreeFastBitMap(&es->mask_bm);
		}
		else
		{
			realloc = FALSE;	// scaled bm can be used
			scale = FALSE;
			scaleBM = &es->scaled_bm;
		}
	}
	else	// there's nothing yet
	{
		realloc = TRUE;
		scale = TRUE;
	}

	/**** allocate new pointers ****/

	if ( realloc )
	{
		/**** alloc scaled ****/

		if ( EnoughFastMem(CPrefs.PageScreenDepth,w+16,h) )
		{
			// MAKE THIS BITMAP 16 PIXELS WIDER TO ACCOMODATE FOR WORD ALIGNED SOFT-BLITS
			// THE CAVEAT IS THAT YOU *MUST* *NOT* FREE MEMORY BASED ON SCALED_W!

			if ( !InitAndAllocBitMap(&es->scaled_bm, CPrefs.PageScreenDepth, w+16, h, MEMF_ANY) )
			{
				UA_WarnUser(-1);
				return;
			}
			es->scaled_w = w;
			es->scaled_h = h;
			scaleBM = &es->scaled_bm;			

			// INSERT HERE NEW MASK BITMAP STUFF
			if ( es->iff->BMH.nPlanes <= 8 )
			{
				if ( (es->photoOpts & REMAP_PHOTO) || (es->photoOpts & HAS_A_MASK) )
				{	
					if ( !InitAndAllocBitMap(	&es->mask_bm, 1, w+16, h, MEMF_ANY) )
					{
						UA_WarnUser(-1);
						return;
					}
					es->mask_w = w;
					es->mask_h = h;
				}
			}
		}
		else
			scaleBM = &sharedBM;
	}

	SetAPen(&sharedRP,0L);
	SetDrMd(&sharedRP,JAM1);
	RectFill(&sharedRP,0,0,w-1+16,h-1);
	WaitBlit();

	if ( scale )
	{
		if ( scaleBM )
		{
			if ( NoNeedToScaleRemap(es,w,h) )
			{
				// NEW!
				// if this picture was loaded with a stencil, and the size isn't changed
				// then the loaded stencil can be used. As soon as scaleremap is called
				// a new mask will be made.
				BltBitMapFM((struct BitMap *)&es->ori_bm,0,0,scaleBM,0,0,w,h,0xc0,0xff,NULL);
				BltBitMapFM((struct BitMap *)&es->ori_mask_bm,0,0,&es->mask_bm,0,0,w,h,0xc0,0xff,NULL);
			}
			else
			{
				SRI.SrcBitMap					= (struct BitMap24 *)&es->ori_bm;
				SRI.DstBitMap					= (struct BitMap24 *)scaleBM;
				SRI.SrcViewModes  		= es->iff->viewModes;
				SRI.DstViewModes  		= CPrefs.PageScreenModes;
				if ( es->photoOpts & REMAP_PHOTO )
					SRI.SrcColorMap			= es->cm;
				else
					SRI.SrcColorMap			= CPrefs.PageCM;

				// NEW
				if ( !es->cm )	// probably a 24 bit picture
					SRI.SrcColorMap			= NULL;
				// NEW
				
				SRI.DstColorMap				= CPrefs.PageCM;
				SRI.SrcX							= 0;
				SRI.SrcY							= 0;
				SRI.SrcWidth					= es->ori_w;
				SRI.SrcHeight					= es->ori_h;
				SRI.XSrcFactor				= es->ori_w;
				SRI.YSrcFactor				= es->ori_h;
				SRI.DestX							= 0;
				SRI.DestY							= 0;
				SRI.DestWidth					= w;
				SRI.DestHeight				= h;
				SRI.XDestFactor				= w;
				SRI.YDestFactor				= h;
				if ( ew->flags & EW_IS_OPAQUE )
					SRI.Flags						= SCAREMF_OPAQUE;
				else
					SRI.Flags						= 0;
				SRI.DitherMode				= es->ditherMode;
				if ( !(es->photoOpts & HAS_A_MASK) && es->mask_bm.Planes[0] )
					SRI.DstMaskPlane		= es->mask_bm.Planes[0];
				else
					SRI.DstMaskPlane		= NULL;
				SRI.TransparentColor	= 0;
	
				WaitBlit();

				if ( !ScaleRemap(&SRI) )
					Message("ScaleRemap Failed 1\n");

/* ================== NEW - SCALE ORIGINAL MASK ========================= */
				if ( (es->photoOpts & HAS_A_MASK) && es->mask_bm.Planes[0] )
				{
					SRI.SrcBitMap					= (struct BitMap24 *)&es->ori_mask_bm;
					SRI.DstBitMap					= (struct BitMap24 *)&es->mask_bm;
					SRI.SrcViewModes  		= 0;
					SRI.DstViewModes  		= 0;
					SRI.SrcColorMap				= CPrefs.PageCM;
					SRI.DstColorMap				= CPrefs.PageCM;
					SRI.SrcX							= 0;
					SRI.SrcY							= 0;
					SRI.SrcWidth					= es->ori_mask_w;
					SRI.SrcHeight					= es->ori_mask_h;
					SRI.XSrcFactor				= es->ori_mask_w;
					SRI.YSrcFactor				= es->ori_mask_h;
					SRI.DestX							= 0;
					SRI.DestY							= 0;
					SRI.DestWidth					= w;
					SRI.DestHeight				= h;
					SRI.XDestFactor				= w;
					SRI.YDestFactor				= h;
					SRI.Flags							= 0;
					SRI.DitherMode				= DITHER_OFF;
					SRI.DstMaskPlane			= NULL;
					SRI.TransparentColor	= 0;
					WaitBlit();
					if ( !ScaleRemap(&SRI) )
						Message("ScaleRemap Failed 2\n");
				}
/* ====================================================================== */
			}
		}
	}

	if ( scaleBM && scaleBM != &sharedBM )	// TRUE if scaleBM is in fast mem
	{
		BltBitMapFM(scaleBM,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		scaleBM = &sharedBM;
	}

	/**** make window transp - picture is now in shared ****/

	if ( forClip )
	{
		*clipW = w;
		*clipH = h;
		return;
	}
	else
		doTrans(ew,es,toX+x1,toY+y1,w,h,0,destRP,toX+x1,toY+y1);
}

/******** PutMovePicture() ********/

BOOL PutMovePicture(struct EditWindow *ew, struct EditSupport *es,
										WORD *destX, WORD *destY, WORD *destW, WORD *destH,
										BOOL forClip,
										WORD *offX, WORD *clipX, WORD *clipY, WORD *clipW, WORD *clipH,
										struct RastPort *destRP, WORD toX, WORD toY)
{
BOOL realloc=TRUE,remap=FALSE;
struct BitMap *remapBM = NULL;
WORD x1,y1,x2,y2,x,y,w,h,ww,hh;
struct ScaleRemapInfo SRI;

	GetWindowVars(ew,&x1,&y1,&x2,&y2);
	GetWindowVarsShadow(ew,&x1,&y1,&x2,&y2);
	ww = x2-x1+1;
	hh = y2-y1+1;

	if ( ew->PhotoOffsetX >= 0 )
	{
		x = 0;
		w = ww - ew->PhotoOffsetX;
		if ( w > es->ori_w )
			w = es->ori_w;
	}

	if ( ew->PhotoOffsetX < 0 )
	{
		x = ew->PhotoOffsetX*-1;
		w = es->ori_w - x;
		if ( w > ww )
			w = ww;
	}

	if ( ew->PhotoOffsetY >= 0 )
	{
		y = 0;
		h = hh - ew->PhotoOffsetY;
		if ( h > es->ori_h )
			h = es->ori_h;
	}

	if ( ew->PhotoOffsetY < 0 )
	{
		y = ew->PhotoOffsetY*-1;
		h = es->ori_h - y;
		if ( h > hh )
			h = hh;
	}

	if ( w <= 0 || h <= 0 )
	{
		*destX = 0;
		*destY = 0;
		*destW = 0;
		*destH = 0;
		return(FALSE);
	}

	/**** try to get pre-cooked bitmap pointers ****/

	if ( (es->photoOpts & REMAP_PHOTO) && es->remapped_bm.Planes[0] )	// there is a remapped bitmap
	{
		if (	es->remapped_w != w || es->remapped_h != h ||
					es->remapped_x != ew->PhotoOffsetX ||
					es->remapped_y != ew->PhotoOffsetY )
		{
			realloc=TRUE;		// remapped no longer valid
			remap=TRUE;			// do a remap
			FreeFastBitMap(&es->remapped_bm);
			FreeFastBitMap(&es->mask_bm);
		}
		else
		{
			realloc=FALSE;	// remapped bm can be used
			remap=FALSE;
			remapBM = &es->remapped_bm;
		}
	}
	else	// there's nothing yet
	{
		realloc = TRUE;
		if ( es->photoOpts & REMAP_PHOTO )
			remap = TRUE;
	}

	SetAPen(&sharedRP,0L);
	SetDrMd(&sharedRP,JAM1);
	RectFill(&sharedRP,0,0,w-1+16,h-1);
	WaitBlit();

	/**** allocate new pointers ****/

	if ( realloc && remap )	// this is skipped when doing a straight blit
	{
		if ( EnoughFastMem(CPrefs.PageScreenDepth,w+16,h) )
		{
			// MAKE THIS BITMAP 16 PIXELS WIDER TO ACCOMODATE FOR WORD ALIGNED SOFT-BLITS
			// THE CAVEAT IS THAT YOU *MUST* *NOT* FREE MEMORY BASED ON REMAPPED_W !!!!!!

			if ( !InitAndAllocBitMap(&es->remapped_bm, CPrefs.PageScreenDepth, w+16, h, MEMF_ANY) )
			{
				UA_WarnUser(-1);
				return(FALSE);
			}
			es->remapped_w = w;
			es->remapped_h = h;

			if ( x>0 )
				es->remapped_x = x*-1;
			else
				es->remapped_x = ew->PhotoOffsetX;

			if ( y>0 )
				es->remapped_y = y*-1;
			else
				es->remapped_y = ew->PhotoOffsetY;

			remapBM = &es->remapped_bm;

			// INSERT HERE NEW MASK BITMAP STUFF
			if ( es->iff->BMH.nPlanes <= 8 )
			{
				if ( (es->photoOpts & REMAP_PHOTO) || (es->photoOpts & HAS_A_MASK) )
				{	
					if ( !InitAndAllocBitMap(	&es->mask_bm, 1, w+16, h, MEMF_ANY) )
					{
						UA_WarnUser(-1);
						return(FALSE);
					}
					es->mask_w = w;
					es->mask_h = h;
				}
			}
		}
		else
			remapBM = &sharedBM;

		//BltBitMapFM(&es->ori_bm, x, y, remapBM, 0, 0, w, h, 0xc0, 0xff, NULL);
	}

	if ( remap )
	{
		if ( remapBM )
		{
			SRI.SrcBitMap					= (struct BitMap24 *)&es->ori_bm;
			SRI.DstBitMap					= (struct BitMap24 *)remapBM;
			SRI.SrcViewModes  		= es->iff->viewModes;
			SRI.DstViewModes  		= CPrefs.PageScreenModes;
			if ( es->photoOpts & REMAP_PHOTO )
				SRI.SrcColorMap			= es->cm;
			else
				SRI.SrcColorMap			= CPrefs.PageCM;

			// NEW
			if ( !es->cm )	// probably a 24 bit picture
				SRI.SrcColorMap			= NULL;
			// NEW

			SRI.DstColorMap				= CPrefs.PageCM;
			SRI.SrcX							= x;
			SRI.SrcY							= y;
			SRI.SrcWidth					= w;
			SRI.SrcHeight					= h;
			SRI.XSrcFactor				= w;
			SRI.YSrcFactor				= h;
			SRI.DestX							= 0;
			SRI.DestY							= 0;
			SRI.DestWidth					= w;
			SRI.DestHeight				= h;
			SRI.XDestFactor				= w;
			SRI.YDestFactor				= h;
			if ( ew->flags & EW_IS_OPAQUE )
				SRI.Flags						= SCAREMF_OPAQUE;
			else
				SRI.Flags						= 0;
			SRI.DitherMode				= es->ditherMode;
			if ( !(es->photoOpts & HAS_A_MASK) && es->mask_bm.Planes[0] )
			{
				SRI.DstMaskPlane		= es->mask_bm.Planes[0];
			}
			else
				SRI.DstMaskPlane		= NULL;
			SRI.TransparentColor	= 0;

			WaitBlit();

			if ( !ScaleRemap(&SRI) )
				Message("ScaleRemap Failed 3\n");

/* ================== NEW - SCALE ORIGINAL MASK ========================= */
			if ( (es->photoOpts & HAS_A_MASK) && es->mask_bm.Planes[0] )
			{
				SRI.SrcBitMap					= (struct BitMap24 *)&es->ori_mask_bm;
				SRI.DstBitMap					= (struct BitMap24 *)&es->mask_bm;
				SRI.SrcViewModes  		= 0;
				SRI.DstViewModes  		= 0;
				SRI.SrcColorMap				= CPrefs.PageCM;
				SRI.DstColorMap				= CPrefs.PageCM;
				SRI.SrcX							= 0;
				SRI.SrcY							= 0;
				SRI.SrcWidth					= w;
				SRI.SrcHeight					= h;
				SRI.XSrcFactor				= w;
				SRI.YSrcFactor				= h;
				SRI.DestX							= 0;
				SRI.DestY							= 0;
				SRI.DestWidth					= w;
				SRI.DestHeight				= h;
				SRI.XDestFactor				= w;
				SRI.YDestFactor				= h;
				SRI.Flags							= 0;
				SRI.DitherMode				= DITHER_OFF;
				SRI.DstMaskPlane			= NULL;
				SRI.TransparentColor	= 0;
				WaitBlit();
				if ( !ScaleRemap(&SRI) )
					Message("ScaleRemap Failed 4\n");
			}
/* ====================================================================== */
		}
	}

	if ( remapBM && remapBM != &sharedBM )			// remapBM is in fast memory
	{
		BltBitMapFM(remapBM,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		x=0;	// remapped x starts at x, ori x somewhere else
	}
	else if ( !remap )													// no remapping, just blitting
	{
		BltBitMapFM((struct BitMap *)&es->ori_bm,x,y,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
	}

	/**** make window transp - picture is now in shared ****/

	if ( ew->PhotoOffsetX >= 0 )
		x1 += ew->PhotoOffsetX;

	if ( ew->PhotoOffsetY >= 0 )
		y1 += ew->PhotoOffsetY;

	if ( forClip )
	{
		*offX = x%16;
		if ( ew->PhotoOffsetX >= 0 )
			*clipX = ew->PhotoOffsetX;
		else
			*clipX = x*-1;
		if ( ew->PhotoOffsetY >= 0 )
			*clipY = ew->PhotoOffsetY;
		else
			*clipY = y*-1;
		*clipW = w;
		*clipH = h;
		return(TRUE);
	}
	else
		doTrans(ew,es,toX+x1,toY+y1,w,h,x%16,destRP,toX+x1,toY+y1);

	return(TRUE);
}

/******** EnoughFastMem() ********/

BOOL EnoughFastMem(WORD d, WORD w, WORD h)
{
LONG needed;

	needed = ((w/8)*h)*d;

	if ( AvailMem(MEMF_FAST|MEMF_LARGEST) < needed )
		return(FALSE);

	return(TRUE);
}

/******** RemapAllPics() ********/

void RemapAllPics(void)
{
BOOL list[MAXEDITWINDOWS];
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
		list[i] = FALSE;

	DrawAllHandles(LEAVE_ACTIVE);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] &&
				EditSupportList[i]->Active )
		{
			if (	EditSupportList[i]->photoOpts & MOVE_PHOTO ||
						EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				SetByteBit(&EditSupportList[i]->photoOpts, REMAP_PHOTO);

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->scaled_bm);
				ClearBitMap(&EditSupportList[i]->scaled_bm);
				EditSupportList[i]->scaled_w = 0;
				EditSupportList[i]->scaled_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->remapped_bm);
				ClearBitMap(&EditSupportList[i]->remapped_bm);
				EditSupportList[i]->remapped_w = 0;
				EditSupportList[i]->remapped_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->mask_bm);
				ClearBitMap(&EditSupportList[i]->mask_bm);
				EditSupportList[i]->mask_w = 0;
				EditSupportList[i]->mask_h = 0;
			}

			list[i] = TRUE;
		}
	}

	RedrawAllOverlapWdwList(list);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** RemapAllPicsAndBackground() ********/

void RemapAllPicsAndBackground(void)
{
int i;

	SetSpriteOfActWdw(SPRITE_BUSY);

	// FIRST REMAP AND REDRAW BACKGROUND

	if ( backES.photoOpts & MOVE_PHOTO || backES.photoOpts & SIZE_PHOTO )
	{
		SetByteBit(&backES.photoOpts, REMAP_PHOTO);

		if ( backES.scaled_bm.Planes[0] )
			FreeFastBitMap( &backES.scaled_bm );
		backES.scaled_w = 0;
		backES.scaled_h = 0;

		if ( backES.remapped_bm.Planes[0] )
			FreeFastBitMap( &backES.remapped_bm );
		backES.remapped_w = 0;
		backES.remapped_h = 0;

		ShowBackground();
	}
	else
	{
		SetDrMd(pageWindow->RPort,JAM1);
		Move(pageWindow->RPort,0,0);
		SetRast(pageWindow->RPort,0L);
		WaitBlit();
	}

	// THEN REMAP AND REDRAW ALL WINDOWS

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] )
		{
			if (	EditSupportList[i]->photoOpts & MOVE_PHOTO ||
						EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				SetByteBit(&EditSupportList[i]->photoOpts, REMAP_PHOTO);

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->scaled_bm);
				ClearBitMap(&EditSupportList[i]->scaled_bm);
				EditSupportList[i]->scaled_w = 0;
				EditSupportList[i]->scaled_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->remapped_bm);
				ClearBitMap(&EditSupportList[i]->remapped_bm);
				EditSupportList[i]->remapped_w = 0;
				EditSupportList[i]->remapped_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->mask_bm);
				ClearBitMap(&EditSupportList[i]->mask_bm);
				EditSupportList[i]->mask_w = 0;
				EditSupportList[i]->mask_h = 0;
			}
		}
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			if ( EditSupportList[i]->restore_bm.Planes[0] )
			{
				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->restore_bm);
				ClearBitMap(&EditSupportList[i]->restore_bm);
			}
			DrawEditWindow( EditWindowList[i], EditSupportList[i] );
		}
	}

	DrawAllHandles(LEAVE_ACTIVE);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** NoNeedToScaleRemap() ********/

BOOL NoNeedToScaleRemap(struct EditSupport *es, WORD w, WORD h)
{
struct ColorMap *cm;

	if ( es->iff->BMH.masking == mskHasMask && w==es->ori_w && h==es->ori_h )
	{
		if ( es->photoOpts & REMAP_PHOTO )
			cm = es->cm;
		else
			cm = CPrefs.PageCM;
		if ( cm && cm!=CPrefs.PageCM )
		{
			if ( !CompareCM(cm,CPrefs.PageCM) )
			{
				return(FALSE);
			}
		}
		return(TRUE);
	}
	return(FALSE);
}

/******** E O F ********/
