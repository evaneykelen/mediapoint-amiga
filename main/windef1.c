#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct CapsPrefs CPrefs;
extern UBYTE SW_pens[];
extern struct EditWindow prefsEW;
extern UWORD chip patterns_l[];
extern UWORD chip patterns_nl[];

/**** functions ****/

/******** OpenSmallPalette() ********/
/*
 * if mode==1 then the palette goes away as soon as the LMB is up
 * if mode==2 you have to click with the LMB to make it go away.
 *
 * if noColorZero is TRUE then color 0 can't be selected (you have to
 * take care that I don't get a well that's 0!)
 *
 */

int OpenSmallPalette(int well, int mode, BOOL noColorZero)
{
int numColors, w, h, numCols, numRows, row, col, color, x, y,lace,x1,y1;
struct Window *window;
struct GadgetRecord GR[5];
BOOL loop=TRUE, mouseMoved;
ULONG signals;
struct IntuiMessage *message;
int oldWell;
struct RasInfo *RI;

	oldWell = well;

	/**** calculate window dimensions ****/

	numColors = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);

	switch(numColors)
	{
		case 2:		w=144;	h=40;	numCols=1;	numRows=2;	break;	
		case 4:		w=72;		h=40; numCols=2;	numRows=2;	break;		
		case 8:		w=72;		h=20; numCols=2;	numRows=4;	break;		
		case 16:	w=36;		h=20;	numCols=4;	numRows=4;	break;		
		case 32:	w=36;		h=10;	numCols=4;	numRows=8;	break;		
		case 64:	w=18;		h=10;	numCols=8;	numRows=8;	break;	
		case 128:	w=18;		h=5;	numCols=8;	numRows=16;	break;	
		case 256:	w=9;		h=5;	numCols=16;	numRows=16;	break;	
	}

	if ( CPrefs.PageScreenModes & LACE )
		lace=2;
	else
		lace=1;

	/**** open a window ****/

	GR[0].x1		= lace-1;
	GR[0].y1		= 0;
	GR[0].x2		= w*numCols + 7;

	if ( lace==2 )
		GR[0].y2	= h*numRows + lace*2 - 1;
	else
		GR[0].y2	= h*numRows + lace*2 + 1;
	GR[0].y2 *= lace;

	GR[0].color = 2;
	GR[0].type	= DIMENSIONS;

	GR[1].x1		= 0;
	GR[1].y1		= 0;
	GR[1].x2		= GR[0].x2-1;
	GR[1].y2		= GR[0].y2-1;
	GR[1].color = 2;
	GR[1].type	= DBL_BORDER_REGION;

	x = (pageWindow->Width  - GR[0].x2) / 2;
	y = (pageWindow->Height - GR[0].y2) / 2;

	RI = pageScreen->ViewPort.RasInfo;
	if ( RI->RyOffset == 0 )
	{	
		if ( IntuitionBase->FirstScreen != pageScreen )
			y = y - IntuitionBase->FirstScreen->Height;
	}
	else
		y = y + RI->RyOffset;
	if ( y < 10 )
		y = 10;
	if ( y+GR[0].y2 >= pageScreen->Height-1 )
		y = pageScreen->Height - GR[0].y2 - 5;

	GR[2].x1		= 0;
	GR[2].y1		= 0;
	GR[2].x2		= x;
	GR[2].y2		= y;
	GR[2].color = 2;
	GR[2].type	= POSPREFS;

	GR[3].x1		= -1;

	window = UA_OpenRequesterWindow(pageWindow,GR,USECOLORS);
	if (!window)
		return(oldWell);
	UA_DrawGadgetList(window,GR);

	/**** render gadgets ****/

	SetDrMd(window->RPort, JAM1);

	h = h * lace;
	color=0;
	y = 2 * lace;

	for(row=0; row<numRows; row++)
	{
		x = 4;
		for(col=0; col<numCols; col++)
		{
			SetAPen(window->RPort, color);
			RectFill(window->RPort, x, y, x+w-2, y+h-2);

			if (noColorZero && color==0)	// ghost color 0
			{
				SetAPen(window->RPort, AREA_PEN);
				SetAfPt(window->RPort, gui_pattern, 1);
				RectFill(window->RPort, x, y, x+w-2, y+h-2);
				SetAfPt(window->RPort, NULL, 0);
			}

			color++;
			x = x + w;
		}
		y += h;
	}

	WaitBlit();

	/**** monitor user ****/

	UA_SwitchMouseMoveOn(window);

	/**** draw first box ****/

	if ( noColorZero && well==0 )
		well=1;

	DrawCross(window, well, w, h, numRows, numCols, 4, 2*lace);

	x = window->MouseX;
	y = MassageY(window->MouseY);

	DrawPaletteBox(window, x, y, w, h, numCols, numRows, 4, 2*lace);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class		= message->Class;
				CED.Code		= message->Code;
				CED.MouseX	= message->MouseX;	//pageWindow->MouseX;
				CED.MouseY	= MassageY(message->MouseY);	//MassageY(pageWindow->MouseY);

				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MENUPICK:
						well = -1;
						loop=FALSE;
						break;

					case IDCMP_MOUSEBUTTONS:
						if (CED.Code==MENUUP)
						{
							well = -1;
							loop=FALSE;
						}
						else
						{
							if (mode==1 && CED.Code == SELECTUP)
								loop=FALSE;
							else if (mode==2 && CED.Code == SELECTDOWN)
								loop=FALSE;
						}
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;

					case IDCMP_RAWKEY:
						if ( CED.Code==RAW_ESCAPE )
						{
							well = -1;
							loop=FALSE;
						}
						else if ( CED.Code==RAW_RETURN )
							loop=FALSE;
						break;
				}
			}
			if (mouseMoved)
			{
				if ( noColorZero )
				{
					x1 = CED.MouseX / w;	
					y1 = CED.MouseY / h;	
					if ( x1 < 0 )
						x1 = 0;
					if ( y1 < 0 )
						y1 = 0;
					if ( x1 > (numRows-1) )
						x1 = numRows-1;
					if ( y1 > (numCols-1) )
						y1 = numCols-1;
					if ( ( ( y1 * numRows ) + x1 ) != 0 )
					{
						well = DrawPaletteBox(window, x, y, w, h, numCols, numRows, 4, 2*lace);
						x = CED.MouseX;
						y = CED.MouseY;
						well = DrawPaletteBox(window, x, y, w, h, numCols, numRows, 4, 2*lace);
					}
				}
				else
				{
					well = DrawPaletteBox(window, x, y, w, h, numCols, numRows, 4, 2*lace);
					x = CED.MouseX;
					y = CED.MouseY;
					well = DrawPaletteBox(window, x, y, w, h, numCols, numRows, 4, 2*lace);
				}
			}
		}
	}

	UA_SwitchMouseMoveOff(window);

	UA_CloseRequesterWindow(window,USECOLORS);

	return(well);
}

/******** DrawCross() ********/

void DrawCross(	struct Window *window, int well,
								int w, int h, int numRows, int numCols,
								int offsetX, int offsetY)
{
int x1,y1;

	x1 = well % numCols;
	y1 = well / numCols;
	x1 = offsetX + x1 * w;
	y1 = offsetY + y1 * h;
	SetDrMd(window->RPort,JAM2|COMPLEMENT);
	Move(window->RPort,x1,y1);
	Draw(window->RPort,x1+w-2,y1+h-2);
	Move(window->RPort,x1,y1+h-2);
	Draw(window->RPort,x1+w-2,y1);
	SetDrMd(window->RPort,JAM2);
}

/******** DrawPaletteBox() ********/

int DrawPaletteBox(	struct Window *window, int mouseX, int mouseY,
										int w, int h, int numRows, int numCols,
										int offsetX, int offsetY)
{
int x1,y1,retval;

	x1 = mouseX / w;
	y1 = mouseY / h;

	if ( x1 < 0 )
		x1 = 0;

	if ( y1 < 0 )
		y1 = 0;

	if ( x1 > (numRows-1) )
		x1 = numRows-1;

	if ( y1 > (numCols-1) )
		y1 = numCols-1;

	retval = ( y1 * numRows ) + x1;

	x1 = offsetX + x1 * w;
	y1 = offsetY + y1 * h;

	DrawFatMarqueeBox(window->RPort, x1, y1, x1+w-2, y1+h-2);

	return( retval );
}

/******** RenderWindowIandB() ********/

void RenderWindowIandB(struct RastPort *rp, struct EditWindow *ew)
{
	RenderWindowInterior(rp, ew);
	RenderWindowBorders(rp, ew);
}

/******** RenderWindowInterior() ********/
 
void RenderWindowInterior(struct RastPort *rp, struct EditWindow *ew)
{
WORD x1,y1,x2,y2,sd_tb,sd_lr;

	GetWindowVars(ew,&x1,&y1,&x2,&y2);

	sd_tb = ew->wdw_shadowDepth;
	sd_lr = ew->wdw_shadowDepth;
	if ( !(CPrefs.PageScreenModes & LACE) )
		sd_lr *= 2;

	if ( ew->wdw_shadowDirection==1 )
	{
		x2 -= sd_lr;
		y2 -= sd_tb;
	}
	else if ( ew->wdw_shadowDirection==2 )
	{
		x1 += sd_lr;
		y2 -= sd_tb;
	}
	else if ( ew->wdw_shadowDirection==3 )
	{
		x1 += sd_lr;
		y1 += sd_tb;
	}
	else if ( ew->wdw_shadowDirection==4 )
	{
		x2 -= sd_lr;
		y1 += sd_tb;
	}

	SetDrMd(rp, JAM1);

	if (ew->BackFillType == 0)	/* Filled */
	{
		SetAPen(rp, ew->BackFillColor);
		RectFill(rp, ew->X+x1, ew->Y+y1, ew->X+x2, ew->Y+y2);
	}
	else if (ew->BackFillType == 1)	/* Pattern */
	{
		if ( CPrefs.PageScreenModes & LACE )
		{
			SetAfPt(rp, patterns_l + (ew->patternNum*18), 4);	// 2^4=16*2=32 bytes
		}
		else
		{
			SetAfPt(rp, patterns_nl + (ew->patternNum*9), 3);	// 2^3=8*2=16 bytes
		}
		SetAPen(rp, ew->BackFillColor);
		RectFill(rp, ew->X+x1, ew->Y+y1, ew->X+x2, ew->Y+y2);
		SetAfPt(rp, NULL, 0);
	}
	WaitBlit();
}

/******** RenderWindowBorders() ********/
 
void RenderWindowBorders(struct RastPort *rp, struct EditWindow *ew)
{
int i;
UWORD a, b, c, d, bw, h1,h2,v1,v2, sd_lr, sd_tb, x1,y1,x2,y2;

	SetDrMd(rp, JAM2);
	SetBPen(rp, 0L);

	bw = ew->BorderWidth;

	sd_tb = ew->wdw_shadowDepth;
	sd_lr = ew->wdw_shadowDepth;
	if ( !(CPrefs.PageScreenModes & LACE) )
		sd_lr *= 2;

	h1=0;
	h2=0;
	v1=0;
	v2=0;

	if ( ew->wdw_shadowDirection==1 )
	{
		h2=sd_lr;
		v2=sd_tb;
	}
	else if ( ew->wdw_shadowDirection==2 )
	{
		h1=sd_lr;
		v2=sd_tb;
	}
	else if ( ew->wdw_shadowDirection==3 )
	{
		h1=sd_lr;
		v1=sd_tb;
	}
	else if ( ew->wdw_shadowDirection==4 )
	{
		h2=sd_lr;
		v1=sd_tb;
	}

	if ( (h1+h2+v1+v2) != 0 )
	{
		SetDrMd(rp, JAM1);
		SetAPen(rp, ew->wdw_shadowPen);

		if ( CPrefs.PageScreenModes & LACE )
		{
			SetAfPt(rp, patterns_l + (ew->patternNum*18), 4);	// 2^4=16*2=32 bytes
		}
		else
		{
			SetAfPt(rp, patterns_nl + (ew->patternNum*9), 3);	// 2^3=8*2=16 bytes
		}

		if ( ew->wdw_shadowDirection==1 )
		{
			x1 = ew->X+ew->Width-sd_lr;
			y1 = ew->Y+sd_tb;
			x2 = ew->X+ew->Width-1;
			y2 = ew->Y+ew->Height-1;
			RectFill(rp,x1,y1,x2,y2);

			x1 = ew->X+sd_lr;
			y1 = ew->Y+ew->Height-sd_tb;
			x2 = ew->X+ew->Width-sd_lr;
			y2 = ew->Y+ew->Height-1;
			RectFill(rp,x1,y1,x2,y2);
		}
		else if ( ew->wdw_shadowDirection==2 )
		{
			x1 = ew->X;
			y1 = ew->Y+sd_tb;
			x2 = ew->X+sd_lr;
			y2 = ew->Y+ew->Height-1;
			RectFill(rp,x1,y1,x2,y2);

			x1 = ew->X+sd_lr;
			y1 = ew->Y+ew->Height-sd_tb;
			x2 = ew->X+ew->Width-sd_lr;
			y2 = ew->Y+ew->Height-1;
			RectFill(rp,x1,y1,x2,y2);
		}
		else if ( ew->wdw_shadowDirection==3 )
		{
			x1 = ew->X;
			y1 = ew->Y;
			x2 = ew->X+ew->Width-sd_lr;
			y2 = ew->Y+sd_tb;
			RectFill(rp,x1,y1,x2,y2);

			x1 = ew->X;
			y1 = ew->Y+sd_tb;
			x2 = ew->X+sd_lr;
			y2 = ew->Y+ew->Height-sd_tb;
			RectFill(rp,x1,y1,x2,y2);
		}
		else if ( ew->wdw_shadowDirection==4 )
		{
			x1 = ew->X+sd_lr;
			y1 = ew->Y;
			x2 = ew->X+ew->Width-1;
			y2 = ew->Y+sd_tb;
			RectFill(rp,x1,y1,x2,y2);

			x1 = ew->X+ew->Width-sd_lr;
			y1 = ew->Y+sd_tb;
			x2 = ew->X+ew->Width-1;
			y2 = ew->Y+ew->Height-sd_tb;
			RectFill(rp,x1,y1,x2,y2);
		}
		WaitBlit();
	}
	
	if (ew->Border & BORDER_TOP)
	{
		SetAPen(rp, ew->BorderColor[0]);

		a=b=c=d=0;
		if (ew->Border & BORDER_LEFT)
			c=1;
		if (ew->Border & BORDER_RIGHT)
			d=1;
		for(i=0; i<bw; i++)
		{
			Move(rp, ew->X+a+h1, ew->Y+i +v1);
			Draw(rp, ew->X+ew->Width-1-b-h2, ew->Y+i +v1);
			a=a+c;
			b=b+d;
		}
	}

	/**** draw bottom border ****/

	if (ew->Border & BORDER_BOTTOM)
	{
		SetAPen(rp, ew->BorderColor[2]);

		a=b=c=d=0;
		if (ew->Border & BORDER_LEFT)
			c=1;
		if (ew->Border & BORDER_RIGHT)
			d=1;
		for(i=0; i<bw; i++)
		{
			Move(rp, ew->X+a+h1, ew->Y+ew->Height-1-i -v2);
			Draw(rp, ew->X+ew->Width-1-b-h2, ew->Y+ew->Height-1-i -v2);
			a=a+c;
			b=b+d;
		}
	}

	/**** draw left border ****/

	if (ew->Border & BORDER_LEFT)
	{
		SetAPen(rp, ew->BorderColor[3]);

		a=b=c=d=0;
		if (ew->Border & BORDER_TOP)
			c=1;
		if (ew->Border & BORDER_BOTTOM)
			d=1;
		for(i=0; i<bw; i++)
		{
			Move(rp, ew->X+i +h1, ew->Y+a+v1);
			Draw(rp, ew->X+i +h1, ew->Y+ew->Height-1-b-v2);
			a=a+c;
			b=b+d;
		}
	}

	/**** draw right border ****/

	if (ew->Border & BORDER_RIGHT)
	{
		SetAPen(rp, ew->BorderColor[1]);

		a=b=c=d=0;
		if (ew->Border & BORDER_TOP)
			c=1;
		if (ew->Border & BORDER_BOTTOM)
			d=1;
		for(i=0; i<bw; i++)
		{
			Move(rp, ew->X+ew->Width-1-i -h2, ew->Y+a+v1);
			Draw(rp, ew->X+ew->Width-1-i -h2, ew->Y+ew->Height-1-b-v2);
			a=a+c;
			b=b+d;
		}
	}
	
	/**** draw crawl borders ****/

	if ( ew->crawl_fontName[0]!='\0' ) // this window carries crawl data yet
	{
		SetDrMd(rp, JAM1 | COMPLEMENT);
		SafeSetWriteMask(rp, 0x3);
		SetDrPt(rp, 0x3333);	// 0011001100110011

		Move(rp, ew->X, ew->Y);
		Draw(rp, ew->X+ew->Width-1,	ew->Y);

		Move(rp, ew->X,							ew->Y+ew->Height-1);
		Draw(rp, ew->X+ew->Width-1,	ew->Y+ew->Height-1);

		SetDrPt(rp, 0xffff);
		SetDrMd(rp, JAM1);
		SafeSetWriteMask(rp, 0xff);
	}

	SetAfPt(rp, NULL, 0);
}

/******** ToggleScaling() ********/

void ToggleScaling(void)
{
BOOL list[MAXEDITWINDOWS];
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
		list[i] = FALSE;

	DrawAllHandles(LEAVE_ACTIVE);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] &&
				EditSupportList[i]->Active )
		{
			if ( EditSupportList[i]->photoOpts & MOVE_PHOTO )
			{
				UnSetByteBit(&(EditSupportList[i]->photoOpts), MOVE_PHOTO);
				SetByteBit(&(EditSupportList[i]->photoOpts), SIZE_PHOTO);
				SetByteBit(&(EditSupportList[i]->photoOpts), REMAP_PHOTO);

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->remapped_bm);
				ClearBitMap(&EditSupportList[i]->remapped_bm);
				EditSupportList[i]->remapped_w = 0;
				EditSupportList[i]->remapped_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->mask_bm);
				ClearBitMap(&EditSupportList[i]->mask_bm);
				EditSupportList[i]->mask_w = 0;
				EditSupportList[i]->mask_h = 0;
			}
			else if ( EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				UnSetByteBit(&(EditSupportList[i]->photoOpts), SIZE_PHOTO);
				SetByteBit(&(EditSupportList[i]->photoOpts), MOVE_PHOTO);

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->scaled_bm);
				ClearBitMap(&EditSupportList[i]->scaled_bm);
				EditSupportList[i]->scaled_w = 0;
				EditSupportList[i]->scaled_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->remapped_bm);
				ClearBitMap(&EditSupportList[i]->remapped_bm);
				EditSupportList[i]->remapped_w = 0;
				EditSupportList[i]->remapped_h = 0;

				RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->mask_bm);
				ClearBitMap(&EditSupportList[i]->mask_bm);
				EditSupportList[i]->mask_w = 0;
				EditSupportList[i]->mask_h = 0;
			}

			list[i] = TRUE;
		}
	}

	RedrawAllOverlapWdwList(list);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** SetDrawMode() ********/
/*
 *	0=solid, 1=pattern, 2=transp., 4=user defined, 5=antialias
 *
 */

void SetDrawMode(int mode)
{
BOOL list[MAXEDITWINDOWS];
int i, j;

	for(i=0; i<MAXEDITWINDOWS; i++)
		list[i] = FALSE;

	DrawAllHandles(LEAVE_ACTIVE);

	// Restore back already because CorrectEW may change dims

	for(i=MAXEDITWINDOWS-1; i>=0; i--)
		if (	EditSupportList[i] && EditSupportList[i]->restore_bm.Planes[0] &&
					EditSupportList[i]->Active )
			RestoreBack( EditWindowList[i], EditSupportList[i] );

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->Active )
		{
			if ( mode==0 || mode==1 || mode==2 )
			{
				EditWindowList[i]->BackFillType = mode;
			}
			else if ( mode==4 )
			{
				EditWindowList[i]->Border								= prefsEW.Border;
				EditWindowList[i]->BorderColor[0]				= prefsEW.BorderColor[0];
				EditWindowList[i]->BorderColor[1]				= prefsEW.BorderColor[1];
				EditWindowList[i]->BorderColor[2]				= prefsEW.BorderColor[2];
				EditWindowList[i]->BorderColor[3]				= prefsEW.BorderColor[3];
				EditWindowList[i]->BorderWidth					= prefsEW.BorderWidth;
				EditWindowList[i]->BackFillColor				= prefsEW.BackFillColor;
				EditWindowList[i]->BackFillType					= prefsEW.BackFillType;
				EditWindowList[i]->flags								= prefsEW.flags;
				EditWindowList[i]->TopMargin						= prefsEW.TopMargin;
				EditWindowList[i]->RightMargin					= prefsEW.RightMargin;
				EditWindowList[i]->BottomMargin					= prefsEW.BottomMargin;
				EditWindowList[i]->LeftMargin						= prefsEW.LeftMargin;			
				EditWindowList[i]->patternNum						= prefsEW.patternNum;			
				EditWindowList[i]->wdw_shadowDepth 			= prefsEW.wdw_shadowDepth;
				EditWindowList[i]->wdw_shadowDirection	= prefsEW.wdw_shadowDirection;
				EditWindowList[i]->wdw_shadowPen				= prefsEW.wdw_shadowPen;

				EditWindowList[i]->antiAliasLevel				= prefsEW.antiAliasLevel;		
				EditWindowList[i]->justification				= prefsEW.justification;	
				EditWindowList[i]->xSpacing							= prefsEW.xSpacing;					
				EditWindowList[i]->ySpacing							= prefsEW.ySpacing;					
				EditWindowList[i]->slantAmount					= prefsEW.slantAmount;			
				EditWindowList[i]->slantValue						= prefsEW.slantValue;				
				EditWindowList[i]->underLineHeight			= prefsEW.underLineHeight;	
				EditWindowList[i]->underLineOffset			= prefsEW.underLineOffset;	
				EditWindowList[i]->shadowDepth					= prefsEW.shadowDepth;			
				EditWindowList[i]->shadow_Pen						= prefsEW.shadow_Pen;				
				EditWindowList[i]->shadowType						= prefsEW.shadowType;				
				EditWindowList[i]->shadowDirection			= prefsEW.shadowDirection;	

				EditWindowList[i]->charFont 						= prefsEW.charFont; 				
				EditWindowList[i]->charStyle 						= prefsEW.charStyle; 				
				EditWindowList[i]->charColor 						= prefsEW.charColor; 				
				EditWindowList[i]->underlineColor 			= prefsEW.underlineColor; 	

				// Set font, size, color and underline color for each and every character

				for(j=0; j<EditWindowList[i]->TEI->textLength; j++)
				{
					if ( EditWindowList[i]->TEI->text[ j ].charCode != '\0' )
					{
						EditWindowList[i]->TEI->text[ j ].charFont = prefsEW.charFont;
						EditWindowList[i]->TEI->text[ j ].charStyle = prefsEW.charStyle;
						EditWindowList[i]->TEI->text[ j ].charColor = prefsEW.charColor;
						EditWindowList[i]->TEI->text[ j ].underlineColor = prefsEW.underlineColor;
					}
				}
			}
			else if ( mode==5 )
			{
				if ( EditWindowList[i]->antiAliasLevel!=0 )
					EditWindowList[i]->antiAliasLevel = 0;
				else
					EditWindowList[i]->antiAliasLevel = 3;
			}

			if ( mode>=0 && mode<= 4 )
			{
				if ( 	EditSupportList[i]->photoOpts & SIZE_PHOTO ||
							EditSupportList[i]->photoOpts & MOVE_PHOTO )
				{
					RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->scaled_bm);
					ClearBitMap(&EditSupportList[i]->scaled_bm);
					EditSupportList[i]->scaled_w = 0;
					EditSupportList[i]->scaled_h = 0;
	
					RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->remapped_bm);
					ClearBitMap(&EditSupportList[i]->remapped_bm);
					EditSupportList[i]->remapped_w = 0;
					EditSupportList[i]->remapped_h = 0;

					RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->mask_bm);
					ClearBitMap(&EditSupportList[i]->mask_bm);
					EditSupportList[i]->mask_w = 0;
					EditSupportList[i]->mask_h = 0;
				}
			}

			list[i] = TRUE;

			CorrectEW(EditWindowList[i]);
		}
	}

	RedrawAllOverlapWdwList( list );

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** InvertLockedState() ********/

void InvertLockedState(void)
{
int i;

	DrawAllHandles(LEAVE_ACTIVE);

	for(i=0; i<MAXEDITWINDOWS; i++)
		if (EditWindowList[i]!=NULL && EditSupportList[i]->Active )
			InvertByteBit(&EditWindowList[i]->flags, EW_LOCKED);

	Delay(2L);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** PaintButton() ********/

void PaintButton(struct Window *window, struct GadgetRecord *GR, int pen)
{
	SetAPen(window->RPort,pen);
	SetDrMd(window->RPort,JAM1);

	if (window->WScreen->ViewPort.Modes & LACE)
		RectFill(window->RPort, (LONG)GR->x1+4, (LONG)GR->y1+3,
														(LONG)GR->x2-4, (LONG)GR->y2-3);
	else
		RectFill(window->RPort, (LONG)GR->x1+4, (LONG)GR->y1+2,
														(LONG)GR->x2-4, (LONG)GR->y2-2);
}

/******** E O F ********/
