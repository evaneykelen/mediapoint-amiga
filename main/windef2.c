#include "nb:pre.h"

/**** externals ****/

extern ULONG allocFlags;
extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Screen *pageScreen;
extern UWORD chip gui_pattern[];
extern UBYTE **msgs;   
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern struct ColorMap *undoCM;
extern struct RendezVousRecord rvrec;
extern struct Window *smallWindow;

/**** gadgets ****/

extern struct GadgetRecord WDef_GR[];

/**** functions ****/

/******** ToggleWinDef() ********/

BOOL ToggleWinDef(void)
{
int wdw, i;

	ScreenAtNormalPos();

	wdw = FirstActiveEditWindow();

	if ( smallWindow && (allocFlags & WINDEF_WDW_FLAG) )
	{
		if ( IntuitionBase->FirstScreen == smallWindow->WScreen )
		{
			MyScreenToBack(smallWindow->WScreen);
		}
		else
		{
			EnableDisableWinDefList(smallWindow,wdw);
			ScreenToFront(smallWindow->WScreen);
			if ( !SmallScreenCorrect(smallWindow->WScreen) )
				goto reopen;
			ActivateWindow(smallWindow);
		}			
	}
	else
	{
reopen:
		for(i=3; i<=12; i++)
			UA_EnableButtonQuiet(&WDef_GR[i]);
		for(i=15; i<=24; i++)
			UA_EnableButtonQuiet(&WDef_GR[i]);

		if ( CPrefs.PageScreenModes & LACE )
		{
			if ( WDef_GR[0].x1 == 0 )	// not doubled 
			{
				UA_DoubleGadgetDimensions(WDef_GR);
				WDef_GR[0].x1 = 1;
			}
		}
		else
		{
			if ( WDef_GR[0].x1 == 1 )	// doubled
			{
				UA_HalveGadgetDimensions(WDef_GR);
				WDef_GR[0].x1 = 0;
			}
		}

		smallWindow = OpenSmallScreen( WDef_GR[0].y2 ); 
		if (!smallWindow)
		{
			UA_WarnUser(-1);
			return(FALSE);
		}
		SetBit(&allocFlags, WINDEF_WDW_FLAG);

		UA_DrawGadgetList(smallWindow, WDef_GR);

		EnableDisableWinDefList(smallWindow,wdw);

		ScreenToFront(smallWindow->WScreen);
	}

	return(TRUE);
}

/******** MonitorWinDef() ********/

int MonitorWinDef(void)
{
BOOL takeAction = TRUE, nextgadget, firsttime, redraw=TRUE;
int ID, val, wdw, color, well;
ULONG signals;
struct EditWindow localEW;
struct EditSupport localES;

	wdw = FirstActiveEditWindow();

	if ( wdw!=-1 )
	{
		CopyMem(EditWindowList[wdw],&localEW,sizeof(struct EditWindow));
		CopyMem(EditSupportList[wdw],&localES,sizeof(struct EditSupport));
	}

	{
	struct RasInfo *RI;
		RI = pageScreen->ViewPort.RasInfo;
		CED.MouseY -= RI->RyOffset;
	}

	do
	{
		if (takeAction)
		{
			if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
			{
				ID = UA_CheckGadgetList(smallWindow, WDef_GR, &CED);

				if ( wdw==-1 && ID!=2 && ID!=-1 )
					ID=-1;

				switch(ID)
				{
					case 2:		// Hide
						UA_HiliteButton(smallWindow, &WDef_GR[2]);
						PaletteToBack();
						//UA_SwitchFlagsOff(smallWindow, IDCMP_INTUITICKS);
						return(NOTHING_TO_EXAMINE);
						break;

					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
						UA_ProcessCycleGadget(smallWindow, &WDef_GR[ID], &CED);
						UA_SetValToCycleGadgetVal(&WDef_GR[ID], &val);
						switch(ID)
						{
							case 3: localEW.BackFillType = val;					break;
							case 4: localEW.patternNum = val;						break;
							case 5:	localEW.BorderWidth = val+1;				break;
							case 6:	localEW.wdw_shadowDirection = val;	break;
							case 7:	localEW.wdw_shadowDepth = val+1;		break;
						}
						InterpretWinDefSettings(&localEW,&localES,ID,redraw);
						redraw=TRUE;
						if (	ID==3 && UA_IsGadgetDisabled(&WDef_GR[4]) &&
									( localEW.BackFillType==1 || localEW.wdw_shadowDirection!=0 ) )
							UA_EnableButton(smallWindow, &WDef_GR[4]);
						if (	ID==6 && UA_IsGadgetDisabled(&WDef_GR[7]) &&
									localEW.wdw_shadowDirection!=0 )
							UA_EnableButton(smallWindow, &WDef_GR[7]);
						if (	ID==6 && UA_IsGadgetDisabled(&WDef_GR[4]) &&
									localEW.wdw_shadowDirection!=0 )
							UA_EnableButton(smallWindow, &WDef_GR[4]);
						break;

					case 8:
do_x:
						nextgadget = UA_ProcessStringGadget(smallWindow, WDef_GR, &WDef_GR[8], &CED);
						UA_SetValToStringGadgetVal(&WDef_GR[8], &val);
						localEW.X = val;
						InterpretWinDefSettings(&localEW,&localES,8,redraw);
						redraw=TRUE;
						UA_SetStringGadgetToVal(smallWindow, &WDef_GR[8], (int)localEW.X);
						if (nextgadget)
							goto do_y;
						break;

					case 9:
do_y:
						nextgadget = UA_ProcessStringGadget(smallWindow, WDef_GR, &WDef_GR[9], &CED);
						UA_SetValToStringGadgetVal(&WDef_GR[9], &val);
						localEW.Y = val;
						InterpretWinDefSettings(&localEW,&localES,9,redraw);
						redraw=TRUE;
						UA_SetStringGadgetToVal(smallWindow, &WDef_GR[9], (int)localEW.Y);
						if (nextgadget)
							goto do_w;
						break;

					case 10:
do_w:
						nextgadget = UA_ProcessStringGadget(smallWindow, WDef_GR, &WDef_GR[10], &CED);
						UA_SetValToStringGadgetVal(&WDef_GR[10], &val);
						localEW.Width = val;
						InterpretWinDefSettings(&localEW,&localES,10,redraw);
						redraw=TRUE;
						UA_SetStringGadgetToVal(smallWindow, &WDef_GR[10], (int)localEW.Width);
						if (nextgadget)
							goto do_h;
						break;

					case 11:
do_h:
						nextgadget = UA_ProcessStringGadget(smallWindow, WDef_GR, &WDef_GR[11], &CED);
						UA_SetValToStringGadgetVal(&WDef_GR[11], &val);
						localEW.Height = val;
						InterpretWinDefSettings(&localEW,&localES,11,redraw);
						redraw=TRUE;
						UA_SetStringGadgetToVal(smallWindow, &WDef_GR[11], (int)localEW.Height);
						if (nextgadget)
							goto do_x;
						break;

					case 12:	// locked state
						UA_HiliteButton(smallWindow, &WDef_GR[ID]);
						InvertByteBit(&localEW.flags,EW_LOCKED);
						if ( localEW.flags & EW_LOCKED )
							UA_PrintInBox(	smallWindow, &WDef_GR[ID],
															WDef_GR[ID].x1+1, WDef_GR[ID].y1, WDef_GR[ID].x2, WDef_GR[ID].y2,
															"\0", PRINT_CENTERED);	// 81 locked
						else
							UA_PrintInBox(	smallWindow, &WDef_GR[ID],
															WDef_GR[ID].x1+1, WDef_GR[ID].y1, WDef_GR[ID].x2, WDef_GR[ID].y2,
															"‚\0", PRINT_CENTERED);	// 82 unlocked
						InterpretWinDefSettings(&localEW,&localES,ID,redraw);
						redraw=TRUE;
						break;
	
					case 15:
					case 16:
					case 17:
					case 18:
						if ( !FirstTimeCase(smallWindow,&localEW) )
						{
							UA_InvertButton(smallWindow, &WDef_GR[ID]);
							if (ID==15)
								InvertByteBit(&localEW.Border, BORDER_TOP);
							else if (ID==16)
								InvertByteBit(&localEW.Border, BORDER_RIGHT);
							else if (ID==17)
								InvertByteBit(&localEW.Border, BORDER_BOTTOM);
							else if (ID==18)
								InvertByteBit(&localEW.Border, BORDER_LEFT);
							InterpretWinDefSettings(&localEW,&localES,ID,redraw);
							redraw=TRUE;
							if ( UA_IsGadgetDisabled(&WDef_GR[5]) && localEW.Border!=0 )
								UA_EnableButton(smallWindow, &WDef_GR[5]);
						}
						break;

					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
						if ( ID==23 )	// backfillcolor
							firsttime = FALSE;
						else
							firsttime = FirstTimeCase(smallWindow,&localEW);

						if ( !firsttime )
						{
							UA_HiliteButton(smallWindow, &WDef_GR[ID]);

							if (ID==19)
								well = localEW.BorderColor[0];
							else if (ID==20)
								well = localEW.BorderColor[1];
							else if (ID==21)
								well = localEW.BorderColor[2];
							else if (ID==22)
								well = localEW.BorderColor[3];
							else if (ID==23)
								well = localEW.BackFillColor;
							else if (ID==24)
								well = localEW.wdw_shadowPen;

							color = OpenSmallPalette(well, 2, FALSE);

							if (color!=-1)
							{
								if (ID==19 && localEW.Border & BORDER_TOP)
									localEW.BorderColor[0]=color;
								else if (ID==20 && localEW.Border & BORDER_RIGHT)
									localEW.BorderColor[1]=color;
								else if (ID==21 && localEW.Border & BORDER_BOTTOM)
									localEW.BorderColor[2]=color;
								else if (ID==22 && localEW.Border & BORDER_LEFT)
									localEW.BorderColor[3]=color;
								else if (ID==23)
									localEW.BackFillColor=color;
								else if (ID==24)
									localEW.wdw_shadowPen=color;

								if (ID==19)
									DoPaintedButton( 8,localEW.BorderColor[0]);
								else if (ID==20)
									DoPaintedButton( 9,localEW.BorderColor[1]);
								else if (ID==21)
									DoPaintedButton(10,localEW.BorderColor[2]);
								else if (ID==22)
									DoPaintedButton(11,localEW.BorderColor[3]);
								else if (ID==23)
									DoPaintedButton(12,localEW.BackFillColor);
								else if (ID==24)
									DoPaintedButton(13,localEW.wdw_shadowPen);

								InterpretWinDefSettings(&localEW,&localES,ID,redraw);
								redraw=TRUE;
							}
						}
						break;
				}
			}
			else if ( CED.Class==IDCMP_RAWKEY || CED.Class==IDCMP_MENUPICK )
			{
				//UA_SwitchFlagsOff(smallWindow, IDCMP_INTUITICKS);
				return(IDCMP_TO_EXAMINE);
			}
			//else if ( CED.Class==IDCMP_INTUITICKS )
			//	ticks++;
		}

		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(smallWindow,FALSE,NULL);
			takeAction=TRUE;
			if ( !(smallWindow->Flags & WFLG_WINDOWACTIVE) )
			{
				//UA_SwitchFlagsOff(smallWindow, IDCMP_INTUITICKS);
				return(IDCMP_TO_EXAMINE);
			}
		}
		else
			takeAction=FALSE;

		//if ( CED.extraClass == DBLCLICKED )
		//	ticks = 0L;

/*
		if ( redraw || DoubleClick(secs,mics,CED.Seconds,CED.Micros) )
		{
			// Time between now and last time is too short to allow a screen redraw
			redraw = FALSE;
			secs = CED.Seconds;
			mics = CED.Micros;
		}
		else
			redraw = TRUE;
*/
	}
	while(1);

	//UA_SwitchFlagsOff(smallWindow, IDCMP_INTUITICKS);

	return(NOTHING_TO_EXAMINE);
}

/******** CloseWinDefScreen() ********/

void CloseWinDefScreen(void)
{
	if ( smallWindow )
	{
		CloseSmallScreen(smallWindow);
		UnSetBit(&allocFlags, WINDEF_WDW_FLAG);
	}
}

/******** DoWinDefSettings() ********/
/*
 * This function is called whenever an edit window is selected
 *
 */

void DoWinDefSettings(void)
{
int wdw;

	if ( !smallWindow || !(allocFlags & WINDEF_WDW_FLAG) )
		return;

	wdw = FirstActiveEditWindow();
	EnableDisableWinDefList(smallWindow, wdw);
}

/******** EnableDisableWinDefList() ********/

void EnableDisableWinDefList(struct Window *window, int wdw)
{
BOOL enableStuff;
int i;

	if ( wdw==-1 )
		enableStuff=FALSE;
	else
		enableStuff=TRUE;

	if ( enableStuff )
	{
		//if ( UA_IsGadgetDisabled(&WDef_GR[3]) )
		{
			for(i=3; i<=12; i++)
				if ( WDef_GR[i].type >= 100 )
					UA_EnableButton(window, &WDef_GR[i]);
			for(i=15; i<=24; i++)
				if ( WDef_GR[i].type >= 100 )
					UA_EnableButton(window, &WDef_GR[i]);
		}
		RenderWinDefButtons(window,wdw);
	}
	else
	{
		//if ( !UA_IsGadgetDisabled(&WDef_GR[3]) )
		{
			for(i= 3; i<=12; i++)
				if ( WDef_GR[i].type < 100 )
					UA_DisableButton(window, &WDef_GR[i], gui_pattern);
			for(i=15; i<=24; i++)
				if ( WDef_GR[i].type < 100 )
					UA_DisableButton(window, &WDef_GR[i], gui_pattern);
		}
	}
}

/******** RenderWinDefButtons() ********/

void RenderWinDefButtons(struct Window *window, int wdw)
{
int i;

	if (wdw==-1)
		return;

	for(i=3; i<=7; i++)
		UA_EnableButtonQuiet(&WDef_GR[i]);

	UA_SetCycleGadgetToVal(window, &WDef_GR[3], EditWindowList[wdw]->BackFillType);
	UA_SetCycleGadgetToVal(window, &WDef_GR[4], EditWindowList[wdw]->patternNum);
	UA_SetCycleGadgetToVal(window, &WDef_GR[5], EditWindowList[wdw]->BorderWidth-1);
	UA_SetCycleGadgetToVal(window, &WDef_GR[6], EditWindowList[wdw]->wdw_shadowDirection);
	UA_SetCycleGadgetToVal(window, &WDef_GR[7], EditWindowList[wdw]->wdw_shadowDepth-1);

	UA_SetStringGadgetToVal(window, &WDef_GR[ 8], (int)EditWindowList[wdw]->X);
	UA_SetStringGadgetToVal(window, &WDef_GR[ 9], (int)EditWindowList[wdw]->Y);
	UA_SetStringGadgetToVal(window, &WDef_GR[10], (int)EditWindowList[wdw]->Width);
	UA_SetStringGadgetToVal(window, &WDef_GR[11], (int)EditWindowList[wdw]->Height);

	if ( EditWindowList[wdw]->flags & EW_LOCKED )
		UA_PrintInBox(	window, &WDef_GR[12], WDef_GR[12].x1+1,WDef_GR[12].y1,WDef_GR[12].x2,WDef_GR[12].y2,
										"\0", PRINT_CENTERED);	// 81 locked
	else
		UA_PrintInBox(	window, &WDef_GR[12], WDef_GR[12].x1+1,WDef_GR[12].y1,WDef_GR[12].x2,WDef_GR[12].y2,
										"‚\0", PRINT_CENTERED);	// 82 unlocked

	UA_ClearButton(window, &WDef_GR[15], AREA_PEN);
	UA_ClearButton(window, &WDef_GR[16], AREA_PEN);
	UA_ClearButton(window, &WDef_GR[17], AREA_PEN);
	UA_ClearButton(window, &WDef_GR[18], AREA_PEN);

	if (EditWindowList[wdw]->Border & BORDER_TOP)
		UA_InvertButton(window, &WDef_GR[15]);
	if (EditWindowList[wdw]->Border & BORDER_RIGHT)
		UA_InvertButton(window, &WDef_GR[16]);
	if (EditWindowList[wdw]->Border & BORDER_BOTTOM)
		UA_InvertButton(window, &WDef_GR[17]);
	if (EditWindowList[wdw]->Border & BORDER_LEFT)
		UA_InvertButton(window, &WDef_GR[18]);

	PaintButton(window, &WDef_GR[19], 8);
	PaintButton(window, &WDef_GR[20], 9);
	PaintButton(window, &WDef_GR[21], 10);
	PaintButton(window, &WDef_GR[22], 11);
	PaintButton(window, &WDef_GR[23], 12);
	PaintButton(window, &WDef_GR[24], 13);

	DoPaintedButton( 8, EditWindowList[wdw]->BorderColor[0]);
	DoPaintedButton( 9, EditWindowList[wdw]->BorderColor[1]);
	DoPaintedButton(10, EditWindowList[wdw]->BorderColor[2]);
	DoPaintedButton(11, EditWindowList[wdw]->BorderColor[3]);
	DoPaintedButton(12, EditWindowList[wdw]->BackFillColor);
	DoPaintedButton(13, EditWindowList[wdw]->wdw_shadowPen);

	DoWinDefGhosting(EditWindowList[wdw]);
}

/******** InterpretWinDefSettings() ********/

void InterpretWinDefSettings(	struct EditWindow *localEW, struct EditSupport *localES,
															int button, BOOL redraw )
{
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
WORD x,y,w,h;
BOOL list[MAXEDITWINDOWS];
int i;
BYTE b;
BOOL refresh=TRUE;

	DrawAllHandles(LEAVE_ACTIVE);

	x = localEW->X;
	y = localEW->Y;
	w = localEW->Width;
	h = localEW->Height;

	DoWinDefGhosting(localEW);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->Active )
		{
			list[i] = TRUE;

			prevX[i] = EditWindowList[i]->X;
			prevY[i] = EditWindowList[i]->Y;
			prevW[i] = EditWindowList[i]->Width;
			prevH[i] = EditWindowList[i]->Height;
		}
		else
			list[i] = FALSE;
	}

	switch( button )
	{
		case 3:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->BackFillType = localEW->BackFillType;
			break;

		case 4:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->patternNum = localEW->patternNum;
			break;

		case 5:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->BorderWidth = localEW->BorderWidth;
			break;

		case 6:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->wdw_shadowDirection = localEW->wdw_shadowDirection;
			break;

		case 7:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->wdw_shadowDepth = localEW->wdw_shadowDepth;
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			if ( button==8 || x!=localEW->X )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
					if ( EditWindowList[i] && EditSupportList[i]->Active )
						EditWindowList[i]->X = localEW->X;
			}
			if ( button==9 || y!=localEW->Y )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
					if ( EditWindowList[i] && EditSupportList[i]->Active )
						EditWindowList[i]->Y = localEW->Y;
			}
			if ( button==10 || w!=localEW->Width )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
					if ( EditWindowList[i] && EditSupportList[i]->Active )
						EditWindowList[i]->Width = localEW->Width;
			}
			if ( button==11 || h!=localEW->Height )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
					if ( EditWindowList[i] && EditSupportList[i]->Active )
						EditWindowList[i]->Height = localEW->Height;
			}
			break;

		case 12:
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] && EditSupportList[i]->Active )
				{
					if ( localEW->flags & EW_LOCKED )
						SetByteBit(&EditWindowList[i]->flags,EW_LOCKED);
					else
						UnSetByteBit(&EditWindowList[i]->flags,EW_LOCKED);
				}
			}
			refresh=FALSE;
			break;

		case 15:
		case 16:
		case 17:
		case 18:
			if ( button==15 )
				b=BORDER_TOP;
			else if ( button==16 )
				b=BORDER_RIGHT;
			else if ( button==17 )
				b=BORDER_BOTTOM;
			else if ( button==18 )
				b=BORDER_LEFT;
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] && EditSupportList[i]->Active )
				{
					if ( localEW->Border & b )
						SetByteBit(&EditWindowList[i]->Border, b);
					else
						UnSetByteBit(&EditWindowList[i]->Border, b);
				}
			}
			break;

		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] && EditSupportList[i]->Active )
				{
					if (button==19)
						EditWindowList[i]->BorderColor[0] = localEW->BorderColor[0];
					else if (button==20)
						EditWindowList[i]->BorderColor[1] = localEW->BorderColor[1];
					else if (button==21)
						EditWindowList[i]->BorderColor[2] = localEW->BorderColor[2];
					else if (button==22)
						EditWindowList[i]->BorderColor[3] = localEW->BorderColor[3];
					else if (button==23)
						EditWindowList[i]->BackFillColor = localEW->BackFillColor;
					else if (button==24)
						EditWindowList[i]->wdw_shadowPen = localEW->wdw_shadowPen;
				}
			}
			break;
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] && EditSupportList[i]->Active )
			CorrectEW(EditWindowList[i]);

if ( redraw )
{
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->Active )
		{
			newX[i] = EditWindowList[i]->X;
			newY[i] = EditWindowList[i]->Y;
			newW[i] = EditWindowList[i]->Width;
			newH[i] = EditWindowList[i]->Height;
		}
		else
			list[i] = FALSE;
	}

	if ( refresh )
		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);
}

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** DoPaintedButton() ********/

void DoPaintedButton(int well, int pen)
{
int r,g,b;
	GetColorComponents(pageScreen->ViewPort.ColorMap, pen, &r, &g, &b);
	SetVPToComponents(&(smallWindow->WScreen->ViewPort), well, r, g, b);
}

/******** DoWinDefGhosting() ********/

void DoWinDefGhosting(struct EditWindow *ew)
{
	if ( ew->BackFillType != 1 && ew->wdw_shadowDirection == 0 )
		UA_DisableButton(smallWindow, &WDef_GR[4], gui_pattern);

	if ( ew->Border == 0 )
		UA_DisableButton(smallWindow, &WDef_GR[5], gui_pattern);

	if ( ew->wdw_shadowDirection == 0 )
		UA_DisableButton(smallWindow, &WDef_GR[7], gui_pattern);
}

/******** FirstTimeCase() ********/

BOOL FirstTimeCase(struct Window *window, struct EditWindow *localEW)
{
int nc,pen;
struct BitMap *bm;
struct ViewPort *vp;
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
BOOL list[MAXEDITWINDOWS];
int i;

	if ( localEW->Border == 0 )	// starting from scratch -- help user a bit
	{
		DrawAllHandles(LEAVE_ACTIVE);

		UA_InvertButton(window, &WDef_GR[15]);
		UA_InvertButton(window, &WDef_GR[16]);
		UA_InvertButton(window, &WDef_GR[17]);
		UA_InvertButton(window, &WDef_GR[18]);
	
		SetByteBit(&localEW->Border, BORDER_TOP);
		SetByteBit(&localEW->Border, BORDER_RIGHT);
		SetByteBit(&localEW->Border, BORDER_BOTTOM);
		SetByteBit(&localEW->Border, BORDER_LEFT);

		vp = &(pageScreen->ViewPort);
		bm = pageScreen->RastPort.BitMap;
		nc = UA_GetNumberOfColorsInScreen(vp->Modes, bm->Depth, CPrefs.AA_available);

		pen = UA_MyFindColor(CPrefs.PageCM,0xffffffff,0xffffffff,0xffffffff,nc-1,nc-1,FALSE);	// no color 0
		localEW->BorderColor[0] = pen;	// top
		localEW->BorderColor[3] = pen;	// left

		pen = UA_MyFindColor(CPrefs.PageCM,0x00000000,0x00000000,0x00000000,nc-1,nc-1,FALSE);
		localEW->BorderColor[1] = pen;	// right
		localEW->BorderColor[2] = pen;	// bottom
		localEW->wdw_shadowPen = pen;

		if ( UA_IsGadgetDisabled(&WDef_GR[5]) )	// border width
		{
			UA_EnableButton(window, &WDef_GR[5]);
			localEW->BorderWidth = 2;
			UA_SetCycleGadgetToVal(window, &WDef_GR[5], localEW->BorderWidth-1);
		}

		DoPaintedButton( 8, localEW->BorderColor[0]);
		DoPaintedButton( 9, localEW->BorderColor[1]);
		DoPaintedButton(10, localEW->BorderColor[2]);
		DoPaintedButton(11, localEW->BorderColor[3]);
		DoPaintedButton(13, localEW->wdw_shadowPen);

		/**** redraw all ****/

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && EditSupportList[i]->Active )
			{
				EditWindowList[i]->Border					= localEW->Border;
				EditWindowList[i]->BorderColor[0]	= localEW->BorderColor[0];
				EditWindowList[i]->BorderColor[1]	= localEW->BorderColor[1];
				EditWindowList[i]->BorderColor[2]	= localEW->BorderColor[2];
				EditWindowList[i]->BorderColor[3]	= localEW->BorderColor[3];
				EditWindowList[i]->BorderWidth		= localEW->BorderWidth;
				EditWindowList[i]->wdw_shadowPen	= localEW->wdw_shadowPen;
			}
		}

		DoWinDefGhosting(localEW);

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && EditSupportList[i]->Active )
			{
				CorrectEW(EditWindowList[i]);

				list[i] = TRUE;

				prevX[i] = EditWindowList[i]->X;
				prevY[i] = EditWindowList[i]->Y;
				prevW[i] = EditWindowList[i]->Width;
				prevH[i] = EditWindowList[i]->Height;

				newX[i] = EditWindowList[i]->X;
				newY[i] = EditWindowList[i]->Y;
				newW[i] = EditWindowList[i]->Width;
				newH[i] = EditWindowList[i]->Height;
			}
			else
				list[i] = FALSE;
		}

		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);

		DrawAllHandles(LEAVE_ACTIVE);

		return(TRUE);
	}

	return(FALSE);
}

/******** E O F ********/
