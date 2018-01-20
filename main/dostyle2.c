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

extern struct GadgetRecord Style_GR[];

/**** functions ****/

/******** ToggleStyle() ********/

BOOL ToggleStyle(void)
{
int wdw, i;

	ScreenAtNormalPos();

	wdw = FirstActiveEditWindow();

	if ( smallWindow && (allocFlags & STYLE_WDW_FLAG) )
	{
		if ( IntuitionBase->FirstScreen == smallWindow->WScreen )
		{
			MyScreenToBack(smallWindow->WScreen);
		}
		else
		{
			EnableDisableStyleList(smallWindow,wdw);
			ScreenToFront(smallWindow->WScreen);
			if ( !SmallScreenCorrect(smallWindow->WScreen) )
				goto reopen;
			ActivateWindow(smallWindow);
		}			
	}
	else
	{
reopen:
		for(i=2; i<=12; i++)
			UA_EnableButtonQuiet(&Style_GR[i]);

		if ( CPrefs.PageScreenModes & LACE )
		{
			if ( Style_GR[0].x1 == 0 )	// not doubled 
			{
				UA_DoubleGadgetDimensions(Style_GR);
				Style_GR[0].x1 = 1;
			}
		}
		else
		{
			if ( Style_GR[0].x1 == 1 )	// doubled
			{
				UA_HalveGadgetDimensions(Style_GR);
				Style_GR[0].x1 = 0;
			}
		}

		smallWindow = OpenSmallScreen( Style_GR[0].y2 ); 
		if (!smallWindow)
		{
			UA_WarnUser(-1);
			return(FALSE);
		}
		SetBit(&allocFlags, STYLE_WDW_FLAG);

		UA_DrawGadgetList(smallWindow, Style_GR);
		UA_DrawSpecialGadgetText(	smallWindow, &Style_GR[12], msgs[Msg_CrawlColor-1],
															SPECIAL_TEXT_AFTER_STRING );

		EnableDisableStyleList(smallWindow,wdw);

		ScreenToFront(smallWindow->WScreen);
	}

	return(TRUE);
}

/******** MonitorStyle() ********/

int MonitorStyle(void)
{
BOOL takeAction = TRUE;
int ID, val, wdw, color;
ULONG signals;
struct EditWindow localEW;
WORD x,y,w,h;

	wdw = FirstActiveEditWindow();

	if ( wdw!=-1 )
		CopyMem(EditWindowList[wdw],&localEW,sizeof(struct EditWindow));

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
				ID = UA_CheckGadgetList(smallWindow, Style_GR, &CED);

				if ( wdw==-1 && ID!=13 && ID!=-1 )
					ID=-1;

				switch(ID)
				{
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
						UA_ProcessCycleGadget(smallWindow, &Style_GR[ID], &CED);
						UA_SetValToCycleGadgetVal(&Style_GR[ID], &val);
						switch(ID)
						{
							case 2:		localEW.antiAliasLevel = val;
												break;
							case 3:		localEW.justification = val;
												w = GFX_JUST_W;
												if ( smallWindow->WScreen->ViewPort.Modes & LACE )
												{
													y = GFX_L_JUST_Y;
													h = GFX_L_JUST_H;
												}
												else
												{
													y = GFX_NL_JUST_Y;
													h = GFX_NL_JUST_H;
												}
												if ( localEW.justification==0 )	x=GFX_JUST_L_X;
												if ( localEW.justification==1 )	x=GFX_JUST_C_X;
												if ( localEW.justification==2 )	x=GFX_JUST_R_X;
												PutImageInCycleGadget(smallWindow, &Style_GR[3],x,y,w,h);
												break;
							case 4:		localEW.xSpacing = val-3;				break;
							case 5:		localEW.ySpacing = val-3;				break;
							case 6:		switch( val )
												{
													case 0:
														localEW.slantAmount = 1;
														localEW.slantValue = -1;
														break;
													case 1:
														localEW.slantAmount = 2;
														localEW.slantValue = -1;
														break;
													case 2:
														localEW.slantAmount = 4;
														localEW.slantValue = -1;
														break;
													case 3:
														localEW.slantAmount = 4;
														localEW.slantValue = 1;
														break;
													case 4:
														localEW.slantAmount = 2;
														localEW.slantValue = 1;
														break;
													case 5:
														localEW.slantAmount = 1;
														localEW.slantValue = 1;
														break;
												}
												break;
							case 7:		localEW.shadowType = val;
												w = GFX_SHAD_W;
												if ( smallWindow->WScreen->ViewPort.Modes & LACE )
												{
													y = GFX_L_SHAD_Y;
													h = GFX_L_SHAD_H;
												}
												else
												{
													y = GFX_NL_SHAD_Y;
													h = GFX_NL_SHAD_H;
												}
												if ( localEW.shadowType==0 ) x=GFX_SHAD_N_X;
												if ( localEW.shadowType==1 ) x=GFX_SHAD_C_X;
												if ( localEW.shadowType==2 ) x=GFX_SHAD_S_X;
												if ( localEW.shadowType==3 ) x=GFX_SHAD_O_X;
												if ( localEW.shadowType==4 ) x=GFX_SHAD_T_X;
												PutImageInCycleGadget(smallWindow, &Style_GR[7],x,y,w,h);
												break;
							case 8:		localEW.shadowDepth = val;				break;
							case 9:		localEW.shadowDirection = val;		break;
							case 10:	localEW.underLineHeight = val+1;	break;
							case 11:	localEW.underLineOffset = val-3;	break;
						}
						InterpretStyleSettings(&localEW,ID);
						break;

					case 12:
						UA_HiliteButton(smallWindow, &Style_GR[ID]);
						color = OpenSmallPalette(localEW.shadow_Pen, 2, FALSE);
						if (color!=-1)
						{
							localEW.shadow_Pen = color;
							InterpretStyleSettings(&localEW,ID);
							PaintButton(smallWindow, &Style_GR[12], 8);
							DoPaintedButton(8, color);
						}
						break;

					case 13:	// Hide
						UA_HiliteButton(smallWindow, &Style_GR[13]);
						PaletteToBack();
						return(NOTHING_TO_EXAMINE);
						break;
				}
			}
			else if ( CED.Class==IDCMP_RAWKEY || CED.Class==IDCMP_MENUPICK )
				return(IDCMP_TO_EXAMINE);
		}

		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(smallWindow,FALSE,NULL);
			takeAction=TRUE;
			if ( !(smallWindow->Flags & WFLG_WINDOWACTIVE) )
			{
				return(IDCMP_TO_EXAMINE);
			}
		}
		else
			takeAction=FALSE;
	}
	while(1);

	return(NOTHING_TO_EXAMINE);
}

/******** CloseStyleScreen() ********/

void CloseStyleScreen(void)
{
	if ( smallWindow )
	{
		CloseSmallScreen(smallWindow);
		UnSetBit(&allocFlags, STYLE_WDW_FLAG);
	}
}

/******** DoStyleSettings() ********/
/*
 * This function is called whenever an edit window is selected
 *
 */

void DoStyleSettings(void)
{
int wdw;

	if ( !smallWindow || !(allocFlags & STYLE_WDW_FLAG) )
		return;

	wdw = FirstActiveEditWindow();
	EnableDisableStyleList(smallWindow, wdw);
}

/******** EnableDisableStyleList() ********/

void EnableDisableStyleList(struct Window *window, int wdw)
{
BOOL enableStuff;
int i;

	if ( wdw==-1 )
		enableStuff=FALSE;
	else
		enableStuff=TRUE;

	if ( enableStuff )
	{
		if ( UA_IsGadgetDisabled(&Style_GR[2]) )
			for(i=2; i<=12; i++)
				UA_EnableButton(window, &Style_GR[i]);
		RenderStyleButtons(window,wdw);
	}
	else
	{
		if ( !UA_IsGadgetDisabled(&Style_GR[2]) )
			for(i=2; i<=12; i++)
				UA_DisableButton(window, &Style_GR[i], gui_pattern);
	}
}

/******** RenderStyleButtons() ********/

void RenderStyleButtons(struct Window *window, int wdw)
{
int i,val;
struct EditWindow *ew;
WORD x,y,w,h;

	if (wdw==-1)
		return;
	else
		ew = EditWindowList[wdw];

	for(i=2; i<=12; i++)
		UA_EnableButtonQuiet(&Style_GR[i]);

	UA_SetCycleGadgetToVal(window, &Style_GR[2], ew->antiAliasLevel);

	UA_SetCycleGadgetToVal(window, &Style_GR[3], (int)ew->justification);
	w = GFX_JUST_W;
	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		y = GFX_L_JUST_Y;
		h = GFX_L_JUST_H;
	}
	else
	{
		y = GFX_NL_JUST_Y;
		h = GFX_NL_JUST_H;
	}
	if ( ew->justification==0 )	x=GFX_JUST_L_X;
	if ( ew->justification==1 )	x=GFX_JUST_C_X;
	if ( ew->justification==2 )	x=GFX_JUST_R_X;
	PutImageInCycleGadget(window, &Style_GR[3],x,y,w,h);

	UA_SetCycleGadgetToVal(window, &Style_GR[4], (int)ew->xSpacing+3);
	UA_SetCycleGadgetToVal(window, &Style_GR[5], (int)ew->ySpacing+3);

	val=-1;
	switch( ew->slantValue )
	{
		case -1:
			switch( ew->slantAmount )
			{
				case 1:
					val = 0;		// -45°
					break;
				case 2:
					val = 1;		// -27°
					break;
				case 4:
					val = 2;		// -14°
					break;
			}
			break;
		case 1:
			switch( ew->slantAmount )
			{
				case 4:
					val = 3;		// +14°
						break;
					case 2:
						val = 4;	// +27°
						break;
					case 1:
						val = 5;	// +45°
						break;
				}
				break;
	}
	if (val!=-1)
		UA_SetCycleGadgetToVal(window, &Style_GR[6], val);

	UA_SetCycleGadgetToVal(window, &Style_GR[7], (int)ew->shadowType);
	w = GFX_SHAD_W;
	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		y = GFX_L_SHAD_Y;
		h = GFX_L_SHAD_H;
	}
	else
	{
		y = GFX_NL_SHAD_Y;
		h = GFX_NL_SHAD_H;
	}
	if ( ew->shadowType==0 ) x=GFX_SHAD_N_X;
	if ( ew->shadowType==1 ) x=GFX_SHAD_C_X;
	if ( ew->shadowType==2 ) x=GFX_SHAD_S_X;
	if ( ew->shadowType==3 ) x=GFX_SHAD_O_X;
	if ( ew->shadowType==4 ) x=GFX_SHAD_T_X;
	PutImageInCycleGadget(window, &Style_GR[7],x,y,w,h);

	UA_SetCycleGadgetToVal(window, &Style_GR[ 8], (int)ew->shadowDepth);
	UA_SetCycleGadgetToVal(window, &Style_GR[ 9], (int)ew->shadowDirection);
	UA_SetCycleGadgetToVal(window, &Style_GR[10], (int)ew->underLineHeight-1);
	UA_SetCycleGadgetToVal(window, &Style_GR[11], (int)ew->underLineOffset+3);

	PaintButton(smallWindow, &Style_GR[12], 8);
	DoPaintedButton(8, ew->shadow_Pen);
}

/******** InterpretStyleSettings() ********/

void InterpretStyleSettings(struct EditWindow *localEW, int button)
{
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
BOOL list[MAXEDITWINDOWS];
int i;
BOOL redraw=TRUE;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->Active )
		{
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

	switch( button )
	{
		case 2:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->antiAliasLevel = localEW->antiAliasLevel;
			break;

		case 3:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->justification = localEW->justification;
			break;

		case 4:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->xSpacing = localEW->xSpacing;
			break;

		case 5:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->ySpacing = localEW->ySpacing;
			break;

		case 6:
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] && EditSupportList[i]->Active )
				{
					EditWindowList[i]->slantAmount = localEW->slantAmount;
					EditWindowList[i]->slantValue = localEW->slantValue;
				}
			}
			break;

		case 7:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->shadowType = localEW->shadowType;
			break;

		case 8:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->shadowDepth = localEW->shadowDepth;
			break;

		case 9:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->shadowDirection = localEW->shadowDirection;
			break;

		case 10:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->underLineHeight = localEW->underLineHeight;
			break;

		case 11:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->underLineOffset = localEW->underLineOffset;
			break;

		case 12:
			for(i=0; i<MAXEDITWINDOWS; i++)
				if ( EditWindowList[i] && EditSupportList[i]->Active )
					EditWindowList[i]->shadow_Pen = localEW->shadow_Pen;
			break;
	}

	if ( redraw )
	{
		DrawAllHandles(LEAVE_ACTIVE);
		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);
		DrawAllHandles(LEAVE_ACTIVE);
	}
}

/******** E O F ********/
