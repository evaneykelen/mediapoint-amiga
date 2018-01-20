#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <fye/fye.h>
#include <clib/fye_protos.h>
#include <pragmas/fye.h>
#include <fye/fyeBase.h>
#include "iff.h"
#include "fyeview_protos.h"

#define SysBase					(FyeBase->ml_SysBase)
#define	GfxBase					(FyeBase->ml_GfxBase)
#define	FyeBoardBase		(FyeBase->ml_FyeBoardBase)
#define FyeBoardIsBusy	(FyeBase->ml_FyeBoardIsBusy)

#define custom (* (struct Custom *) 0xDFF000)

ULONG fcs_cleanup (ULONG error, struct FyeScreen * fs, struct FyeBase *FyeBase)
{
UBYTE	i;

	if (fs->oldview)
	{
		ON_DISPLAY;
		LoadView (fs->oldview);
		WaitTOF();

		if (fs->vp.ColorMap)
			FreeColorMap(fs->vp.ColorMap);

		if (fs->bmp[0].Planes[0])
			FreeVPortCopLists(&(fs->vp));

		for (i=0; i<6; i++)
		{
			if (fs->lof[i])
				FreeCprList (fs->lof[i]);

			if (fs->shf[i])
				FreeCprList (fs->shf[i]);
		}

		if ( fs->v.LOFCprList )
			FreeCprList(fs->v.LOFCprList);

		if ( fs->v.SHFCprList )
			FreeCprList(fs->v.SHFCprList);

		FreeVPortCopLists(&(fs->vp));

	}

	FreeMem (fs, sizeof (struct FyeScreen));

	return (error);
}

/*
ULONG FyeCleanupScreen ( struct FyeScreen * fs)
{
	return (fcs_cleanup (0,fs));
}
*/

ULONG FyeCreateScreen(struct FyeScreen **fsptr,
											struct PictHeader *ph,
											PLANEPTR *bitmap,
											ULONG mode, struct FyeBase *FyeBase)
{
struct FyeScreen	* fs;
UBYTE	i;
UWORD	cmap[] = { 0x000, 0x111, 0x222, 0x333, 0x444, 0x555, 0x666, 0x777,
             	   0x888, 0x999, 0xaaa, 0xbbb, 0xccc, 0xddd, 0xeee, 0xfff };

	*fsptr=NULL;

	fs = AllocMem (sizeof (struct FyeScreen), MEMF_PUBLIC | MEMF_CLEAR);
	if (!fs)
		return (FYE_ERRINSUFFICIENTMEMORY);

	fs->oldview = GfxBase->ActiView;

	InitView(&(fs->v));			/* Initializing View ... 			*/
	InitVPort(&(fs->vp));		/* Initializing ViewPort ...	*/

	fs->v.ViewPort = &(fs->vp);
	fs->v.DxOffset= 0;
	fs->v.DyOffset= 0;

	ph->bmhd.pageWidth =ph->bmhd.w;
	ph->bmhd.pageHeight=ph->bmhd.h;

	fs->vp.DWidth   = ph->bmhd.pageWidth;
	fs->vp.DHeight  = ph->bmhd.pageHeight;
	fs->vp.RasInfo  = &(fs->ri);

	fs->vp.Modes    =  LACE | HIRES;

	fs->vp.ColorMap = GetColorMap(16L); /* Loading ColorMap ... */
	if (!(fs->vp.ColorMap))
		return (fcs_cleanup (FYE_ERRCOLORMAP,fs,FyeBase));

	LoadRGB4(&(fs->vp), cmap, 16L);

	InitBitMap(&(fs->bmp[0]),DEPTH,ph->bmhd.w,ph->bmhd.h);

	fs->ri.BitMap   = &(fs->bmp[0]);
	fs->ri.RxOffset = ph->bmhd.x;
	fs->ri.RyOffset = ph->bmhd.y;
	fs->ri.Next     = NULL;

	for (i=0; i<4;i++)
		fs->bmp[0].Planes[i]=bitmap[i];

	fs->v.Modes |= fs->vp.Modes;

	fs->vp.DxOffset = ph->vpdx;
	fs->vp.DyOffset = ph->vpdy;

	MakeVPort(&(fs->v), &(fs->vp));	/* Making ViewPort ... */
	MrgCop(&(fs->v));								/* Merging copper lists ... */

	*fsptr=fs;
	return(NULL);
}

