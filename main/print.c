#include "nb:pre.h"

/**** externals ****/

extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern char *scriptCommands[];
extern struct RendezVousRecord rvrec;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Screen *pageScreen;
extern struct CapsPrefs CPrefs;
extern struct Library *medialinkLibBase;
extern struct BitMap sharedBM;
extern struct RastPort sharedRP;

/**** functions ****/

/******** ShowPrint() ********/

void ShowPrint(int which)
{
BOOL retval;
int i;
//struct BitMap copyBM;
//struct RastPort copyRP;
//BOOL bm_ok=FALSE;

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		PaletteToBack();

	SetSpriteOfActWdw(SPRITE_BUSY);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		rvrec.EW[i]	= EditWindowList[i];
		rvrec.ES[i]	= EditSupportList[i];
	}

	retval = which;	// 1 for print script and 2 for print page

	rvrec.aPtr = (APTR)&sharedRP;	//copyRP;

	if ( EHI.activeScreen==STARTSCREEN_PAGE )
	{
		Move(&sharedRP,0,0);
		SetRast(&sharedRP,0L);
		WaitBlit();			

		BltBitMapRastPort(pageScreen->RastPort.BitMap,
											0,pageWindow->TopEdge,
											&sharedRP,
											0,0,pageWindow->Width,pageWindow->Height,
											0xc0);
		WaitBlit();			
	}

	if ( !LoadModule("Print", &retval) )
		UA_WarnUser(151);

	//if ( EHI.activeScreen==STARTSCREEN_PAGE && bm_ok )
	//CloseCopyOfBitmap(&copyBM);
	
	rvrec.aPtr = NULL;

	if ( retval )
		updateCAPSconfig(2);	// update PRINTERPREFS lines

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

#if 0
/******** OpenCopyOfBitmap() ********/
/*
 * Try to make a copy of the PLS screen. Allocate at least 1 plane, if others fail
 * don't bother, at least some planes then get printed....
 *
 */

BOOL OpenCopyOfBitmap(struct BitMap *bm, struct RastPort *rp)
{
int i;
WORD d,w,h;

	d = CPrefs.PageScreenDepth;
	w = CPrefs.PageScreenWidth;
	h = CPrefs.PageScreenHeight;

	InitBitMap(bm,d,w,h);

	for(i=0; i<8; i++)
		bm->Planes[i] = NULL;

	for(i=0; i<d; i++)
	{
		bm->Planes[i] = AllocRaster(w,h);
		if (bm->Planes[i]==NULL && i==0)
			return(FALSE);
	}

	BltBitMap(pageScreen->RastPort.BitMap,
						0,0,
						bm,
						0,0,w,h,
						0xc0, 0xff, NULL);
	WaitBlit();

	InitRastPort(rp);
	rp->BitMap = bm;

	return(TRUE);
}

/******** CloseCopyOfBitmap() ********/

void CloseCopyOfBitmap(struct BitMap *bm)
{
int i;
WORD w,h;

	w = CPrefs.PageScreenWidth;
	h = CPrefs.PageScreenHeight;

	for(i=0; i<8; i++)
		if (bm->Planes[i] != NULL)
			FreeRaster(bm->Planes[i],w,h);
}
#endif

/******** E O F ********/
