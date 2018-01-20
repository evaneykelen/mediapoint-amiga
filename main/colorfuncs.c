#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct ColorMap *undoCM;
extern ULONG *LoadRGB32Table;

/**** functions ****/

/******** Spread() ********/

void SpreadCols(int start, int end)
{
int i,j,how,numCols,oldEnd;
float step_r, step_g, step_b, rr, gg, bb;
int R_gun_1, G_gun_1, B_gun_1, R_gun_2, G_gun_2, B_gun_2;
ULONG r,g,b;

	if (start == end)
		return;

	oldEnd = end;

	/**** save color in undo table ****/

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);

	/**** swap start and end if necessary ****/

	if (start > end)
		swapInts(&start, &end);

	/**** spread colors ****/

	GetColorComponents(	pageScreen->ViewPort.ColorMap,
											start, &R_gun_1, &G_gun_1, &B_gun_1);

	GetColorComponents(	pageScreen->ViewPort.ColorMap,
											end, &R_gun_2, &G_gun_2, &B_gun_2);
 
	step_r = (float)(R_gun_2-R_gun_1)/(end-start);
	step_g = (float)(G_gun_2-G_gun_1)/(end-start);
	step_b = (float)(B_gun_2-B_gun_1)/(end-start);

	rr = (float)R_gun_1;
	gg = (float)G_gun_1;
	bb = (float)B_gun_1;

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);

	if ( CPrefs.AA_available && GfxBase->LibNode.lib_Version >= 39 )
	{
		for(i=start,j=0; i<end; i++,j++)
		{
			r=(ULONG)rr;
			g=(ULONG)gg;
			b=(ULONG)bb;

			TurnSmallIntoLarge(&r,&g,&b);

			LoadRGB32Table[1+(j*3)+0] = r;
			LoadRGB32Table[1+(j*3)+1] = g;
			LoadRGB32Table[1+(j*3)+2] = b;

			rr += step_r;
			gg += step_g;
			bb += step_b;
		}

		LoadRGB32Table[                 0 ] = ((end-start) << 16) + start;
		LoadRGB32Table[ ((end-start)*3)+1 ] = 0L;
		LoadRGB32(&(pageScreen->ViewPort),LoadRGB32Table);

		CreateColorsInPalette(how, 2);
	}
	else
	{
		for (i=start; i<end; i++)
		{
			SetColorComponents(pageScreen->ViewPort.ColorMap, i, (int)rr, (int)gg, (int)bb);
			rr += step_r;
			gg += step_g;
			bb += step_b;
		}
		SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
		CreateColorsInPalette(how, 2);
	}
}

/******** SwapCols() ********/

void SwapCols(int start, int end)
{
int how,numCols;
int R_gun_1, G_gun_1, B_gun_1, R_gun_2, G_gun_2, B_gun_2;

	if (start == end)
		return;

	/**** save color in undo table ****/

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);

	/**** swap colors ****/

	GetColorComponents(	pageScreen->ViewPort.ColorMap,
											start, &R_gun_1, &G_gun_1, &B_gun_1);

	GetColorComponents(	pageScreen->ViewPort.ColorMap,
											end, &R_gun_2, &G_gun_2, &B_gun_2);

	SetColorComponents(	pageScreen->ViewPort.ColorMap, start,
											R_gun_2, G_gun_2, B_gun_2);

	SetColorComponents(	pageScreen->ViewPort.ColorMap, end,
											R_gun_1, G_gun_1, B_gun_1);

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);

	CreateColorsInPalette(how, 2);

	SetScreenToPartialCM(pageScreen,pageScreen->ViewPort.ColorMap,start,start);
	SetScreenToPartialCM(pageScreen,pageScreen->ViewPort.ColorMap,end,end);
	//SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
}

/******** CopyCols() ********/

void CopyCols(int start, int end)
{
int how,numCols;
int R_gun, G_gun, B_gun;

	if (start == end)
		return;

	/**** save color in undo table ****/

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);

	/**** swap colors ****/

	GetColorComponents(	pageScreen->ViewPort.ColorMap,
											start, &R_gun, &G_gun, &B_gun);

	SetColorComponents(	pageScreen->ViewPort.ColorMap, end,
											R_gun, G_gun, B_gun);

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);

	CreateColorsInPalette(how, 2);

	SetScreenToPartialCM(pageScreen,pageScreen->ViewPort.ColorMap,start,start);
	SetScreenToPartialCM(pageScreen,pageScreen->ViewPort.ColorMap,end,end);
	//SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
}

/******** UndoCols() ********/

void UndoCols(void)
{
int how, numCols;
struct ColorMap *localCM;

//	if (CPrefs.AA_available)

	if (GfxBase->LibNode.lib_Version >= 39)
		localCM = GetColorMap(256);
	else
		localCM = GetColorMap(32);

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, localCM);

	CopyCMtoCM(undoCM, pageScreen->ViewPort.ColorMap);

	CopyCMtoCM(localCM, undoCM);

	FreeColorMap(localCM);

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);

	CreateColorsInPalette(how, 2);

	SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
}

/******** HarmonizeCols() ********/
/*
 * min and max are values ranging from 0 to 15
 *
 */

void HarmonizeCols(int min, int max, int bright, BOOL onlyPage)
{
int i,how,numCols, R_gun, G_gun, B_gun, cmax;

	/**** save color in undo table ****/

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	if ( !CPrefs.AA_available )
	{
		min = min/16;
		max = max/16;
		bright = bright/16;
		cmax = 15;
	}
	else
		cmax = 255;

	bright *= 2;

	for (i=0; i<numCols; i++)
	{
		GetColorComponents(pageScreen->ViewPort.ColorMap,i,&R_gun,&G_gun,&B_gun);

		// change brightness

		R_gun += bright;		
		if ( R_gun < 0 )
			R_gun = 0;
		else if ( R_gun > cmax )
			R_gun = cmax;

		G_gun += bright;		
		if ( G_gun < 0 )
			G_gun = 0;
		else if ( G_gun > cmax )
			G_gun = cmax;

		B_gun += bright;		
		if ( B_gun < 0 )
			B_gun = 0;
		else if ( B_gun > cmax )
			B_gun = cmax;

		// change limits

		if ( R_gun < min )
			R_gun = min;
		if ( R_gun > max )
			R_gun = max;

		if ( G_gun < min )
			G_gun = min;
		if ( G_gun > max )
			G_gun = max;

		if ( B_gun < min )
			B_gun = min;
		if ( B_gun > max )
			B_gun = max;

		SetColorComponents(pageScreen->ViewPort.ColorMap,i,R_gun,G_gun,B_gun);
	}

	if ( !onlyPage )
	{
		how = HowToFillTheWells(numCols);
		CreateColorsInPalette(how, 2);
	}

	SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
}

/******** ColorRampCols() ********/

void ColorRampCols(int r, int g, int b, BOOL onlyPage)
{
int i,how,numCols, R_gun, G_gun, B_gun, cmax;

	/**** save color in undo table ****/

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	if ( !CPrefs.AA_available )
	{
		r = r/16;
		g = g/16;
		b = b/16;
		cmax = 15;
	}
	else
		cmax = 255;

	for (i=0; i<numCols; i++)
	{
		GetColorComponents(pageScreen->ViewPort.ColorMap,i,&R_gun,&G_gun,&B_gun);

		// change brightness

		R_gun += r;		
		if ( R_gun < 0 )
			R_gun = 0;
		else if ( R_gun > cmax )
			R_gun = cmax;

		G_gun += g;		
		if ( G_gun < 0 )
			G_gun = 0;
		else if ( G_gun > cmax )
			G_gun = cmax;

		B_gun += b;		
		if ( B_gun < 0 )
			B_gun = 0;
		else if ( B_gun > cmax )
			B_gun = cmax;

		SetColorComponents(pageScreen->ViewPort.ColorMap,i,R_gun,G_gun,B_gun);
	}

	if ( !onlyPage )
	{
		how = HowToFillTheWells(numCols);
		CreateColorsInPalette(how, 2);
	}

	SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
}

/******** E O F ********/
