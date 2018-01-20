#include "pre.h"
#include <hardware/blit.h>

/**** externals ****/

extern struct BitMap sharedBM;
extern struct RastPort sharedRP;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct CapsPrefs CPrefs;
extern struct BitMap transpBM;
extern struct RastPort transpRP;

/**** functions ****/

/******** ClipBlitTransparent() ********/

void ClipBlitTransparent(	struct RastPort *drp,
													WORD x, WORD y, WORD w, WORD h, WORD offX,
													struct BitMap *maskBM )
{
struct RastPort *srp = &sharedRP;
struct BitMap *shadowBM = &transpBM;
int d;

	if ( !maskBM )
	{
		/**** copy all planes of source to 1 plane shadow mask ****/
		BltBitMap(srp->BitMap,offX,0,shadowBM,offX,0,w,h,0xe0,0xff,NULL);
		WaitBlit();
	}
	else
	{
		/**** copy existing mask to shadow mask ****/
		d = shadowBM->Depth;
		shadowBM->Depth = 1;
		BltBitMapFM(maskBM,offX,0,shadowBM,offX,0,w,h,0xc0,0xff,NULL);
		shadowBM->Depth = d;
		WaitBlit();
//BltBitMapFM(maskBM,     0,0,pageScreen->RastPort.BitMap,0,0,w,h,0xc0,0xff,NULL);
//BltBitMapFM(srp->BitMap,0,0,pageScreen->RastPort.BitMap,0,h,w,h,0xc0,0xff,NULL);
	}

	/**** copy source to dest using a mask ****/

	BltMaskBitMapRastPort(srp->BitMap,offX,0,drp,x,y,w,h,
												ABC | ABNC | ANBC, shadowBM->Planes[0]);
	WaitBlit();
}

/******** MakeMovePic() ********/

void MakeMovePic(int lastHit)
{
struct BitMap *bm;
WORD w,h, x1,y1,x2,y2,pox,poy;

	SetAPen(&transpRP,0L);
	SetDrMd(&transpRP,JAM1);
	RectFill(	&transpRP,0,0,
						EditWindowList[lastHit]->Width-1,
						EditWindowList[lastHit]->Height-1);
	WaitBlit();

	SetAPen(&sharedRP,0L);
	SetDrMd(&sharedRP,JAM1);
	RectFill(	&sharedRP,0,0,
						EditWindowList[lastHit]->Width-1,
						EditWindowList[lastHit]->Height-1);
	WaitBlit();

	GetWindowVars(EditWindowList[lastHit],&x1,&y1,&x2,&y2);

	pox=0;
	poy=0;
	if ( !(EditSupportList[lastHit]->photoOpts & SIZE_PHOTO) )
	{
		pox = EditWindowList[lastHit]->PhotoOffsetX;
		poy = EditWindowList[lastHit]->PhotoOffsetY;
	}

	if ( EditSupportList[lastHit]->mask_bm.Planes[0] )
	{
		bm = &EditSupportList[lastHit]->mask_bm;
		if ( EditSupportList[lastHit]->mask_w < (x2-x1+1) )
			w = EditSupportList[lastHit]->mask_w;
		else
			w = x2-x1+1;
		if ( EditSupportList[lastHit]->mask_h < (y2-y1+1) )
			h = EditSupportList[lastHit]->mask_h;
		else
			h = y2-y1+1;
		BltBitMapFM(bm,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		x1 += pox;
		y1 += poy;
		if ( (x1+w-1) > (transpBM.BytesPerRow*8) )
			x1=0;
		if ( (y1+h-1) > transpBM.Rows )
			y1=0;
		if ( x1<0 )
			x1=0;
		if ( y1<0 )
			y1=0;
		BltBitMap(&sharedBM,0,0,&transpBM,x1,y1,w,h,0xe0,0xff,NULL);		
		WaitBlit();
	}
	else if ( EditSupportList[lastHit]->scaled_bm.Planes[0] )
	{
		bm = &EditSupportList[lastHit]->scaled_bm;
		if ( EditSupportList[lastHit]->scaled_w < (x2-x1+1) )
			w = EditSupportList[lastHit]->scaled_w;
		else
			w = x2-x1+1;
		if ( EditSupportList[lastHit]->scaled_h < (y2-y1+1) )
			h = EditSupportList[lastHit]->scaled_h;
		else
			h = y2-y1+1;
		BltBitMapFM(bm,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		if ( (x1+w-1) > (transpBM.BytesPerRow*8) )
			x1=0;
		if ( (y1+h-1) > transpBM.Rows )
			y1=0;
		if ( x1<0 )
			x1=0;
		if ( y1<0 )
			y1=0;
		BltBitMap(&sharedBM,0,0,&transpBM,x1,y1,w,h,0xe0,0xff,NULL);		
		WaitBlit();
	}
	else if ( EditSupportList[lastHit]->remapped_bm.Planes[0] )
	{
		bm = &EditSupportList[lastHit]->remapped_bm;
		if ( EditSupportList[lastHit]->remapped_w < (x2-x1+1) )
			w = EditSupportList[lastHit]->remapped_w;
		else
			w = x2-x1+1;
		if ( EditSupportList[lastHit]->remapped_h < (y2-y1+1) )
			h = EditSupportList[lastHit]->remapped_h;
		else
			h = y2-y1+1;
		BltBitMapFM(bm,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		x1 += pox;
		y1 += poy;
		if ( (x1+w-1) > (transpBM.BytesPerRow*8) )
			x1=0;
		if ( (y1+h-1) > transpBM.Rows )
			y1=0;
		if ( x1<0 )
			x1=0;
		if ( y1<0 )
			y1=0;
		BltBitMap(&sharedBM,0,0,&transpBM,x1,y1,w,h,0xe0,0xff,NULL);		
		WaitBlit();
	}
	else if ( EditSupportList[lastHit]->ori_bm.Planes[0] )
	{
		bm = (struct BitMap *)&EditSupportList[lastHit]->ori_bm;
		if ( EditSupportList[lastHit]->ori_w < (x2-x1+1) )
			w = EditSupportList[lastHit]->ori_w;
		else
			w = x2-x1+1;
		if ( EditSupportList[lastHit]->ori_h < (y2-y1+1) )
			h = EditSupportList[lastHit]->ori_h;
		else
			h = y2-y1+1;
		BltBitMapFM(bm,0,0,&sharedBM,0,0,w,h,0xc0,0xff,NULL);
		x1 += pox;
		y1 += poy;
		if ( (x1+w-1) > (transpBM.BytesPerRow*8) )
			x1=0;
		if ( (y1+h-1) > transpBM.Rows )
			y1=0;
		if ( x1<0 )
			x1=0;
		if ( y1<0 )
			y1=0;
		BltBitMap(&sharedBM,0,0,&transpBM,x1,y1,w,h,0xe0,0xff,NULL);		
		WaitBlit();
	}
	else
	{
		bm = pageWindow->RPort->BitMap;
		w = EditWindowList[lastHit]->Width;
		h = EditWindowList[lastHit]->Height;
		BltBitMap(bm,
							EditWindowList[lastHit]->X+pageWindow->LeftEdge,
							EditWindowList[lastHit]->Y+pageWindow->TopEdge,
							&transpBM,0,0,w,h,0xe0,0xff,NULL);
		WaitBlit();
	}
}

/******** DrawMovePic() ********/

void DrawMovePic(struct EditWindow *ew, WORD x, WORD y)
{
int depth;

	//WaitTOF();
	depth = transpBM.Depth;
	transpBM.Depth = 1;
	ClipBlit(&transpRP,0,0,pageWindow->RPort,x,y,ew->Width,ew->Height,0x60);
	WaitBlit();
	transpBM.Depth = depth;
}

/******** E O F ********/
