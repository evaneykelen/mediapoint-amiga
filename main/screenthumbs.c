#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct NewWindow NewWindowStructure;
extern struct Screen **DA_Screens;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct eventHandlerInfo EHI;
extern struct Screen *thumbScreen;
extern struct Window *thumbWindow;
extern UWORD GrayMap[];
extern UWORD GrayMap2[];
extern TEXT special_char[];
extern UWORD palettes[];

/**** static globals ****/

static struct TextAttr thumbs_textAttr;
static struct BitMap to_bm;	//, to_bm2;
static struct RastPort to_rp;

/**** gadgets ****/

extern struct GadgetRecord ScreenThumbs_GR[];

/**** functions ****/

/******** ShowThumbScreens() ********/
/*
 * returns screen number corresponding to DA_Screens entry value
 * or.... -1 on error
 *
 */

int ShowThumbScreens(void)
{
int i,x,y,destXSize, destYSize,count,height,pos;
struct ViewPort *vp;
struct ColorMap *cm;
int choice=-1;
struct Screen *chosenScreen;
struct Rectangle dclip;
struct ScaleRemapInfo SRI;
UWORD cmap[32];
BOOL query;
Tag ti_Tag;
ULONG ti_Data, modes;
//struct GadgetRecord GR;

	/**** calculate useful parameters ****/

	destXSize = 126;
 	if ( (CPrefs.PageScreenModes&LACE) && (CPrefs.ScriptPalNtsc==PAL_MODE) )
		destYSize = 102;
 	else if ( (CPrefs.PageScreenModes&LACE) && (CPrefs.ScriptPalNtsc==NTSC_MODE) )
		destYSize = 80;
 	else if (CPrefs.ScriptPalNtsc==PAL_MODE)	/* not lace */
		destYSize = 51;
 	else
		destYSize = 40;

	/**** get number of open screens */

	Fill_DA_Menu(FALSE);

	count=0;
	for(i=0; i<10; i++)
	{
		if ( DA_Screens[i]!=NULL )
			count++;
	}

	/**** open a screen ****/

	if ( CPrefs.PageScreenModes & LACE )
	{
		if (count>5)
			height=destYSize*2 + 34;
		else
			height=destYSize + 34;
	}
	else
	{
		if (count>5)
			height=destYSize*2 + 20;
		else
			height=destYSize + 20;
	}

	thumbs_textAttr.ta_Name	= (UBYTE *)APPFONT;
	if ( CPrefs.PageScreenModes & LACE )
		thumbs_textAttr.ta_YSize = 20;
	else
		thumbs_textAttr.ta_YSize = 10;
	thumbs_textAttr.ta_Style = FS_NORMAL;
	thumbs_textAttr.ta_Flags = FPF_DESIGNED;

	modes = CPrefs.pageMonitorID | HIRES;

	if ( !ModeHasThisWidth(modes,640) || ModeNotAvailable(modes) )
	{
		if ( CPrefs.PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	if ( CPrefs.PageScreenModes & LACE )
		modes |= LACE;

	query = QueryOverscan(modes, &dclip, OSCAN_TEXT);

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		pos = ((dclip.MaxX - dclip.MinX + 1) - CPrefs.ThumbnailScreenWidth) / 2;
		dclip.MinX += pos;
		dclip.MaxX = dclip.MinX + CPrefs.ThumbnailScreenWidth - 1; 

		dclip.MinY = dclip.MaxY - height + 1;
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	thumbScreen = OpenScreenTags(		NULL,
																	SA_Depth, 		CPrefs.ThumbnailScreenDepth,
																	SA_Font,			&thumbs_textAttr,
																	SA_DisplayID,	modes,
																	SA_ShowTitle,	FALSE,
																	SA_Behind,		TRUE,
																	SA_DetailPen,	0,
																	SA_BlockPen,	2,
																	SA_Type,			CUSTOMSCREEN,
																	SA_Title,			(UBYTE *)special_char,
																	SA_Quiet,			TRUE,
																	SA_AutoScroll,TRUE,
																	ti_Tag,				ti_Data,
																	TAG_END
																);
	if (thumbScreen == NULL)
	{
		UA_WarnUser(120);
		return(-1);
	}

	thumbScreen->Title = (UBYTE *)special_char;

	GenlockOff(thumbScreen);

	/**** open a window ****/

	NewWindowStructure.LeftEdge			= 0;
	NewWindowStructure.TopEdge			= 0;
	NewWindowStructure.Width				= CPrefs.ThumbnailScreenWidth;
	NewWindowStructure.Height				= height;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= 0;	// else a msgport is created
	NewWindowStructure.Flags				= WFLG_BACKDROP | WFLG_ACTIVATE |
																		WFLG_RMBTRAP | WFLG_NOCAREREFRESH | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen				= (struct Screen *)thumbScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	thumbWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (thumbWindow==NULL)
	{
		UA_WarnUser(121);
		CloseScreen(thumbScreen);
		return(-1);
	}

	ShowTitle(thumbScreen, FALSE);

	/**** attach message port to window ****/

	thumbWindow->UserPort = capsPort;
	ModifyIDCMP(thumbWindow, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY);

	/**** make requesters appear on thumbnail screen ****/

	EHI.thumbsVisible = TRUE;

	/**** set font ****/

	if ( CPrefs.PageScreenModes & LACE )
		SetFont(thumbWindow->RPort, largeFont);
	else
		SetFont(thumbWindow->RPort, smallFont);

	/**** render gadgets ****/

	SetDrMd(thumbWindow->RPort, JAM1);

	Move(thumbWindow->RPort,0,0);
	SetRast(thumbWindow->RPort,AREA_PEN);
	WaitBlit();

	SetAPen(thumbWindow->RPort, LO_PEN);
	DrawSimpleBox(thumbWindow->RPort,0,0,CPrefs.ThumbnailScreenWidth-1,height-1);
	DrawSimpleBox(thumbWindow->RPort,1,0,CPrefs.ThumbnailScreenWidth-2,height-1);
	if ( CPrefs.PageScreenModes & LACE )
	{
		DrawSimpleBox(thumbWindow->RPort,0,1,CPrefs.ThumbnailScreenWidth-1,height-2);
		DrawSimpleBox(thumbWindow->RPort,1,1,CPrefs.ThumbnailScreenWidth-2,height-2);
	}

	if ( CPrefs.PageScreenModes & LACE )
	{
		ScreenThumbs_GR[0].y1 = height - 32;
		ScreenThumbs_GR[0].y2 = height - 32 + 26;
	}
	else
	{
		ScreenThumbs_GR[0].y1 = height - 20;
		ScreenThumbs_GR[0].y2 = height - 20 + 13;
	}

	if ( CPrefs.PageScreenModes & LACE )
	{
		ScreenThumbs_GR[1].y1 = height - 32;
		ScreenThumbs_GR[1].y2 = height - 32 + 26;
	}
	else
	{
		ScreenThumbs_GR[1].y1 = height - 20;
		ScreenThumbs_GR[1].y2 = height - 20 + 13;
	}

	UA_DrawGadgetList(thumbWindow, ScreenThumbs_GR);
	UA_DrawDefaultButton(thumbWindow,&ScreenThumbs_GR[1]);

	UA_DisableButton(thumbWindow, &ScreenThumbs_GR[0], gui_pattern);	/* cancel */
	UA_DisableButton(thumbWindow, &ScreenThumbs_GR[1], gui_pattern);	/* grab */

	/**** set right colors ****/

	if ( CPrefs.ThumbnailScreenDepth==4 )	// 16 colors
	{
		for(i=0; i<8; i++)
			cmap[i] = palettes[CPrefs.colorSet*8 + i];
		for(i=0; i<8; i++)
			cmap[i+8] = GrayMap[i];
		SetScreenToColorTable4(thumbScreen, cmap, 16);
	}
	else if ( CPrefs.ThumbnailScreenDepth==5 )	// 32 colors
	{
		for(i=0; i<8; i++)
			cmap[i] = palettes[CPrefs.colorSet*8 + i];
		for(i=0; i<24; i++)
			cmap[i+8] = GrayMap2[i];
		SetScreenToColorTable4(thumbScreen, cmap, 32);
	}
	SetScreenToPartialCM(thumbScreen, CPrefs.PageCM, 0, 0);	// NEW

	/**** show the user what we created ****/

	ScreenToFront(thumbScreen);

	/**** alloc bitmap ****/

	InitBitMap(&to_bm, CPrefs.ThumbnailScreenDepth, destXSize, destYSize);
	for (i=0; i<8; i++)
		to_bm.Planes[i] = NULL;

	to_bm.Planes[0] = (PLANEPTR)AllocMem(
															RASSIZE(destXSize,destYSize)*CPrefs.ThumbnailScreenDepth,
															MEMF_CHIP | MEMF_CLEAR );
	if ( !to_bm.Planes[0] )
	{
		UA_WarnUser(119);
		return(-1);
	}

	for (i=0; i<CPrefs.ThumbnailScreenDepth; i++)
		to_bm.Planes[i] = to_bm.Planes[0] + RASSIZE(destXSize,destYSize)*i;

	//CopyMem(&to_bm, &to_bm2, sizeof(struct BitMap));

	InitRastPort(&to_rp);
	to_rp.BitMap = &to_bm;

	/**** handle events ****/

	InvalidateTable();

//	GR.txt = NULL;
//	GR.type = HIBOX_REGION;
//	GR.color = 2;

	x=0;
	y=0;
	for(i=0; i<count; i++)
	{
		if (DA_Screens[i] != NULL && DA_Screens[i]->RastPort.BitMap!=NULL)
		{
			vp = &DA_Screens[i]->ViewPort;
			cm = vp->ColorMap;

			/**** scale ****/

			SetDrMd(thumbWindow->RPort,JAM1);
			SetAPen(thumbWindow->RPort,LO_PEN);
			WritePixel(thumbWindow->RPort,x+3,y+3);	// fool DrawThumbBox
			DrawThumbBox(x+3, y+3, destXSize-5, destYSize-5);
#if 0
			GR.x1 = x-2;
			if ( CPrefs.ThumbnailScreenModes & LACE )
				GR.y1 = y-2;
			else
				GR.y1 = y-1;
			GR.x2 = x+destXSize+1;
			if ( CPrefs.ThumbnailScreenModes & LACE )
				GR.y2 = y+destYSize+1;
			else
				GR.y2 = y+destYSize;
			UA_DrawTwoColorBox(thumbWindow,&GR);
#endif
			Move(&to_rp,0,0); SetRast(&to_rp,0); WaitBlit();

			//to_bm.Depth	= DA_Screens[i]->RastPort.BitMap->Depth;

			SRI.SrcBitMap					= (struct BitMap24 *)DA_Screens[i]->RastPort.BitMap;
			SRI.DstBitMap					= (struct BitMap24 *)&to_bm;
			SRI.SrcViewModes  		= (ULONG)(ULONG)vp->Modes;
			SRI.DstViewModes  		= HIRES;
			SRI.SrcColorMap				= cm;
			SRI.DstColorMap				= thumbScreen->ViewPort.ColorMap;
			SRI.SrcX							= 0;
			SRI.SrcY							= 0;
			SRI.SrcWidth					= DA_Screens[i]->Width;
			SRI.SrcHeight					= DA_Screens[i]->Height;
			SRI.XSrcFactor				= DA_Screens[i]->Width;
			SRI.YSrcFactor				= DA_Screens[i]->Height;
			SRI.DestX							= 0;
			SRI.DestY							= 0;
			SRI.DestWidth					= destXSize-9;
			SRI.DestHeight				= destYSize-9;
			SRI.XDestFactor				= destXSize-9;
			SRI.YDestFactor				= destYSize-9;
			SRI.Flags							= SCAREMF_OPAQUE;
			SRI.DitherMode				= DITHER_FLOYD;
			SRI.DstMaskPlane			= NULL;
			SRI.TransparentColor	= 0;

			if ( !ScaleRemap(&SRI) )
				Message("ScaleRemap Failed\n");

			BltBitMap(&to_bm,0,0,thumbWindow->RPort->BitMap,x+5,y+5,destXSize-9,destYSize-9,0xc0,0xff,NULL);
			WaitBlit();

			x=x+destXSize;
			if (x>=(thumbScreen->Width-(destXSize/2)))
			{
				x=0;
				y=y+destYSize;
			}
		}
	}

	UA_EnableButton(thumbWindow, &ScreenThumbs_GR[0]);	/* cancel */

	chosenScreen = Monitor_ScreenThumbs(count, destXSize, destYSize, height);

	for(i=0; i<10; i++)
	{
		if (DA_Screens[i] == chosenScreen)
		{
			choice = i;
			break;
		}
	}

	/**** make requesters appear on thumbnail screen ****/

	EHI.thumbsVisible = FALSE;

	/**** close the shop ****/

	FreeMem(to_bm.Planes[0], RASSIZE(destXSize,destYSize)*CPrefs.ThumbnailScreenDepth);

	MyScreenToBack(thumbScreen);

	UA_EnableButton(thumbWindow, &ScreenThumbs_GR[0]);
	UA_EnableButton(thumbWindow, &ScreenThumbs_GR[1]);

	UA_CloseWindowSafely(thumbWindow);
	CloseScreen(thumbScreen);

	return(choice);
}

/********	Monitor_ScreenThumbs() ********/

struct Screen *Monitor_ScreenThumbs(int count, int destXSize, int destYSize, int height)
{
BOOL loop=TRUE, disabled=TRUE, retval=FALSE;
int ID, row, col, extra, pos, mouseheight, prevX, prevY, X, Y;

	if (count>5)
		mouseheight=destYSize*2;
	else
		mouseheight=destYSize;

	prevX=-1;
	prevY=-1;
	while(loop)
	{
		UA_doStandardWait(thumbWindow,&CED);

		if ( !(thumbWindow->Flags & WFLG_WINDOWACTIVE) )
			CED.Class = -1;

		if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE && !UA_IsGadgetDisabled(&ScreenThumbs_GR[0]))
				goto do_cancel;
			if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&ScreenThumbs_GR[1]))
				goto do_grab;
		}
		else if (CED.Class == IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(thumbWindow, ScreenThumbs_GR, &CED);
			if (ID==0)	/* cancel */
			{
do_cancel:
				UA_HiliteButton(thumbWindow, &ScreenThumbs_GR[0]);
				loop=FALSE;
				retval=FALSE;
				break;
			}
			else if (ID==1)	/* grab */
			{
do_grab:
				UA_HiliteButton(thumbWindow, &ScreenThumbs_GR[1]);
				loop=FALSE;
				retval=TRUE;
				break;
			}
			else
			{
				if (thumbWindow->MouseY < mouseheight)
				{
					if (count>5)
						extra=5;
					else
						extra=1;
					col = thumbWindow->MouseX/destXSize;
					row = thumbWindow->MouseY/destYSize;
					pos = col+(row*extra);
					if (DA_Screens[pos] != NULL)
					{
						X = col*destXSize;
						Y = row*destYSize; 
						/* zet vorige uit */
						if (prevX!=-1)
							DrawThumbBox(prevX+3, prevY+3, destXSize-5, destYSize-5);
						if (disabled)
						{
							UA_EnableButton(thumbWindow, &ScreenThumbs_GR[1]);	/* grab */
							disabled=FALSE;
						}
						/* zet nieuwe aan */
						prevX = X;
						prevY = Y;
						DrawThumbBox(X+3, Y+3, destXSize-5, destYSize-5);
					}
				}
			}
		}
	}

	if ( retval )
		return(DA_Screens[pos]);
	else
		return(NULL);
}

/******** DrawThumbBox() ********/

void DrawThumbBox(int x, int y, int w, int h)
{
int pen1,pen2;

	if (thumbWindow->WScreen->ViewPort.Modes & LACE)
	{
		y=y-1;
		h=h+2;
	}

	if ( ReadPixel(thumbWindow->RPort,x,y)==HI_PEN )	// top-left corner
	{
		// box is beveled at this moment (ie not selected) - make it selected
		pen1=12;	// yellow of GrayMap
		pen2=12;	// yellow of GrayMap
	}
	else
	{
		pen1=LO_PEN;
		pen2=HI_PEN;
	}

	// draw bottom and right lines

	SetAPen(thumbWindow->RPort,pen1);

	Move(thumbWindow->RPort, x+1, 					(y+h-1));
	Draw(thumbWindow->RPort, (x+w-1),				(y+h-1));
	Draw(thumbWindow->RPort, (x+w-1),				y);
	Move(thumbWindow->RPort, (x+w-1)-1,			y+1);
	Draw(thumbWindow->RPort, (x+w-1)-1,			(y+h-1)-1);

	if (thumbWindow->WScreen->ViewPort.Modes & LACE)
	{
		Move(thumbWindow->RPort, x+2, 				(y+h-1)-1);
		Draw(thumbWindow->RPort, (x+w-1)-2,		(y+h-1)-1);
	}

	// draw top and left lines

	SetAPen(thumbWindow->RPort,pen2);

	Move(thumbWindow->RPort, x, 						(y+h-1));
	Draw(thumbWindow->RPort, x, 						y);
	Draw(thumbWindow->RPort, (x+w-1)-1, 		y);
	Move(thumbWindow->RPort, x+1, 					y+1);
	Draw(thumbWindow->RPort, x+1, 					(y+h-1)-1);

	if (thumbWindow->WScreen->ViewPort.Modes & LACE)
	{
		Move(thumbWindow->RPort, x+2, 				y+1);
		Draw(thumbWindow->RPort, (x+w-1)-2, 	y+1);
	}
}

/******** E O F ********/
