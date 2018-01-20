#include "mllib_includes.h"

#define SLIDER_W 15

extern struct MsgPort *capsport;

/**** functions ****/

/******** SetSliderGadg() ********/

void __saveds __asm LIBUA_SetSliderGadg(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR,
																					register __d0 int pos,
																					register __d1 int units,
																					register __a3 struct GadgetRecord *showGR,
																					register __d2 int addedVal )
{
struct GadgetRecord gadget;
UBYTE lace;
int knobWidth;
TEXT buf[5];
BOOL new=FALSE;

	knobWidth = (GR->x2-GR->x1) / units;
	if ( knobWidth < 2 )
	{
		knobWidth = SLIDER_W;
		new=TRUE;
	}

	LIBUA_ClearButton(window, GR, AREA_PEN);

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	if ( !new )
	{
		gadget.x1		= GR->x1 + 2 + pos*knobWidth;
		gadget.x2		= gadget.x1 + knobWidth - 1;
	}
	else
	{
		gadget.x1		= GR->x1 + 2 + pos;
		gadget.x2		= gadget.x1 + knobWidth;
	}
	gadget.y1			= GR->y1+lace+1;
	gadget.y2			= GR->y2-lace-1;
	gadget.color	= 0;
	gadget.type		= BUTTON_GADGET;
	gadget.ptr		= NULL;
	gadget.txt		= NULL;

	if (gadget.x1 < GR->x1+2)
	{
		gadget.x1 = GR->x1+2;
		gadget.x2 = gadget.x1 + knobWidth - 1;
	}
	else if (gadget.x2 > GR->x2-2)
	{
		gadget.x2 = GR->x2-2;
		gadget.x1 = gadget.x2-knobWidth + 1;
	}

	LIBUA_DrawGadget(window, &gadget);

	sprintf(buf, "%d", pos+addedVal );
	if ( showGR!=NULL )
		LIBUA_PrintInBox(	window, showGR, showGR->x1, showGR->y1, showGR->x2, showGR->y2,
											buf, PRINT_CENTERED);
}

/******** ProcessSliderGadg() ********/

void __saveds __asm LIBUA_ProcessSliderGadg(	register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __a2 int *pos,
																							register __d0 int units,
																							register __a3 struct GadgetRecord *showGR,
																							register __d1 struct EventData *CED,
																							register __a5 void (*hookfunc)(int *),
																							register __d2 int *data,
																							register __d3 int which,
																							register __d4 int addedVal )
{
struct GadgetRecord gadget;
UBYTE lace;
int i,knobWidth;
ULONG f;
BOOL new=FALSE;

	knobWidth = (GR->x2-GR->x1) / units;
	if ( knobWidth < 2 )
	{
		knobWidth = SLIDER_W;
		new=TRUE;
	}

	SafeSetWriteMask(window->RPort, 0x7);

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	if ( !new )
	{
		gadget.x1		= GR->x1 + 2 + *(pos)*knobWidth;
		gadget.x2		= gadget.x1 + knobWidth - 1;
	}
	else
	{
		gadget.x1		= GR->x1 + 2 + *pos;
		gadget.x2		= gadget.x1 + knobWidth;
	}
	gadget.y1			= GR->y1+lace+1;
	gadget.y2			= GR->y2-lace-1;
	gadget.color	= 0;
	gadget.type		= BUTTON_GADGET;
	gadget.ptr		= NULL;
	gadget.txt		= NULL;

	if (gadget.x1 < GR->x1+2)
	{
		gadget.x1 = GR->x1+2;
		gadget.x2 = gadget.x1 + knobWidth - 1;
	}
	else if (gadget.x2 > GR->x2-2)
	{
		gadget.x2 = GR->x2-2;
		gadget.x1 = gadget.x2-knobWidth + 1;
	}

	if (	(window->MouseX > gadget.x1) && (window->MouseX < gadget.x2) &&
				(window->MouseY > gadget.y1) && (window->MouseY < gadget.y2) )
	{
		i=*pos;
		DragSliderGadg(	window, GR, knobWidth, &i, units, showGR, new,
										hookfunc, data, which, addedVal );
		*pos=i;
	}
	else
	{
		i = (GR->x2-GR->x1)/units;
		if ( i>0 )
		{
			f = (ULONG)(window->MouseX-GR->x1)/(ULONG)i;
			if (	CED->Qualifier & IEQUALIFIER_LSHIFT ||
						CED->Qualifier & IEQUALIFIER_RSHIFT )
				*pos = (int)f;
			else
			{
				if (*pos > (int)f)	/* van bijv. 15 naar 14 */
					*pos = *pos - 1;
				else if (*pos < (int)f)	/* van bijv. 14 naar 15 */
					*pos = *pos + 1;
			}
			if (*pos<0)
				*pos=0;
			else if (*pos>(units-1))
				*pos=units-1;
			LIBUA_SetSliderGadg(window, GR, *pos, units, showGR, addedVal);
		}
	}

	SafeSetWriteMask(window->RPort, 0xff);
}

/******* DragSliderGadg() ********/

void DragSliderGadg(struct Window *window, struct GadgetRecord *GR,
										int knobWidth, int *pos, int units, struct GadgetRecord *showGR,
										BOOL new,
										void (*hookfunc)(int *), int *data, int which, int addedVal)
{
ULONG signals, sigmask;
BOOL loop,mouseMoved;
struct GadgetRecord gadget;
int diff;
UBYTE lace;
struct IntuiMessage *message;
TEXT buf[5];
struct EventData CED;
void (*func)(int *);

	if (window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	/**** hilite gadget ****/

	if ( !new )
	{
		gadget.x1		= GR->x1 + 2 + (*pos)*knobWidth;
		gadget.x2		= gadget.x1 + knobWidth - 1;
	}
	else
	{
		gadget.x1		= GR->x1 + 2 + *pos;
		gadget.x2		= gadget.x1 + knobWidth;
	}
	gadget.y1 		= GR->y1+lace+1;
	gadget.y2 		= GR->y2-lace-1;
	gadget.color 	= 0;
	gadget.type 	= BUTTON_GADGET;
	gadget.ptr 		= NULL;
	gadget.txt 		= NULL;

	if (gadget.x1 < GR->x1+2)
	{
		gadget.x1 = GR->x1+2;
		gadget.x2 = gadget.x1 + knobWidth - 1;
	}
	else if (gadget.x2 > GR->x2-2)
	{
		gadget.x2 = GR->x2-2;
		gadget.x1 = gadget.x2-knobWidth + 1;
	}

	LIBUA_DrawGadget(window, &gadget);
	LIBUA_InvertButton(window, &gadget);

	diff = gadget.x1 - window->MouseX;

	/**** handle events ****/

	LIBUA_SwitchMouseMoveOn(window);

	gadget.x1 = GR->x1+2;
	gadget.x2 = GR->x2-2;

	loop=TRUE;

	sigmask = 1L << capsport->mp_SigBit;
	while(loop)
	{
		signals = Wait(sigmask);
		if (signals & sigmask)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsport))
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
				SetDrMd(window->RPort, JAM1);
				my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));
				RectFill(window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
																(LONG)gadget.x2, (LONG)gadget.y2);
				gadget.x1 = window->MouseX + diff;
				if ( !new )
				{
					gadget.x2 = gadget.x1 + knobWidth - 1;
					*pos = (gadget.x1 - GR->x1 - 2) / knobWidth;
				}
				else
				{
					gadget.x2 = gadget.x1 + knobWidth;
					*pos = (gadget.x1 - GR->x1 - 2);
				}
				if (gadget.x1 < GR->x1+2)
				{
					gadget.x1 = GR->x1+2;
					if ( !new )
						gadget.x2 = gadget.x1 + knobWidth - 1;
					else
						gadget.x2 = gadget.x1 + knobWidth;
					*pos = 0;
				}
				else if (gadget.x2 > GR->x2-2)
				{
					gadget.x2 = GR->x2-2;
					if ( !new )
						gadget.x1 = gadget.x2-knobWidth + 1;
					else
						gadget.x1 = gadget.x2-knobWidth;
					*pos = units-1;
				}
				LIBUA_DrawGadget(window, &gadget);
				LIBUA_InvertButton(window, &gadget);

				/**** display value ****/

				sprintf(buf, "%d", (int)((*pos)+addedVal));
				if ( showGR!=NULL )
					LIBUA_PrintInBox(	window, showGR,
														showGR->x1, showGR->y1, showGR->x2, showGR->y2,
														buf, PRINT_CENTERED);

				if ( hookfunc )
				{
					data[which] = *(pos)+1;
					func = hookfunc;
					(*(func))(data);
				}
			}
		}
	}

	LIBUA_SwitchMouseMoveOff(window);

	SetDrMd(window->RPort, JAM1);
	my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));
	RectFill(	window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
						(LONG)gadget.x2, (LONG)gadget.y2);

	gadget.x1 		= GR->x1+2 + (*pos)*knobWidth;
	gadget.x2 		= gadget.x1 + knobWidth - 1;
	gadget.y1 		= GR->y1+lace+1;
	gadget.y2 		= GR->y2-lace-1;
	gadget.color 	= 0;
	gadget.type 	= BUTTON_GADGET;
	gadget.ptr 		= NULL;
	gadget.txt 		= NULL;

	if (gadget.x1 < GR->x1+2)
	{
		gadget.x1 = GR->x1+2;
		gadget.x2 = gadget.x1 + knobWidth - 1;
	}
	else if (gadget.x2 > GR->x2-2)
	{
		gadget.x2 = GR->x2-2;
		gadget.x1 = gadget.x2-knobWidth + 1;
	}

	SetDrMd(window->RPort, JAM1);
	my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));
	RectFill(	window->RPort, (LONG)gadget.x1, (LONG)gadget.y1,
						(LONG)gadget.x2, (LONG)gadget.y2);

	LIBUA_DrawGadget(window, &gadget);
}

/******** E O F ********/
