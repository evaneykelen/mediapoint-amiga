#include "nb:pre.h"

/**** defines ****/

#define SLIDER_W 15

/**** externals ****/

extern struct EventData CED;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct CapsPrefs CPrefs;

/**** functions ****/

/******** SetPaletteSlider() ********/

void SetPaletteSlider(	struct Window *window, struct GadgetRecord *GR,
												int pos, int steps_factor)
{
struct GadgetRecord gadget;
UBYTE lace;

	UA_ClearButton(window, GR, AREA_PEN);

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	gadget.x1 = GR->x1+2+pos*steps_factor;
	gadget.x2 = gadget.x1 + SLIDER_W * (steps_factor/8);

	if (gadget.x2 == gadget.x1)
		gadget.x2 = gadget.x1 + SLIDER_W;

	gadget.y1 = GR->y1+lace+1;
	gadget.y2 = GR->y2-lace-1;
	gadget.color = 2;
	gadget.type = BUTTON_GADGET;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	UA_DrawGadget(window, &gadget);
}

/******** ProcessPaletteSlider() ********/
/*
 * gun is 1,2,3 for r,g,b
 *
 */

void ProcessPaletteSlider(struct Window *window, struct GadgetRecord *GR,
													int *pos, int steps_factor, int gun, int well)
{
struct GadgetRecord gadget;
UBYTE lace;
int total,i,w;
float f;

	SafeSetWriteMask(window->RPort, 0x7);

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	/**** if steps_factor==1 (AA) *pos is 0..255 ****/
	/**** if steps_factor==16 		*pos is 0..15  ****/

	gadget.x1 = GR->x1+2+((*pos)*steps_factor);
	gadget.x2 = gadget.x1 + SLIDER_W * (steps_factor/8);

	if (gadget.x2 == gadget.x1)
		gadget.x2 = gadget.x1 + SLIDER_W;

	gadget.y1 = GR->y1+lace+1;
	gadget.y2 = GR->y2-lace-1;
	gadget.color = 2;
	gadget.type = BUTTON_GADGET;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	/**** slider sensitivity ****/

	if (	(window->MouseX > gadget.x1) && (window->MouseX < gadget.x2) &&
				(window->MouseY > gadget.y1) && (window->MouseY < gadget.y2) )
	{
		i=*pos;

		w = SLIDER_W * (steps_factor/8);
		if (w==0)
			w=SLIDER_W;

//		DragPaletteSlider(window, GR, SLIDER_W * (steps_factor/8), &i, steps_factor, gun, well);
		DragPaletteSlider(window, GR, w, &i, steps_factor, gun, well);
		*pos=i;
	}
	else
	{
		if ( steps_factor==1 )
			total = 256;
		else
			total = 16;
		i = (GR->x2-GR->x1)/total;
		f = (float)(window->MouseX-GR->x1)/(float)i;
		if (	CED.Qualifier & IEQUALIFIER_LSHIFT ||
					CED.Qualifier & IEQUALIFIER_RSHIFT )
			*pos = (int)f;
		else
		{
#if 0
			if ( !CPrefs.AA_available ) // && GfxBase->LibNode.lib_Version >= 39)
			{
				if (*pos > (int)f)
					*pos = *pos - 17;
				else if (*pos < (int)f)
					*pos = *pos + 17;
			}
			else
			{
#endif
				if (*pos > (int)f)	/* van bijv. 15 naar 14 */
					*pos = *pos - 1;
				else if (*pos < (int)f)	/* van bijv. 14 naar 15 */
					*pos = *pos + 1;
#if 0
			}
#endif

#if 0
			if (*pos > (int)f)	/* van bijv. 15 naar 14 */
				*pos = *pos - 1;
			else if (*pos < (int)f)	/* van bijv. 14 naar 15 */
				*pos = *pos + 1;
#endif

		}
		if (*pos<0)
			*pos=0;
		else if (*pos>(total-1))
			*pos=total-1;

#if 0
	if ( !CPrefs.AA_available ) //&& GfxBase->LibNode.lib_Version >= 39)
	{
		*pos = *pos / 17;
		*pos = *pos * 17;
	}
#endif

		SetPaletteSlider(window, GR, *pos, steps_factor);
	}

#if 0
	if ( !CPrefs.AA_available && GfxBase->LibNode.lib_Version >= 39)
	{
		*pos = *pos / 17;
		*pos = *pos * 17;
	}
#endif

	SafeSetWriteMask(window->RPort, 0xff);
}

/******* DragPaletteSlider() ********/

void DragPaletteSlider(	struct Window *window, struct GadgetRecord *GR,
												int w, int *pos, int steps_factor, int gun, int well)
{
ULONG signals;
BOOL loop,moved=FALSE,mouseMoved;
struct GadgetRecord gadget;
int r,g,b,diff,numCols,how;
UBYTE lace;
struct IntuiMessage *message;
struct ViewPort *vp;
struct ColorMap *cm;

	vp = &(pageScreen->ViewPort);
	cm = vp->ColorMap;

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);

	/**** hilite gadget ****/

	gadget.x1 = GR->x1+2+((*pos)*steps_factor);
	gadget.x2 = gadget.x1+w;
	if (lace==1)
	{
		gadget.y1 = GR->y1+2;
		gadget.y2 = GR->y2-2;
	}
	else
	{
		gadget.y1 = GR->y1+1;
		gadget.y2 = GR->y2-1;
	}
	gadget.color = 2;
	gadget.type = BUTTON_GADGET;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	UA_DrawGadget(window, &gadget);
	UA_InvertButton(window, &gadget);

	diff = gadget.x1 - window->MouseX;

	/**** handle events ****/

	UA_SwitchMouseMoveOn(window);

	gadget.x1 = GR->x1+2;
	gadget.x2 = GR->x2-2;

	GetColorComponents(cm, well, &r, &g, &b);

	loop=TRUE;
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
				moved=TRUE;
				SetDrMd(window->RPort, JAM1);
				SetAPen(window->RPort, UA_GetRightPen(window,GR,AREA_PEN));	// NEW
				RectFill(window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
																(LONG)gadget.x2, (LONG)gadget.y2);
				gadget.x1 = window->MouseX + diff;
				gadget.x2 = gadget.x1 + w;
				if (gadget.x1 < GR->x1+2)
				{
					gadget.x1 = GR->x1+2;
					gadget.x2 = gadget.x1+w;
				}
				else if (gadget.x2 > GR->x2-2)
				{
					gadget.x2 = GR->x2-2;
					gadget.x1 = gadget.x2-w;
				}
				UA_DrawGadget(window, &gadget);
				UA_InvertButton(window, &gadget);
				*pos = (gadget.x1 - GR->x1 - 2) / steps_factor;
				GetColorComponents(cm, well, &r, &g, &b);
				if (gun==1)
					r=*pos;
				else if (gun==2)
					g=*pos;
				else if (gun==3)
					b=*pos;
				SetSliderColor(r, g, b, well, numCols, how);
			}
		}
	}

	UA_SwitchMouseMoveOff(window);

	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, UA_GetRightPen(window,GR,AREA_PEN));
	RectFill(	window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
						(LONG)gadget.x2, (LONG)gadget.y2);

#if 0
	if ( !CPrefs.AA_available ) // && GfxBase->LibNode.lib_Version >= 39)
	{
		*pos = *pos / 17;
		*pos = *pos * 17;
	}
#endif

	gadget.x1 = GR->x1+2+((*pos)*steps_factor);
	gadget.x2 = gadget.x1+w;
	if (lace==1)
	{
		gadget.y1 = GR->y1+2;
		gadget.y2 = GR->y2-2;
	}
	else
	{
		gadget.y1 = GR->y1+1;
		gadget.y2 = GR->y2-1;
	}
	gadget.color = 2;
	gadget.type = BUTTON_GADGET;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, UA_GetRightPen(window,GR,AREA_PEN));
	RectFill(	window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
						(LONG)gadget.x2, (LONG)gadget.y2);

	UA_DrawGadget(window, &gadget);
}

/******** E O F ********/
