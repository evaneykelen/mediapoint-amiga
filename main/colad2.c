#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *paletteWindow;
extern struct Screen *paletteScreen;
extern struct Library *medialinkLibBase;
extern struct Screen *pageScreen;
extern int selectedWell;
extern struct EventData CED;
extern struct eventHandlerInfo EHI;
extern struct ColorMap *undoCM;

/**** gadgets ****/

extern struct GadgetRecord ColorAdjust_GR[];
extern struct GadgetRecord Harmonize_GR[];
extern struct GadgetRecord ColorRamp_GR[];

STATIC void HarmonizeFunc(int *);
STATIC void ColorRampFunc(int *);

/**** functions ****/

/******** InitPaletteControls() ********/

void InitPaletteControls(void)
{
	/**** select initial well ****/

	SelectNewWell(-1, 1);	// color 1

	InitSampleAndSliders(selectedWell);	// selectedWell is now 1
}

/******** SelectNewWell(void) ********/
/*
 * if skip==-1 don't render previous well
 *
 */

void SelectNewWell(int skip, int newWell)
{
	if (skip!=-1)
		SelectAWell(selectedWell);

	selectedWell = newWell;
	SelectAWell(selectedWell);

	InitSampleAndSliders(selectedWell);
}

/******** InitSampleAndSliders() ********/

void InitSampleAndSliders(int well)
{
int r,g,b;

	if ( !paletteWindow)
		return;

	/**** init color 0 and 4 of palette screen ****/

	GetColorComponents(pageScreen->ViewPort.ColorMap, well, &r, &g, &b);
	SetVPToComponents(&paletteScreen->ViewPort, 4, r, g, b);

	GetColorComponents(pageScreen->ViewPort.ColorMap, 0, &r, &g, &b);
	SetVPToComponents(&paletteScreen->ViewPort, 0, r, g, b);

	/**** fill color sample area ****/

	PaintButton(paletteWindow, &ColorAdjust_GR[9], 4);

	/**** init sliders ****/

	SetSliders();
}

/******** SetSliders() ********/

void SetSliders(void)
{
int r,g,b,steps_factor;

	if ( !paletteWindow)
		return;

	GetColorComponents(pageScreen->ViewPort.ColorMap, selectedWell, &r, &g, &b);

	if (CPrefs.AA_available) //GfxBase->LibNode.lib_Version >= 39)
		steps_factor = 1;
	else
		steps_factor = 16;

	SetPaletteSlider(paletteWindow, &ColorAdjust_GR[6], r, steps_factor);
	SetPaletteSlider(paletteWindow, &ColorAdjust_GR[7], g, steps_factor);
	SetPaletteSlider(paletteWindow, &ColorAdjust_GR[8], b, steps_factor);

	PrintPaintColor(r,g,b);
}

/******** PrintPaintColor() ********/

void PrintPaintColor(int r, int g, int b)
{
TEXT buf[10];

	if ( !paletteWindow)
		return;

	sprintf(buf, "%d", r);

	UA_PrintInBox(	paletteWindow, &ColorAdjust_GR[11],
									ColorAdjust_GR[11].x1, ColorAdjust_GR[11].y1-1,
									ColorAdjust_GR[11].x2, ColorAdjust_GR[11].y2, 
									buf, PRINT_CENTERED);

	sprintf(buf, "%d", g);

	UA_PrintInBox(	paletteWindow, &ColorAdjust_GR[12],
									ColorAdjust_GR[12].x1, ColorAdjust_GR[12].y1-1,
									ColorAdjust_GR[12].x2, ColorAdjust_GR[12].y2, 
									buf, PRINT_CENTERED);

	sprintf(buf, "%d", b);

	UA_PrintInBox(	paletteWindow, &ColorAdjust_GR[13],
									ColorAdjust_GR[13].x1, ColorAdjust_GR[13].y1-1,
									ColorAdjust_GR[13].x2, ColorAdjust_GR[13].y2, 
									buf, PRINT_CENTERED);
}

/******** Harmonize() ********/

void Harmonize(void)
{
struct Window *window;
BOOL loop, retVal;
int ID;
int min=0;
int max=255;
int bright=127;
int data[5];

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);	// current colors to undo cm

	retVal = FALSE;
	loop   = TRUE;

	/**** open a window ****/

	EHI.activeScreen = -1;				// fool the requester opener

	window = UA_OpenRequesterWindow(paletteWindow,Harmonize_GR,USECOLORS);
	if (!window)
	{
		EHI.activeScreen = STARTSCREEN_PAGE;
		UA_WarnUser(226);
		return;
	}
	EHI.activeScreen = STARTSCREEN_PAGE;

	/**** render gadget ****/

	UA_DrawGadgetList(window, Harmonize_GR);

	UA_SetSliderGadg(window, &Harmonize_GR[ 5], min,    256, &Harmonize_GR[ 7], 1);
	UA_SetSliderGadg(window, &Harmonize_GR[ 6], max,    256, &Harmonize_GR[ 8], 1);
	UA_SetSliderGadg(window, &Harmonize_GR[11], bright, 256, &Harmonize_GR[12], -127);

	UA_DrawSpecialGadgetText(window, &Harmonize_GR[11], " ", SPECIAL_TEXT_BEFORE_STRING);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Harmonize_GR, &CED);
			switch(ID)
			{
				case 5:		// Min
					data[0] = min;
					data[1] = max;
					data[2] = bright;
					UA_ProcessSliderGadg(	window, &Harmonize_GR[5], &min, 256, &Harmonize_GR[7],
																&CED, HarmonizeFunc, data, 0, 1 );
					if (min>=max)
						min=max-1;
					if (min<0)
						min=0;
					UA_SetSliderGadg(window, &Harmonize_GR[5], min, 256, &Harmonize_GR[7], 1);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					HarmonizeCols(min,max,bright-127,TRUE);
					break;

				case 6:		// Max
					data[0] = min;
					data[1] = max;
					data[2] = bright;
					UA_ProcessSliderGadg(	window, &Harmonize_GR[6], &max, 256, &Harmonize_GR[8],
																&CED, HarmonizeFunc, data, 1, 1 );
					if (max<=min)
						max=min+1;
					if (max>255)
						max=255;
					UA_SetSliderGadg(window, &Harmonize_GR[6], max, 256, &Harmonize_GR[8], 1);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					HarmonizeCols(min,max,bright-127,TRUE);
					break;

				case 11:	// Brighness
					data[0] = min;
					data[1] = max;
					data[2] = bright;
					UA_ProcessSliderGadg(	window, &Harmonize_GR[11], &bright, 256, &Harmonize_GR[12],
																&CED, HarmonizeFunc, data, 2, -127 );
					UA_SetSliderGadg(window, &Harmonize_GR[11], bright, 256, &Harmonize_GR[12], -127);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					HarmonizeCols(min,max,bright-127,TRUE);
					break;

				case 9:		// OK
do_ok:
					UA_HiliteButton(window, &Harmonize_GR[9]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 10:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Harmonize_GR[10]);
					loop=FALSE;
					retVal=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// Cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,USECOLORS);

	if (retVal)
	{
		CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
		HarmonizeCols(min,max,bright-127,FALSE);
	}
	else
		SetScreenToCM(pageScreen, undoCM);
}

/******** HarmonizeFunc() ********/

STATIC void HarmonizeFunc(int *data)
{
	CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
	HarmonizeCols(data[0],data[1],data[2]-127,TRUE);
}

/******** ColorRamp() ********/

void ColorRamp(void)
{
struct Window *window;
BOOL loop, retVal;
int ID;
int r=0, g=0, b=0;
int data[5];

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);	// current colors to undo cm

	retVal = FALSE;
	loop   = TRUE;

	/**** open a window ****/

	EHI.activeScreen = -1;				// fool the requester opener

	window = UA_OpenRequesterWindow(paletteWindow,ColorRamp_GR,USECOLORS);
	if (!window)
	{
		EHI.activeScreen = STARTSCREEN_PAGE;
		UA_WarnUser(226);
		return;
	}
	EHI.activeScreen = STARTSCREEN_PAGE;

	/**** render gadget ****/

	UA_DrawGadgetList(window, ColorRamp_GR);

	UA_SetSliderGadg(window, &ColorRamp_GR[ 5], r, 256, &ColorRamp_GR[ 7], 1);
	UA_SetSliderGadg(window, &ColorRamp_GR[ 6], g, 256, &ColorRamp_GR[ 8], 1);
	UA_SetSliderGadg(window, &ColorRamp_GR[11], b, 256, &ColorRamp_GR[12], 1);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, ColorRamp_GR, &CED);
			switch(ID)
			{
				case 5:		// R
					data[0] = r;
					data[1] = g;
					data[2] = b;
					UA_ProcessSliderGadg(	window, &ColorRamp_GR[5], &r, 256, &ColorRamp_GR[7],
																&CED, ColorRampFunc, data, 0, 1 );
					if (r<0)
						r=0;
					UA_SetSliderGadg(window, &ColorRamp_GR[5], r, 256, &ColorRamp_GR[7], 1);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					ColorRampCols(r,g,b,TRUE);
					break;

				case 6:		// G
					data[0] = r;
					data[1] = g;
					data[2] = b;
					UA_ProcessSliderGadg(	window, &ColorRamp_GR[6], &g, 256, &ColorRamp_GR[8],
																&CED, ColorRampFunc, data, 1, 1 );
					if (g<0)
						g=0;
					UA_SetSliderGadg(window, &ColorRamp_GR[6], g, 256, &ColorRamp_GR[8], 1);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					ColorRampCols(r,g,b,TRUE);
					break;

				case 11:	// B
					data[0] = r;
					data[1] = g;
					data[2] = b;
					UA_ProcessSliderGadg(	window, &ColorRamp_GR[11], &b, 256, &ColorRamp_GR[12],
																&CED, ColorRampFunc, data, 2, 1 );
					if (b<0)
						b=0;
					UA_SetSliderGadg(window, &ColorRamp_GR[11], b, 256, &ColorRamp_GR[12], 1);
					CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
					ColorRampCols(r,g,b,TRUE);
					break;

				case 9:		// OK
do_ok:
					UA_HiliteButton(window, &ColorRamp_GR[9]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 10:	// Cancel
do_cancel:
					UA_HiliteButton(window, &ColorRamp_GR[10]);
					loop=FALSE;
					retVal=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// Cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,USECOLORS);

	if (retVal)
	{
		CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
		ColorRampCols(r,g,b,FALSE);
	}
	else
		SetScreenToCM(pageScreen, undoCM);
}

/******** ColorRampFunc() ********/

STATIC void ColorRampFunc(int *data)
{
	CopyCMtoCM(undoCM,pageScreen->ViewPort.ColorMap);
	ColorRampCols(data[0],data[1],data[2],TRUE);
}

/******** E O F ********/
