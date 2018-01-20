#include "mllib_includes.h"

#define LINEHEIGHT 14

/**** externals ****/

extern struct MsgPort *capsport;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** PrintNewList() ********/
/*
 * init the static of this function by a dummy call with top set to -1
 *
 */

void __saveds __asm LIBUA_PrintNewList(	register __a0 struct ScrollRecord *SR,
																				register __d0 int top,
																				register __a1 TEXT *ptrlist[],
																				register __d1 BOOL updateButtons )
{
int i,y,w,lineHeight;
static prevTop=0;

	if (top==-1)
	{
		prevTop=9999;
		return;
	}
	else if (top==prevTop)
		return;
	else
		prevTop=top;

	y=0;
	//lineHeight = (SR->GR->y2 - SR->GR->y1) / SR->numDisplay;
	lineHeight = LINEHEIGHT;
	if (SR->window->WScreen->ViewPort.Modes & LACE)
		lineHeight*=2;

	for(i=0; i<SR->numDisplay; i++)
	{
		if ( (top+i) >= SR->numEntries )
			break;

		if (SR->entryWidth==-1)
		{
			if ( SR->sublist )
				w = ((SR->GR->x2-SR->GR->x1)/2);
			else
				w = 0;
			PrintNewListLine(	SR->window, SR->GR,
												SR->GR->x1, SR->GR->y1+y,
												SR->GR->x2-w, SR->GR->y1+y + SR->window->RPort->TxHeight,
												(UBYTE *)ptrlist[top+i], updateButtons);
		}

		y += lineHeight;
	}
}

/******** ScrollNewList() ********/
/*
 * GR is the whole display box
 *
 */

void __saveds __asm LIBUA_ScrollNewList(	register __a0 struct ScrollRecord *SR,
																					register __a1 int *top,
																					register __a2 struct Gadget *g,
																					register __a3 TEXT *ptrlist[],
																					register __a5 struct EventData *oldCED )
{
ULONG signals, sigMask;
BOOL loop=TRUE;
struct IntuiMessage *message;
BOOL mouseMoved=FALSE;
int myTop;
struct EventData CED;
LONG f;

	if ( oldCED->Qualifier & IEQUALIFIER_LSHIFT || oldCED->Qualifier & IEQUALIFIER_RSHIFT )
	{
		if ( SR->numEntries > SR->numDisplay )
		{
			f = ( (oldCED->MouseY - g->TopEdge) * SR->numEntries) / g->Height;
			if ( f < 0 )
				f = 0;
			*top = f;
			if ( (*top+SR->numDisplay) > SR->numEntries )
				*top = SR->numEntries-SR->numDisplay;
			LIBUA_SetPropSlider(SR->window, g, SR->numEntries, SR->numDisplay, *top);
			LIBUA_PrintNewList(SR, *top, ptrlist, FALSE);
			return;
		}
	}

	myTop = *top;

	LIBUA_GetPropSlider(SR->window, g, SR->numEntries, SR->numDisplay, &myTop);

	LIBUA_PrintNewList(SR, myTop, ptrlist, FALSE);

	LIBUA_SwitchMouseMoveOn(SR->window);

	sigMask = 1L << capsport->mp_SigBit;

	while(loop)
	{
		signals = Wait( sigMask );
		if (signals & sigMask)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsport))
			{
				CED.Class	= message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED.Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				if (g->Flags & GFLG_SELECTED)
				{
					LIBUA_GetPropSlider(SR->window, g, SR->numEntries, SR->numDisplay, &myTop);
					LIBUA_PrintNewList(SR, myTop, ptrlist, FALSE);
					loop=TRUE;
 				}
				else
					loop=FALSE;
			}
		}
	}

	LIBUA_SwitchMouseMoveOff(SR->window);

	*top = myTop;
}

/******** SelectNewListLine() ********/
/*
 * returns -1 clicked outside box or lower than lowest line of text
 *
 * if !multiple, list may be NULL
 *
 */

int __saveds __asm LIBUA_SelectNewListLine(	register __a0 struct ScrollRecord *SR,
																						register __d0 int top,
																						register __a1 struct EventData *CED )
{
int lineHeight, line;
struct GadgetRecord gadget;
WORD x1,y1,x2,y2;

	SafeSetWriteMask(SR->window->RPort, 0x7);
	SetDrMd(SR->window->RPort, JAM2|COMPLEMENT);

	//lineHeight = (SR->GR->y2 - SR->GR->y1) / SR->numDisplay;
	lineHeight = LINEHEIGHT;
	if (SR->window->WScreen->ViewPort.Modes & LACE)
		lineHeight*=2;

	line = (CED->MouseY - SR->GR->y1) / lineHeight;
	if (SR->numEntries < SR->numDisplay)
	{
		if ( line-1 >= (SR->numEntries-1) )
			return(-1);
	}
	else if (line<0 || line>=SR->numDisplay)
		return(-1);

	// Hilite button

	x1 = SR->GR->x1;
	y1 = SR->GR->y1+line*lineHeight;
	x2 = SR->GR->x2;
	y2 = SR->GR->y1+line*lineHeight+SR->window->RPort->TxHeight;

	if (SR->window->WScreen->ViewPort.Modes & LACE)
	{
		y1 += 1;
		y2 += 5;
	}
	else
	{
		y1 -= 1;
		y2 += 1;
	}

	gadget.x1 = x1 + 4;
	gadget.y1 = y1 + 3;
	gadget.x2 = x2 - 4;
	gadget.y2 = y2 + 3;
	gadget.color = 1;
	gadget.type = BUTTON_GADGET;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	LIBUA_HiliteButton(SR->window, &gadget);

	SetDrMd(SR->window->RPort, JAM1);
	SafeSetWriteMask(SR->window->RPort, 0xff);

	return(line);
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** PrintNewListLine() ********/

void PrintNewListLine(struct Window *window, struct GadgetRecord *GR,
											int x1, int y1, int x2, int y2,
											STRPTR oristr, BOOL updateButtons)
{
int y;
struct GadgetRecord gadget;

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		y1 += 1;
		y2 += 5;
	}
	else
	{
		y1 -= 1;
		y2 += 1;
	}

	gadget.x1 = x1 + 4;
	gadget.y1 = y1 + 3;
	gadget.x2 = x2 - 4;
	gadget.y2 = y2 + 3;
	gadget.color = 1;
	gadget.type = HIBOX_REGION;
	gadget.ptr = NULL;
	gadget.txt = NULL;

	if ( !updateButtons )
		LIBUA_DrawTwoColorBox(window, &gadget);

	if ( oristr[0] == 'c' )
	{
		gadget.x1 = gadget.x1 + 6;
		gadget.y1 = gadget.y1 + 3;
		gadget.x2 = gadget.x1 + 16;
		gadget.y2 = gadget.y1 + 6;
		gadget.type = CHECK_GADGET;

		if (window->WScreen->ViewPort.Modes & LACE)
		{
			gadget.y1 += 2;
			gadget.y2 = gadget.y1 + 13;
		}

		UA_DrawCheck(window, &gadget);

		if ( oristr[1]==0x21 )
			LIBUA_InvertButton(window, &gadget);
	}
	else if ( oristr[0] == 'r' )
	{
		gadget.x1 = gadget.x1 + 6;
		gadget.y1 = gadget.y1 + 3;
		gadget.x2 = gadget.x1 + 16;
		gadget.y2 = gadget.y1 + 6;
		gadget.type = RADIO_GADGET;

		if (window->WScreen->ViewPort.Modes & LACE)
		{
			gadget.y1 += 2;
			gadget.y2 = gadget.y1 + 13;
		}

		UA_DrawRadio(window, &gadget);

		if ( oristr[1]==0x21 )
			LIBUA_InvertRadioButton(window, &gadget);
	}

	if ( !updateButtons )
	{
		y = y2 - y1 - window->RPort->TxHeight;
		y = y/2;
		y += (y1 + window->RPort->TxBaseline + 4);
		my_SetAPen(window, ChooseTextPen(window,GR));
		Move(window->RPort, (LONG)x1+30, (LONG)y);
		Text(window->RPort, &oristr[2], strlen(&oristr[2]));
	}
}

/******** E O F ********/
