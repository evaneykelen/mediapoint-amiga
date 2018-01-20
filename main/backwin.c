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
extern struct EditSupport **Clipboard_SL;
extern struct EditSupport **Undo_SL;
extern struct Screen **DA_Screens;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct ColorMap *undoCM;
extern int lastUndoableAction;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern struct Screen *pageScreen;

/**** globals ****/

int tileWidth=0, tileHeight=0;
BOOL do_the_tile = FALSE;

/**** functions ****/

/******** ClearBackWindow() ********/

void ClearBackWindow(void)
{
int i;

	backEW.X 								= 0;
	backEW.Y 								= 0;
	backEW.Width 						= 0;
	backEW.Height 					= 0;
	backEW.TopMargin				= DEFAULT_TM;
	backEW.BottomMargin			= DEFAULT_BM;
	backEW.LeftMargin				= DEFAULT_LM;
	backEW.RightMargin			= DEFAULT_RM;
	backEW.Border						= DEFAULT_BORDER;
	backEW.BorderColor[0]		= DEFAULT_BCOLOR;
	backEW.BorderColor[1]		= DEFAULT_BCOLOR;
	backEW.BorderColor[2]		= DEFAULT_BCOLOR;
	backEW.BorderColor[3]		= DEFAULT_BCOLOR;
	backEW.BorderWidth			= DEFAULT_BWIDTH;
	backEW.BackFillType			= 2;	// TRANSP BECAUSE CEES LIKES THAT
	backEW.BackFillColor		= 0;
	backEW.PhotoOffsetX			= 0;
	backEW.PhotoOffsetY			= 0;
	backEW.patternNum				= DEFAULT_PATTERN;
	backEW.DrawSeqNum				= 0;

	backEW.TEI							= NULL;

	backEW.charFont					= largeFont;
	backEW.underlineColor		= 2;
	backEW.charStyle				= 0;
	backEW.charColor				= 2;

	backEW.flags						= 0;//EW_IS_BACKWIN;

	for(i=0; i<3; i++)
	{
		backEW.in1[i]					= -1;
		backEW.in2[i]					= -1;
		backEW.in3[i]					= -1;
		backEW.out1[i]				= -1;
		backEW.out2[i]				= -1;
		backEW.out3[i]				= -1;
		backEW.inDelay[i]			= 0;
		backEW.outDelay[i]		= 0;
	}

	backEW.antiAliasLevel		= 0;
	backEW.justification		= 0;
	backEW.xSpacing					= 0;
	backEW.ySpacing					= 0;
	backEW.slantAmount			= 2;
	backEW.slantValue				= 1;
	backEW.underLineHeight	= 1;
	backEW.underLineOffset	= 0;
	backEW.shadowDepth			= 0;
	backEW.shadow_Pen				= 0;
	backEW.shadowType				= 0;
	backEW.shadowDirection	= 0;

	backEW.wdw_shadowDepth		= 4;
	backEW.wdw_shadowDirection = 0;
	backEW.wdw_shadowPen		= 1;

	backEW.crawl_fontName[0]= '\0';
	backEW.crawl_fontSize		= 0;
	backEW.crawl_speed			= 0;
	backEW.crawl_flags			= 0;
	backEW.crawl_text				= NULL;
	backEW.crawl_length			= 0;
	backEW.crawl_color			= 1;

	backEW.bx								= -1;
	backEW.by								= -1;
	backEW.bwidth						= -1;
	backEW.bheight					= -1;
	backEW.jumpType					= 0;
	backEW.renderType				= RENDERTYPE_INVERT;
	backEW.audioCue					= 0;
	backEW.keyCode					= -1;	
	backEW.rawkeyCode				= -1;
	backEW.buttonName[0]		= '\0';
	backEW.assignment[0]		= '\0';

	backEW.TEI							= NULL;

	backEW.animIsAnim				= FALSE;
	backEW.animLoops				= 0;
	backEW.animSpeed				= 0;
	backEW.animFromDisk			= 0;
	backEW.animAddFrames		= 0;

	/**** init all EditSupport vars ****/

	backES.Active						= FALSE;
	backES.iff							= NULL;
	backES.photoOpts				= 0;
	backES.picPath[0]				= '\0';
	backES.cm								= NULL;

	ClearBitMap24(&backES.ori_bm);
	backES.ori_w						= 0;
	backES.ori_h						= 0;

	ClearBitMap(&backES.scaled_bm);
	backES.scaled_w					= 0;
	backES.scaled_h					= 0;

	ClearBitMap(&backES.remapped_bm);
	backES.remapped_w				= 0;
	backES.remapped_h				= 0;

	ClearBitMap(&backES.mask_bm);
	backES.mask_w						= 0;
	backES.mask_h						= 0;

	ClearBitMap(&backES.ori_mask_bm);
	backES.ori_mask_w				= 0;
	backES.ori_mask_h				= 0;

	backES.ditherMode				= DITHER_OFF;	//FLOYD;

	// tile 

	tileWidth=0;
	tileHeight=0;
	do_the_tile = FALSE;
}

/******** ImportABackground() ********/

void ImportABackground(	BOOL resizeIt, int remapIt, STRPTR picpath,
												STRPTR picname )
{
TEXT filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
BOOL retval;
int i;

	if ( !picpath )
	{
		/**** show file requester ****/

		retval = OpenAFile(	CPrefs.import_picture_Path, filename,
												msgs[Msg_SelectPic-1], pageWindow,
												DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_NOINFO, FALSE);
		if (!retval)
		{
			//ResetStandardColors(pageWindow);
			return;
		}

		CloseDir();
	}

	/**** fill backEW  ****/

	backEW.Width					= CPrefs.PageScreenWidth;
	backEW.Height					= CPrefs.PageScreenHeight;
	backEW.BackFillType		= 2;
	backEW.BackFillColor	= 0;
	backEW.PhotoOffsetX		= 0;
	backEW.PhotoOffsetY		= 0;

	/**** fill backES ****/

	if ( picpath )
	{
		strcpy(fullPath, picpath);
		strcpy(filename, picname);
	}
	else
		UA_MakeFullPath(CPrefs.import_picture_Path, filename, fullPath);

	strcpy(backES.picPath, fullPath);

	backES.photoOpts = 0;

	if ( resizeIt )
		backES.photoOpts = SIZE_PHOTO;
	else
		backES.photoOpts = MOVE_PHOTO;

	if ( remapIt )
		backES.photoOpts |= REMAP_PHOTO;

	/**** load picture ****/

	retval = doActualILBMLoading(backES.picPath, filename, &backES, &backEW, TRUE);
	if (!retval)
		Message(msgs[Msg_ReadingClipFailed-1], filename);
	else if ( !picpath )
		ProcessLoadedColors(&backES, remapIt);

	/**** show background ****/

	if ( retval )
	{
		ShowBackground();

		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] )
				DrawEditWindow( EditWindowList[i], EditSupportList[i] );
	}
}

/******** ShowBackground() ********/

void ShowBackground(void)
{
WORD dummy,x,y,col,row,c,r,w,h;

	backEW.BackFillType	= 0;	// SOLID FOR A WHILE

	//ValidateBoundingBox(&backEW.X, &backEW.Y, &backEW.Width, &backEW.Height);
	CorrectEW(&backEW);

	SetDrMd(pageWindow->RPort,JAM1);
	Move(pageWindow->RPort,0,0);
	SetRast(pageWindow->RPort,0L);
	WaitBlit();

	if ( backES.iff->BMH.w < CPrefs.PageScreenWidth )
		x = (CPrefs.PageScreenWidth-backES.iff->BMH.w)/2;
	else
		x = 0;

	if ( backES.iff->BMH.h < CPrefs.PageScreenHeight )
		y = (CPrefs.PageScreenHeight-backES.iff->BMH.h)/2;
	else
		y = 0;

	if ( do_the_tile && (tileWidth*tileHeight) > 0 )
		backEW.PhotoOffsetX	= 0;
	else
		backEW.PhotoOffsetX	= x;

	backEW.PhotoOffsetY	= 0;	//y; to solve PAL/NTSC centering problems

	if ( backES.photoOpts & MOVE_PHOTO )
	{
		PutMovePicture(	&backEW, &backES, &dummy, &dummy, &dummy, &dummy,
										FALSE, &dummy, &dummy, &dummy, &dummy, &dummy,
										pageWindow->RPort,0,0);

		// tile if tiling's what we want...

		if ( do_the_tile && (tileWidth*tileHeight) > 0 )
		{
			col = CPrefs.PageScreenWidth / tileWidth;
			row = CPrefs.PageScreenHeight / tileHeight;

			for(c=0; c<=col; c++)
			{
				for(r=0; r<=row; r++)
				{
					if ( !(c==0 && r==0) )
					{
						x = c*tileWidth;
						w = tileWidth;													// eg 11 pixels
						if ( (x+w) > CPrefs.PageScreenWidth )		// x=630 + 11 = 641
							w = CPrefs.PageScreenWidth-x;					// w=640-630 = 10

						y = r*tileHeight;
						h = tileHeight;
						if ( (y+h) > CPrefs.PageScreenHeight )
							h = CPrefs.PageScreenHeight-y;

						if ( x>=0 && y>=0 )
							ClipBlit(	pageWindow->RPort, 0,0, pageWindow->RPort, x,y,w,h, 0xc0 );
					}						
				}
			}
		}
		WaitBlit();
	}
	else if ( backES.photoOpts & SIZE_PHOTO )
		PutSizePicture(	&backEW, &backES,FALSE,&dummy,&dummy,
										pageWindow->RPort,0,0);

	backEW.BackFillType	= 2;	// TRANSP AGAIN
}

/******** ProcessLoadedColors() ********/

void ProcessLoadedColors(struct EditSupport *es, BOOL remapIt)
{
BOOL palettesEqual;

	if ( es->cm == NULL )
		return;

	/**** first check if loaded palette is equal to current palette ****/

	if ( CompareCM(es->cm, CPrefs.PageCM) )
		palettesEqual=TRUE;
	else
		palettesEqual=FALSE;

	/**** if not equal, ask if palette must be used ****/

	if ( !palettesEqual && !remapIt )
	{
		if (UA_OpenGenericWindow(	pageWindow, TRUE, TRUE, msgs[Msg_OK-1], msgs[Msg_Cancel-1],
															QUESTION_ICON, msgs[Msg_SpecialsUsePalette-1], TRUE, NULL))
		{
			SetScreenToCM(pageScreen, es->cm);
			lastUndoableAction=PAGE_UNDO_IMPORT;
		}
		else
			es->photoOpts |= REMAP_PHOTO;
	}

	SyncAllColors(TRUE);
}

/******** E O F ********/
