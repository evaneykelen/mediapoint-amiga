#include "nb:pre.h"

/**** defines ****/

#define custom (*((struct Custom *)0xdff000))
#define PALETTE_WIDTH			576	

/**** externals ****/

extern ULONG allocFlags;
extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct eventHandlerInfo EHI;
extern struct MenuRecord **page_MR;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct NewWindow NewWindowStructure;
extern UWORD palettes[];
extern struct Screen *pageScreen;
extern struct ColorMap *undoCM;
extern struct Screen *paletteScreen;
extern struct Window *paletteWindow;
extern TEXT special_char[];
extern ULONG *LoadRGB32Table;
extern struct Window *smallWindow;

/**** static globals ****/

static struct UCopList *clist;
static int scanlines_nl[]	= { 0,  9, 16, 23, 30 };
static int scanlines_l[]	= { 0, 18, 31, 44, 56 };
static int scanlines[]		= { 0, 0, 0, 0, 0 };
static struct TextAttr palette_textAttr;
static UWORD rectfills_non_lace[] = { 2, 3,  7, 10, 14, 17, 21, 24, 28, 29 };
static UWORD rectfills_lace[]			= { 4, 6, 15, 19, 28, 32, 41, 45, 54, 56 };

/**** gadgets ****/

extern struct GadgetRecord ColorAdjust_GR[];

/**** functions ****/

/******** GetNumberOfModeColors() ********/
/*
 * Calculates how many color wells the user may use
 *
 */

int GetNumberOfModeColors(ULONG viewmodes, int depth)
{
int depthList[] = { 0, 2, 4, 8, 16, 32, 16, 16, 16 };
int depthList2[] = { 0, 2, 4, 8, 16, 32, 64, 128, 256 };

	if (depth>8)
		return(256);

	if ( CPrefs.AA_available )
	{
		if (depth==8)	// HAM8 mode
		{
			if (viewmodes & HAM)
				return(64);	// colormap size
			else
				return(256);
		}

		if (depth==6)
		{
			if (viewmodes & HAM)
				return(16);	// colormap size
			else if (viewmodes & EXTRA_HALFBRITE)
				return(64);	// colormap size
			else
				return(64);
		}

		/**** if depth==1 then numColors=2 ****/
		/**** if depth==2 then numColors=4 ****/
		/**** if depth==3 then numColors=8 ****/
		/**** if depth==4 then numColors=16 ****/
		/**** if depth==5 then numColors=32 ****/
		/**** if depth==6 then numColors=64 ****/
		/**** if depth==7 then numColors=128 ****/
		/**** if depth==8 then numColors=256 ****/

		return((int)depthList2[depth]);
	}
	else
	{
		if (viewmodes & HAM)
			return(16);	// colormap size

		if (viewmodes & EXTRA_HALFBRITE)
			return(64);	// colormap size

		/**** if depth==1 then numColors=2 ****/
		/**** if depth==2 then numColors=4 ****/
		/**** if depth==3 then numColors=8 ****/
		/**** if depth==4 then numColors=16 ****/
		/**** if depth==5 then numColors=32 ****/
		/**** if depth==6 then numColors=16 ****/

		return((int)depthList[depth]);
	}
}

/******** OpenPalette() ********/

BOOL OpenPalette(BOOL initUndo)
{
int height, depth, nc, how, pos, md;
ULONG modes;
struct Rectangle dclip;
BOOL query;
Tag ti_Tag;
ULONG ti_Data;

	SetSpriteOfActWdw(SPRITE_BUSY);

	/**** choose font ****/

	palette_textAttr.ta_Name = (UBYTE *)APPFONT;
	if ( CPrefs.PageScreenModes & LACE )
		palette_textAttr.ta_YSize = 20;
	else
		palette_textAttr.ta_YSize = 10;
	palette_textAttr.ta_Style = FS_NORMAL;
	palette_textAttr.ta_Flags = FPF_DESIGNED;

	/**** calculate depth ****/

	nc = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);

	if ( CPrefs.AA_available )
	{
		if ( nc==256 || nc==128 )
			depth = 8;
		else if ( nc==64 )
			depth = 7;
		else if ( nc==32 )
			depth = 6;
		else if ( nc==16 )
			depth = 5;
		else
			depth = 4;
	}
	else
		depth = 4;

	modes = CPrefs.pageMonitorID | HIRES;

	if ( !ModeHasThisWidth(modes,640) || ModeNotAvailable(modes) )
	{
		if ( CPrefs.PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	md = GetMaxDepth(modes);
	if ( md==-1 || md<depth )
	{
		if ( CPrefs.PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	if ( CPrefs.PageScreenModes & LACE )
		modes |= LACE;

	//if ( CPrefs.PageScreenHeight >= 800 )
	//	modes &= ~LACE;

	if ( CPrefs.overScan==0 )
		query = QueryOverscan(modes, &dclip, OSCAN_TEXT);
	else if ( CPrefs.overScan==1 )
		query = QueryOverscan(modes, &dclip, OSCAN_TEXT);
	else if ( CPrefs.overScan==2 )
		query = QueryOverscan(modes, &dclip, OSCAN_STANDARD);
	else if ( CPrefs.overScan==3 )
		query = QueryOverscan(modes, &dclip, OSCAN_MAX);
	else if ( CPrefs.overScan==4 )
		query = QueryOverscan(modes, &dclip, OSCAN_VIDEO);

	/**** calculate top of screen ****/

	height = 78;	// height of palette screen
	if ( CPrefs.PageScreenModes & LACE )
		height *= 2;

	/**** query overscan ****/

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		if ( (dclip.MaxX-dclip.MinX+1) > 640 )
		{
			pos = ((dclip.MaxX - dclip.MinX + 1) - 640) / 2;
			dclip.MinX += pos;
			dclip.MaxX = dclip.MinX + 640 - 1; 
		}
		dclip.MinY = dclip.MaxY - height + 1;
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	paletteScreen = OpenScreenTags(	NULL,
																	SA_Depth, 		depth,
																	SA_Font,			&palette_textAttr,
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

	UnSetBit(&allocFlags, PALETTESCREEN_FLAG);

	if (paletteScreen == NULL)
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(70);
		return(FALSE);
	}
	else
		SetBit(&allocFlags, PALETTESCREEN_FLAG);

	paletteScreen->Title = (UBYTE *)special_char;

	GenlockOff(paletteScreen);

	/**** open window ****/

	NewWindowStructure.LeftEdge			= 0;
	NewWindowStructure.TopEdge			= 0;
	NewWindowStructure.Width				= 640;
	NewWindowStructure.Height				= height;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= 0;
	NewWindowStructure.Flags				= WFLG_BACKDROP |	WFLG_RMBTRAP |
																		WFLG_BORDERLESS | WFLG_NOCAREREFRESH;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen				= (struct Screen *)paletteScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	paletteWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (paletteWindow==NULL)
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);	//ChangeSpriteImage(SPRITE_NORMAL);
		UA_WarnUser(71);
		return(FALSE);
	}

	paletteWindow->UserPort = capsPort;
	ModifyIDCMP(paletteWindow, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY);
		
	SetBit(&allocFlags, PALETTEWINDOW_FLAG);

	/**** set font for this window ****/

	if ( CPrefs.PageScreenModes & LACE )
		SetFont(paletteWindow->RPort, largeFont);
	else
		SetFont(paletteWindow->RPort, smallFont);

	/**** set colors ****/

	SetScreenToColorTable4(paletteScreen, &palettes[CPrefs.colorSet*8], 8);
	SetScreenToPartialCM(paletteScreen, CPrefs.PageCM, 0, 0);

	/**** remove title ****/

	ShowTitle(paletteScreen, FALSE);

	/**** render gadgets ****/

	Move(paletteWindow->RPort, 0L, 0L);
	SetRast(paletteWindow->RPort, AREA_PEN);
	WaitBlit();

	if ( CPrefs.PageScreenModes&LACE )
	{
		if ( ColorAdjust_GR[0].x1 == 0 )	// not doubled
		{
			UA_DoubleGadgetDimensions(ColorAdjust_GR);
			ColorAdjust_GR[0].x1 = 1;
		}
	}
	else	/* no lace */
	{
		if ( ColorAdjust_GR[0].x1 == 1 )	// is doubled
		{
			UA_HalveGadgetDimensions(ColorAdjust_GR);
			ColorAdjust_GR[0].x1 = 0;
		}
	}
	
	UA_DrawGadgetList(paletteWindow, ColorAdjust_GR);
	UA_DrawDefaultButton(paletteWindow, &ColorAdjust_GR[2]);

	/**** draw the small colored buttons ****/

	how = DrawWells(paletteWindow);

	/**** fill color sample area ****/

	PaintButton(paletteWindow, &ColorAdjust_GR[9], 4);

	/**** set colors of palette ****/

	CreateColorsInPalette(how, 1);
	InitPaletteControls();
	if ( initUndo )
		CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);

	/**** show the user what we've made ****/

	ScreenToFront(paletteScreen);
	SetSpriteOfActWdw(SPRITE_NORMAL);
	ActivateWindow(paletteWindow);

	return(TRUE);
}

/******** ClosePalette() ********/

void ClosePalette(void)
{
	ScreenAtNormalPos();
	if ( TestBit(allocFlags,PALETTEWINDOW_FLAG) && TestBit(allocFlags,PALETTESCREEN_FLAG) )
	{
		MyScreenToBack(paletteScreen);
		UA_CloseWindowSafely(paletteWindow);
		CloseScreen(paletteScreen);
		UnSetBit(&allocFlags, COPPERLIST_FLAG);	// This is gone when screen is closed
		UnSetBit(&allocFlags, PALETTEWINDOW_FLAG);
		UnSetBit(&allocFlags, PALETTESCREEN_FLAG);
		paletteScreen=NULL;
		paletteWindow=NULL;
	}
}

/******** TogglePaletteOnOff() ********/

void TogglePaletteOnOff(void)
{
	ScreenAtNormalPos();

	if (	EHI.activeScreen==STARTSCREEN_PAGE )
	{
		if (	TestBit(allocFlags, PALETTEWINDOW_FLAG) &&
					TestBit(allocFlags, PALETTESCREEN_FLAG) )
		{
			if ( SmallScreenCorrect(paletteScreen) )
			{
				if ( !page_MR[PMISC_MENU]->disabled[PMISC_PALETTE] )
				{
					if ( IntuitionBase->FirstScreen == paletteScreen )
						PaletteToBack();
					else
						ScreenToFront(paletteScreen);
				}
			}
			else
				OpenPalette(TRUE);
		}
		else	// yet to be opened
			OpenPalette(TRUE);
	}
}

/******** PaletteToBack() ********/

void PaletteToBack(void)
{
	InvalidateTable();
	ScreenAtNormalPos();

	/**** PALETTE ****/

	if (	EHI.activeScreen==STARTSCREEN_PAGE &&
				TestBit(allocFlags, PALETTEWINDOW_FLAG) &&
				TestBit(allocFlags, PALETTESCREEN_FLAG) )
	{
		if ( IntuitionBase->FirstScreen == paletteScreen )
			SyncAllColors(FALSE);
		MyScreenToBack(paletteScreen);
	}

	/**** SMALL SCREEN (EG SPECIALS) ****/

	if ( EHI.activeScreen==STARTSCREEN_PAGE && smallWindow )
			MyScreenToBack( smallWindow->WScreen );
}

/******** PaletteToFront() ********/

void PaletteToFront(void)
{
	if (	EHI.activeScreen==STARTSCREEN_PAGE &&
				TestBit(allocFlags, PALETTEWINDOW_FLAG) &&
				TestBit(allocFlags, PALETTESCREEN_FLAG) )
	{
		ScreenToFront(paletteScreen);
		SmallScreenCorrect(paletteScreen);
	}
}

/******** RefreshPalette() ********/

void RefreshPalette(void)
{
int how,numCols;

	if (	EHI.activeScreen==STARTSCREEN_PAGE &&
				TestBit(allocFlags, PALETTEWINDOW_FLAG) &&
				TestBit(allocFlags, PALETTESCREEN_FLAG) )
	{
		numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
		if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
			numCols=32;
		how = HowToFillTheWells(numCols);
		CreateColorsInPalette(how, 2);
		InitPickWell();
	}
}

/******** CreateColorsInPalette() ********/
/*
 * mode==1, init mode, mode==2 'clist-already-there-mode'
 *
 */

void CreateColorsInPalette(int how, int mode)
{
struct ViewPort *vp;
struct ColorMap *cm;
BOOL makecoplist=FALSE;
int start, end;

	if ( !paletteWindow)
		return;

  vp = &(paletteScreen->ViewPort);
	cm = vp->ColorMap;

	if ( how==1 || how==9 || how==10 || how==11 )
		makecoplist=TRUE;
	else
	{
		if (how==2)				{ start=32;	end=159; 	}
		else if (how==3)	{ start=16;	end=79; 	}
		else if (how==4)	{ start=8;	end=39; 	}
		else if (how==5)	{ start=8;	end=23; 	}
		else if (how==6)	{ start=8;	end=15; 	}
		else if (how==7)	{ start=8;	end=11; 	}
		else if (how==8)	{ start=8;	end=9; 		}

		if (how==3 && (CPrefs.PageScreenModes & EXTRA_HALFBRITE))
		{
			CopyCMPartial(pageScreen->ViewPort.ColorMap, cm, 0, 16, 32);
			CopyCMPartial(pageScreen->ViewPort.ColorMap, cm, 0, 48, 32);
		}
		else
			CopyCMPartial(pageScreen->ViewPort.ColorMap, cm, 0, start, end-start+1);	// NEW woensdag 25-aug-93 23:46:23

		SetScreenToCM(paletteScreen, cm);
	}

	if (mode==1)
	{
		if ( makecoplist )
		{
			if (MakeCopperList(how, FALSE))
			{
				Forbid();
			  vp = &(paletteScreen->ViewPort);
			  vp->UCopIns=clist;
				Permit();
				RethinkDisplay();
			}
		}
	}
	else
	{
		if ( makecoplist )
		{
			MakeCopperList(how, FALSE);
			RebuildCopperList();
		}
	}
}

/******** MakeCopperList() ********/

BOOL MakeCopperList(int how, BOOL Quick)
{
struct ViewPort *vp;
struct ColorMap *paletteCM;
int i, j, k;
ULONG rgb_long, r, g, b;
UWORD rgb_word;
UWORD copCode;

	if ( !paletteWindow)
		return(FALSE);

	if ( (CPrefs.pageMonitorID & MONITOR_ID_MASK)==PAL_MONITOR_ID ||
			 (CPrefs.pageMonitorID & MONITOR_ID_MASK)==NTSC_MONITOR_ID )
		copCode = 0x40;
	else
		copCode = 0x81;

	//copCode = 0x40;

	if ( CPrefs.PageScreenModes & LACE )
		for(i=0; i<5; i++)
			scanlines[i] = scanlines_l[i];
	else
		for(i=0; i<5; i++)
			scanlines[i] = scanlines_nl[i];

	/**** allocate memory for our own CopperList ****/

	clist = AllocMem(sizeof(struct UCopList), MEMF_CHIP | MEMF_CLEAR);
	if (clist==NULL)
	{
		UA_WarnUser(72);
		return(FALSE);
	}
	else
		SetBit(&allocFlags, COPPERLIST_FLAG);
	
	/**** Create new CopperList instructions ****/

  vp = &(paletteScreen->ViewPort);
	paletteCM = vp->ColorMap;

	if (how==1)				// 256 colors
	{
		if (!Quick)
		{
			TransferCMtoCM(pageScreen->ViewPort.ColorMap, paletteCM, 0, 64, 128);
			SetScreenToCM(paletteScreen,paletteCM);
		}

		CINIT(clist, 550);

	  CWAIT(clist, scanlines[1], 0);

		if ( CPrefs.AA_available )
			GetRGB32(pageScreen->ViewPort.ColorMap,0,256,&LoadRGB32Table[1]);	// OS function

		for(j=0x4000, k=0; j<0x8000; j=j+0x2000, k++)
		{
		  CMOVE(clist, custom.bplcon3, j+(0xc00+copCode));

			for(i=0; i<32; i++)
			{
				if ( CPrefs.AA_available )
				{
					r = LoadRGB32Table[1+(128+i+k*32)*3+0];
					g = LoadRGB32Table[1+(128+i+k*32)*3+1];
					b = LoadRGB32Table[1+(128+i+k*32)*3+2];
					r = r & 0x000000f0;
					r <<= 4;	
					g = g & 0x000000f0;
					b = b & 0x000000f0;
					b >>= 4;
				}
				else
				{
					rgb_long = GetColorCM32(pageScreen->ViewPort.ColorMap, 128+i+k*32);
					r = rgb_long & 0x00f00000;	
					r >>= 12;
					g = rgb_long & 0x0000f000;
					g >>= 8;
					b = rgb_long & 0x000000f0;
					b >>= 4;
				}
				rgb_word = r | g | b; 
			  CMOVE(clist, custom.color[i], rgb_word);
			}

		  CMOVE(clist, custom.bplcon3, j+(0xe00+copCode));

			for(i=0; i<32; i++)
			{
				if ( CPrefs.AA_available )
				{
					r = LoadRGB32Table[1+(128+i+k*32)*3+0];
					g = LoadRGB32Table[1+(128+i+k*32)*3+1];
					b = LoadRGB32Table[1+(128+i+k*32)*3+2];
					r = r & 0x0000000f;
					r <<= 8;	
					g = g & 0x0000000f;
					g <<= 4;	
					b = b & 0x0000000f;
				}
				else
				{
					rgb_long = GetColorCM32(pageScreen->ViewPort.ColorMap, 128+i+k*32);
					r = rgb_long & 0x000f0000;
					r >>= 8;
					g = rgb_long & 0x00000f00;
					g >>= 4;
					b = rgb_long & 0x0000000f;
				}
				rgb_word = r | g | b; 
			  CMOVE(clist, custom.color[i], rgb_word);
			}
		}

	  CWAIT(clist, scanlines[2], 0);

		for(j=0x8000, k=2; j<0xc000; j=j+0x2000, k++)
		{
		  CMOVE(clist, custom.bplcon3, j+(0xc00+copCode));

			for(i=0; i<32; i++)
			{
				if ( CPrefs.AA_available )
				{
					r = LoadRGB32Table[1+(128+i+k*32)*3+0];
					g = LoadRGB32Table[1+(128+i+k*32)*3+1];
					b = LoadRGB32Table[1+(128+i+k*32)*3+2];
					r = r & 0x000000f0;
					r <<= 4;	
					g = g & 0x000000f0;
					b = b & 0x000000f0;
					b >>= 4;
				}
				else
				{
					/**** 0x00rrggbb ****/
					rgb_long = GetColorCM32(pageScreen->ViewPort.ColorMap, 128+i+k*32);
					r = rgb_long & 0x00f00000;	
					r >>= 12;
					g = rgb_long & 0x0000f000;
					g >>= 8;
					b = rgb_long & 0x000000f0;
					b >>= 4;
				}
				rgb_word = r | g | b; 
			  CMOVE(clist, custom.color[i], rgb_word);
			}

		  CMOVE(clist, custom.bplcon3, j+(0xe00+copCode));

			for(i=0; i<32; i++)
			{
				if ( CPrefs.AA_available )
				{
					r = LoadRGB32Table[1+(128+i+k*32)*3+0];
					g = LoadRGB32Table[1+(128+i+k*32)*3+1];
					b = LoadRGB32Table[1+(128+i+k*32)*3+2];
					r = r & 0x0000000f;
					r <<= 8;	
					g = g & 0x0000000f;
					g <<= 4;	
					b = b & 0x0000000f;
				}
				else
				{
					/**** 0x00rrggbb ****/
					rgb_long = GetColorCM32(pageScreen->ViewPort.ColorMap, 128+i+k*32);
					r = rgb_long & 0x000f0000;
					r >>= 8;
					g = rgb_long & 0x00000f00;
					g >>= 4;
					b = rgb_long & 0x0000000f;
				}
				rgb_word = r | g | b; 
			  CMOVE(clist, custom.color[i], rgb_word);
			}
		}

	  CEND(clist);
	}
	else if (how==10)	// 32 colors
	{
		CINIT(clist, 40);	// probably only need 1+8+1+8+1+8+1=28

		if (!Quick)
		{
			TransferCMtoCM(pageScreen->ViewPort.ColorMap, paletteCM, 0, 8, 8);
			SetScreenToCM(paletteScreen,paletteCM);
		}

		/**** 8...15 are filled with 0...7	 							****/
		/**** new 8...15 must be filled with 8...15				****/
		/**** new 8...15 must be filled with 16...23			****/
		/**** new 8...15 must be filled with 24...31			****/

	  CWAIT(clist, scanlines[1], 0);
		doCMoves(pageScreen->ViewPort.ColorMap, 8, 8, 8);	// well, offset, num

	  CWAIT(clist, scanlines[2], 0);
		doCMoves(pageScreen->ViewPort.ColorMap, 8, 16, 8);

	  CWAIT(clist, scanlines[3], 0);
		doCMoves(pageScreen->ViewPort.ColorMap, 8, 24, 8);

	  CEND(clist);
	}
	else if (how==11)	// 16 colors
	{
		CINIT(clist, 20);	// probably only need 1+8+1=10

		if (!Quick)
		{
			TransferCMtoCM(pageScreen->ViewPort.ColorMap, paletteCM, 0, 8, 8);
			SetScreenToCM(paletteScreen,paletteCM);
		}

		/**** 8...15 are filled with 0...7	 							****/
		/**** new 8...15 must be filled with 8...15				****/

	  CWAIT(clist, scanlines[2], 0);
		doCMoves(pageScreen->ViewPort.ColorMap, 8, 8, 8);	// well, offset, num

	  CEND(clist);
	}

	return(TRUE);
}

/******** doCMoves() ********/

void doCMoves(struct ColorMap *cm, int wellStart, int colorOffset, int num)
{
int i;

	for(i=0; i<num; i++)
	  CMOVE(clist, custom.color[wellStart+i], GetRGB4(cm, colorOffset+i));
}

/******** doCMovesHalfB() ********/

void doCMovesHalfB(struct ColorMap *cm, int wellStart, int colorOffset, int num)
{
int i;
UWORD rgb;

	for(i=0; i<num; i++)
	{
		rgb = GetRGB4(cm, colorOffset+i);
		GetHalfBrite(&rgb);
	  CMOVE(clist, custom.color[wellStart+i], rgb);
	}
}

/******** GetHalfBrite() ********/

void GetHalfBrite(UWORD *rgb)
{
UWORD r,g,b;

	r = (*rgb & 0x0f00) >> 8;
	g = (*rgb & 0x00f0) >> 4;
	b = (*rgb & 0x000f);
	r /= 2;
	g /= 2;
	b /= 2;
	*rgb  = r << 8;
	*rgb |= g << 4;
	*rgb |= b;
}

/******** GetHalfBrite32() ********/

void GetHalfBrite32(ULONG *rgb)
{
ULONG r,g,b;

	r = (*rgb & 0x0ff0000) >> 16;
	g = (*rgb & 0x000ff00) >> 8;
	b = (*rgb & 0x00000ff);
	r /= 2;
	g /= 2;
	b /= 2;
	*rgb  = r << 16;
	*rgb |= g << 8;
	*rgb |= b;
}

/******** RebuildCopperList() ********/

void RebuildCopperList(void)
{
void *dspins, *sprins, *clrins;
struct ViewPort *vp;

	if ( !paletteWindow)
		return;

  vp = &(paletteScreen->ViewPort);
	dspins=vp->DspIns;
	sprins=vp->SprIns;
	clrins=vp->ClrIns;
	Forbid();
	vp->DspIns=0;
	vp->SprIns=0;
	vp->ClrIns=0;
	FreeVPortCopLists(vp);	// get rid of old copper list
	vp->DspIns=dspins;
	vp->SprIns=sprins;
	vp->ClrIns=clrins;
	vp->UCopIns=clist; 			// Point to new copperlist
	Permit();
	RethinkDisplay();
}

/******** SyncAllColors() ********/
/*
 * updates CPrefs.PageCM with latest pageScreen colors
 *
 */

void SyncAllColors(BOOL refreshPalette)
{
int max;
struct ViewPort *vp;
struct ColorMap *cm;
int numCMColors1, numCMColors2;

	vp = &(pageScreen->ViewPort);
	cm = vp->ColorMap;

	numCMColors1 = cm->Count;
	numCMColors2 = CPrefs.PageCM->Count;

	if ( numCMColors1 > numCMColors2 )
		max = numCMColors2;
	else
		max = numCMColors1;

	CopyCMPartial(cm, CPrefs.PageCM, 0, 0, max);

	if (refreshPalette)
		RefreshPalette();

	InvalidateTable();
}

/******** HowToFillTheWells() ********/

int HowToFillTheWells(int numColors)
{
int how;

	if ( CPrefs.AA_available )
	{
		if ( numColors==256 )
			how = 1;
		else if ( numColors==128 )
			how = 2;
		else if ( numColors==64 )
			how = 3;
		else if ( numColors==32 )
			how = 4;
		else if ( numColors==16 )
			how = 5;
		else if ( numColors==8 )
			how = 6;
		else if ( numColors==4 )
			how = 7;
		else if ( numColors==2 )
			how = 8;
	}
	else
	{
		if ( numColors==64 )
			how = 9;
		else if ( numColors==32 )
			how = 10;
		else if ( numColors==16 )
			how = 11;
		else if ( numColors==8 )
			how = 6;
		else if ( numColors==4 )
			how = 7;
		else if ( numColors==2 )
			how = 8;
	}

	return(how);
}

/******** DrawWells() ********/
/*
 *  returns ID telling how wells were drawn.
 */

int DrawWells(struct Window *window)
{
int x,w,h,numCols,numRows,top,add,well,col,row,numColors,start,end,how;
BOOL laced=FALSE;

	if ( CPrefs.PageScreenModes & LACE )
		laced=TRUE;

	numColors = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numColors=32;

	if ( laced )
	{
		top = 6;
		add = 3;
		switch(numColors)
		{
			case 2:		w=287;	h=49;	numCols=2;	numRows=1; break;
			case 4:		w=143;	h=49;	numCols=4;	numRows=1; break;
			case 8:		w=71;		h=49;	numCols=8;	numRows=1; break;
			case 16:	w=71;		h=23;	numCols=8;	numRows=2; break;
			case 32:	w=71;		h=10;	numCols=8;	numRows=4; break;
			case 64:	w=35;		h=10;	numCols=16;	numRows=4; break;
			case 128:	w=17;		h=10;	numCols=32;	numRows=4; break;
			case 256:	w=8;		h=10;	numCols=64;	numRows=4; break;
		}
	}
	else	// non-laced
	{
		top = 2;
		add = 2;
		switch(numColors)
		{
			case 2:		w=287;	h=26;	numCols=2;	numRows=1; break;
			case 4:		w=143;	h=26;	numCols=4;	numRows=1; break;
			case 8:		w=71;		h=26;	numCols=8;	numRows=1; break;
			case 16:	w=71;		h=12;	numCols=8;	numRows=2; break;
			case 32:	w=71;		h=5;	numCols=8;	numRows=4; break;
			case 64:	w=35;		h=5;	numCols=16;	numRows=4; break;
			case 128:	w=17;		h=5;	numCols=32;	numRows=4; break;
			case 256:	w=8;		h=5;	numCols=64;	numRows=4; break;
		}
	}

	/**** find out how to fill the wells ****/

	how = HowToFillTheWells(numColors);

	switch(how)	// start and end specify which color regs to draw in
	{
		case 1:		start=64;	end=191;	break;
		case 2:		start=32;	end=159;	break;
		case 3:		start=16;	end=79;		break;
		case 4:		start=8;	end=39;		break;
		case 5:		start=8;	end=23;		break;
		case 6:		start=8;	end=15;		break;
		case 7:		start=8;	end=11;		break;
		case 8:		start=8;	end=9;		break;
		case 9:		start=0;	end=15;		break;
		case 10:	start=8;	end=15;		break;
		case 11:	start=8;	end=15;		break;
	}

	/**** render wells ****/

	SetAPen(window->RPort, LO_PEN);

	if ( CPrefs.PageScreenModes & LACE )
		RectFill(	window->RPort,
							31-2, rectfills_lace[0],
							31+numCols*(w+1), rectfills_lace[9] );
	else
		RectFill(	window->RPort,
							31-2, rectfills_non_lace[0],
							31+numCols*(w+1), rectfills_non_lace[9] );

	SetDrMd(window->RPort, JAM1);
	well = start;	// color reg.

	add=0;
	for(row=0; row<numRows; row++)
	{
		x = 31;

		if ( numRows==1 )
		{
			for(col=0; col<numCols; col++)
			{
				SetAPen(window->RPort, well);

				if ( CPrefs.PageScreenModes & LACE )
					RectFill(window->RPort, x, rectfills_lace[1],				 x+w-1, rectfills_lace[8]);
				else
					RectFill(window->RPort, x, rectfills_non_lace[1],		 x+w-1, rectfills_non_lace[8]);

				well++;
				if (well>end)
					well=start;
				x = x + (w+1);
			}
		}
		else if ( numRows==2 )
		{
			for(col=0; col<numCols; col++)
			{
				SetAPen(window->RPort, well);

				if ( CPrefs.PageScreenModes & LACE )
					RectFill(window->RPort, x, rectfills_lace[1+add],			x+w-1, rectfills_lace[4+add]);
				else
					RectFill(window->RPort, x, rectfills_non_lace[1+add], x+w-1, rectfills_non_lace[4+add]);

				well++;
				if (well>end)
					well=start;
				x = x + (w+1);
			}
			add+=4;
		}
		else if ( numRows==4 )
		{
			for(col=0; col<numCols; col++)
			{
				SetAPen(window->RPort, well);

				if ( CPrefs.PageScreenModes & LACE )
					RectFill(window->RPort, x, rectfills_lace[1+add],			x+w-1, rectfills_lace[2+add]);
				else
					RectFill(window->RPort, x, rectfills_non_lace[1+add], x+w-1, rectfills_non_lace[2+add]);

				well++;
				if (well>end)
					well=start;
				x = x + (w+1);
			}
			add+=2;
		}
	}

	return(how);
}

/******** SelectAWell() ********/

void SelectAWell(int well)
{
int numColors,w,h,numCols,numRows,add,top,x,y;
BOOL laced=FALSE;

	if ( !paletteWindow)
		return;

	if ( CPrefs.PageScreenModes & LACE )
		laced=TRUE;

	numColors = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numColors=32;

	if ( laced )
	{
		top = rectfills_lace[1];
		add = 4;
		switch(numColors)
		{
			case 2:		w=287;	h=48;	numCols=2;	numRows=1; break;
			case 4:		w=143;	h=48;	numCols=4;	numRows=1; break;
			case 8:		w=71;		h=48;	numCols=8;	numRows=1; break;
			case 16:	w=71;		h=22;	numCols=8;	numRows=2; break;
			case 32:	w=71;		h= 9;	numCols=8;	numRows=4; break;
			case 64:	w=35;		h= 9;	numCols=16;	numRows=4; break;
			case 128:	w=17;		h= 9;	numCols=32;	numRows=4; break;
			case 256:	w=8;		h= 9;	numCols=64;	numRows=4; break;
		}
	}
	else	// non-laced
	{
		top = rectfills_non_lace[1];
		add = 3;
		switch(numColors)
		{
			case 2:		w=287;	h=25;	numCols=2;	numRows=1; break;
			case 4:		w=143;	h=25;	numCols=4;	numRows=1; break;
			case 8:		w=71;		h=25;	numCols=8;	numRows=1; break;
			case 16:	w=71;		h=11;	numCols=8;	numRows=2; break;
			case 32:	w=71;		h=4;	numCols=8;	numRows=4; break;
			case 64:	w=35;		h=4;	numCols=16;	numRows=4; break;
			case 128:	w=17;		h=4;	numCols=32;	numRows=4; break;
			case 256:	w=8;		h=4;	numCols=64;	numRows=4; break;
		}
	}

	x = 31 + ((well%numCols) * (w+1));
	y = top + ((well/numCols) * (h+add));

	DrawMarqueeBox(paletteWindow->RPort, x-1, y-1, x+w, y+h+1);
}

/******** FindSelectedWell() ********/

int FindSelectedWell(int mouseX, int mouseY)
{
int numColors,w,h,numCols,numRows,x,y,ymin,ymax;

	numColors = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numColors=32;

	if ( CPrefs.PageScreenModes & LACE )
	{
		ymin = rectfills_lace[0]-1;
		ymax = rectfills_lace[9]+1;
	}
	else
	{
		ymin = rectfills_non_lace[0]-1;
		ymax = rectfills_non_lace[9]+1;
	}

	switch(numColors)
	{
		case 2:		numCols=2;	numRows=1; break;
		case 4:		numCols=4;	numRows=1; break;
		case 8:		numCols=8;	numRows=1; break;
		case 16:	numCols=8;	numRows=2; break;
		case 32:	numCols=8;	numRows=4; break;
		case 64:	numCols=16;	numRows=4; break;
		case 128:	numCols=32;	numRows=4; break;
		case 256:	numCols=64;	numRows=4; break;
	}

	w = PALETTE_WIDTH / numCols;
	h = (ymax-ymin) / numRows;
	x = -1;

	if ( 	(mouseX > 31) &&
				(mouseX < (31+PALETTE_WIDTH-2)) &&
				mouseY >= ymin && mouseY <= ymax )
	{
		x = (mouseX-31) / w;
		y = (mouseY-ymin) / h;
		x = ((y * numCols) + x);
	}

	return(x);
}

/******** SmallScreenCorrect() ********/

BOOL SmallScreenCorrect(struct Screen *screen)
{
	if ( screen->TopEdge >= pageScreen->Height-10 )
	{
		ClosePalette();
		CloseSmallScrWdwStuff();
		return(FALSE);
	}
	return(TRUE);
}

/******** E O F ********/
