#include "nb:pre.h"

#define NUM_WDW_OPTS 13
#define NUM_BGND_OPTS 11

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
extern BOOL do_the_tile;
extern struct ColorMap *undoCM;
extern struct RendezVousRecord rvrec;
extern struct Window *smallWindow;

/**** static globals ****/

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};
static int sp_topEntry = 0;
static UBYTE *windowOptsList[25];
static UBYTE *backgroundOptsList[25];
static struct ScrollRecord SR;

/**** gadgets ****/

extern struct GadgetRecord Specials_GR[];
extern struct GadgetRecord SmallWarning_GR[];

/**** functions ****/

/******** ToggleSpecials() ********/

BOOL ToggleSpecials(void)
{
int i, numLines, numDisp, page, current_page, wdw, actWdw;

	ScreenAtNormalPos();

	/**** init vars ****/

	UA_SetValToCycleGadgetVal(&Specials_GR[2], &current_page);

	// START SMARTSTEP

	actWdw = NumActiveEditWindows();
	if ( actWdw > 0 )
		page=1;	// show picture controls
	else
		page=0;	// show background controls

	// END SMARTSTEP

	if ( page==0 )
		numLines = NUM_BGND_OPTS;
	else
		numLines = NUM_WDW_OPTS;
	numDisp = 5;
	wdw = FirstActiveEditWindow();

	if ( smallWindow && (allocFlags & SPECIALS_WDW_FLAG) )
	{
		if ( IntuitionBase->FirstScreen == smallWindow->WScreen )
			MyScreenToBack(smallWindow->WScreen);
		else
		{
			// START REST OF SMARTSTEP
			if ( current_page != page )
			{
				UA_SetCycleGadgetToVal(smallWindow, &Specials_GR[2], page);
				if ( page==0 )
					numLines = NUM_BGND_OPTS;
				else
					numLines = NUM_WDW_OPTS;
				SR.numEntries = numLines;
				sp_topEntry=0;
				UA_SetPropSlider(smallWindow, &PropSlider, SR.numEntries, SR.numDisplay, sp_topEntry);
				Specials_GR[4].type = HIBOX_REGION;
				UA_DrawTwoColorBox(smallWindow, &Specials_GR[4]);
				Specials_GR[4].type = BUTTON_GADGET;
				DrawSpecialsPage(smallWindow,page,wdw,windowOptsList,backgroundOptsList);
				UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
				if ( page==0 )
					UA_PrintNewList(&SR,sp_topEntry,backgroundOptsList,FALSE);
				else if ( page==1 )
					UA_PrintNewList(&SR,sp_topEntry,windowOptsList,FALSE);
				EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);
			}
			// END REST OF SMARTSTEP
			EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);
			ScreenToFront(smallWindow->WScreen);
			if ( !SmallScreenCorrect(smallWindow->WScreen) )
				goto reopen;
			ActivateWindow(smallWindow);
		}			
	}
	else
	{
reopen:
		UA_EnableButtonQuiet(&Specials_GR[2]);
		UA_EnableButtonQuiet(&Specials_GR[4]);

		/**** set up user interface ****/

		for(i=0; i<25; i++)
		{	
			windowOptsList[i] = NULL;
			backgroundOptsList[i] = NULL;
		}

		windowOptsList[ 0] 		  = msgs[Msg_Ex_Remap-1];
		windowOptsList[ 1] 		  = msgs[Msg_Ex_Resize-1];
		windowOptsList[ 2] 		  = msgs[Msg_Ex_Transparent-1];
		windowOptsList[ 3] 		  = msgs[Msg_Ex_NoDither-1];
		windowOptsList[ 4] 		  = msgs[Msg_Ex_Floyd-1];
		windowOptsList[ 5] 		  = msgs[Msg_Ex_Burkes-1];
		windowOptsList[ 6] 		  = msgs[Msg_Ex_Ordered-1];
		windowOptsList[ 7] 		  = msgs[Msg_Ex_Random-1];
		windowOptsList[ 8] 		  = msgs[Msg_Ex_Delete-1];
		windowOptsList[ 9] 		  = msgs[Msg_Ex_UseColors-1];
		windowOptsList[10] 			= msgs[Msg_Ex_Conform-1];
		windowOptsList[11] 			= msgs[Msg_Ex_Offset-1];
		windowOptsList[12] 			= msgs[Msg_Ex_AnimSettings-1];

		backgroundOptsList[ 0]	= msgs[Msg_Ex_Remap-1];
		backgroundOptsList[ 1]  = msgs[Msg_Ex_Resize-1];
		backgroundOptsList[ 2]  = msgs[Msg_Ex_Transparent-1];
		backgroundOptsList[ 3]  = msgs[Msg_Ex_NoDither-1];
		backgroundOptsList[ 4]  = msgs[Msg_Ex_Floyd-1];
		backgroundOptsList[ 5]  = msgs[Msg_Ex_Burkes-1];
		backgroundOptsList[ 6]  = msgs[Msg_Ex_Ordered-1];
		backgroundOptsList[ 7]  = msgs[Msg_Ex_Random-1];
		backgroundOptsList[ 8]  = msgs[Msg_Ex_Delete-1];
		backgroundOptsList[ 9]  = msgs[Msg_Ex_UseColors-1];
		backgroundOptsList[10]  = msgs[Msg_Ex_Tile-1];

		/**** open a window ****/

		if ( CPrefs.PageScreenModes & LACE )
		{
			if ( Specials_GR[0].x1 == 0 )	// not doubled 
			{
				UA_DoubleGadgetDimensions(Specials_GR);
				Specials_GR[0].x1 = 1;
			}
		}
		else
		{
			if ( Specials_GR[0].x1 == 1 )	// doubled
			{
				UA_HalveGadgetDimensions(Specials_GR);
				Specials_GR[0].x1 = 0;
			}
		}

		smallWindow = OpenSmallScreen( Specials_GR[0].y2 ); 
		if (!smallWindow)
		{
			UA_WarnUser(-1);
			return(FALSE);
		}
		SetBit(&allocFlags, SPECIALS_WDW_FLAG);

		/**** render gadgets ****/

		UA_DrawGadgetList(smallWindow, Specials_GR);

		/**** set buttons ****/

		DrawSpecialsPage(smallWindow,page,wdw,windowOptsList,backgroundOptsList);

		/**** init list ****/

		PropSlider.LeftEdge	= Specials_GR[5].x1+4;
		PropSlider.TopEdge	= Specials_GR[5].y1+2;
		PropSlider.Width		= Specials_GR[5].x2-Specials_GR[5].x1-7;
		PropSlider.Height		= Specials_GR[5].y2-Specials_GR[5].y1-3;

		if ( UA_IsWindowOnLacedScreen(smallWindow) )
		{
			PropSlider.TopEdge += 2;
			PropSlider.Height	-= 4;
		}

		InitPropInfo(	(struct PropInfo *)PropSlider.SpecialInfo,
									(struct Image *)PropSlider.GadgetRender);
		AddGadget(smallWindow, &PropSlider, -1L);

		/**** init scroll record ****/

		SR.GR							= &Specials_GR[4];
		SR.window					= smallWindow;
		SR.list						= NULL;
		SR.sublist				= NULL;
		SR.selectionList	= NULL;
		SR.entryWidth			= -1;
		SR.numDisplay			= numDisp;
		SR.numEntries			= numLines;

		sp_topEntry = 0;
		UA_SetPropSlider(smallWindow, &PropSlider, numLines, numDisp, sp_topEntry);

		UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
		if ( page==0 )
			UA_PrintNewList(&SR,sp_topEntry,backgroundOptsList,FALSE);
		else if ( page==1 )
			UA_PrintNewList(&SR,sp_topEntry,windowOptsList,FALSE);

		EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);

		ScreenToFront(smallWindow->WScreen);
	}

	return(TRUE);
}

/******** MonitorSpecials() ********/

int MonitorSpecials(void)
{
BOOL takeAction = TRUE;
int ID, page, numLines, line, wdw;
ULONG signals;
UBYTE **list;

	UA_SetValToCycleGadgetVal(&Specials_GR[2], &page);
	wdw = FirstActiveEditWindow();

	{
	struct RasInfo *RI;
		RI = pageScreen->ViewPort.RasInfo;
		CED.MouseY -= RI->RyOffset;
	}

	do
	{
		if (takeAction)
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				if ( page==0 )
					UA_ScrollNewList(&SR,&sp_topEntry,&PropSlider,backgroundOptsList,&CED);
				else if ( page==1 )
					UA_ScrollNewList(&SR,&sp_topEntry,&PropSlider,windowOptsList,&CED);
			}
			else if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
			{
				ID = UA_CheckGadgetList(smallWindow, Specials_GR, &CED);

				switch(ID)
				{
					case 2:		// cycle
						UA_ProcessCycleGadget(smallWindow, &Specials_GR[ID], &CED);
						UA_SetValToCycleGadgetVal(&Specials_GR[ID], &page);
						if ( page==0 )
							numLines = NUM_BGND_OPTS;
						else
							numLines = NUM_WDW_OPTS;
						SR.numEntries = numLines;
						sp_topEntry=0;
						UA_SetPropSlider(smallWindow, &PropSlider, SR.numEntries, SR.numDisplay, sp_topEntry);
						Specials_GR[4].type = HIBOX_REGION;
						UA_DrawTwoColorBox(smallWindow, &Specials_GR[4]);
						Specials_GR[4].type = BUTTON_GADGET;
						DrawSpecialsPage(smallWindow,page,wdw,windowOptsList,backgroundOptsList);
						UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
						if ( page==0 )
							UA_PrintNewList(&SR,sp_topEntry,backgroundOptsList,FALSE);
						else if ( page==1 )
							UA_PrintNewList(&SR,sp_topEntry,windowOptsList,FALSE);
						EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);
						break;

					case 3:		// Hide
						UA_HiliteButton(smallWindow, &Specials_GR[3]);
						PaletteToBack();
						return(NOTHING_TO_EXAMINE);
						break;

					case 4:	// scroll area
						line = UA_SelectNewListLine(&SR,sp_topEntry,&CED);
						if ( line != -1 )
						{
							line += sp_topEntry;

							if ( page==0 )
								list = backgroundOptsList;
							else
								list = windowOptsList;

							// keep selected radio buttons selected
	
							if ( line==3 && *( list[3] + 1 ) == 0x21 )
								*( list[3] + 1 ) = 0x20;
							else if ( line==4 && *( list[4] + 1 ) == 0x21 )
								*( list[4] + 1 ) = 0x20;
							else if ( line==5 && *( list[5] + 1 ) == 0x21 )
								*( list[5] + 1 ) = 0x20;
							else if ( line==6 && *( list[6] + 1 ) == 0x21 )
								*( list[6] + 1 ) = 0x20;
							else if ( line==7 && *( list[7] + 1 ) == 0x21 )
								*( list[7] + 1 ) = 0x20;

							// toggle it
						
							if ( *( list[line] + 1 ) == 0x20 )
								*( list[line] + 1 ) = 0x21;
							else
								*( list[line] + 1 ) = 0x20;

							// keep mutual exclusivity of dither modes

							if ( line==3 )
							{
								*( list[4] + 1 ) = 0x20;
								*( list[5] + 1 ) = 0x20;
								*( list[6] + 1 ) = 0x20;
								*( list[7] + 1 ) = 0x20;
							}
							else if ( line==4 )
							{
								*( list[3] + 1 ) = 0x20;
								*( list[5] + 1 ) = 0x20;
								*( list[6] + 1 ) = 0x20;
								*( list[7] + 1 ) = 0x20;
							}
							else if ( line==5 )
							{
								*( list[3] + 1 ) = 0x20;
								*( list[4] + 1 ) = 0x20;
								*( list[6] + 1 ) = 0x20;
								*( list[7] + 1 ) = 0x20;
							}
							else if ( line==6 )
							{
								*( list[3] + 1 ) = 0x20;
								*( list[4] + 1 ) = 0x20;
								*( list[5] + 1 ) = 0x20;
								*( list[7] + 1 ) = 0x20;
							}
							else if ( line==7 )
							{
								*( list[3] + 1 ) = 0x20;
								*( list[4] + 1 ) = 0x20;
								*( list[5] + 1 ) = 0x20;
								*( list[6] + 1 ) = 0x20;
							}

							// START SMARTSTEP
							if (	line>=3 && line<=7 &&
										(
										*( list[3] + 1 ) == 0x21 || *( list[4] + 1 ) == 0x21 ||
										*( list[5] + 1 ) == 0x21 || *( list[6] + 1 ) == 0x21 ||
										*( list[7] + 1 ) == 0x21
										)	)
							{
								*( list[0] + 1 ) = 0x21;	// remap on if one of the dither modes is on
								ProcessExtrasSelection(page,wdw,windowOptsList,backgroundOptsList,0,TRUE);
							}
							// END SMARTSTEP

							ProcessExtrasSelection(page,wdw,windowOptsList,backgroundOptsList,line,FALSE);
							DrawSpecialsPage(smallWindow, page, wdw, windowOptsList, backgroundOptsList);
							EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);
						}
						break;
				}
			}
			else if (CED.Class==IDCMP_RAWKEY || CED.Class==IDCMP_MENUPICK)
				return(IDCMP_TO_EXAMINE);
		}

		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(smallWindow,FALSE,NULL);
			takeAction=TRUE;
			if ( !(smallWindow->Flags & WFLG_WINDOWACTIVE) )
				return(IDCMP_TO_EXAMINE);
		}
		else
			takeAction=FALSE;
	}
	while(1);

	return(NOTHING_TO_EXAMINE);
}

/******** DrawSpecialsPage() ********/

void DrawSpecialsPage(struct Window *window, int page, int wdw,
											UBYTE *windowOptsList[], UBYTE *backgroundOptsList[])
{
int i;
struct EditWindow *ew;
struct EditSupport *es;
UBYTE **list;

	if ( page==0 )
	{
		if ( !(backES.photoOpts & MOVE_PHOTO) && !(backES.photoOpts & SIZE_PHOTO) )
			return;	// no picture in background

		ew = &backEW;
		es = &backES;
		list = backgroundOptsList;
	}
	else if ( page==1 )
	{	
		if ( wdw==-1 )	// no window chosen
			return;

		ew = EditWindowList[wdw];
		es = EditSupportList[wdw];
		list = windowOptsList;
	}

	for(i=0; i<25; i++)
		if ( list[i] )
			*( list[i] + 1 ) = 0x20;	// all off
	
	if ( es->photoOpts & REMAP_PHOTO )
		*( list[0] + 1 ) = 0x21;

	if ( es->photoOpts & SIZE_PHOTO )
		*( list[1] + 1 ) = 0x21;

	if ( ew->flags & EW_IS_OPAQUE )
		*( list[2] + 1 ) = 0x21;

	if ( es->photoOpts & REMAP_PHOTO )	// only when remapped there is a dither mode
	{
		if ( es->ditherMode == DITHER_OFF )
			*( list[3] + 1 ) = 0x21;

		if ( es->ditherMode == DITHER_FLOYD )
			*( list[4] + 1 ) = 0x21;

		if ( es->ditherMode == DITHER_BURKES )
			*( list[5] + 1 ) = 0x21;

		if ( es->ditherMode == DITHER_ORDERED )
			*( list[6] + 1 ) = 0x21;

		if ( es->ditherMode == DITHER_RANDOM )
			*( list[7] + 1 ) = 0x21;
	}

	if ( page==0 )  // background
	{
		if ( do_the_tile )
			*( list[10] + 1 ) = 0x21;
	}
}

/******** ProcessExtrasSelection() ********/

void ProcessExtrasSelection(int page, int wdw,
														UBYTE *windowOptsList[], UBYTE *backgroundOptsList[],
														int line, BOOL noRedraw )
{
UBYTE **list;
BOOL redraw=FALSE;
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
WORD x,y,w,h;
int i, first;
float factor;
int ditherMode;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			prevX[i] = EditWindowList[i]->X;
			prevY[i] = EditWindowList[i]->Y;
			prevW[i] = EditWindowList[i]->Width;
			prevH[i] = EditWindowList[i]->Height;

			newX[i] = EditWindowList[i]->X;
			newY[i] = EditWindowList[i]->Y;
			newW[i] = EditWindowList[i]->Width;
			newH[i] = EditWindowList[i]->Height;
		}
	}

	if ( page==0 )
		list = backgroundOptsList;
	else if ( page==1 )
		list = windowOptsList;

	switch( line )
	{
		case 0:
		case 1:
		case 2:
			if ( *( list[line] + 1 ) == 0x20 )							// NO - remap,resize,opaque
			{
				if ( page==0 )
				{
					if ( line==0 )
						UnSetByteBit(&backES.photoOpts, REMAP_PHOTO);
					else if ( line==1 )
					{
						UnSetByteBit(&backES.photoOpts, SIZE_PHOTO);
						SetByteBit(&backES.photoOpts, MOVE_PHOTO);
					}
					else if ( line==2 )
						UnSetByteBit(&backEW.flags, EW_IS_OPAQUE);
				}
				else if ( page==1 )
				{
					for(i=0; i<MAXEDITWINDOWS; i++)
					{
						if ( EditSupportList[i] && EditSupportList[i]->Active )
						{
							if ( line==0 )
								UnSetByteBit(&EditSupportList[i]->photoOpts, REMAP_PHOTO);
							else if ( line==1 )
							{
								UnSetByteBit(&EditSupportList[i]->photoOpts, SIZE_PHOTO);
								SetByteBit(&EditSupportList[i]->photoOpts, MOVE_PHOTO);
							}
							else if ( line==2 )
								UnSetByteBit(&EditWindowList[i]->flags, EW_IS_OPAQUE);
						}
					}
				}
			}
			else if ( *( list[line] + 1 ) == 0x21 )					// YES - remap,resize,opaque
			{
				if ( page==0 )
				{
					if ( line==0 )
						SetByteBit(&backES.photoOpts, REMAP_PHOTO);
					else if ( line==1 )
					{
						SetByteBit(&backES.photoOpts, SIZE_PHOTO);
						UnSetByteBit(&backES.photoOpts, MOVE_PHOTO);
					}
					else if ( line==2 )
						SetByteBit(&backEW.flags, EW_IS_OPAQUE);
				}
				else if ( page==1 )
				{
					for(i=0; i<MAXEDITWINDOWS; i++)
					{
						if ( EditSupportList[i] && EditSupportList[i]->Active )
						{
							if ( line==0 )
								SetByteBit(&EditSupportList[i]->photoOpts, REMAP_PHOTO);
							else if ( line==1 )
							{
								SetByteBit(&EditSupportList[i]->photoOpts, SIZE_PHOTO);
								UnSetByteBit(&EditSupportList[i]->photoOpts, MOVE_PHOTO);
							}
							else if ( line==2 )
								SetByteBit(&EditWindowList[i]->flags, EW_IS_OPAQUE);
						}
					}
				}
			}
			redraw=TRUE;
			break;

		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			if (line==3)
				ditherMode = DITHER_OFF;
			else if (line==4)
				ditherMode = DITHER_FLOYD;
			else if (line==5)
				ditherMode = DITHER_BURKES;
			else if (line==6)
				ditherMode = DITHER_ORDERED;
			else
				ditherMode = DITHER_RANDOM;
			if ( *( list[line] + 1 ) == 0x21 )
			{
				if ( page==0 )
					backES.ditherMode = ditherMode;
				else
				{
					for(i=0; i<MAXEDITWINDOWS; i++)
						if ( EditSupportList[i] && EditSupportList[i]->Active )
							EditSupportList[i]->ditherMode = ditherMode;
				}
			}
			redraw=TRUE;
			break;

		case 8:
			if ( UA_OpenGenericWindow(smallWindow, TRUE, TRUE,
																msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																QUESTION_ICON, msgs[Msg_DeletePic-1], TRUE,
																SmallWarning_GR) )
			{
				if ( page==0 )
				{
					RemovePic24FromWindow(&backES, &backES.ori_bm);
					ClearBitMap24(&backES.ori_bm);
					RemovePicFromWindow(&backES, &backES.scaled_bm);
					ClearBitMap(&backES.scaled_bm);
					RemovePicFromWindow(&backES, &backES.remapped_bm);
					ClearBitMap(&backES.remapped_bm);
					RemovePicFromWindow(&backES, &backES.mask_bm);
					ClearBitMap(&backES.mask_bm);
					RemovePicFromWindow(&backES, &backES.ori_mask_bm);
					ClearBitMap(&backES.ori_mask_bm);
					backES.ori_w			= 0;
					backES.ori_h			= 0;
					backES.scaled_w		= 0;
					backES.scaled_h		= 0;
					backES.remapped_w	= 0;
					backES.remapped_h	= 0;														
					backES.mask_w			= 0;
					backES.mask_h			= 0;														
					backES.ori_mask_w	= 0;
					backES.ori_mask_h	= 0;														
					backES.photoOpts	= 0;
					backES.picPath[0]	= '\0';
					SetByteBit(&backES.photoOpts, MODIFIED_PHOTO);
				}
				else if ( page==1 )
				{
					for(i=0; i<MAXEDITWINDOWS; i++)
					{
						if ( EditSupportList[i] && EditSupportList[i]->Active )
						{
							RemovePic24FromWindow(EditSupportList[i], &EditSupportList[i]->ori_bm);
							RemovePicFromWindow(EditSupportList[i], &EditSupportList[i]->scaled_bm);
							RemovePicFromWindow(EditSupportList[i], &EditSupportList[i]->remapped_bm);
							RemovePicFromWindow(EditSupportList[i], &EditSupportList[i]->mask_bm);
							RemovePicFromWindow(EditSupportList[i], &EditSupportList[i]->ori_mask_bm);
							ClearBitMap24(&EditSupportList[i]->ori_bm);
							ClearBitMap(&EditSupportList[i]->scaled_bm);
							ClearBitMap(&EditSupportList[i]->remapped_bm);
							ClearBitMap(&EditSupportList[i]->mask_bm);
							ClearBitMap(&EditSupportList[i]->ori_mask_bm);
							EditSupportList[i]->ori_w				= 0;
							EditSupportList[i]->ori_h				= 0;
							EditSupportList[i]->scaled_w		= 0;
							EditSupportList[i]->scaled_h		= 0;
							EditSupportList[i]->remapped_w	= 0;
							EditSupportList[i]->remapped_h	= 0;
							EditSupportList[i]->mask_w			= 0;
							EditSupportList[i]->mask_h			= 0;
							EditSupportList[i]->ori_mask_w	= 0;
							EditSupportList[i]->ori_mask_h	= 0;
							EditSupportList[i]->photoOpts		= 0;
							EditSupportList[i]->picPath[0]	= '\0';
							SetByteBit(&EditSupportList[i]->photoOpts, MODIFIED_PHOTO);
						}
					}
				}
				redraw=TRUE;
			}
			break;

		case 9:		// use colors
			if ( page==0 && backES.cm )
				SetScreenToCM(pageScreen, backES.cm);
			else if ( page==1 )
			{
				first = FirstActiveEditWindow();
				if ( first!=-1 && EditSupportList[first] && EditSupportList[first]->cm )
					SetScreenToCM(pageScreen, EditSupportList[first]->cm);
			}
			SyncAllColors(TRUE);
			break;

		case 10:	// bgnd: tile	| window: window to pic size
			if ( page==0 )
			{
				if ( *( list[line] + 1 ) == 0x21 )
					do_the_tile = TRUE;
				else
					do_the_tile = FALSE;
			}
			else if ( page==1 )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
				{
					if ( EditSupportList[i] && EditSupportList[i]->ori_bm.Planes[0] )
					{
						x = EditWindowList[i]->X;
						y = EditWindowList[i]->Y;
						w = EditSupportList[i]->iff->BMH.w;
						h = EditSupportList[i]->iff->BMH.h;
						if ( x+w > CPrefs.PageScreenWidth )
							x = CPrefs.PageScreenWidth-w;
						if ( x<0 )
							x=0;
						if ( y+h > CPrefs.PageScreenHeight )
							y = CPrefs.PageScreenHeight-h;
						if ( y<0 )
							y=0;
						if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
						{
							if ( EditSupportList[i]->iff->BMH.w > EditSupportList[i]->iff->BMH.h )
								factor = (float)(EditSupportList[i]->iff->BMH.h) / (float)(EditSupportList[i]->iff->BMH.w);
							else
								factor = (float)(EditSupportList[i]->iff->BMH.w) / (float)(EditSupportList[i]->iff->BMH.h);
							w = (int)((float)w * factor);
							h = (int)((float)h * factor);
							if (	(EditSupportList[i]->iff->viewModes & LACE) &&
										!(CPrefs.PageScreenModes & LACE) )
							{
								// Photo is laced, Page is not.
								h = h / 2;
							}
							else if (	!(EditSupportList[i]->iff->viewModes & LACE) &&
												(CPrefs.PageScreenModes & LACE) )
							{
								// Photo is NOT laced, Page is.
								h = h * 2;
							}
							if (	(EditSupportList[i]->iff->viewModes & HIRES) &&
										CPrefs.PageScreenModes < 640 )
							{
								// Photo is hires, Page is not.
								w = w / 2;
							}
							else if (	!(EditSupportList[i]->iff->viewModes & HIRES) &&
												CPrefs.PageScreenWidth >= 640 )
							{
								// Photo is NOT hires, Page is.
								w = w * 2;
							}
						}
						ValidateBoundingBox(&x, &y, &w, &h);
						newX[i] = x;
						newY[i] = y;
						newW[i] = w;
						newH[i] = h;
					}
				}
			}
			redraw=TRUE;
			break;

		case 11:	// set pic offset to 0,0
			if ( page==1 )
			{
				for(i=0; i<MAXEDITWINDOWS; i++)
				{
					if ( EditSupportList[i] )
					{
						EditWindowList[i]->PhotoOffsetX = 0;
						EditWindowList[i]->PhotoOffsetY = 0;
					}
				}
			}
			redraw=TRUE;
			break;

		case 12:	// Anim settings
			ClipAnimSettings(smallWindow);
			break;
	}

	if ( redraw && !noRedraw )	
	{
		if ( page==0 )
		{
			if ( backES.scaled_bm.Planes[0] )
				FreeFastBitMap( &backES.scaled_bm );
			backES.scaled_w = 0;
			backES.scaled_h = 0;

			if ( backES.remapped_bm.Planes[0] )
				FreeFastBitMap( &backES.remapped_bm );
			backES.remapped_w = 0;
			backES.remapped_h = 0;
		}
		else if ( page==1 )
		{
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditSupportList[i] && EditSupportList[i]->Active )
				{
					if ( EditSupportList[i]->scaled_bm.Planes[0] )
						FreeFastBitMap( &EditSupportList[i]->scaled_bm );
					EditSupportList[i]->scaled_w = 0;
					EditSupportList[i]->scaled_h = 0;

					if ( EditSupportList[i]->remapped_bm.Planes[0] )
						FreeFastBitMap( &EditSupportList[i]->remapped_bm );
					EditSupportList[i]->remapped_w = 0;
					EditSupportList[i]->remapped_h = 0;

					if ( EditSupportList[i]->mask_bm.Planes[0] )
						FreeFastBitMap( &EditSupportList[i]->mask_bm );
					EditSupportList[i]->mask_w = 0;
					EditSupportList[i]->mask_h = 0;
				}
			}
		}

		DrawAllHandles(LEAVE_ACTIVE);
		RefreshAllWindows(page, prevX, prevY, prevW, prevH, newX, newY, newW, newH);
		DrawAllHandles(LEAVE_ACTIVE);
	}
}

/******** RefreshAllWindows() ********/
/*
 * page = 0 -> do background
 * page = 1 -> do windows
 *
 */

void RefreshAllWindows(	int page,
												WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
												WORD *newX, WORD *newY, WORD *newW, WORD *newH )
{
BOOL list[MAXEDITWINDOWS];
int i;

	if ( page==0 )			// background
	{
		UnSetByteBit(&backES.photoOpts, MODIFIED_PHOTO);

		// redraw background

		SetSpriteOfActWdw(SPRITE_BUSY);
		ShowBackground();
		SetSpriteOfActWdw(SPRITE_NORMAL);

		// redraw all windows -- force remembering of new restore backgrounds

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] )
			{
				if ( EditSupportList[i]->restore_bm.Planes[0] )
				{
					RemovePicFromWindow(EditSupportList[i],&EditSupportList[i]->restore_bm);
					ClearBitMap(&EditSupportList[i]->restore_bm);
				}
				DrawEditWindow( EditWindowList[i], EditSupportList[i] );
			}
		}
	}
	else if ( page==1 )	// windows
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
			list[i] = FALSE;

		for(i=0; i<MAXEDITWINDOWS; i++)
			if (	EditSupportList[i] && EditSupportList[i]->Active &&
						EditSupportList[i]->ori_bm.Planes[0] )
				list[i] = TRUE;

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditSupportList[i] && ( EditSupportList[i]->photoOpts & MODIFIED_PHOTO ) )
			{
				list[i] = TRUE;
				UnSetByteBit(&EditSupportList[i]->photoOpts, MODIFIED_PHOTO);
			}
		}

		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);
	}
}

/******** CloseSpecialsScreen() ********/

void CloseSpecialsScreen(void)
{
	if ( smallWindow )
	{
		CloseSmallScreen(smallWindow);
		UA_EnableButtonQuiet(&Specials_GR[2]);
		UA_EnableButtonQuiet(&Specials_GR[4]);
		UnSetBit(&allocFlags, SPECIALS_WDW_FLAG);
	}
}

/******** DoSpecialsSettings() ********/
/*
 * This function is called whenever an edit window is selected
 *
 */

void DoSpecialsSettings(BOOL force)
{
int page, current_page, wdw, actWdw;

	UA_SetValToCycleGadgetVal(&Specials_GR[2], &current_page);
	//if ( !force && current_page==0 )
	//	return;
	if ( !smallWindow || !(allocFlags & SPECIALS_WDW_FLAG) )
		return;
	wdw = FirstActiveEditWindow();

	// START SMARTSTEP
	actWdw = NumActiveEditWindows();
	if ( actWdw > 0 )
		page=1;	// show picture controls
	else
		page=0;	// show background controls
	if ( current_page != page )
	{
		UA_SetCycleGadgetToVal(smallWindow, &Specials_GR[2], page);
		if ( page==0 )
			SR.numEntries = NUM_BGND_OPTS;
		else
			SR.numEntries = NUM_WDW_OPTS;
		sp_topEntry=0;
		UA_SetPropSlider(smallWindow, &PropSlider, SR.numEntries, SR.numDisplay, sp_topEntry);
		Specials_GR[4].type = HIBOX_REGION;
		UA_DrawTwoColorBox(smallWindow, &Specials_GR[4]);
		Specials_GR[4].type = BUTTON_GADGET;
		DrawSpecialsPage(smallWindow,page,wdw,windowOptsList,backgroundOptsList);
		UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
		if ( page==0 )
			UA_PrintNewList(&SR,sp_topEntry,backgroundOptsList,FALSE);
		else if ( page==1 )
			UA_PrintNewList(&SR,sp_topEntry,windowOptsList,FALSE);
		EnableDisableSpecialsList(smallWindow,page,wdw,&SR,sp_topEntry,windowOptsList,backgroundOptsList);
return;
	}
	// END SMARTSTEP

if ( !force && current_page==0 )
	return;

	DrawSpecialsPage(smallWindow, page, wdw, windowOptsList, backgroundOptsList);
	EnableDisableSpecialsList(smallWindow, page, wdw, &SR, sp_topEntry,	windowOptsList, backgroundOptsList);
}

/******** EnableDisableSpecialsList() ********/

void EnableDisableSpecialsList(	struct Window *window, int page, int wdw,
																struct ScrollRecord *SR, int sp_topEntry,
																UBYTE *windowOptsList[], UBYTE *backgroundOptsList[] )
{
BOOL enableList, flag;

	if ( page==0 )
	{
		if ( !(backES.photoOpts & MOVE_PHOTO) && !(backES.photoOpts & SIZE_PHOTO) )
			enableList=FALSE;
		else
			enableList=TRUE;
	}
	else if ( page==1 )
	{	
		if (	wdw==-1 ||
					(	!(EditSupportList[wdw]->photoOpts & MOVE_PHOTO) &&
						!(EditSupportList[wdw]->photoOpts & SIZE_PHOTO) )	)
			enableList=FALSE;
		else
			enableList=TRUE;
	}

	if ( enableList )
	{
		if ( UA_IsGadgetDisabled(&Specials_GR[4]) )
		{
			Specials_GR[4].type = HIBOX_REGION;
			UA_DrawTwoColorBox(window, &Specials_GR[4]);
			Specials_GR[4].type = BUTTON_GADGET;
			flag=FALSE;
		}
		else
			flag = TRUE;

		UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
		if ( page==0 )
			UA_PrintNewList(SR,sp_topEntry,backgroundOptsList,flag);
		else if ( page==1 )
			UA_PrintNewList(SR,sp_topEntry,windowOptsList,flag);

		OnGadget(&PropSlider, window, NULL);
	}
	else
	{
		if ( !UA_IsGadgetDisabled(&Specials_GR[4]) )
		{
			UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
			if ( page==0 )
				UA_PrintNewList(SR,sp_topEntry,backgroundOptsList,TRUE);
			else if ( page==1 )
				UA_PrintNewList(SR,sp_topEntry,windowOptsList,TRUE);

			UA_DisableButton(window, &Specials_GR[4], gui_pattern);	// scroll list
		}
		OffGadget(&PropSlider, window, NULL);
	}
}

/******** E O F ********/
