#include "mllib_includes.h"

/**** externals ****/

extern struct MsgPort *capsport;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** PrintStandardList() ********/
/*
 * GR is the whole display box
 *
 * init the static of this function by a dummy call with top set to -1
 *
 */

void __saveds __asm LIBUA_PrintStandardList(register __a0 struct ScrollRecord *SR,
																						register __d1 int top,
																						register __a1 TEXT *ptrlist[])
{
int i,y,w,lineHeight,lace;
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
	lineHeight = (SR->GR->y2 - SR->GR->y1) / SR->numDisplay;

	if (SR->window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

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

			PrintStdListLine(	SR->window, SR->GR,
												SR->GR->x1, SR->GR->y1+y,
												SR->GR->x2-w, SR->GR->y1+y + SR->window->RPort->TxHeight +1,
												(UBYTE *)ptrlist[top+i]);

			if ( SR->sublist )
				PrintStdListLine(	SR->window, SR->GR,
													SR->GR->x1 + w - 3,
													SR->GR->y1+y,
													SR->GR->x2, SR->GR->y1+y + SR->window->RPort->TxHeight +1,
													(UBYTE *)SR->sublist[top+i]);
		}
		else
			PrintStdListLine(	SR->window, SR->GR,
												SR->GR->x1, SR->GR->y1+y,
												SR->GR->x2, SR->GR->y1+y + SR->window->RPort->TxHeight +1,
												SR->list+((top+i)*SR->entryWidth));

		y += SR->window->RPort->TxHeight;

		if ( SR->selectionList!=NULL && *(SR->selectionList+top+i) == 1 )
		{
			SafeSetWriteMask(SR->window->RPort, 0x7);
			SetDrMd(SR->window->RPort, JAM2|COMPLEMENT);

			RectFill(	SR->window->RPort,
								SR->GR->x1+2,
								SR->GR->y1+2 + lineHeight*i,
								SR->GR->x2-2,
								SR->GR->y1+3 + lace + SR->window->RPort->TxBaseline + lineHeight*i);

			SetDrMd(SR->window->RPort, JAM1);
			SafeSetWriteMask(SR->window->RPort, 0xff);
		}
	}
}

/******** ScrollStandardList() ********/
/*
 * GR is the whole display box
 *
 */

void __saveds __asm LIBUA_ScrollStandardList(	register __a0 struct ScrollRecord *SR,
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
			LIBUA_PrintStandardList(SR, *top, ptrlist);
			return;
		}
	}

	myTop = *top;

	LIBUA_GetPropSlider(SR->window, g, SR->numEntries, SR->numDisplay, &myTop);

	LIBUA_PrintStandardList(SR, myTop, ptrlist);

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
					LIBUA_PrintStandardList(SR, myTop, ptrlist);
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

/******** SelectStandardListLine() ********/
/*
 * GR is the whole display box
 *
 * returns -1 clicked outside box or lower than lowest line of text
 *
 * if !multiple, list may be NULL
 *
 */

int __saveds __asm LIBUA_SelectStandardListLine(register __a0 struct ScrollRecord *SR,
																								register __d0 int top,
																								register __d1 BOOL multiple,
																								register __a1 struct EventData *CED,
																								register __d2 BOOL deselect,
																								register __d3 BOOL select )
{
int lineHeight, line, i, lace, didLine=-1;
UBYTE old;
BOOL rectfill;

	lineHeight = (SR->GR->y2 - SR->GR->y1) / SR->numDisplay;

	line = (CED->MouseY - SR->GR->y1) / lineHeight;
	if (SR->numEntries < SR->numDisplay)
	{
		if ( line-1 >= (SR->numEntries-1) )
			return(-1);
	}
	else if (line<0 || line>=SR->numDisplay)
		return(-1);

	SafeSetWriteMask(SR->window->RPort, 0x7);
	SetDrMd(SR->window->RPort, JAM2|COMPLEMENT);

	/**** I know, this breaks the rule that this routine should be	****/
	/**** a general purpose one but no one else uses DIR_PRECODE		****/
	/**** and it save a lot of code elsewhere. Why is this? -->			****/
	/**** When selecting multiple files, you can select more than		****/
	/**** one file but only one dir.																****/

	/**** ONCE AGAIN: USE MULTIPLE ONLY WITH DIRS !!!!!!!!!					****/

	if (SR->window->WScreen->ViewPort.Modes & LACE)
		lace=1;
	else
		lace=0;

	if (multiple && SR->list)
	{
		/**** deselect all items if a dir gets chosen ****/

		if ( *(SR->list+((line+top)*SIZE_FILENAME)) == DIR_PRECODE )
			multiple = FALSE;

		/**** deselect selected dir if a file gets chosen ****/

		for(i=0; i<SR->numEntries; i++)
		{
			if ( 	(*(SR->selectionList+i) == 1) &&
						( *(SR->list+i*SIZE_FILENAME) == DIR_PRECODE) )
			{
				*(SR->selectionList+i) = 0;
				/**** if selected dir is visible, remove it right away ****/
				if ( (i >= top) && (i < (top+SR->numDisplay)) )
				{
					RectFill(	SR->window->RPort,
										SR->GR->x1+2,
										SR->GR->y1+2 + lineHeight*(i-top),
										SR->GR->x2-2,
										SR->GR->y1+3 + lace+SR->window->RPort->TxBaseline + lineHeight*(i-top));
				}
				break;	/* only one dir can be involved */
			}
		}
	}

	if (!multiple)
	{
		for(i=0; i<SR->numEntries; i++)
		{
			if ( SR->selectionList && *(SR->selectionList+i)==1 )
			{
				didLine=i;
				*(SR->selectionList+i) = 0;
				if ( (i >= top) && (i < (top+SR->numDisplay)) )
				{
					RectFill(	SR->window->RPort,
										SR->GR->x1+2,
										SR->GR->y1+2+lineHeight*(i-top),
										SR->GR->x2-2,
										SR->GR->y1+3+lace+SR->window->RPort->TxBaseline+lineHeight*(i-top));
				}
			}
		}
	}

	if ( SR->selectionList!=NULL )
		old = *(SR->selectionList+top+line);

	if ( select && deselect )
	{
		// if didLine != -1 -> a selected line has been deselected
		// don't select the same line again, any other is fine.

		if ( SR->selectionList && didLine!=(top+line) )
		{
			*(SR->selectionList+top+line) = 1;

			RectFill(	SR->window->RPort,
								SR->GR->x1+2,
								SR->GR->y1+2+lineHeight*line,
								SR->GR->x2-2,
								SR->GR->y1+3+lace+SR->window->RPort->TxBaseline+lineHeight*line);
		}
		SetDrMd(SR->window->RPort, JAM1);
		SafeSetWriteMask(SR->window->RPort, 0xff);
		return(line);
	}

	/**** sometimes you can only select, sometimes only deselect ****/

	rectfill = FALSE;

	if ( SR->selectionList!=NULL )
	{
		if ( select )		// you may only select
		{
			if (old==0)		// is currently NOT selected
			{
				*(SR->selectionList+top+line) = 1;
				if ( old != *(SR->selectionList+top+line) )
					rectfill = TRUE;
			}
		}

		if ( deselect )	// you may only deselect
		{
			if (old==1)		// is currently selected
			{
				*(SR->selectionList+top+line) = 0;
				if ( old != *(SR->selectionList+top+line) )
					rectfill = TRUE;
			}
		}

		if ( !select && !deselect )
		{
			if ( *(SR->selectionList+top+line) == 1 )
				*(SR->selectionList+top+line) = 0;
			else
				*(SR->selectionList+top+line) = 1;
			rectfill = TRUE;
		}
	}
	else
	{
		rectfill=TRUE;
	}

	if ( rectfill )
	{
		RectFill(	SR->window->RPort,
							SR->GR->x1+2,
							SR->GR->y1+2+lineHeight*line,
							SR->GR->x2-2,
							SR->GR->y1+3+lace+SR->window->RPort->TxBaseline+lineHeight*line);
	}

	SetDrMd(SR->window->RPort, JAM1);
	SafeSetWriteMask(SR->window->RPort, 0xff);

	return(line);
}

/******** ShortenString() ********/

void __saveds __asm LIBUA_ShortenString(register __a0 struct RastPort *rp,
																				register __a1 STRPTR str,
																				register __d0 int numPixels)
{
int i;

	i=strlen(str);
	if (i==0)
	{
		strcpy(str, "Untitled");	// SAS function, survived strcpy
		return;
	}
	while(i!=0 && TextLength(rp, str, i-1) > numPixels)
	{
		*(str+i-1) = '\0';
		i--;
	}
}

/******** ShortenStringFront() ********/

void __saveds __asm LIBUA_ShortenStringFront(	register __a0 struct RastPort *rp,
																							register __a1 STRPTR str,
																							register __d0 int numPixels)
{
int i;

	i=strlen(str)-1;
	if (i<1)
		return;
	while(TextLength(rp, str, i) > numPixels)
	{
		strcpy(str, str+1);	// SAS function, survived strcpy
		i--;
	}
}

/******** PrintInBox() ********/
/*
 *	mode = PRINT_LEFTPART 	1
 *	mode = PRINT_RIGHTPART 	2
 *	mode = PRINT_CENTERED		3
 *
 */

void __saveds __asm LIBUA_PrintInBox(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 int x1,
																			register __d1 int y1,
																			register __d2 int x2,
																			register __d3 int y2,
																			register __a2 STRPTR oristr,
																			register __d4 int mode)
{
TEXT str[256];
int x,y,len;

	stccpy(str, oristr, 256);	// SAS function

	my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));

	if (window->WScreen->ViewPort.Modes & LACE)
		RectFill(window->RPort, x1+2, y1+2, x2-2, y2-2);
	else
		RectFill(window->RPort, x1+2, y1+1, x2-2, y2-1);

	if (mode==PRINT_LEFTPART)
	{
		LIBUA_ShortenString(window->RPort, str, (x2-x1)-16);
		x = x1+4;
	}
	else if (mode==PRINT_RIGHTPART)
	{
		LIBUA_ShortenStringFront(window->RPort, str, (x2-x1)-16);
		x = x1+4;
	}
	else
	{
		len = TextLength(window->RPort, str, strlen(str));
		x = x2-x1;
		x = (x-len)/2;
		x = x1+x;
	}

	y = y2 - y1 - window->RPort->TxHeight;
	y = y/2;
	y += (y1 + window->RPort->TxBaseline + 1);

	if (	GR->type==STRING_GADGET || GR->type==SPECIAL_STRING_GADGET ||
				GR->type==BUTTON_GADGET || GR->type==HIBOX_REGION )
		my_SetAPen(window, ChooseTextPen(window,GR));
	else
		my_SetAPen(window, ChoosePen(window,GR,LO_PEN));

	Move(window->RPort, (LONG)x, (LONG)y);
	Text(window->RPort, str, strlen(str));
}

/******** printSeveralLines() ********/

void __saveds __asm LIBUA_printSeveralLines(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR,
																						register __d0 int x,
																						register __d1 int y,
																						register __d2 int width,
																						register __d3 int height,
																						register __a2 STRPTR str)
{
char *strPtr;
TEXT subStr[256], smallStr[2];	// see len check
int numChars, len, oldX, i;

	len = strlen(str);
	for(i=0; i<len; i++)
		if ( *(str+i)==0x27 )
			*(str+i)=0x95;

	oldX = x;
	smallStr[1]='\0';

	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));

	Move(window->RPort, x, y);

	strPtr = str;
	while(1)
	{
		numChars = stcarg(strPtr, " ':/");			// SAS function
		stccpy(subStr, strPtr, numChars+1);

		if (numChars>0)
		{
			smallStr[0] = *(strPtr+numChars);
			strcat(subStr,smallStr);
		}

		len = strlen(subStr);	// SAS function

		if (len>256)
			LIBUA_WarnUser(1002);

		if ( (x + TextLength(window->RPort, subStr, len)) > width )
		{
			x = oldX;
			y += window->RPort->TxBaseline+4;
			if (y>height)
				break;
		}

		Move(window->RPort, x, y);
		Text(window->RPort, subStr, strlen(subStr));

		x += TextLength(window->RPort, subStr, strlen(subStr));

		strPtr += (numChars+1);

		if (*(strPtr-1)==0)
			break;
	}
}

/******** SwitchMouseMoveOn() ********/

void __saveds __asm LIBUA_SwitchMouseMoveOn(register __a0 struct Window *window)
{
	LIBUA_SwitchFlagsOn(window,IDCMP_MOUSEMOVE);
	Forbid();
	window->Flags |= WFLG_REPORTMOUSE;
	Permit();
}

/******** SwitchMouseMoveOff() ********/

void __saveds __asm LIBUA_SwitchMouseMoveOff(register __a0 struct Window *window)
{
	LIBUA_SwitchFlagsOff(window,IDCMP_MOUSEMOVE);
	Forbid();
	window->Flags &= ~WFLG_REPORTMOUSE;
	Permit();
}

/******** GetPropSlider() ********/

void __saveds __asm LIBUA_GetPropSlider(register __a0	struct Window *window,
																				register __a1	struct Gadget *g,
																				register __d0	int numEntries,
																				register __d1	int numDisplay,
																				register __a2	int *top)
{
struct PropInfo *p;
ULONG longTop, longNE, longND;

	longNE = (ULONG)numEntries;
	longND = (ULONG)numDisplay;

	p = (struct PropInfo *)g->SpecialInfo;
	if (numEntries > numDisplay)
		longTop = (p->VertPot * (longNE-longND))/MAXBODY;
	else
		longTop = 0L;

	*top = (int)longTop;
}

/******** GetHorizPropSlider() ********/

void __saveds __asm LIBUA_GetHorizPropSlider(	register __a0	struct Window *window,
																							register __a1	struct Gadget *g,
																							register __d0	int numEntries,
																							register __d1	int numDisplay,
																							register __a2	int *top)
{
struct PropInfo *p;
ULONG longTop, longNE, longND;

	longNE = (ULONG)numEntries;
	longND = (ULONG)numDisplay;

	p = (struct PropInfo *)g->SpecialInfo;
	if (numEntries > numDisplay)
		longTop = (p->HorizPot * (longNE-longND))/MAXBODY;
	else
		longTop = 0L;

	*top = (int)longTop;
}

/******** InitPropSlider() ********/

void __saveds __asm LIBUA_InitPropSlider(	register __a0	struct Window *window,
																					register __a1	struct Gadget *g,
																					register __d0	ULONG numEntries,
																					register __d1	ULONG numDisplay,
																					register __d2	LONG topEntry)
{
ULONG value1, value2;

	if (numEntries<=0)
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEVERT|PROPBORDERLESS, NULL, NULL, MAXBODY, MAXBODY, 1L);
	else
	{	
		if (numEntries > numDisplay)
		{
			value1 = (MAXBODY*topEntry) / numEntries;
			value2 = (MAXBODY*numDisplay) / numEntries;
		}
		else
		{
			value1 = NULL;
			value2 = MAXBODY;
		}
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEVERT|PROPBORDERLESS,
									NULL, value1, MAXBODY, value2, 1L);
	}

	RefreshGList(g, window, NULL, 1);
}

/******** InitHorizPropSlider() ********/

void __saveds __asm LIBUA_InitHorizPropSlider(register __a0	struct Window *window,
																							register __a1	struct Gadget *g,
																							register __d0	ULONG numEntries,
																							register __d1	ULONG numDisplay,
																							register __d2	LONG topEntry)
{
ULONG value1, value2;

	if (numEntries<=0)
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS, MAXBODY, MAXBODY, NULL, NULL, 1L);
	else
	{	
		if (numEntries > numDisplay)
		{
			value1 = (MAXBODY*topEntry) / numEntries;
			value2 = (MAXBODY*numDisplay) / numEntries;
		}
		else
		{
			value1 = NULL;
			value2 = MAXBODY;
		}
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									value1, NULL, value2, MAXBODY, 1L);
	}

	RefreshGList(g, window, NULL, 1);
}

/******** SetPropSlider() ********/

void __saveds __asm LIBUA_SetPropSlider(register __a0	struct Window *window,
																				register __a1	struct Gadget *g,
																				register __d0	ULONG numEntries,
																				register __d1	ULONG numDisplay,
																				register __d2	LONG topEntry)
{
struct PropInfo *p;
ULONG f1,f2;

	if (numEntries<=0)
	{
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEVERT|PROPBORDERLESS,
									NULL, NULL, MAXBODY, MAXBODY, 1L);
		return;
	}

	p = (struct PropInfo *)g->SpecialInfo;

	if (numEntries > numDisplay)
	{
		if ( topEntry >= (numEntries-numDisplay))
			f1 = MAXBODY;
		else
			f1 = (MAXBODY*topEntry+(numEntries-numDisplay)) / (numEntries-numDisplay);

		f2 = MAXBODY * ( (numDisplay*MAXBODY)/numEntries );
		f2 /= MAXBODY;

		NewModifyProp(g, window, NULL, AUTOKNOB|FREEVERT|PROPBORDERLESS,
									0L, f1, MAXBODY, f2, 1L);
	}
	else
	{
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEVERT|PROPBORDERLESS,
									NULL, NULL, MAXBODY, MAXBODY, 1L);
	}

	RefreshGList(g, window, NULL, 1);
}

/******** SetHorizPropSlider() ********/

void __saveds __asm LIBUA_SetHorizPropSlider(	register __a0 struct Window *window,
																							register __a1 struct Gadget *g,
																							register __d0	ULONG numEntries,
																							register __d1	ULONG numDisplay,
																							register __d2	LONG topEntry)
{
struct PropInfo *p;
ULONG f1,f2;

	if (numEntries<=0)
	{
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									MAXBODY, MAXBODY, NULL, NULL, 1L);
		return;
	}

	p = (struct PropInfo *)g->SpecialInfo;

	if (numEntries > numDisplay)
	{
		if ( topEntry >= (numEntries-numDisplay))
			f1 = MAXBODY;
		else
			f1 = (MAXBODY*topEntry+(numEntries-numDisplay)) / (numEntries-numDisplay);

		f2 = MAXBODY * ( (numDisplay*MAXBODY)/numEntries );
		f2 /= MAXBODY;

		NewModifyProp(g, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									f1, 0L, f2, MAXBODY, 1L);
	}
	else
	{
		NewModifyProp(g, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									MAXBODY, MAXBODY, 0L, 0L, 1L);
	}

	RefreshGList(g, window, NULL, 1);
}

/******** SwitchFlagsOn() ********/

void __saveds __asm LIBUA_SwitchFlagsOn(	register __a0 struct Window *window,
																					register __d0 ULONG flags )
{
	Forbid();
	window->IDCMPFlags |= flags;
	if ( window->IDCMPFlags!=NULL )
		ModifyIDCMP(window, window->IDCMPFlags);
	Permit();
}

/******** SwitchFlagsOff() ********/

void __saveds __asm LIBUA_SwitchFlagsOff(	register __a0 struct Window *window,
																					register __d0 ULONG flags )
{
	Forbid();
	window->IDCMPFlags &= ~flags;
	if ( window->IDCMPFlags!=NULL )
		ModifyIDCMP(window, window->IDCMPFlags);
	Permit();
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** PrintStdListLine() ********/

void PrintStdListLine(struct Window *window, struct GadgetRecord *GR,
											int x1, int y1, int x2, int y2,
											STRPTR oristr)
{
TEXT str[256];
int y;

	stccpy(str, oristr, 256); //SAS function

	my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));

	RectFill(window->RPort, x1+2, y1+2, x2-2, y2);

	LIBUA_ShortenString(window->RPort, str, (x2-x1)-20);

	y = y2 - y1 - window->RPort->TxHeight;
	y = y/2;
	y += (y1 + window->RPort->TxBaseline + 2);

	my_SetAPen(window, ChooseTextPen(window,GR));

	Move(window->RPort, (LONG)x1+4, (LONG)y);
	Text(window->RPort, str, strlen(str));
}

/******** E O F ********/
