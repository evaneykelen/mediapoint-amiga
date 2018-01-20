#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct BitMap effBM;
extern UWORD chip gui_pattern[];
extern UBYTE **msgs;   
extern ULONG effDoubled;

/**** gadgets ****/

extern struct GadgetRecord WT_GR[];

/**** functions ****/

/******** SetWdwTransitions() ********/

void SetWdwTransitions(void)
{
struct Window *window;
BOOL loop, retVal=FALSE, laced=FALSE, tab, easy=FALSE;	//, fast=FALSE;
int ID, wdw, val, i, j, numWdw, *undo_mem, hitWdw;
WORD effectNr, speed, thick;

	if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
		easy=FALSE;
	else
		easy=TRUE;

#if 0
	if ( CED.Qualifier&IEQUALIFIER_LALT || CED.Qualifier&IEQUALIFIER_RALT )
	{
		fast=TRUE;
		easy=TRUE;
	}
	else
		fast=FALSE;
#endif

	/**** alloc undo memory ****/

	/**** in1[3], in2[3] etc. is 96 bytes ****/

	undo_mem = (int *)AllocMem(96*MAXEDITWINDOWS, MEMF_CLEAR);
	if (!undo_mem)
	{
		UA_WarnUser(227);
		return;
	}

	/**** init vars ****/

	retVal	= FALSE;
	loop		= TRUE;
	wdw			= FirstActiveEditWindow();

	if (wdw==-1)	// no wdw active, take first one
		wdw=0;

	numWdw=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
		if (EditSupportList[i]!=NULL)
			numWdw++;

	/**** fill undo buffer ****/

	for(i=0; i<numWdw; i++)
	{
		for(j=0; j<3; j++)
		{
			undo_mem[(24*i)+j+ 0] = (int)EditWindowList[i]->in1[j];
			undo_mem[(24*i)+j+ 3] = (int)EditWindowList[i]->in2[j];
			undo_mem[(24*i)+j+ 6] = (int)EditWindowList[i]->in3[j];

			undo_mem[(24*i)+j+ 9] = (int)EditWindowList[i]->out1[j];
			undo_mem[(24*i)+j+12] = (int)EditWindowList[i]->out2[j];
			undo_mem[(24*i)+j+15] = (int)EditWindowList[i]->out3[j];

			undo_mem[(24*i)+j+18] = (int)EditWindowList[i]->inDelay[j];
			undo_mem[(24*i)+j+21] = (int)EditWindowList[i]->outDelay[j];
		}
	}

	/**** double or halve gadgets ****/

	if ( !easy )
	{
		if ( UA_IsWindowOnLacedScreen(pageWindow) )
		{
			laced=TRUE;
			if ( WT_GR[0].x1 == 0 )	/* not doubled */
			{
				UA_DoubleGadgetDimensions(WT_GR);
				WT_GR[0].x1 = 1;
			}
		}
		else
		{
			if ( WT_GR[0].x1 == 1 )	/* doubled */
			{
				UA_HalveGadgetDimensions(WT_GR);
				WT_GR[0].x1 = 0;
			}
		}

		/**** open a window ****/

		window = UA_OpenRequesterWindow(pageWindow,WT_GR,STDCOLORS);
		if (!window)
		{
			FreeMem(undo_mem, 96*MAXEDITWINDOWS);
			UA_WarnUser(225);
			return;
		}
	
		/**** render gadgets ****/

		if (CPrefs.PageScreenModes & LACE)
			DoubleEffBM(TRUE);
		else
			DoubleEffBM(FALSE);

		RenderWdwTr(window, laced, wdw);

		if (numWdw==1)
		{
			UA_DisableButton(window, &WT_GR[18], gui_pattern);	// apply to all
			UA_DisableButton(window, &WT_GR[20], gui_pattern);	// next wdw
			UA_DisableButton(window, &WT_GR[21], gui_pattern);	// prev wdw
		}
	}

/*
	if ( fast )
	{
		retVal = ChooseEffectFast();
		loop=FALSE;	// skip while loop
	}
	else if ( easy )
*/

	if ( easy )
	{
		retVal = Monitor_PLS_Effect(wdw,numWdw);
		loop=FALSE;	// skip while loop
	}
	else
		DrawThickBorder(wdw);

	/**** monitor user ****/

	while(loop)
	{
		doStandardWait(window);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
			DetermineClickEvent(&hitWdw, FALSE);

		if ( numWdw>1 && CED.Class==IDCMP_RAWKEY && CED.Code==RAW_TAB )
		{
			if (	CED.Qualifier&IEQUALIFIER_LSHIFT ||
						CED.Qualifier&IEQUALIFIER_RSHIFT )
				goto prev_wdw;
			else
				goto next_wdw;
		}

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, WT_GR, &CED);

			if ( !(window->Flags & WFLG_WINDOWACTIVE) )
			{
				ID=-1;
				if (hitWdw!=-1)	// choose new edit window
				{
					DrawThickBorder(wdw);
					wdw=hitWdw;
					DrawThickBorder(wdw);

					if (numWdw==1)
					{
						UA_EnableButton(window, &WT_GR[18]);	// apply to all
						UA_EnableButton(window, &WT_GR[20]);	// next wdw
						UA_EnableButton(window, &WT_GR[21]);	// prev wdw
					}
					RenderWdwTr(window, laced, wdw);
					if (numWdw==1)
					{
						UA_DisableButton(window, &WT_GR[18], gui_pattern);	// apply to all
						UA_DisableButton(window, &WT_GR[20], gui_pattern);	// next wdw
						UA_DisableButton(window, &WT_GR[21], gui_pattern);	// prev wdw
					}
				}
			}

quick_again:

			switch(ID)
			{
				case 6:	// Window In Effect
				case 7:	// Picture In Effect
				case 8:	// Text In Effect
					UA_HiliteButton(window, &WT_GR[ID]);
					effectNr	= EditWindowList[wdw]->in1[ID-6];
					speed			= EditWindowList[wdw]->in2[ID-6];
					thick			= EditWindowList[wdw]->in3[ID-6];
					if ( Monitor_Effect(&effectNr, &speed, &thick, 2) )
					{
						EditWindowList[wdw]->in1[ID-6] = effectNr;
						EditWindowList[wdw]->in2[ID-6] = speed;
						EditWindowList[wdw]->in3[ID-6] = thick;
						PrintWTInfo(window, wdw);
					}
					break;

				case 9:		// Window In Delay
				case 10:	// Picture In Delay
				case 11:	// Text In Delay
				case 15:	// Window Out Delay
				case 16:	// Picture Out Delay
				case 17:	// Text Out Delay
					tab = UA_ProcessStringGadget(window, WT_GR, &WT_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&WT_GR[ID], &val);
					switch(ID)
					{
						case 9:
						case 10:
						case 11:
							EditWindowList[wdw]->inDelay[ID-9] = val;
							break;
						case 15:
						case 16:
						case 17:
							EditWindowList[wdw]->outDelay[ID-15] = val;
							break;
					}
					if (tab)
					{
						switch(ID)
						{
							case  9: val=10; break;
							case 10: val=11; break;
							case 11: val=15; break;
							case 15: val=16; break;
							case 16: val=17; break;
							case 17: val= 9; break;
							default: val=-1; break;
						}
						if (val!=-1)
						{
							ID=val;
							goto quick_again;
						}
					}
					break;

				case 12:	// Window Out Effect
				case 13:	// Picture Out Effect
				case 14:	// Text Out Effect
					UA_HiliteButton(window, &WT_GR[ID]);
					effectNr	= EditWindowList[wdw]->out1[ID-12];
					speed			= EditWindowList[wdw]->out2[ID-12];
					thick			= EditWindowList[wdw]->out3[ID-12];
					if ( Monitor_Effect(&effectNr, &speed, &thick, 2) )
					{
						EditWindowList[wdw]->out1[ID-12] = effectNr;
						EditWindowList[wdw]->out2[ID-12] = speed;
						EditWindowList[wdw]->out3[ID-12] = thick;
						PrintWTInfo(window, wdw);
					}
					break;

				case 18:	// apply this to all
					UA_HiliteButton(window, &WT_GR[ID]);
					for(i=0; i<numWdw; i++)
					{
						if (i!=wdw)
						{
							for(j=0; j<3; j++)
							{
								EditWindowList[i]->in1[j]			= EditWindowList[wdw]->in1[j];
								EditWindowList[i]->in2[j]			= EditWindowList[wdw]->in2[j];
								EditWindowList[i]->in3[j]			= EditWindowList[wdw]->in3[j];

								EditWindowList[i]->out1[j]		= EditWindowList[wdw]->out1[j];
								EditWindowList[i]->out2[j]		= EditWindowList[wdw]->out2[j];
								EditWindowList[i]->out3[j]		= EditWindowList[wdw]->out3[j];

								EditWindowList[i]->inDelay[j]		= EditWindowList[wdw]->inDelay[j];
								EditWindowList[i]->outDelay[j]	= EditWindowList[wdw]->outDelay[j];
							}
						}
					}
					break;

				case 20:	// next window
next_wdw:
					UA_HiliteButton(window, &WT_GR[20]);
					DrawThickBorder(wdw);
					wdw++;
					if (wdw==numWdw)
						wdw=0;
					DrawThickBorder(wdw);
					PrintWTInfo(window, wdw);
					break;

				case 21:	// prev window
prev_wdw:
					UA_HiliteButton(window, &WT_GR[21]);
					DrawThickBorder(wdw);
					wdw--;
					if (wdw<0)
						wdw=numWdw-1;
					DrawThickBorder(wdw);
					PrintWTInfo(window, wdw);
					break;

				case 22:	// OK
do_ok:
					UA_HiliteButton(window, &WT_GR[22]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 23:	// Cancel
do_cancel:
					UA_HiliteButton(window, &WT_GR[23]);
					loop=FALSE;
					retVal=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// Cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	if ( !easy )
	{	
		DrawThickBorder(wdw);
		if (numWdw==1)
		{
			UA_EnableButton(window, &WT_GR[18]);	// apply to all
			UA_EnableButton(window, &WT_GR[20]);	// next wdw
			UA_EnableButton(window, &WT_GR[21]);	// prev wdw
		}

		UA_CloseRequesterWindow(window,STDCOLORS);

		WT_GR[6].type = HIBOX_REGION;
		WT_GR[7].type = HIBOX_REGION;
		WT_GR[8].type = HIBOX_REGION;
	}

	if (!retVal)
	{
		for(i=0; i<numWdw; i++)
		{
			for(j=0; j<3; j++)
			{
				EditWindowList[i]->in1[j]				= (WORD)undo_mem[(24*i)+j+ 0];
				EditWindowList[i]->in2[j]				= (WORD)undo_mem[(24*i)+j+ 3];
				EditWindowList[i]->in3[j]				= (WORD)undo_mem[(24*i)+j+ 6];
	
				EditWindowList[i]->out1[j]			= (WORD)undo_mem[(24*i)+j+ 9];
				EditWindowList[i]->out2[j]			= (WORD)undo_mem[(24*i)+j+12];
				EditWindowList[i]->out3[j]			= (WORD)undo_mem[(24*i)+j+15];

				EditWindowList[i]->inDelay[j]		= undo_mem[(24*i)+j+18];
				EditWindowList[i]->outDelay[j]	= undo_mem[(24*i)+j+21];
			}
		}
	}

	FreeMem(undo_mem, 96*MAXEDITWINDOWS);
}

/******** PrintWTInfo() ********/

void PrintWTInfo(struct Window *window, int wdw)
{
int i, add;
WORD x,y;
TEXT str[5];
int effbm_h;

	if ( effDoubled==1 )
		effbm_h=18;
	else
		effbm_h=9;

	if ( UA_IsWindowOnLacedScreen(window) )
		add = 3;
	else
		add = 2;

	for(i=0; i<3; i++)
	{
		/**** in delays ****/

		UA_SetStringGadgetToVal(window, &WT_GR[9+i], (int)EditWindowList[wdw]->inDelay[i]);

		/**** out delays ****/

		UA_SetStringGadgetToVal(window, &WT_GR[15+i], (int)EditWindowList[wdw]->outDelay[i]);

		/**** in effect icons ****/

		UA_ClearButton(window, &WT_GR[6+i], AREA_PEN);

		if ( EditWindowList[wdw]->in1[i] != -1 )
		{
			GetEffectPos(EditWindowList[wdw]->in1[i], &x, &y);
			if (x!=-1)
			{
				RenderEffectIcon(window->RPort,x,y,WT_GR[6+i].x1+5,WT_GR[6+i].y1+add,21,effbm_h);
/*
				BltBitMapRastPort(&effBM, x,y, window->RPort,
													WT_GR[6+i].x1+5, WT_GR[6+i].y1+add, 21, effbm_h, 0xc0);
*/
			}
		}
		else
		{
			SetAPen(window->RPort, LO_PEN);
			SetDrMd(window->RPort, JAM1);
			Move(	window->RPort, WT_GR[6+i].x1+10,
						WT_GR[6+i].y1 + window->RPort->TxBaseline + add);
			Text(window->RPort, "š", 1L);
		}
		
		/**** out effect icons ****/

		UA_ClearButton(window, &WT_GR[12+i], AREA_PEN);

		if ( EditWindowList[wdw]->out1[i] != -1 )
		{
			GetEffectPos(EditWindowList[wdw]->out1[i], &x, &y);
			if (x!=-1)
			{
				RenderEffectIcon(window->RPort,x,y,WT_GR[12+i].x1+5,WT_GR[12+i].y1+add,21,effbm_h);
/*
				BltBitMapRastPort(&effBM, x,y, window->RPort,
													WT_GR[12+i].x1+5, WT_GR[12+i].y1+add, 21, effbm_h, 0xc0);
*/
			}
		}
		else
		{
			SetAPen(window->RPort, LO_PEN);
			SetDrMd(window->RPort, JAM1);
			Move(	window->RPort, WT_GR[12+i].x1+10,
						WT_GR[12+i].y1 + window->RPort->TxBaseline + add);
			Text(window->RPort, "š", 1L);
		}
	}

	sprintf(str, "%d", (int)wdw+1);
	UA_PrintInBox(	window, &WT_GR[19], WT_GR[19].x1,WT_GR[19].y1,WT_GR[19].x2,WT_GR[19].y2,
									str, PRINT_CENTERED);
}

/******** RenderWdwTr() ********/

void RenderWdwTr(struct Window *window, BOOL laced, int wdw)
{
//int add;

	WT_GR[6].type = HIBOX_REGION;
	WT_GR[7].type = HIBOX_REGION;
	WT_GR[8].type = HIBOX_REGION;

	UA_DrawGadgetList(window, WT_GR);

	WT_GR[6].type = BUTTON_GADGET;
	WT_GR[7].type = BUTTON_GADGET;
	WT_GR[8].type = BUTTON_GADGET;

	UA_DrawSpecialGadgetText(window, &WT_GR[ 6], msgs[Msg_In-1],    SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &WT_GR[ 9], msgs[Msg_Delay-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &WT_GR[12], msgs[Msg_Out-1],   SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &WT_GR[15], msgs[Msg_Delay-1], SPECIAL_TEXT_TOP);

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);

	PrintHorizText(window, &WT_GR[20], 3, "–\0");
	PrintHorizText(window, &WT_GR[21], 3, "—\0");

	/**** set gadgets to values and states ****/

	PrintWTInfo(window, wdw);
}

/******** DrawThickBorder() ********/

void DrawThickBorder(int wdw)
{
	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[wdw]->X+1, EditWindowList[wdw]->Y+1,
									EditWindowList[wdw]->X+EditWindowList[wdw]->Width-2,
									EditWindowList[wdw]->Y+EditWindowList[wdw]->Height-2 );
	DrawMarqueeBox(	pageWindow->RPort,
									EditWindowList[wdw]->X+2, EditWindowList[wdw]->Y+2,
									EditWindowList[wdw]->X+EditWindowList[wdw]->Width-3,
									EditWindowList[wdw]->Y+EditWindowList[wdw]->Height-3 );
}

/******** RenderEffectIcon() ********/

void RenderEffectIcon(struct RastPort *rp, WORD srcX, WORD srcY,
											WORD dstX, WORD dstY, WORD dstW, WORD dstH)
{
int pen;

	if ( rp->BitMap->Depth==1 )
		pen=0;	
	else
		pen=2;	// white
 
	SetAPen(rp,pen);
	SetDrMd(rp,JAM1);
	RectFill(rp,dstX,dstY,dstX+dstW-1,dstY+dstH-1);

	SetAPen(rp,1L);	// black
	BltTemplate(effBM.Planes[0]+80*srcY+(srcX/8), 0, 80, rp, dstX, dstY, dstW, dstH);
}

/******** E O F ********/
