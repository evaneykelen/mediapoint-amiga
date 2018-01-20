#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Library *medialinkLibBase;
extern ULONG *LoadRGB32Table;
extern struct Screen *pageScreen;

/**** functions ****/

/******** SetColorCM4() ********/
/*
 * set 'CM' to 'rgb' at index 'well'
 *
 */

void SetColorCM4(struct ColorMap *cm, UWORD rgb, int well)
{
int r,g,b;

	r = (rgb & 0x0f00) >> 8;
	g = (rgb & 0x00f0) >> 4;
	b = (rgb & 0x000f);
	SetRGB4CM(cm, well, r, g, b);	// OS function
}

/******** SetColorCM32() ********/
/*
 * set 'CM' to 'rgb' at index 'well'
 *
 * This functions shifts to 4 bit color handling if necessary.
 *
 * rgb MUST be 0x00rrggbb !
 * 
 */

void SetColorCM32(struct ColorMap *cm, ULONG rgb, int well)
{
ULONG r,g,b,gun;

	if ( CPrefs.AA_available )
	{
		/**** rgb is 0x00rrggbb ****/

		gun = (rgb & 0x00ff0000);
		r = gun;										// r is 0x00ff0000
		r |= (gun << 8);						// r is 0xffff0000
		r |= (gun >> 8);						// r is 0xffffff00
		r |= (gun >> 16);						// r is 0xffffffff

		gun = (rgb & 0x0000ff00);
		g = gun;
		g |= (gun << 16);
		g |= (gun << 8);
		g |= (gun >> 8);

		gun = (rgb & 0x000000ff);
		b = gun;
		b |= (gun << 24);
		b |= (gun << 16);
		b |= (gun << 8);

		SetRGB32CM(cm, well, r, g, b);	// OS function
	}
	else
	{
		/*         0x00rrggbb */

		r = (rgb & 0x000f0000) >> 16;
		g = (rgb & 0x00000f00) >> 8;
		b = (rgb & 0x0000000f);

		SetRGB4CM(cm, well, r, g, b);	// OS function
	}
}

/******** SetScreenToCM() ********/
/*
 * This function knows about old style and new style color maps.
 *
 */

void SetScreenToCM(struct Screen *screen, struct ColorMap *cm)
{
struct ViewPort *vp;
int numColors, numPageColors;
struct BitMap *bm;
int numCMColors;

	if ( screen==pageScreen )
		InvalidateTable();

	vp = &(screen->ViewPort);
	bm = screen->RastPort.BitMap;

	numCMColors = cm->Count;
	numPageColors = UA_GetNumberOfColorsInScreen(vp->Modes, bm->Depth, CPrefs.AA_available);

	if ( numCMColors > numPageColors )
		numColors = numPageColors;
	else
		numColors = numCMColors;

	if ( numPageColors > numCMColors )
		numColors = numCMColors;

	if (CPrefs.AA_available) 
	{
		GetRGB32(cm, 0, numColors, &LoadRGB32Table[1]);	// OS function
		LoadRGB32Table[ 0 ] = 0L;
		LoadRGB32Table[ 0 ] = numColors << 16;
		LoadRGB32Table[ (numColors*3)+1 ] = 0L;
		LoadRGB32(&(screen->ViewPort), LoadRGB32Table);
	}
	else
		LoadRGB4(vp, cm->ColorTable, numColors);
}

/******** SetScreenToColorTable4() ********/
/*
 * takes a screen and colormap and pokes the colors into the viewport
 *
 */

void SetScreenToColorTable4(struct Screen *screen, UWORD *colorTable, int max)
{
int numColors;
struct ViewPort *vp;
struct RastPort *rp;

	if ( screen==pageScreen )
		InvalidateTable();

	vp = &(screen->ViewPort);
	rp = &(screen->RastPort);

	numColors = UA_GetNumberOfColorsInScreen(vp->Modes, rp->BitMap->Depth, CPrefs.AA_available);
	if (numColors>max)
		numColors=max;
	LoadRGB4(vp, colorTable, (LONG)numColors);
}

/******** SetScreenToPartialCM() ********/

void SetScreenToPartialCM(struct Screen *screen, struct ColorMap *cm,
													int start, int end)
{
struct ViewPort *vp;
struct ColorMap *cm2;
int i,num;
UWORD rgb;

	if ( screen==pageScreen )
		InvalidateTable();

	vp = &(screen->ViewPort);

	if (start==end)
		num=1;
	else
		num=(end-start)+1;

	if (CPrefs.AA_available)
	{
		GetRGB32(cm, start, num, &LoadRGB32Table[1]);	// OS function
		LoadRGB32Table[ 0 ] = 0L;
		LoadRGB32Table[ 0 ] = (num << 16) + start;
		LoadRGB32Table[ (num*3)+1 ] = 0L;
		LoadRGB32(&(screen->ViewPort), LoadRGB32Table);
	}
	else
	{
		cm2 = vp->ColorMap;
		for(i=start; i<(end+1); i++)
		{
			rgb = GetColorCM4(cm, i);
			SetColorCM4(cm2, rgb, i);
		}
		SetScreenToCM(screen, cm2);
	}
}

/******** GetColorCM4() ********/

UWORD GetColorCM4(struct ColorMap *cm, int well)
{
	return( (UWORD)GetRGB4(cm, (LONG)well) );
}

/******** GetColorCM32() ********/
/*
 * This functions shifts to 4 bit color handling if necessary.
 *
 * returned rgb is formatted as 0x00rrggbb !
 * 
 */

ULONG GetColorCM32(struct ColorMap *cm, int well)
{
ULONG table[10];
ULONG r, g, b, rgb;

	if (CPrefs.AA_available)
	{
		GetRGB32(cm, well, 1, &table[0]);	// OS function
		r = (table[0] & 0x000000ff) << 16;
		g = (table[1] & 0x000000ff) << 8;
	  b = (table[2] & 0x000000ff);
	}
	else
	{
		table[0] = GetRGB4(cm, well);

		r = (table[0] & 0x00000f00);	// r is now 0x00000r00
		r |= ( r << 4 );							// r is now 0x0000rr00
		r <<= 8;											// r is now 0x00rr0000

		g = (table[0] & 0x000000f0);	// g is now 0x000000g0
		g |= ( g << 4 );							// g is now 0x00000gg0
		g <<= 4;											// g is now 0x0000gg00

		b = (table[0] & 0x0000000f);	// b is now 0x0000000b
		b |= ( b << 4 );							// b is now 0x000000bb
	}

	rgb = r | g | b;

	return(rgb);
}

/******** CopyCMtoCM() ********/
/*
 * This function knows about old style and new style color maps.
 *
 */

void CopyCMtoCM(struct ColorMap *from_cm, struct ColorMap *to_cm)
{
int i,max;
UWORD rgb_word;
struct ViewPort vp;

	InitVPort(&vp);
	vp.ColorMap = to_cm;

	if ( from_cm->Count > to_cm->Count )
		max = to_cm->Count;
	else
		max = from_cm->Count;

	if ( CPrefs.AA_available )
	{
		GetRGB32(from_cm, 0, max, &LoadRGB32Table[1]);	// OS function
		LoadRGB32Table[ 0 ] = 0L;
		LoadRGB32Table[ 0 ] = max << 16;
		LoadRGB32Table[ (max*3)+1 ] = 0L;
		LoadRGB32(&vp, LoadRGB32Table);
	}
	else
	{
		for(i=0; i<max; i++)
		{
			rgb_word = GetColorCM4(from_cm, i);
			SetColorCM4(to_cm, rgb_word, i);
		}
	}
}

/******** CopyCMPartial() ********/
/*
 * This function knows about old style and new style color maps.
 *
 */

void CopyCMPartial(	struct ColorMap *from_cm, struct ColorMap *to_cm, 
										int from, int to, int num )
{
struct ViewPort vp;
int i;
UWORD rgb;

	InitVPort(&vp);
	vp.ColorMap = to_cm;

	if (num<=1)
		num=1;

	if (CPrefs.AA_available)
	{
		GetRGB32(from_cm, from, num, &LoadRGB32Table[1]);	// OS function
		LoadRGB32Table[ 0 ] = 0L;
		LoadRGB32Table[ 0 ] = (num << 16) + to;
		LoadRGB32Table[ (num*3)+1 ] = 0L;
		LoadRGB32(&vp,LoadRGB32Table);
	}
	else
	{
		for(i=0; i<num; i++)
		{
			rgb = GetColorCM4(from_cm, from+i);
			SetColorCM4(to_cm, rgb, to+i);
		}
	}
}

/******** TransferCMtoCM() ********/
/*
 * This function knows about old style and new style color maps.
 *
 */

void TransferCMtoCM(struct ColorMap *from_cm, struct ColorMap *to_cm,
										int from, int to, int num)
{
int i;
UWORD rgb_word;
struct ViewPort vp;

	InitVPort(&vp);
	vp.ColorMap = to_cm;

	if ( num==0 )
		return;

	if (CPrefs.AA_available)
	{
		GetRGB32(from_cm, from, num, &LoadRGB32Table[1]);	// OS function
		LoadRGB32Table[ 0 ] = 0L;
		LoadRGB32Table[ 0 ] = (num << 16) + to;
		LoadRGB32Table[ (num*3)+1 ] = 0L;
		LoadRGB32(&vp, LoadRGB32Table);
	}
	else
	{
		for(i=0; i<num; i++)
		{
			rgb_word = GetColorCM4(from_cm, from+i);
			SetColorCM4(to_cm, rgb_word, to+i);
		}
	}
}

/******** CompareCM() ********/

BOOL CompareCM(struct ColorMap *cm1, struct ColorMap *cm2)
{
int i, nc;
int numCMColors1, numCMColors2;

	numCMColors1 = cm1->Count;
	numCMColors2 = cm2->Count;

	if (numCMColors1 > numCMColors2)
		nc = numCMColors2;
	else
		nc = numCMColors1;

	for(i=0; i<nc; i++)
		if ( GetColorCM32(cm1, i) != GetColorCM32(cm2, i)	)
			return(FALSE);

	return(TRUE);
}

/******** CompareCMPartial() ********/

BOOL CompareCMPartial(struct ColorMap *cm1, struct ColorMap *cm2,
											int start, int end)
{
int i;

	for(i=start; i<(end+1); i++)
		if ( GetColorCM32(cm1, i) != GetColorCM32(cm2, i)	)
			return(FALSE);

	return(TRUE);
}

/******** CompareCMandColorTable() ********/

BOOL CompareCMandColorTable4(struct ColorMap *cm, UWORD *colorTable,
														int start, int end)
{
int i;

	for(i=start; i<(end+1); i++)
		if ( GetColorCM4(cm, i) != colorTable[i] )
			return(FALSE);

	return(TRUE);
}

/******** GetColorComponents() ********/

void GetColorComponents(struct ColorMap *cm, int well, int *r, int *g, int *b)
{
ULONG table[10];
ULONG rr,gg,bb;

	if (CPrefs.AA_available) //GfxBase->LibNode.lib_Version >= 39)
	{
		GetRGB32(cm, well, 2, &table[0]);	// OS function

		rr = (table[0] & 0x000000ff);
		gg = (table[1] & 0x000000ff);
	  bb = (table[2] & 0x000000ff);
		*r = rr;
		*g = gg;
		*b = bb;
	}
	else
	{
		table[0] = GetRGB4(cm, well);

		/* table[0] is now 0x00000rgb */

		rr = (table[0] & 0x00000f00);
		rr >>= 8;
		gg = (table[0] & 0x000000f0);
		gg >>= 4;
		bb = (table[0] & 0x0000000f);

		*r = rr;
		*g = gg;
		*b = bb;
	}
}

/******** SetColorComponents() ********/

void SetColorComponents(struct ColorMap *cm, int well, int r, int g, int b)
{
ULONG gun;
ULONG rr,gg,bb;

	if (CPrefs.AA_available) //GfxBase->LibNode.lib_Version >= 39)
	{
		gun = r;
		rr = gun;									// rr is 0x000000ff
		rr |= (gun << 8);					// rr is 0x0000ffff
		rr |= (gun << 16);				// rr is 0x00ffffff
		rr |= (gun << 24);				// rr is 0xffffffff

		gun = g;
		gg = gun;
		gg |= (gun << 8);
		gg |= (gun << 16);
		gg |= (gun << 24);

		gun = b;
		bb = gun;
		bb |= (gun << 8);
		bb |= (gun << 16);
		bb |= (gun << 24);

		SetRGB32CM(cm, well, rr, gg, bb);	// OS function
	}
	else
		SetRGB4CM(cm, well, r, g, b);			// OS function
}

/******** SetVPToComponents() ********/

void SetVPToComponents(struct ViewPort *vp, int well, int r, int g, int b)
{
ULONG gun;
ULONG rr,gg,bb;

	if (CPrefs.AA_available) //GfxBase->LibNode.lib_Version >= 39)
	{
		gun = r;
		rr = gun;									// rr is 0x000000ff
		rr |= (gun << 8);					// rr is 0x0000ffff
		rr |= (gun << 16);				// rr is 0x00ffffff
		rr |= (gun << 24);				// rr is 0xffffffff

		gun = g;
		gg = gun;
		gg |= (gun << 8);
		gg |= (gun << 16);
		gg |= (gun << 24);

		gun = b;
		bb = gun;
		bb |= (gun << 8);
		bb |= (gun << 16);
		bb |= (gun << 24);

		SetRGB32(vp, well, rr, gg, bb);	// OS function
	}
	else
		SetRGB4(vp, well, r, g, b);	// OS function
}

/******** Turn4into32() ********/

void Turn4into32(UWORD rgb, ULONG *r, ULONG *g, ULONG *b)
{
	*r = (ULONG)rgb & 0x00000f00;	// *r = 0x00000f00
	*r |= (*r >> 4);							// *r = 0x00000ff0
	*r >>= 4;											// *r = 0x000000ff
	*r |= (*r << 8);							// *r = 0x0000ffff
	*r |= (*r	<< 16);							// *r = 0xffffffff

	*g = (ULONG)rgb & 0x000000f0;	// *g = 0x000000f0
	*g |= (*g >> 4);							// *g = 0x000000ff
	*g |= (*g << 8);							// *g = 0x0000ffff
	*g |= (*g	<< 16);							// *g = 0xffffffff

	*b = (ULONG)rgb & 0x0000000f;	// *b = 0x0000000f
	*b |= (*b << 4);							// *b = 0x000000ff
	*b |= (*b << 8);							// *b = 0x0000ffff
	*b |= (*b	<< 16);							// *b = 0xffffffff
}

/******** TurnSmallIntoLarge() ********/

void TurnSmallIntoLarge(ULONG *r, ULONG *g, ULONG *b)
{
ULONG l;

	l = *r;						//  l is 0x000000ff
	*r |= (l << 8);		// *r is 0x0000ffff
	*r |= (l << 16);	// *r is 0x00ffffff
	*r |= (l << 24);	// *r is 0xffffffff

	l = *g;						//  l is 0x000000ff
	*g |= (l << 8);		// *g is 0x0000ffff
	*g |= (l << 16);	// *g is 0x00ffffff
	*g |= (l << 24);	// *g is 0xffffffff

	l = *b;						//  l is 0x000000ff
	*b |= (l << 8);		// *b is 0x0000ffff
	*b |= (l << 16);	// *b is 0x00ffffff
	*b |= (l << 24);	// *b is 0xffffffff
}

/******** E O F ********/
