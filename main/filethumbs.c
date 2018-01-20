#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct NewWindow NewWindowStructure;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct TextFont *tiny_smallFont;
extern struct TextFont *tiny_largeFont;
extern struct Library *medialinkLibBase;
extern UWORD GrayMap[];
extern UWORD GrayMap2[];
extern UWORD IconColorMap[];
extern struct Screen *thumbScreen;
extern struct Window *thumbWindow;
extern UWORD chip gui_pattern[];
extern struct eventHandlerInfo EHI;
extern struct Process *process;
extern UBYTE **msgs;
extern TEXT special_char[];
extern UWORD palettes[];

/**** globals ****/

static int prevX1, prevX2, prevY1, prevY2;
struct TextAttr filethumbs_textAttr;
static struct ColorMap *iconCM=NULL;
static struct BitMap24 to_bm, to_bm_chip;
static struct RastPort to_rp;

/**** gadgets ****/

extern struct GadgetRecord Thumbnail_GR[];

/**** functions ****/

/********* ShowThumbNails() ********/
/*
 * Mode = SELECT_ONE_FILE or SELECT_MULTIPLE_FILES
 *
 */

int ShowThumbNails(	STRPTR path, UBYTE *selectionList, int Mode,
										UBYTE *listPtr, int numEntries)
{
int i, destXSize, destYSize, numThumbs, page, max, x, y, retCode, factor;
int stepX, stepY, xOff, yOff, xMin, yMin, row, col, shown, itemsPerRow, pos, dy, offs;
TEXT fullPath[SIZE_FULLPATH];
UBYTE *dirPtr, *selPtr;
//APTR tempWdwPtr;
struct Rectangle dclip;
struct GadgetRecord GR;
UWORD cmap[32];
BOOL query;
Tag ti_Tag;
ULONG ti_Data;

	/**** init vars ****/

	dirPtr = listPtr;
	selPtr = selectionList;
	prevX1 = prevX2 = prevY1 = prevY2 = -1;

	/**** calculate offsets (skip dir entries) ****/

	i=0;
	while( *(listPtr+i*SIZE_FILENAME) == DIR_PRECODE )
	{
		dirPtr += SIZE_FILENAME;
		selPtr++;
		numEntries--;
		i++;
	}

	if (numEntries<=0)	/* I only saw dirs, back out */
		return(-1);

	/**** If file is selected, remove file icon to show thumb processor	****/
	/**** that file is selected. This makes it possible to cancel				****/
	/**** selection without having to duplicate the selection list.			****/

	for(i=0; i<numEntries; i++)
	{
		if ( *(selPtr+i) == 1)
			*(dirPtr+i*SIZE_FILENAME) = 0x20;
	}

	/**** open a screen ****/

	filethumbs_textAttr.ta_Name	= (UBYTE *)APPFONT;
	if ( CPrefs.ThumbnailScreenModes & LACE )
		filethumbs_textAttr.ta_YSize = 20;
	else
		filethumbs_textAttr.ta_YSize = 10;
	filethumbs_textAttr.ta_Style = FS_NORMAL;
	filethumbs_textAttr.ta_Flags = FPF_DESIGNED;

	query = QueryOverscan(CPrefs.scriptMonitorID | CPrefs.ThumbnailScreenModes, &dclip, OSCAN_TEXT);

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		pos = ((dclip.MaxX - dclip.MinX + 1) - CPrefs.ThumbnailScreenWidth) / 2;
		dclip.MinX += pos;
		dclip.MaxX = dclip.MinX + CPrefs.ThumbnailScreenWidth - 1; 
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	thumbScreen = OpenScreenTags(		NULL,
																	SA_Depth, 		CPrefs.ThumbnailScreenDepth,
																	SA_Font,			&filethumbs_textAttr,
																	SA_DisplayID,	CPrefs.scriptMonitorID | CPrefs.ThumbnailScreenModes,
																	SA_ShowTitle,	FALSE,
																	SA_Behind,		TRUE,
																	SA_DetailPen,	0,
																	SA_BlockPen,	2,
																	SA_Type,			CUSTOMSCREEN,
																	SA_Title,			(UBYTE *)special_char,
																	SA_Quiet,			TRUE,
																	ti_Tag,				ti_Data,
																	TAG_END
																);

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		FreeSharedBM();

	if (thumbScreen == NULL)
	{
		UA_WarnUser(118);
		if ( EHI.activeScreen == STARTSCREEN_PAGE )
			AllocateSharedBM(CPrefs.PageScreenWidth+16,CPrefs.PageScreenHeight,8);
		return(-1);
	}

	GenlockOff(thumbScreen);

	thumbScreen->Title = (UBYTE *)special_char;

	ShowTitle(thumbScreen, FALSE);

	/**** open a window ****/

	NewWindowStructure.LeftEdge			= 0;
	NewWindowStructure.TopEdge			= (thumbScreen->Height - CPrefs.ThumbnailScreenHeight) / 2;
	NewWindowStructure.Width				= CPrefs.ThumbnailScreenWidth;
	NewWindowStructure.Height				= CPrefs.ThumbnailScreenHeight;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= 0;
	NewWindowStructure.Flags				= WFLG_BACKDROP | WFLG_ACTIVATE |
																		WFLG_RMBTRAP | WFLG_BORDERLESS |
																		WFLG_NOCAREREFRESH;
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
		UA_WarnUser(117);
		CloseScreen(thumbScreen);
		for(i=0; i<numEntries; i++)
			*(dirPtr+i*SIZE_FILENAME) = (UBYTE)FILE_PRECODE;
		if ( EHI.activeScreen == STARTSCREEN_PAGE )
			AllocateSharedBM(CPrefs.PageScreenWidth+16,CPrefs.PageScreenHeight,8);
		return(-1);
	}

	/**** attach message port to window ****/

	thumbWindow->UserPort = capsPort;
	ModifyIDCMP(thumbWindow, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY);

	/**** make requesters appear on thumbnail screen ****/

	EHI.thumbsVisible = TRUE;
#if 0
	Forbid();
	tempWdwPtr = process->pr_WindowPtr;
	process->pr_WindowPtr = (APTR)thumbWindow;
	Permit();
#endif
	/**** set font ****/

	if ( CPrefs.ThumbnailScreenModes & LACE )
		SetFont(thumbWindow->RPort, largeFont);
	else
		SetFont(thumbWindow->RPort, smallFont);

	/**** calculate useful parameters ****/

	if (CPrefs.ThumbnailScreenModes & LACE)
		factor = 2;
	else
		factor = 1;

	offs = 25*factor;

	if ( CPrefs.thumbnailSize==LARGE_THUMBNAILS )
	{
		numThumbs = 30;
		itemsPerRow=6;
		dy = (thumbWindow->Height - offs) / (numThumbs/itemsPerRow);
		destXSize = 100;
		destYSize = dy-5*factor;
	}
	else if ( CPrefs.thumbnailSize==SMALL_THUMBNAILS )
	{
		numThumbs = 80;
		itemsPerRow=10;
		dy = (thumbWindow->Height - offs) / (numThumbs/itemsPerRow);
		destXSize = 57;
		destYSize = dy-5*factor;
	}

#if 0
	else if ( CPrefs.ScriptPalNtsc==NTSC_MODE && CPrefs.thumbnailSize==LARGE_THUMBNAILS )
	{
		numThumbs = 30;

		itemsPerRow=6;
		dy = (thumbWindow->Height - offs) / (numThumbs/itemsPerRow);

		destXSize = 100;
		destYSize = dy-5*factor;	//31;
	}
	else if ( CPrefs.ScriptPalNtsc==NTSC_MODE && CPrefs.thumbnailSize==SMALL_THUMBNAILS )
	{
		numThumbs = 80;

		itemsPerRow=10;
		dy = (thumbWindow->Height - offs) / (numThumbs/itemsPerRow);

		destXSize = 57;
		destYSize = dy-5*factor;	//16;
	}
#endif

/*
	if (CPrefs.ThumbnailScreenModes & LACE)
		factor = 2;
	else
		factor = 1;
*/

	if (factor==1)
	{
		xOff=3;
		yOff=1;
		xMin=2;
		yMin=3;
	}
	else
	{
		xOff=3;
		yOff=1;
		xMin=2;
		yMin=8;
	}

	//destYSize *= factor;

	/**** draw and render gadgets ****/

	for(i=0; i<4; i++)
	{
		Thumbnail_GR[i].y1 = thumbWindow->Height-(15*factor);
		Thumbnail_GR[i].y2 = thumbWindow->Height-(2*factor);
	}

	UA_DrawGadgetList(thumbWindow, Thumbnail_GR);

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

	/**** create icon's colormap ****/

	iconCM = GetColorMap(16);
	if ( iconCM==NULL )
		return(-1);
	for(i=0; i<16; i++)
		SetColorCM4(iconCM, IconColorMap[i], i);

	/**** show the user the screen ****/

	ScreenToFront(thumbScreen);

	/**** check if there are less thumbs to fill more than one screen ****/

	if ( numEntries <= numThumbs )
	{
		UA_DisableButton(thumbWindow, &Thumbnail_GR[0], gui_pattern);	/* prev */
		UA_DisableButton(thumbWindow, &Thumbnail_GR[1], gui_pattern);	/* next */
	}

	if ( CPrefs.thumbnailSize==LARGE_THUMBNAILS )
	{
		stepX = 106;
		stepY = dy;
	}
	else if ( CPrefs.thumbnailSize==SMALL_THUMBNAILS )
	{
		stepX = 63;
		stepY = dy;
	}

#if 0
	if ( CPrefs.ScriptPalNtsc==PAL_MODE && CPrefs.thumbnailSize==LARGE_THUMBNAILS )
	{
		stepX = 106;
		stepY = dy;	//45*factor;
	}
	else if ( CPrefs.ScriptPalNtsc==PAL_MODE && CPrefs.thumbnailSize==SMALL_THUMBNAILS )
	{
		stepX = 63;
		stepY = dy;	//27*factor;
	}
	else if ( CPrefs.ScriptPalNtsc==NTSC_MODE && CPrefs.thumbnailSize==LARGE_THUMBNAILS )
	{
		stepX = 106;
		stepY = dy;	//36*factor;
	}
	else if ( CPrefs.ScriptPalNtsc==NTSC_MODE && CPrefs.thumbnailSize==SMALL_THUMBNAILS )
	{
		stepX = 63;
		stepY = dy;	//21*factor;
	}
#endif

	/**** alloc bitmaps ****/

	to_bm.MagicCookie = MAGIC_COOKIE_BM24;
	to_bm_chip.MagicCookie = MAGIC_COOKIE_BM24;

	AllocPlanes24(	&to_bm, CPrefs.ThumbnailScreenDepth, destXSize+16, destYSize,
									MEMF_ANY, BM24F_NONE );
	AllocPlanes24(	&to_bm_chip, CPrefs.ThumbnailScreenDepth, destXSize+16, destYSize,
									MEMF_CHIP, BM24F_NONE );
	if ( !to_bm_chip.Planes[0] && !to_bm.Planes[0] )
	{
		UA_WarnUser(119);
		if ( EHI.activeScreen == STARTSCREEN_PAGE )
			AllocateSharedBM(CPrefs.PageScreenWidth+16,CPrefs.PageScreenHeight,8);
		return(-1);
	}

	InitRastPort(&to_rp);
	to_rp.BitMap = (struct BitMap *)&to_bm_chip;

	/**** handle events ****/

/*
	if (numThumbs==30)
		itemsPerRow=6;
	else if (numThumbs==80)
		itemsPerRow=10;
*/

	/**** color table ****/

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		InvalidateTable();
	else if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
		AllocateTable();

	i=0;
	while(1)
	{
		if ( (i+numThumbs) > numEntries )
			max = numEntries-i;
		else
			max = numThumbs;

		/**** show real thumbs ****/

		x=5;	// here the first thumb is drawn ( nothing is added first time )
		y=2;	// here the first thumb is drawn ( nothing is added first time )
		row = -1;
		col = -1;
		shown=0;

		GR.txt = NULL;
		GR.type = HIBOX_REGION;
		GR.color = 2;
		for(page=0; page<max; page++)
		{
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

			x += stepX;
			if ( x>=(stepX*itemsPerRow) )
			{
				x=5;
				y += stepY;
			}
		}

		x=5;	// here the first thumb is drawn ( nothing is added first time )
		y=2;	// here the first thumb is drawn ( nothing is added first time )
		row = -1;
		col = -1;

		for(page=0; page<max; page++)
		{
			/**** get path of next picture ****/

			UA_MakeFullPath(path, dirPtr+1+i*SIZE_FILENAME, fullPath); // +1 skips ID

			/**** render the thumbnail ****/

			DrawThumb(x, y, destXSize, destYSize, fullPath, dirPtr+1+i*SIZE_FILENAME);

			/**** if the file is selected, draw a box around it ****/

			if ( *(dirPtr+i*SIZE_FILENAME) == 0x20 )
			{
				col = x / stepX;	//destXSize;											
				row = y / stepY;	//destYSize;											

				if (thumbWindow->WScreen->ViewPort.Modes & LACE)
				{
					if ( ReadPixel(thumbWindow->RPort, xOff+col*stepX, yOff+row*stepY-1)==HI_PEN )
						DrawThumbBox(xOff+col*stepX, yOff+row*stepY, stepX-xMin, stepY-yMin);
				}
				else
				{
					if ( ReadPixel(thumbWindow->RPort, xOff+col*stepX, yOff+row*stepY)==HI_PEN )
						DrawThumbBox(xOff+col*stepX, yOff+row*stepY, stepX-xMin, stepY-yMin);
				}

				prevX1 = xOff+col*stepX;
				prevY1 = yOff+row*stepY;
				prevX2 = stepX-xMin;
				prevY2 = stepY-yMin;
			}

			/**** monitor LMB ****/

			shown++;

			retCode = QuickInterfere(	dirPtr, (i/numThumbs)*numThumbs, stepX, stepY, max, Mode,
																factor, row, col, numEntries, numThumbs, destXSize, destYSize);
			if (retCode != -1)
			{
				i += ((numThumbs-shown)+1);

				/**** have to 'umwandeln' retVal ****/

				if (retCode==0)
					retCode++;
				else if (retCode==1)
					retCode++;
				else if (retCode==2)
					retCode=4;
				else if (retCode==3)
					retCode=3;

				goto skip_fast;
			}

			/**** calculate the next position ****/

			x += stepX;
			if ( x>=(stepX*itemsPerRow) )
			{
				x=5;
				y += stepY;
			}

			i++;
		}

		retCode = Monitor_Thumbs(	dirPtr, i-max, stepX, stepY, max, Mode,
															factor, row, col, numEntries, numThumbs,
															destXSize, destYSize );

skip_fast:

		if (retCode==1)	/* prev */
		{
			if (i<=numThumbs)	// new - is true on first page
				i=numEntries-(numEntries % numThumbs);	// new
			else if (max != numThumbs)	/* last page shown */
				i = numEntries-(max+numThumbs); //i -= (max+numThumbs);
			else
				i -= (2*numThumbs);
			if (i<0)
			{
				i=numEntries-(numEntries % numThumbs);
				if (i<0)
					i=0;
			}
			if (i==numEntries)	// new
				i=numEntries-numThumbs;	// new
		}
		else if (retCode==2)	/* next */
		{
			if (i==numEntries)	// new
				i=0;	// new
			else if (max != numThumbs)	/* last page shown */
				i=0;
		}
		else if (retCode==3 || retCode==4 || retCode==5)
			break;

		/**** clear the screen ****/

		SetAPen(thumbWindow->RPort, 0L);
		WaitTOF();
		RectFill(thumbWindow->RPort, 0L, 0L, CPrefs.ThumbnailScreenWidth-1, Thumbnail_GR[0].y1-3L);
		WaitBlit();

		prevX1 = prevX2 = prevY1 = prevY2 = -1;
	}

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		InvalidateTable();
	else if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
		ReleaseTable();

	if (retCode==3 || retCode==5)	/* OK (button or dbl click) */
	{
		for(i=0; i<numEntries; i++)
		{
			if ( *(dirPtr+i*SIZE_FILENAME) == 0x20 )	/* selected */
			{
				*(dirPtr+i*SIZE_FILENAME) = (UBYTE)FILE_PRECODE;
				*(selPtr+i) = 1;
			}
			else
				*(selPtr+i) = 0;
		}
	}
	else if (retCode==4) /* cancel */
	{
		for(i=0; i<numEntries; i++)
			*(dirPtr+i*SIZE_FILENAME) = (UBYTE)FILE_PRECODE;
	}

	/**** close the shop ****/

	FreePlanes24( &to_bm );
	FreePlanes24( &to_bm_chip );

	MyScreenToBack(thumbScreen);

	FreeColorMap(iconCM);

	/**** restore gadgets ****/

	UA_EnableButton(thumbWindow, &Thumbnail_GR[0]);	/* prev */
	UA_EnableButton(thumbWindow, &Thumbnail_GR[1]);	/* next */

	/**** don't make requesters appear on thumbnail screen ****/

	EHI.thumbsVisible = FALSE;
#if 0
	Forbid();
	process->pr_WindowPtr = tempWdwPtr;
	Permit();
#endif
	/**** close stuff ****/

	UA_CloseWindowSafely(thumbWindow);
	CloseScreen(thumbScreen);

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		AllocateSharedBM(CPrefs.PageScreenWidth+16,CPrefs.PageScreenHeight,8);

	return(retCode);
}

/********	Monitor_Thumbs() ********/
/*
 * 1 = prev, 2 = next, 3 = ok, 4 = cancel
 * Mode==SELECT_ONE_FILE, SELECT_MULTIPLE_FILES
 */

int Monitor_Thumbs(	UBYTE *listPtr, int offset, int stepX, int stepY,
										int max, int Mode, int factor, int in_row, int in_col,
										int numEntries, int numThumbs, int dxs, int dys)
{
int ID,col,row,itemsPerRow,mx;

	while(1)
	{
		UA_doStandardWait(thumbWindow,&CED);

		if ( !(thumbWindow->Flags & WFLG_WINDOWACTIVE) )
			CED.Class = -1;

		if (CED.Class == IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			if (CED.extraClass == DBLCLICKED)
			{
				col = CED.MouseX / stepX;											
				row = CED.MouseY / stepY;											
				if (numThumbs==30)
					itemsPerRow=6;
				else if (numThumbs==80)
					itemsPerRow=10;
				if (numEntries<numThumbs)
					mx=numEntries;
				else
					mx=numThumbs;
				if ( (col + (row * itemsPerRow)) < mx )
				{
					UA_HiliteButton(thumbWindow, &Thumbnail_GR[3]);
					return(5);
				}
			}

			ID = UA_CheckGadgetList(thumbWindow, Thumbnail_GR, &CED);
			if (ID==0)			/* prev */
			{
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[ID]);
				return(1);
			}
			else if (ID==1)	/* next */
			{
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[ID]);
				return(2);
			}
			else if (ID==2)	/* cancel */
			{
do_cancel:
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[2]);
				return(4);
			}
			else if (ID==3)	/* OK */
			{
do_ok:
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[3]);
				return(3);
			}
			else
			{
				SelectThumbNail(listPtr, offset, stepX, stepY,
												max, Mode, factor, in_row, in_col, numEntries,
												&CED, numThumbs, dxs, dys);
			}
		}
		else if (CED.Class==RAWKEY)
		{
			if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&Thumbnail_GR[3]))
				goto do_ok;
			else if (CED.Code==RAW_ESCAPE && !UA_IsGadgetDisabled(&Thumbnail_GR[2]))
				goto do_cancel;
		}
	}
}

/******** DrawThumb() ********/

void DrawThumb(	int x, int y, int destXSize, int destYSize, STRPTR fullPath,
								STRPTR fileName)
{
struct IFF_FRAME iff;
struct BitMap24 iff_bm;
int allOK;
ULONG type, IFF_ID;

	/**** init iff struct ****/

	FastInitIFFFrame(&iff);

	IFF_ID = FastOpenIFF(&iff, (STRPTR)fullPath);

	if (iff.Error == IFF_ERROR_DOS_FAILURE )
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_UnableToReadPic-1], fileName);
		return;
	}
	else if (	iff.Error == IFF_ERROR_NOT_IFF || iff.Error == IFF_ERROR_ID_NOT_FOUND )
	{
		FastCloseIFF(&iff);
		if (!ReadAndRenderIcon(fileName, fullPath, destXSize, destYSize, x, y, thumbWindow->RPort))
		{
			/* if readandrender failed, it complaint already about a missing */
			/* .info file. But if this is no doc but an ilbm, complain about */
			/* a missing ID FORM. */

			type = checkFileType(fullPath, NULL);
			if ( type != PAGETALK_ID_1 ) 
				Message(msgs[Msg_ProblemsWithIFFPic-1], fileName);
		}
		return;
	}
	else if (iff.Error == IFF_ERROR_NO_MEMORY )
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_NotEnoughMemory-1]);
		return;
	}
	else if ( iff.Error == IFF_ERROR_OK && (IFF_ID==ILBM || IFF_ID==ANIM) )
	{
		FastParseChunk(&iff,BMHD);
		if (iff.Error == IFF_ERROR_ID_NOT_FOUND)
		{
			// BMHD misses (probably a colormap)
		}
		else
		{
			allOK=0;
			if (iff.BMH.nPlanes <= 24)
			{
				if (iff.BMH.nPlanes > 0)
				{
					iff_bm.MagicCookie = MAGIC_COOKIE_BM24;
					allOK = AllocPlanes24(	&iff_bm, (UBYTE)iff.BMH.nPlanes, (SHORT)iff.BMH.w,
																	(SHORT)iff.BMH.h, MEMF_ANY, BM24F_NONE );
				}
				if ( !allOK )
				{
					FastCloseIFF(&iff);
					//Message(msgs[Msg_NotEnoughMemory-1]);
					return;
				}

				FastParseChunk( &iff, CAMG );
				if (iff.Error != NULL && iff.Error != IFF_ERROR_ID_NOT_FOUND)
				{
					allOK = FALSE;
					Message(msgs[Msg_IFFChunkMisses-1], "CAMG");
				}

				FastParseChunk( &iff, CMAP );

				FastDecodeBody( &iff, (struct BitMap24 *)&iff_bm, NULL );
				if (iff.Error != NULL)
				{
					allOK = FALSE;
					Message(msgs[Msg_IFFChunkMisses-1], "BODY");	// BODY misses
				}

				if ( allOK )
					ScaleAndRenderThumbnail(&iff_bm, &iff, destXSize, destYSize, x, y);

				FreePlanes24( &iff_bm );
			}
			else	/* warn user for too deep picture */
			{
				SetAPen(thumbWindow->RPort, 1L);
				SetDrMd(thumbWindow->RPort, JAM1);
				Move(thumbWindow->RPort, x+5, y+10+thumbWindow->RPort->TxBaseline);
				Text(thumbWindow->RPort, msgs[Msg_ImageToo-1], strlen(msgs[Msg_ImageToo-1]));
				Move(thumbWindow->RPort, x+5, y+10+thumbWindow->RPort->TxBaseline+thumbWindow->RPort->TxHeight);
				Text(thumbWindow->RPort, msgs[Msg_Large-1], strlen(msgs[Msg_Large-1]));
			}
		}
	}
	else
		Message(msgs[Msg_ProblemsWithIFFPic-1], fileName);

	FastCloseIFF(&iff);
}

/******** ReadAndRenderIcon() ********/

BOOL ReadAndRenderIcon(	STRPTR filename, STRPTR path, int destXSize,
												int destYSize, int x, int y, struct RastPort *rp)
{
struct DiskObject *diskObject;
struct Image *wbImage;
struct Gadget *wbGadget;
TEXT fname[SIZE_FULLPATH];
struct IFF_FRAME iff;
struct BitMap bm;
int i;

	diskObject = (struct DiskObject *)GetDiskObject(path);
	if (diskObject==NULL)
	{
		if ( CPrefs.ThumbnailScreenModes & LACE )
			SetFont(thumbWindow->RPort, tiny_largeFont);
		else
			SetFont(thumbWindow->RPort, tiny_smallFont);

		strcpy(fname,filename);
		UA_ShortenString(thumbWindow->RPort, fname, destXSize-16);

		SetAPen(thumbWindow->RPort, 1L);
		SetDrMd(thumbWindow->RPort, JAM1);
		Move(thumbWindow->RPort, x+5, y+10+thumbWindow->RPort->TxBaseline);
		Text(thumbWindow->RPort, fname, strlen(fname));

		if ( CPrefs.ThumbnailScreenModes & LACE )
			SetFont(thumbWindow->RPort, largeFont);
		else
			SetFont(thumbWindow->RPort, smallFont);

		return(FALSE);
	}
	else
	{
		wbGadget = &(diskObject->do_Gadget);
		wbImage = wbGadget->GadgetRender;

		iff.BMH.nPlanes = wbImage->Depth;
		iff.BMH.w = wbImage->Width;
		iff.BMH.h = wbImage->Height;
		iff.viewModes = HIRES;
		iff.colorMap = iconCM;
		InitBitMap(&bm,wbImage->Depth,wbImage->Width,wbImage->Height);
		for(i=0; i<wbImage->Depth; i++)
			bm.Planes[i] = (PLANEPTR)wbImage->ImageData + i*RASSIZE(wbImage->Width,wbImage->Height);
		ScaleAndRenderThumbnail((struct BitMap24 *)&bm, &iff, destXSize, destYSize, x, y);

		FreeDiskObject(diskObject);
	}

	return(TRUE);
}

/******** QuickInterfere() ********/

int QuickInterfere(	UBYTE *listPtr, int offset, int stepX, int stepY,
										int max, int Mode, int factor, int in_row, int in_col,
										int numEntries, int numThumbs, int dxs, int dys)
{
struct IntuiMessage *message;
struct IntuiMessage localMsg;
int ID;

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
	{
		CopyMem(message, &localMsg, sizeof(struct IntuiMessage));
		ReplyMsg((struct Message *)message);
		/**** copy interesting fields ****/
		CED.Class	= localMsg.Class;
		CED.Code = localMsg.Code;
		CED.MouseX = localMsg.MouseX;
		CED.MouseY = localMsg.MouseY;
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(thumbWindow, Thumbnail_GR, &CED);
			if (ID==0 || ID==1 || ID==2 || ID==3)
			{
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[ID]);
				while(message = (struct IntuiMessage *)GetMsg(capsPort))
					ReplyMsg((struct Message *)message);
				return(ID);
			}
			SelectThumbNail(listPtr, offset, stepX, stepY,
											max, Mode, factor, in_row, in_col, numEntries,
											&CED, numThumbs, dxs, dys);
		}
		else if ( CED.Class==RAWKEY )
		{
			ID=-1;
			if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&Thumbnail_GR[3]))
				ID=3;
			else if (CED.Code==RAW_ESCAPE && !UA_IsGadgetDisabled(&Thumbnail_GR[2]))
				ID=2;
			if ( ID!=-1 )
			{
				UA_HiliteButton(thumbWindow, &Thumbnail_GR[ID]);
				while(message = (struct IntuiMessage *)GetMsg(capsPort))
					ReplyMsg((struct Message *)message);
				return(ID);
			}
		}
	}

	return(-1);
}

/******** SelectThumbNail() ********/

void SelectThumbNail(	UBYTE *listPtr, int offset, int stepX, int stepY,
											int max, int Mode, int factor, int in_row, int in_col,
											int numEntries, struct EventData *ED, int numThumbs,
											int dxs, int dys)
{
int col,row,pos,itemsPerRow;
int xOff, yOff, xMin, yMin, i, prevPos=-1;
UBYTE ch;

	if (offset<0)
		offset=0;

	if (factor==1)
	{
		xOff=3;
		yOff=1;
		xMin=2;
		yMin=3;
	}
	else
	{
		xOff=3;
		yOff=1;
		xMin=2;
		yMin=8;
	}

	col = ED->MouseX / stepX;											
	row = ED->MouseY / stepY;											

	if (numThumbs==30)
		itemsPerRow=6;
	else if (numThumbs==80)
		itemsPerRow=10;

	pos = col + (row * itemsPerRow);

	if (pos<max && col<itemsPerRow)
	{
		if (Mode==SELECT_ONE_FILE && prevX1!=-1)
			DrawThumbBox(prevX1, prevY1, prevX2, prevY2);

		prevX1 = xOff+col*stepX;
		prevY1 = yOff+row*stepY;
		prevX2 = stepX-xMin;
		prevY2 = stepY-yMin;

		DrawThumbBox(xOff+col*stepX, yOff+row*stepY, stepX-xMin, stepY-yMin);

		ch = *(listPtr+(offset+pos)*SIZE_FILENAME);

		if (Mode==SELECT_ONE_FILE)
		{
			for(i=0; i<numEntries; i++)
			{
				if ( *(listPtr+i*SIZE_FILENAME) == 0x20 )
					prevPos = i; 
				*(listPtr+i*SIZE_FILENAME) = (UBYTE)FILE_PRECODE;
			}
		}

		if ( (Mode==SELECT_ONE_FILE) && (prevPos==(offset+pos)) )
		{
			*(listPtr+prevPos*SIZE_FILENAME) = 0x20;	// keep it selected
			return;
		}

		if ( ch == 0x20 )	/* selected */
			*(listPtr+(offset+pos)*SIZE_FILENAME) = (UBYTE)FILE_PRECODE;
		else
			*(listPtr+(offset+pos)*SIZE_FILENAME) = 0x20;
	}	
}

/******** ScaleAndRenderThumbnail() ********/

void ScaleAndRenderThumbnail(	struct BitMap24 *source_bm, struct IFF_FRAME *iff,
															int destXSize, int destYSize, int x, int y )
{
struct ScaleRemapInfo SRI;

	Move(&to_rp,0,0);
	SetRast(&to_rp,0);
	WaitBlit();
	
	SRI.SrcBitMap					= source_bm;
	SRI.DstBitMap					= &to_bm;
	SRI.SrcViewModes  		= (ULONG)iff->viewModes;
	SRI.DstViewModes  		= HIRES;
	SRI.SrcColorMap				= iff->colorMap;
	SRI.DstColorMap				= thumbScreen->ViewPort.ColorMap;
	SRI.SrcX							= 0;
	SRI.SrcY							= 0;
	SRI.SrcWidth					= iff->BMH.w;
	SRI.SrcHeight					= iff->BMH.h;
	SRI.XSrcFactor				= iff->BMH.w;
	SRI.YSrcFactor				= iff->BMH.h;
	SRI.DestX							= 0;
	SRI.DestY							= 0;
	SRI.DestWidth					= destXSize;
	SRI.DestHeight				= destYSize;
	SRI.XDestFactor				= destXSize;
	SRI.YDestFactor				= destYSize;
	SRI.Flags							= SCAREMF_OPAQUE;
	SRI.DitherMode				= DITHER_FLOYD;
	SRI.DstMaskPlane			= NULL;
	SRI.TransparentColor	= 0;

	if ( !ScaleRemap(&SRI) )
		Message("ScaleRemap Failed\n");
	
	// soft blit from fast to chip

	BltBitMapFM((struct BitMap *)&to_bm,
							0,0,
							(struct BitMap *)&to_bm_chip,
							0,0,destXSize,destYSize,
							0xc0,0xff,NULL);
	
	// hard blit from soft to screen
	
	BltBitMapRastPort((struct BitMap *)&to_bm_chip,0,0,thumbWindow->RPort,x,y,destXSize,destYSize,0xc0);
	WaitBlit();
}
				
/******** E O F ********/
