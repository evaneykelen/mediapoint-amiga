#include "mllib_includes.h"

extern struct MsgPort *capsport;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** Monitor_PopUp() ********/

void __saveds __asm LIBUA_Monitor_PopUp(register __a0 struct PopUpRecord *PUR)
{
ULONG sigmask, signals;
struct IntuiMessage *message;
BOOL loop=TRUE;
struct RastPort *rp;
struct GadgetRecord *GR;
struct EventData CED;
BOOL mouseMoved, dontUse=FALSE;
int y,prev,height,j,add,px,py;
int old_active;

	if ( !PUR->ptr )
		return;

	old_active = PUR->active;

	rp = PUR->window->RPort;
	GR = PUR->GR;
	height = rp->TxHeight-1;
	if ( PUR->fit < PUR->number )
		j=PUR->fit;
	else
		j=PUR->number;
	sigmask = 1L << capsport->mp_SigBit;
	add = 2 + PUR->window->RPort->TxHeight;

	// calc which line must be highlighted

	y = PUR->window->MouseY;
	y = y / rp->TxHeight;
	if ( y<0 )
		y=0;
	else if ( y>(j-1))
		y=j-1;

	// highlight line

	SafeSetWriteMask(rp, 0x3);
	SetDrMd(rp, JAM2 | COMPLEMENT);
	RectFill(rp, 5,
							 add+
							 rp->TxHeight*y,
							 GR[0].x2-5,
							 add+height+
							 rp->TxHeight*y);
	SetDrMd(rp, JAM2);
	SafeSetWriteMask(rp, 0xff);
	prev = y;

	px = TextLength(PUR->window->RPort, "", 2);
	px = (GR[0].x2 - px) / 2; 

	/* START: PRINT ARROWS */
	if ( PUR->window->WScreen->ViewPort.Modes & LACE )
		py = rp->TxBaseline+1+add-rp->TxHeight;
	else
		py = rp->TxBaseline+add-rp->TxHeight;
	if ( PUR->top==0 )
		SetAPen(rp, 5);
	else
		SetAPen(rp, 1);
	SetDrMd(rp, JAM1);
	Move(PUR->window->RPort, (LONG)px, (LONG)py);
	Text(PUR->window->RPort, "", 1);

	if ( PUR->window->WScreen->ViewPort.Modes & LACE )
		py = 1+add+rp->TxHeight*(PUR->fit+1);
	else
		py = add+rp->TxHeight*(PUR->fit+1);
	if ( PUR->top < PUR->number-PUR->fit )
		SetAPen(rp, 1);
	else
		SetAPen(rp, 5);
	SetDrMd(rp, JAM1);
	Move(PUR->window->RPort, (LONG)px, (LONG)py);
	Text(PUR->window->RPort, "", 1);
	/* END: PRINT ARROWS */

	// event loop

	LIBUA_SwitchMouseMoveOn(PUR->window);
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
				CED.MouseX = message->MouseX; 
				CED.MouseY = message->MouseY; 
				ReplyMsg((struct Message *)message);
				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						else if (CED.Code == MENUUP)
						{
							loop=FALSE;
							dontUse=TRUE;
						}
						break;
					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
					case IDCMP_RAWKEY:
						loop=FALSE;
						dontUse=TRUE;
						break;
				}
			}
			if (loop && mouseMoved)
			{
/*
				if (	CED.MouseX < 0 || CED.MouseY < 0 ||
							CED.MouseX > PUR->window->Width || CED.MouseY > PUR->window->Height )
*/

				if (	CED.MouseX < 0 || CED.MouseX > PUR->window->Width )
				{
					if ( prev != -1 )
					{
						SafeSetWriteMask(rp, 0x3);
						SetDrMd(rp, JAM2 | COMPLEMENT);
						RectFill(rp, 5,
												 add+
												 rp->TxHeight*prev,
												 GR[0].x2-5,
												 add+height+
												 rp->TxHeight*prev);
						SetDrMd(rp, JAM2);
						SafeSetWriteMask(rp, 0xff);
						prev=-1;
						dontUse=TRUE;
					}
				}
				else	// mouse inside window
				{
					dontUse=FALSE;

					if ( PUR->fit < PUR->number )
					{
						if ( CED.MouseY < rp->TxHeight )	// need to scroll
						{
							if ( PUR->top>0 )
							{
								// first dehighlight line

								if ( prev != -1 )
								{
									SafeSetWriteMask(rp, 0x3);
									SetDrMd(rp, JAM2 | COMPLEMENT);
									RectFill(rp, 5,
															 add+
															 rp->TxHeight*prev,
															 GR[0].x2-5,
															 add+height+
															 rp->TxHeight*prev);
									SetDrMd(rp, JAM2);
									SafeSetWriteMask(rp, 0xff);
									prev=-1;
								}

								// scroll window contents down, creating an empty line at the top.
		
								SetAPen(rp, 1);
								SetBPen(rp, 5);
								SetDrMd(rp, JAM1);
	
								if ( PUR->window->WScreen->ViewPort.Modes & LACE )
									ScrollRaster(	rp, 0, -(rp->TxHeight),
																2, add, PUR->window->Width-3, PUR->window->Height-1*rp->TxHeight-rp->TxBaseline+6 );
								else
									ScrollRaster(	rp, 0, -(rp->TxHeight),
																2, add, PUR->window->Width-3, PUR->window->Height-1*rp->TxHeight-rp->TxBaseline+3 );
								PUR->top--;
	
								// print (new) first line
	
								if ( PUR->top==PUR->active )
									SetAPen(rp, 2);
	
								if ( PUR->window->WScreen->ViewPort.Modes & LACE )
									y = rp->TxBaseline+1+add;
								else
									y = rp->TxBaseline+add;
	
								Move(rp, (LONG)5, (LONG)y);
								Text(rp, PUR->ptr+PUR->top*PUR->width, strlen(PUR->ptr+PUR->top*PUR->width));
							}
						}
	
						if ( CED.MouseY > (PUR->window->Height-rp->TxHeight) )	// need to scroll
						{
							if ( PUR->top < PUR->number-PUR->fit )
							{
								// first dehighlight line
	
								if ( prev != -1 )
								{
									SafeSetWriteMask(rp, 0x3);
									SetDrMd(rp, JAM2 | COMPLEMENT);
									RectFill(rp, 5,
															 add+
															 rp->TxHeight*prev,
															 GR[0].x2-5,
															 add+height+
															 rp->TxHeight*prev);
									SetDrMd(rp, JAM2);
									SafeSetWriteMask(rp, 0xff);
									prev=-1;
								}
		
								// scroll window contents up, creating an empty line at the bottom.
		
								SetAPen(rp, 1);
								SetBPen(rp, 5);
								SetDrMd(rp, JAM1);
		
								ScrollRaster(	rp, 0, rp->TxHeight,
														2, add, PUR->window->Width-3, PUR->window->Height-rp->TxHeight );
								PUR->top++;
	
								// print (new) last line
	
								if ( (PUR->top+PUR->fit-1)==PUR->active )
									SetAPen(rp, 2);
	
								if ( PUR->window->WScreen->ViewPort.Modes & LACE )
									y = rp->TxBaseline+1+add+(rp->TxHeight*(PUR->fit-1));
								else
									y = rp->TxBaseline+add+(rp->TxHeight*(PUR->fit-1));
	
								Move(rp, (LONG)5, (LONG)y);
								Text(rp, PUR->ptr+(PUR->top+PUR->fit-1)*PUR->width,
											strlen(PUR->ptr+(PUR->top+PUR->fit-1)*PUR->width));
							}
						}
					}
	
					// calc hightlighted line
	
					y = CED.MouseY - add;
					y = y / rp->TxHeight;
					if ( y<0 )
						y=0;
					else if ( y>(j-1))
						y=j-1;
	
					// highlight line
	
					if ( y!=prev )
					{
						SafeSetWriteMask(rp, 0x3);
						SetDrMd(rp, JAM2 | COMPLEMENT);
	
						if ( prev!=-1 )
						{
							RectFill(rp, 5,
													 add+
													 rp->TxHeight*prev,
													 GR[0].x2-5,
													 add+height+
													 rp->TxHeight*prev);
						}
		
						RectFill(rp, 5,
												 add+
												 rp->TxHeight*y,
												 GR[0].x2-5,
												 add+height+
												 rp->TxHeight*y);
	
						SetDrMd(rp, JAM2);
						SafeSetWriteMask(rp, 0xff);
						prev = y;
					}
	
					/* START: PRINT ARROWS */
					if ( PUR->window->WScreen->ViewPort.Modes & LACE )
						py = rp->TxBaseline+1+add-rp->TxHeight;
					else
						py = rp->TxBaseline+add-rp->TxHeight;
					if ( PUR->top==0 )
						SetAPen(rp, 5);
					else
						SetAPen(rp, 1);
					SetDrMd(rp, JAM1);
					Move(PUR->window->RPort, (LONG)px, (LONG)py);
					Text(PUR->window->RPort, "", 1);
	
					if ( PUR->window->WScreen->ViewPort.Modes & LACE )
						py = 1+add+rp->TxHeight*(PUR->fit+1);
					else
						py = add+rp->TxHeight*(PUR->fit+1);
					if ( PUR->top < PUR->number-PUR->fit )
						SetAPen(rp, 1);
					else
						SetAPen(rp, 5);
					SetDrMd(rp, JAM1);
					Move(PUR->window->RPort, (LONG)px, (LONG)py);
					Text(PUR->window->RPort, "", 1);
					/* END: PRINT ARROWS */
				}
			}
		}
	}
	LIBUA_SwitchMouseMoveOff(PUR->window);

	if ( dontUse )
		PUR->active = old_active;
	else
		PUR->active = y + PUR->top;
}

/******** OpenPopUpWindow() ********/

BOOL __saveds __asm LIBUA_OpenPopUpWindow(register __a0 struct Window *onWindow,
																					register __a1 struct GadgetRecord *onGR,
																					register __a2 struct PopUpRecord *PUR)
{
struct GadgetRecord *GR;
int i,j,w1,w2,y,n;

	if ( !PUR->ptr )
		return(FALSE);

	GR = PUR->GR;

	w1=0;
	for(i=0; i<PUR->number; i++)
	{
		w2=TextLength(onWindow->RPort, PUR->ptr+i*PUR->width, strlen(PUR->ptr+i*PUR->width));
		if (w2>w1)
			w1=w2;
	}

	// 0

	if ( (w1+15) > GR[0].x2 )
		GR[0].x2 = w1+15;				// DIMENSIONS

	if ( GR[0].x2 > (onWindow->WScreen->Width/2) )
		GR[0].x2 = onWindow->WScreen->Width/2;

	GR[0].y2 = onWindow->RPort->TxHeight*(2+PUR->number);
	if ( onWindow->WScreen->ViewPort.Modes & LACE )
		GR[0].y2 += 10;
	else
		GR[0].y2 += 5;

	if ( GR[0].y2 > (onWindow->WScreen->Height/2) )
	{
		n = (onWindow->WScreen->Height/2)/onWindow->RPort->TxHeight;
		GR[0].y2 = onWindow->RPort->TxHeight*n;
		if ( onWindow->WScreen->ViewPort.Modes & LACE )
			GR[0].y2 += 10;
		else
			GR[0].y2 += 5;
	}

	// 1

	GR[1].x2 = GR[0].x2-1;	// DBL_BORDER
	if ( onWindow->WScreen->ViewPort.Modes & LACE )
		GR[1].y2 = GR[0].y2-2;
	else
		GR[1].y2 = GR[0].y2-1;

	// 2

	GR[2].x2 = onGR->x1+onWindow->LeftEdge+21;	// PREFPOS	x pos
	GR[2].y2 = onGR->y1+onWindow->TopEdge;			// 					y pos

	if ( (GR[2].x2 + GR[0].x2) > onWindow->WScreen->Width )
		GR[2].x2 = onWindow->WScreen->Width - GR[0].x2;
	if ( (GR[2].y2 + GR[0].y2) > onWindow->WScreen->Height )
		GR[2].y2 = onWindow->WScreen->Height - GR[0].y2;

	if ( GR[2].x2 < 0 )
		GR[2].x2 = onWindow->WScreen->Width;
	if ( GR[2].y2 < 0 )
		GR[2].y2 = onWindow->WScreen->Height;

	if ( onWindow->WScreen->ViewPort.Modes & LACE )
		GR[0].x1 = 1;
	else
		GR[0].x1 = 0;	// fool ScaleGadgetlist in winreq

	if ( onWindow->WScreen->ViewPort.Modes & LACE )
		PUR->fit = (GR[0].y2 - 10) / onWindow->RPort->TxHeight;
	else	
		PUR->fit = (GR[0].y2 - 5) / onWindow->RPort->TxHeight;
	PUR->fit-=2;

	PUR->window = LIBUA_OpenRequesterWindow(onWindow, PUR->GR, STDCOLORS);
	if ( PUR->window==NULL)
		return(FALSE);

	LIBUA_DrawGadgetList(PUR->window, PUR->GR);

	SetAPen(PUR->window->RPort, 1);
	SetDrMd(PUR->window->RPort, JAM1);
	if ( onWindow->WScreen->ViewPort.Modes & LACE )
		y = PUR->window->RPort->TxBaseline + 3;
	else
		y = PUR->window->RPort->TxBaseline + 2;
	if ( PUR->fit < PUR->number )
		j=PUR->fit;
	else
		j=PUR->number;
	PUR->top=0;
	if ( PUR->active >= PUR->fit )
		PUR->top = PUR->active;
	if ( PUR->top<0 )
		PUR->top=0;
	if ( PUR->top >= PUR->number-PUR->fit+1 )
		PUR->top = PUR->number-PUR->fit;

	y=y+PUR->window->RPort->TxHeight;	// NEW

	for(i=0; i<j; i++)
	{
		if (PUR->top+i==PUR->active)
			SetAPen(PUR->window->RPort, 2);
		Move(PUR->window->RPort, (LONG)5, (LONG)y);
		Text(PUR->window->RPort, PUR->ptr+(PUR->top+i)*PUR->width, strlen(PUR->ptr+(PUR->top+i)*PUR->width));
		y += onWindow->RPort->TxHeight;
		if (PUR->top+i==PUR->active)
			SetAPen(PUR->window->RPort, 1);
	}

	return(TRUE);
}

/******** ClosePopUpWindow() ********/

void __saveds __asm LIBUA_ClosePopUpWindow(register __a0 struct PopUpRecord *PUR)
{
	LIBUA_CloseRequesterWindow(PUR->window, STDCOLORS);
}

/******** PrintPopUpChoice() ********/

void __saveds __asm LIBUA_PrintPopUpChoice(	register __a0 struct Window *onWindow,
																						register __a1 struct GadgetRecord *onGR,
																						register __a2 struct PopUpRecord *PUR)
{
int y;
TEXT str[128];

	LIBUA_ClearButton(onWindow, onGR, AREA_PEN);

	y = onGR->y2 - onGR->y1 - onWindow->RPort->TxHeight;
	y = y/2;
	y += onGR->y1 + onWindow->RPort->TxBaseline + 1;

	my_SetAPen(onWindow, ChooseTextPen(onWindow,onGR));
	SetDrMd(onWindow->RPort, JAM1);
	Move(onWindow->RPort, (LONG)onGR->x1+6, (LONG)y);
	Text(onWindow->RPort, " \0", 2);

	if ( PUR->ptr )
	{
		strcpy(str, PUR->ptr+PUR->active*PUR->width);
		LIBUA_ShortenString(onWindow->RPort, str, onGR->x2-onGR->x1-16-25);
		Move(onWindow->RPort, (LONG)onGR->x1+25, (LONG)y);
		Text(onWindow->RPort, str, strlen(str));
	}
}

/******** PrintPopUpChoice2() ********/

void __saveds __asm LIBUA_PrintPopUpChoice2(	register __a0 struct Window *onWindow,
																							register __a1 struct GadgetRecord *onGR,
																							register __a2 char *txt )
{
int y;
TEXT str[128];

	LIBUA_ClearButton(onWindow, onGR, AREA_PEN);

	y = onGR->y2 - onGR->y1 - onWindow->RPort->TxHeight;
	y = y/2;
	y += onGR->y1 + onWindow->RPort->TxBaseline + 1;

	my_SetAPen(onWindow, ChooseTextPen(onWindow,onGR));
	SetDrMd(onWindow->RPort, JAM1);
	Move(onWindow->RPort, (LONG)onGR->x1+6, (LONG)y);
	Text(onWindow->RPort, " \0", 2);

	if ( txt )
	{
		strcpy(str,txt);
		LIBUA_ShortenString(onWindow->RPort, str, onGR->x2-onGR->x1-16-25);
		Move(onWindow->RPort, (LONG)onGR->x1+25, (LONG)y);
		Text(onWindow->RPort, str, strlen(str));
	}
}

/******** E O F ********/
