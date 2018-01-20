#include "nb:pre.h"

/**** externals ****/

extern ULONG allocFlags;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern UWORD chip gui_pattern[];
extern struct eventHandlerInfo EHI;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Gadget PropSlider1;
extern struct BitMap effBM;
extern UWORD *lookUpList_Brush;
extern int NUMBRUSHES;
extern UBYTE *brushNames;
extern UWORD *brushNumbers;
extern UBYTE **brushList;
extern ULONG effDoubled;

/**** gadgets ****/

extern struct GadgetRecord PLS_ChooseEffect_2_GR[];

/**** functions ****/

/******** Monitor_PLS_Effect() ********/

BOOL Monitor_PLS_Effect(int wdw, int numWdw)
{
int numEntries, topEntry, line, thickMax, thickPos, val, ID, i, hitWdw, aa;
UBYTE *selectionList;
WORD effect, speed, thick;
struct Window *window, *activeWindow;
BOOL loop=TRUE, retval=FALSE, dblClicked=FALSE;
struct GadgetRecord *GR;
struct ScrollRecord SR;
BOOL touched[MAXEDITWINDOWS];

	/**** init vars ****/

	numEntries = NUMBRUSHES;
	topEntry=0;
	line=-1;

	for(i=0; i<MAXEDITWINDOWS; i++)
		touched[ i ] = FALSE;

	GR = PLS_ChooseEffect_2_GR;

	/**** scan page xapp for effect graphics and effect names ****/

	if ( !TestBit(allocFlags, EFFECTINFO_FLAG) )
	{
		if ( !GetInfoFromPageXaPP() )
			return(FALSE);
	}

	/**** open requester ****/

	//SetStandardColors(pageWindow);

	window = OpenEffectWindow(pageWindow, GR, numEntries, topEntry, 8);
	if ( !window )
		return(FALSE);

	selectionList = (UBYTE *)AllocMem(numEntries, MEMF_ANY | MEMF_CLEAR);
	if (selectionList==NULL)
	{
		UA_CloseRequesterWindow(window,STDCOLORS);
		UA_WarnUser(105);
		return(FALSE);
	}

	/**** draw gadgets ****/

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);

	PrintHorizText(window, &GR[17], 3, "–\0");
	PrintHorizText(window, &GR[18], 3, "—\0");

	/**** see which effect is selected ****/

	effect	= EditWindowList[wdw]->in1[0];
	speed		= EditWindowList[wdw]->in2[0];
	thick		= EditWindowList[wdw]->in3[0];

	if (numWdw==1)
	{
		UA_DisableButton(window, &GR[17], gui_pattern);	// next wdw
		UA_DisableButton(window, &GR[18], gui_pattern);	// prev wdw
		UA_DisableButton(window, &GR[21], gui_pattern);	// apply to all
	}

	/**** init scroll record ****/

	SR.GR							= &GR[2];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= &selectionList[0];
	SR.entryWidth			= -1;
	SR.numDisplay			= 8;
	SR.numEntries			= numEntries;

	GR[2].x1 = GR[2].x1 + 25; 

	if (CPrefs.PageScreenModes & LACE)
		DoubleEffBM(TRUE);
	else
		DoubleEffBM(FALSE);

	/**** init speed and thick gadgets ****/

	if ( effect == -1 )	// no effect selected yet, ghost speed and chunck
	{
		UA_SetSliderGadg(window, &GR[7], 0, 20, &GR[9], 1);		// speed
		UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
		UA_DisableButton(window, &GR[7], gui_pattern);				// speed
		UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
	}
	else
	{
		if ( UA_IsGadgetDisabled(&GR[7]) )
			UA_EnableButton(window, &GR[7]);	// speed
		if ( UA_IsGadgetDisabled(&GR[8]) )
			UA_EnableButton(window, &GR[8]);	// chunck

		/********** SET SLIDERS ************/
		UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);
		GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
		ConvertThickToChunck(thick, &thickPos, &thickMax);
		thick = thickPos;
		if ( thickPos!=-1 )
			UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
		else	// this is a no chunck effect
		{
			UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
			UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
		}

		for(i=0; i<numEntries; i++)	// clear list
			*(selectionList+i) = 0;
		if ( effect != -1 )
			*( selectionList + lookUpList_Brush[ effect ] ) = 1;	// hilite effect

		/********** SLIDERS SET ************/
	}

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR,topEntry,brushList);
	PrintIconList(&GR[2], window, topEntry, 8);

	Show_PLS_Eff(window,GR,wdw);

	DrawThickBorder(wdw);

	/**** monitor user ****/

	while(loop)
	{
		doStandardWait(window);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		Forbid();
		activeWindow = IntuitionBase->ActiveWindow;
		Permit();

		/**** process page window events ****/

		if ( activeWindow!=window )	// page or edit windows hit
		{
			if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
				DetermineClickEvent(&hitWdw, FALSE);
			if ( hitWdw != -1 )
			{
				DrawThickBorder(wdw);
				wdw = hitWdw;
				DrawThickBorder(wdw);

reselect:

				effect	= EditWindowList[wdw]->in1[0];
				speed		= EditWindowList[wdw]->in2[0];
				thick		= EditWindowList[wdw]->in3[0];

					if ( UA_IsGadgetDisabled(&GR[7]) )
						UA_EnableButton(window, &GR[7]);	// speed
					if ( UA_IsGadgetDisabled(&GR[8]) )
						UA_EnableButton(window, &GR[8]);	// chunck

				if ( effect == -1 )	// no effect selected yet, ghost speed and chunck
				{
					UA_SetSliderGadg(window, &GR[7], 0, 20, &GR[9], 1);		// speed
					UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
					UA_DisableButton(window, &GR[7], gui_pattern);				// speed
					UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
				}
				else
				{
					/********** SET SLIDERS ************/
					if ( UA_IsGadgetDisabled(&GR[7]) )
						UA_EnableButton(window, &GR[7]);	// speed
					if ( UA_IsGadgetDisabled(&GR[8]) )
						UA_EnableButton(window, &GR[8]);	// chunck

					UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);

					GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
					ConvertThickToChunck(thick, &thickPos, &thickMax);
					thick = thickPos;

					if ( thick!=-1 )
						UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
					else	// this is a no chunck effect
					{
						UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
						UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
					}
					/********** SLIDERS SET ************/
				}

				for(i=0; i<numEntries; i++)	// clear list
					*(selectionList+i) = 0;
				if ( effect != -1 )
					*( selectionList + lookUpList_Brush[ effect ] ) = 1;	// hilite effect

				UA_PrintStandardList(NULL,-1,NULL);	// init static
				UA_PrintStandardList(&SR,topEntry,brushList);

				Show_PLS_Eff(window,GR,wdw);
			}
			CED.Class = NULL;
			ActivateWindow(window);
		}

		/**** process effect window events ****/

		if ( numWdw>1 && CED.Class==IDCMP_RAWKEY && CED.Code==RAW_TAB )
		{
			if (	CED.Qualifier&IEQUALIFIER_LSHIFT ||
						CED.Qualifier&IEQUALIFIER_RSHIFT )
				goto prev_wdw;
			else
				goto next_wdw;
		}
		else if (CED.Class==IDCMP_MOUSEBUTTONS)
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				ScrollEffectList(	&GR[2], window, NULL, selectionList, -1,
													numEntries, &topEntry, 8, &PropSlider1, brushList, &SR);
				CED.Code = 0;
			}

			/**** process buttons ****/

			if (CED.Code==SELECTDOWN)
			{
				GR[2].x1 = GR[2].x1 - 25; 
				ID = UA_CheckGadgetList(window, GR, &CED);
				GR[2].x1 = GR[2].x1 + 25; 
				switch(ID)
				{
					case 2:	// scroll area
						line = UA_SelectStandardListLine(&SR,topEntry,FALSE,&CED,FALSE,FALSE);
						if (line!=-1)
						{
							effect = *( brushNumbers + (5*(topEntry+line)) );

							/**** effect is now known: derive all other vars from it ****/

							if ( UA_IsGadgetDisabled(&GR[7]) )
								UA_EnableButton(window, &GR[7]);	// speed
							if ( UA_IsGadgetDisabled(&GR[8]) )
								UA_EnableButton(window, &GR[8]);	// chunck

							/**** speed ****/

							speed = GetSpeed(effect);
							UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);

							/**** thick ****/

							thick = GetThick(effect);
							GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
							ConvertThickToChunck(thick, &thickPos, &thickMax);
							thick = thickPos;

							if ( thick!=-1 )
								UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
							else	// this is a no chunck effect
							{
								UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
								UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
							}

							//Show_PLS_Eff(window,GR,wdw);	OLD PLACE

							GetThickMax(effect, &aa);	// returns 1,2,3 or 4
							ConvertChunckToThick(thick, &val, aa);
							EditWindowList[wdw]->in1[0] = effect;
							EditWindowList[wdw]->in2[0] = speed;
							EditWindowList[wdw]->in3[0] = val;
							touched[wdw] = TRUE;

							Show_PLS_Eff(window,GR,wdw);	// NEW PLACE

							if (dblClicked)
								goto do_ok;

						}
						else
							effect = -1;
						break;

					case 4:	// OK
do_ok:
						UA_HiliteButton(window, &GR[4]);
						retval=TRUE;
						loop=FALSE;
						break;

					case 5:	// Cancel
do_cancel:
						UA_HiliteButton(window, &GR[5]);
						retval=FALSE;
						loop=FALSE;
						break;

					case 7:	// speed
						val = speed-1;
						UA_ProcessSliderGadg(window, &GR[ID], &val, 20, &GR[9], &CED, NULL,NULL,0,1);
						speed = val+1;
						EditWindowList[wdw]->in2[0] = speed;
						touched[wdw] = TRUE;
						break;

					case 8:	// chunck size
						val = thick;
						UA_ProcessSliderGadg(window, &GR[ID], &val, thickMax, &GR[10], &CED, NULL,NULL,0,1);
						thick = val;
						GetThickMax(effect, &aa);	// returns 1,2,3 or 4
						ConvertChunckToThick(thick, &val, aa);
						EditWindowList[wdw]->in3[0] = val;
						touched[wdw] = TRUE;
						break;

					case 17:	// next window
next_wdw:
						UA_HiliteButton(window, &GR[17]);
						DrawThickBorder(wdw);
						wdw++;
						if ( wdw==numWdw )
							wdw=0;
						DrawThickBorder(wdw);
						goto reselect;
						break;

					case 18:	// prev window
prev_wdw:
						UA_HiliteButton(window, &GR[18]);
						DrawThickBorder(wdw);
						wdw--;
						if (wdw<0)
							wdw=numWdw-1;
						DrawThickBorder(wdw);
						goto reselect;
						break;

					case 19:	// delay
						UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
						UA_SetValToStringGadgetVal(&GR[ID], &val);
						EditWindowList[wdw]->inDelay[0] = val;
						touched[wdw] = TRUE;
						break;

					case 20:	// no effect
						UA_HiliteButton(window, &GR[ID]);
						EditWindowList[wdw]->in1[0] = -1;
						EditWindowList[wdw]->in2[0] = 0;
						EditWindowList[wdw]->in3[0] = 0;
						touched[wdw] = TRUE;
						Show_PLS_Eff(window,GR,wdw);
						for(i=0; i<numEntries; i++)	// clear list
							*(selectionList+i) = 0;
						UA_PrintStandardList(NULL,-1,NULL);	// init static
						UA_PrintStandardList(&SR,topEntry,brushList);
						UA_SetSliderGadg(window, &GR[7], 0, 20, &GR[9], 1);		// speed
						UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
						UA_DisableButton(window, &GR[7], gui_pattern);				// speed
						UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
						break;

					case 21:	// all
						UA_HiliteButton(window, &GR[ID]);
						for(i=0; i<numWdw; i++)
						{
							EditWindowList[i]->in1[0]			= EditWindowList[wdw]->in1[0];
							EditWindowList[i]->in2[0]			= EditWindowList[wdw]->in2[0];
							EditWindowList[i]->in3[0]			= EditWindowList[wdw]->in3[0];
							EditWindowList[i]->inDelay[0]	= EditWindowList[wdw]->inDelay[0];
							touched[i] = TRUE;
						}
						break;
				}
			}
		}
		else if (CED.Class==IDCMP_RAWKEY )
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if ( CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&GR[4]) )	// OK
				goto do_ok;
		}
	}

	DrawThickBorder(wdw);

	UA_EnableButton(window, &GR[7]);	// speed
	UA_EnableButton(window, &GR[8]);	// chunck

	UA_EnableButton(window, &GR[17]);	// next wdw
	UA_EnableButton(window, &GR[18]);	// prev wdw
	UA_EnableButton(window, &GR[21]);	// apply to all

	UA_CloseRequesterWindow(window,STDCOLORS);

	//ResetStandardColors(pageWindow);

	FreeMem(selectionList, numEntries);

	GR[2].x1 = GR[2].x1 - 25; 

	if (retval)
	{
		for(i=0; i<numWdw; i++)
		{
			if ( touched[ i ] )
			{
				EditWindowList[i]->in1[1] = -1;
				EditWindowList[i]->in1[2] = -1;

				EditWindowList[i]->in2[1] = -1;
				EditWindowList[i]->in2[2] = -1;

				EditWindowList[i]->in3[1] = -1;
				EditWindowList[i]->in3[2] = -1;

				EditWindowList[i]->out1[0] = -1;
				EditWindowList[i]->out1[1] = -1;
				EditWindowList[i]->out1[2] = -1;

				EditWindowList[i]->out2[0] = -1;
				EditWindowList[i]->out2[1] = -1;
				EditWindowList[i]->out2[2] = -1;

				EditWindowList[i]->out3[0] = -1;
				EditWindowList[i]->out3[1] = -1;
				EditWindowList[i]->out3[2] = -1;
			}
		}
	}

	return(retval);
}

/******** Show_PLS_Eff() ********/

void Show_PLS_Eff(struct Window *window, struct GadgetRecord *GR, int wdw)
{
TEXT str[10];
WORD x,y;
int add;
int effbm_h;

	if ( effDoubled==1 )
		effbm_h=18;
	else
		effbm_h=9;

	if ( UA_IsWindowOnLacedScreen(window) )
		add=5;
	else
		add=3;

	/**** print wdw nr ****/

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);

	UA_ClearButton(window, &GR[16], AREA_PEN);
	sprintf(str, "%d", wdw+1);
	UA_PrintInBox(window, &GR[16], GR[16].x1, GR[16].y1, GR[16].x2, GR[16].y2, str, PRINT_CENTERED);

	/**** draw effect bitmap ****/

	UA_ClearButton(window, &GR[22], AREA_PEN);

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);

	if ( EditWindowList[wdw]->in1[0] != -1 )
	{
		GetEffectPos(EditWindowList[wdw]->in1[0], &x, &y);
		if (x!=-1)
		{
			RenderEffectIcon(window->RPort,x,y,GR[22].x1+5,GR[22].y1+add,21,effbm_h);
		}
	}
	else
	{
		Move(window->RPort, GR[22].x1+10,	GR[22].y1 + window->RPort->TxBaseline + add);
		Text(window->RPort, "š", 1L);
	}

	/**** show delay ****/

	UA_SetStringGadgetToVal(window, &GR[19], (int)EditWindowList[wdw]->inDelay[0]);
}

/******** E O F ********/
