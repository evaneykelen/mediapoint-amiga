#include "nb:pre.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct MsgPort *capsPort;
extern struct CapsPrefs CPrefs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern UWORD allColors[];
extern UWORD palettes[];
extern struct Screen *pageScreen;
extern struct Window *pageWindow;
extern struct eventHandlerInfo EHI;
extern struct RendezVousRecord rvrec;

/**** functions ****/

#if 0
/******** SetStandardColors() ********/

void SetStandardColors(struct Window *window)
{
	//SetMenuColors(window);
	UA_SetMenuColors(&rvrec,window);
}

/******** ResetStandardColors() ********/

void ResetStandardColors(struct Window *window)
{
	//ResetMenuColors(window);
	UA_ResetMenuColors(&rvrec,window);
}
#endif

#if 0
/******** SetAllStandardColors() ********/
/*
 * Called by CloseDocumentProc
 *
 */

void SetAllStandardColors(void)
{
int i;
UWORD rgb[32];

	for(i=0; i<32; i++)
		rgb[i] = allColors[i];

//	for(i=0; i<8; i++)
//		rgb[i] = palettes[i+CPrefs.colorSet*8];

	LoadRGB4(&(pageScreen->ViewPort), rgb, 32);
}

/******** SetAllColorsToZero() ********/
/*
 * Called by CloseDocumentProc
 *
 */

void SetAllColorsToZero(void)
{
int i;
UWORD rgb[32];

	for(i=0; i<32; i++)
		rgb[i] = palettes[CPrefs.colorSet*8];
	LoadRGB4(&(pageScreen->ViewPort), rgb, 32);
}
#endif

/******** ScaleGadgetList() ********/

void ScaleGadgetList(struct Screen *screen, struct GadgetRecord *GR)
{
	if ( GR->type != DIMENSIONS )
		return;

	if ( screen->ViewPort.Modes & LACE )
	{
		if ( GR->x1==0 )	// not doubled
		{
			UA_DoubleGadgetDimensions(GR);
			GR->x1 = 1;
		}
	}
	else
	{
		if ( GR->x1==1 )	// doubled
		{
			UA_HalveGadgetDimensions(GR);
			GR->x1 = 0;
		}
	}
}

/******** E O F ********/
