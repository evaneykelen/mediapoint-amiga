/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include <graphics/displayinfo.h>

/**** externals ****/

extern ULONG allocFlags;
extern struct MsgPort *capsPort;
extern struct CapsPrefs CPrefs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct RendezVousRecord rvrec;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct eventHandlerInfo EHI;
extern struct Document pageDoc;
extern struct TextAttr page_textAttr;
extern struct TextAttr script_textAttr;
extern TEXT pageScreenTitle[];
extern TEXT scriptScreenTitle[];
extern struct NewWindow NewWindowStructure;
extern struct Screen *pageScreen;
extern struct Window *pageWindow;
extern struct Screen *scriptScreen;
extern struct Window *scriptWindow;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;
extern ULONG numEntries1, numDisplay1, numEntries2, numDisplay2;
extern LONG topEntry1, topEntry2;
extern struct ObjectInfo ObjectRecord;
extern struct Gadget ScriptSlider1;
extern struct Gadget ScriptSlider2;
extern UWORD chip gui_pattern[];
extern UWORD palettes[];
extern struct Document scriptDoc;
extern struct EventData CED;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern UWORD *emptySprite;
extern struct Window *smallWindow;
extern struct Process *process;

/**** static globals ****/

struct Screen *playScreen = NULL;

static TEXT SEname_text[] = "ScriptEditor";
TEXT special_char[] = "MediaPoint Screen";
static char disabled_gadgets = 0;
struct Window *playWindow = NULL;	// also used in commod.c and script.c
struct TextAttr small_textAttr;
//STATIC WORD dri_Pens[] = { 1,1,1,1,1,1,1,1,1,~0 };
//STATIC WORD dri_Pens[] = { 0,0,0,0,0,0,0,0,0,0,0,2,~0 };
STATIC WORD dri_Pens[] = { ~0 };

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

#ifndef USED_FOR_PLAYER

/******** OpenPageScreen() ********/

BOOL OpenPageScreen(void)
{
TEXT pmenu_spaces[10];
struct Rectangle dclip;
ULONG modes;
int pos;
BOOL query;
Tag ti_Tag;
ULONG ti_Data;

	/**** create string for space between menu items ****/

	if ( CPrefs.PageScreenWidth >= 640 )
		stccpy(pmenu_spaces, (STRPTR)HIRES_INTERWIDTHTEXT, 10);
	else
		stccpy(pmenu_spaces, (STRPTR)LORES_INTERWIDTHTEXT, 10);

	/**** open the screen ****/

	page_textAttr.ta_Name  = (UBYTE *)APPFONT;
	page_textAttr.ta_YSize = 10;
	page_textAttr.ta_Style = FS_NORMAL;
	page_textAttr.ta_Flags = FPF_DESIGNED;

	modes = CPrefs.pageMonitorID;

	if ( CPrefs.PageScreenModes & HAM_KEY )
		modes |= HAM_KEY;

	if ( CPrefs.PageScreenModes & EXTRAHALFBRITE_KEY )
		modes |= EXTRAHALFBRITE_KEY;

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

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		if ( (dclip.MaxX-dclip.MinX+1) > CPrefs.PageScreenWidth )
		{
			pos = ((dclip.MaxX - dclip.MinX + 1) - CPrefs.PageScreenWidth) / 2;
			dclip.MinX += pos;
			dclip.MaxX = dclip.MinX + CPrefs.PageScreenWidth - 1; 
		}
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	pageScreen = OpenScreenTags(		NULL,
																	SA_Depth, 		CPrefs.PageScreenDepth,
																	SA_Font,			&page_textAttr,
																	SA_DisplayID,	modes,
																	SA_ShowTitle,	FALSE,
																	SA_Behind,		TRUE,
																	SA_Pens,			(ULONG)dri_Pens,
																	SA_Type,			CUSTOMSCREEN,
																	SA_Quiet,			TRUE,
																	SA_Title,			(UBYTE *)special_char,
																	ti_Tag,				ti_Data,
																	TAG_END
																);

	UnSetBit(&allocFlags, PAGESCREEN_FLAG);
	if (pageScreen == NULL)
	{
		UA_WarnUser(99);
		return(FALSE);
	}
	ShowTitle(pageScreen, FALSE);
	SetBit(&allocFlags, PAGESCREEN_FLAG);

	GenlockOff(pageScreen);

	/**** fill missing pieces in rendezevous record ****/

	rvrec.pagescreen = pageScreen;

	/**** create the menu bar ****/

	sprintf(pageScreenTitle, "%s%s%s%s%s%s%s%s%s%s%s",
					msgs[Msg_Menu_DA-1], pmenu_spaces,
					msgs[Msg_Menu_File-1], pmenu_spaces,
					msgs[Msg_Menu_Edit-1], pmenu_spaces,
					msgs[Msg_Menu_Font-1], pmenu_spaces,
					msgs[Msg_Menu_PMisc-1], pmenu_spaces,
					msgs[Msg_Menu_Screen-1] );

	pageScreen->Title = (UBYTE *)special_char;

	/**** set font ****/

	SetFont(&(pageScreen->RastPort), smallFont);

	/**** recalc menu widths ****/

	CalculateMenuParams();

	/**** re-allocate (if necessary) the shared bm ****/

	if ( !AllocateSharedBM(CPrefs.PageScreenWidth+16, CPrefs.PageScreenHeight, 8) )
		return(FALSE);

	SetAPen(&(pageScreen->RastPort),0L);
	if ( (((pageScreen->Height-CPrefs.PageScreenHeight)/2)-1) >= 1 )
		RectFill(	&(pageScreen->RastPort),0,0,pageScreen->Width-1,
							((pageScreen->Height-CPrefs.PageScreenHeight)/2)-1 );

	return(TRUE);
}

/******** OpenPageWindow() ********/

BOOL OpenPageWindow(void)
{
	/**** open the window ****/

	NewWindowStructure.LeftEdge 		= 0;
	NewWindowStructure.TopEdge 			= (pageScreen->Height - CPrefs.PageScreenHeight) / 2;
	NewWindowStructure.Width 				= CPrefs.PageScreenWidth;
	NewWindowStructure.Height 			= CPrefs.PageScreenHeight;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 2;
	NewWindowStructure.IDCMPFlags 	= 0;
	NewWindowStructure.Flags				= WFLG_BACKDROP | WFLG_RMBTRAP | WFLG_NOCAREREFRESH |
																		WFLG_SMART_REFRESH | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	=	NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= pageScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	UnSetBit(&allocFlags, PAGEWINDOW_FLAG);

	pageWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (pageWindow == NULL)
	{
		UA_WarnUser(101);
		return(FALSE);
	}

	SetBit(&allocFlags, PAGEWINDOW_FLAG);

	/**** set the communications port up ****/

	pageWindow->UserPort = capsPort;
	ModifyIDCMP(pageWindow, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
													IDCMP_GADGETDOWN | IDCMP_GADGETUP);

	/**** set font ****/

	SetFont(pageWindow->RPort, smallFont);

	/**** set the colors ****/

	SetScreenToCM(pageScreen, CPrefs.PageCM);

#if 0
	/**** re-init window pointer****/
	Forbid();
	process->pr_WindowPtr = (APTR)pageWindow;
	Permit();
#endif

	return(TRUE);
}

/******** OpenScriptScreen() ********/

BOOL OpenScriptScreen(void)
{
TEXT smenu_spaces[10];
struct Rectangle dclip;
ULONG modes;
int pos;//,i;
BOOL query;
Tag ti_Tag;
ULONG ti_Data;

	/**** create string for space between menu items ****/

	stccpy(smenu_spaces, (STRPTR)HIRES_INTERWIDTHTEXT, 10);

	/**** open the screen ****/

	script_textAttr.ta_Name  = (UBYTE *)APPFONT;
	script_textAttr.ta_YSize = 10;
	script_textAttr.ta_Style = FS_NORMAL;
	script_textAttr.ta_Flags = FPF_DESIGNED;

	modes = CPrefs.scriptMonitorID | CPrefs.ScriptScreenModes;

	if ( ModeNotAvailable(modes) )
		modes = CPrefs.scriptMonitorID;

	if ( ModeNotAvailable(modes) )
	{
		if ( CPrefs.PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	query = QueryOverscan(modes, &dclip, OSCAN_TEXT);

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		if ( (dclip.MaxX-dclip.MinX+1) > CPrefs.ScriptScreenWidth )
		{
			pos = ((dclip.MaxX - dclip.MinX + 1) - CPrefs.ScriptScreenWidth) / 2;
			dclip.MinX += pos;
			dclip.MaxX = dclip.MinX + CPrefs.ScriptScreenWidth - 1; 
		}
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	scriptScreen = OpenScreenTags(	NULL,
																	SA_Depth, 		CPrefs.ScriptScreenDepth,
																	SA_Font,			&script_textAttr,
																	SA_DisplayID,	modes,
																	SA_ShowTitle,	FALSE,
																	SA_Behind,		TRUE,
																	SA_Pens,			(ULONG)dri_Pens,
																	SA_Type,			CUSTOMSCREEN,
																	SA_Quiet,			TRUE,
																	SA_Title,			(UBYTE *)special_char,
																	ti_Tag,				ti_Data,
																	TAG_END
																);

	UnSetBit(&allocFlags, SCRIPTSCREEN_FLAG);
	if (scriptScreen == NULL)
	{
		UA_WarnUser(102);
		return(FALSE);
	}
	ShowTitle(scriptScreen, FALSE);
	SetBit(&allocFlags, SCRIPTSCREEN_FLAG);

	GenlockOff(scriptScreen);

	/**** tell the medialink.library where the script screen hangs out ****/

	scriptScreen->UserData = (UBYTE *)SEname_text;

	/**** fill missing pieces in rendez-vous record ****/

	rvrec.scriptscreen = scriptScreen;

	/**** create the menu bar ****/

	sprintf(scriptScreenTitle, "%s%s%s%s%s%s%s%s%s%s%s",
						msgs[Msg_Menu_DA-1], smenu_spaces,
						msgs[Msg_Menu_File-1], smenu_spaces,
						msgs[Msg_Menu_Edit-1], smenu_spaces,
						msgs[Msg_Menu_Xfer-1], smenu_spaces,
						msgs[Msg_Menu_SMisc-1], smenu_spaces,
						msgs[Msg_Menu_Screen-1] );

	scriptScreen->Title = (UBYTE *)special_char;

	/**** set font ****/

	SetFont(&(scriptScreen->RastPort), smallFont);

	/**** recalc menu widths ****/

	CalculateMenuParams();

	SetAPen(&(scriptScreen->RastPort),0L);
	if ( (((scriptScreen->Height-CPrefs.ScriptScreenHeight)/2)-1) >= 1 )
		RectFill(	&(scriptScreen->RastPort),0,0,scriptScreen->Width-1,
							((scriptScreen->Height-CPrefs.ScriptScreenHeight)/2)-1 );

	return(TRUE);
}

/******** OpenScriptWindow() ********/

BOOL OpenScriptWindow(void)
{
	/**** open the window ****/

	NewWindowStructure.LeftEdge 		= 0;
	NewWindowStructure.TopEdge 			= (scriptScreen->Height - CPrefs.ScriptScreenHeight) / 2;
	NewWindowStructure.Width 				= CPrefs.ScriptScreenWidth;
	NewWindowStructure.Height 			= CPrefs.ScriptScreenHeight;
	NewWindowStructure.DetailPen		= 7;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags 	= 0;
	NewWindowStructure.Flags				= WFLG_BACKDROP | WFLG_RMBTRAP | WFLG_NOCAREREFRESH |
																		WFLG_SMART_REFRESH | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	=	NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= scriptScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	UnSetBit(&allocFlags, SCRIPTWINDOW_FLAG);

	scriptWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (scriptWindow == NULL)
	{
		UA_WarnUser(103);
		return(FALSE);
	}

	SetBit(&allocFlags, SCRIPTWINDOW_FLAG);

	/**** set the communications port up ****/

	scriptWindow->UserPort = capsPort;
	ModifyIDCMP(scriptWindow, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
														IDCMP_GADGETDOWN | IDCMP_GADGETUP);

	/**** set the colors ****/

	SetScreenToColorTable4(scriptScreen, &palettes[CPrefs.colorSet*8], 8);

	/**** show how this program is called ****/

	SetFont(scriptWindow->RPort, smallFont);

	SetAPen(scriptWindow->RPort, HI_PEN);
	SetDrMd(scriptWindow->RPort, JAM1);
	Move(scriptWindow->RPort, 298, 8);
	Text(scriptWindow->RPort, msgs[Msg_AppTitle-1], strlen(msgs[Msg_AppTitle-1]));

	return(TRUE);
}

/******** OpenNewPageScreen() ********/

void OpenNewPageScreen(BOOL drawWins, BOOL screenToFront, BOOL openPlay, BOOL optimize)
{
int i;

	if ( openPlay )
	{
		OpenPlayScreen(CPrefs.pageMonitorID);
		PlayScreenToFront();
	}

	ClosePalette();
	CloseSmallScrWdwStuff();

	/**** free most bm's of all windows ****/

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			if ( EditSupportList[i]->restore_bm.Planes[0] )
			{
				FreeFastBitMap( &EditSupportList[i]->restore_bm );
				EditSupportList[i]->restore_w = 0;
				EditSupportList[i]->restore_h = 0;
			}

			if ( EditSupportList[i]->scaled_bm.Planes[0] )
			{
				FreeFastBitMap( &EditSupportList[i]->scaled_bm );
				EditSupportList[i]->scaled_w = 0;
				EditSupportList[i]->scaled_h = 0;
			}

			if ( EditSupportList[i]->remapped_bm.Planes[0] )
			{
				FreeFastBitMap( &EditSupportList[i]->remapped_bm );
				EditSupportList[i]->remapped_w = 0;
				EditSupportList[i]->remapped_h = 0;
			}

			if ( EditSupportList[i]->mask_bm.Planes[0] )
			{
				FreeFastBitMap( &EditSupportList[i]->mask_bm );
				EditSupportList[i]->mask_w = 0;
				EditSupportList[i]->mask_h = 0;
			}
		}
	}

	if ( backES.scaled_bm.Planes[0] )
		FreeFastBitMap( &backES.scaled_bm );
	backES.scaled_w = 0;
	backES.scaled_h = 0;

	if ( backES.remapped_bm.Planes[0] )
		FreeFastBitMap( &backES.remapped_bm );
	backES.remapped_w = 0;
	backES.remapped_h = 0;

	/**** close page window and screen ****/

	if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )
	{
		UA_CloseWindowSafely(pageWindow);
		UnSetBit(&allocFlags, PAGEWINDOW_FLAG);
	}

	if ( TestBit(allocFlags, PAGESCREEN_FLAG) )
	{
		CloseScreen(pageScreen);
		UnSetBit(&allocFlags, PAGESCREEN_FLAG);
	}

	/**** open new screen and window ****/

	if ( !OpenPageScreen() )
		return;

	if ( !OpenPageWindow() )
		return;

	if ( optimize )
		OptimizePalette(TRUE);

	if ( screenToFront )	
	{
		ScreenToFront(pageScreen);
		ActivateWindow(pageWindow);
	}

	SetSpriteOfActWdw(SPRITE_BUSY);

	/**** open new edit windows ****/

	if ( optimize )
		DeleteAllPicsAndBackground();

	if ( backES.ori_bm.Planes[0] )	// there's a background
	{
		backEW.Width = CPrefs.PageScreenWidth;
		backEW.Height = CPrefs.PageScreenHeight;
		if ( drawWins )
			ShowBackground();
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] != NULL)
		{
			CorrectEW(EditWindowList[i]);
			i = OpenEditWindow(i,0,0,0,0);	// -1,-1,-1,-1);
			if (i != -1 && drawWins)
				DrawEditWindow(EditWindowList[i], EditSupportList[i]);
		}
	}

	DrawAllHandles(LEAVE_ACTIVE);

	if ( openPlay )
		ClosePlayScreen();

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** TempClosePageScreen() ********/

void TempClosePageScreen(void)
{
	ClosePalette();
	CloseSmallScrWdwStuff();

	if ( !TestBit(allocFlags, PAGEWINDOW_FLAG) )
		return;

	/**** free as much memory ****/

	FreeSharedBM();

	/**** close page window and screen ****/

	if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )
		UA_CloseWindowSafely(pageWindow);
	pageWindow=NULL;

	if ( TestBit(allocFlags, PAGESCREEN_FLAG) )
		CloseScreen(pageScreen);
	pageScreen=NULL;

	UnSetBit(&allocFlags, PAGEWINDOW_FLAG);
	UnSetBit(&allocFlags, PAGESCREEN_FLAG);

	rvrec.pagescreen = NULL;
}

/******** ReopenPageScreen() ********/

BOOL ReopenPageScreen(void)
{
	/**** open new screen and window ****/

	if ( !OpenPageScreen() )
		return(FALSE);

	if ( !OpenPageWindow() )
		return(FALSE);

	do_New(&pageDoc);

	return(TRUE);
}

/******** TempCloseScriptScreen() ********/

void TempCloseScriptScreen(void)
{
	disabled_gadgets = 0;

	if ( UA_IsGadgetDisabled(&Script_GR[4]) )	// play
		disabled_gadgets |= 1;
	if ( UA_IsGadgetDisabled(&Script_GR[5]) )	// parent
		disabled_gadgets |= 2;
	if ( UA_IsGadgetDisabled(&Script_GR[7]) )	// edit
		disabled_gadgets |= 4;
	if ( UA_IsGadgetDisabled(&Script_GR[8]) )	// show
		disabled_gadgets |= 8;

	UA_EnableButtonQuiet(&Script_GR[4]);
	UA_EnableButtonQuiet(&Script_GR[7]);
	UA_EnableButtonQuiet(&Script_GR[8]);
	UA_EnableButtonQuiet(&Script_GR[5]);

	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
		UA_CloseWindowSafely(scriptWindow);
	scriptWindow=NULL;

	if ( TestBit(allocFlags, SCRIPTSCREEN_FLAG) )
		CloseScreen(scriptScreen);
	scriptScreen=NULL;

	UnSetBit(&allocFlags, SCRIPTWINDOW_FLAG);
	UnSetBit(&allocFlags, SCRIPTSCREEN_FLAG);
}

/******** ReopenScriptScreen() ********/

void ReopenScriptScreen(void)
{
int nodeType;
struct ScriptNodeRecord *this_node;

	/**** open screen and window ****/

	OpenScriptScreen();

	OpenScriptWindow();

	ActivateWindow(scriptWindow);

	DrawScriptScreen();

	if ( !(TestBit(allocFlags,XAPPSLOADED_FLAG)) )
		ReloadXapps();

	DrawDottedLines();

	/**** re-disable gadgets ****/

	if ( disabled_gadgets & 1 )
		UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);
	if ( disabled_gadgets & 2 && CPrefs.userLevel > 2 )
			UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);
	if ( disabled_gadgets & 4 )
		UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);
	if ( disabled_gadgets & 8 )
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);

	disabled_gadgets = 0;

	/**** redraw object list ****/

	if ( scriptDoc.opened )
	{
		DrawObjectList(topEntry1, TRUE, TRUE);
		UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
		FindSelectedIcon(-1);	// reset Alt selecting

		/**** redraw xapp list ****/

		if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] )	// root encountered
			ShowMainEventIcons();
		else
		{
			nodeType = FindParentType(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
			if (nodeType==TALK_STARTSER)
				ShowSerialEventIcons();
			else
				ShowParallelEventIcons();
		}

		/**** print sub branch name ****/

		this_node = FindParentNode(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
		if ( this_node )
			PrintSubBranchName(this_node->objectName);
		else
			PrintSubBranchName(NULL);
	}
	else
	{
		DrawClosedScr();

		UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);	// play
		UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);	// parent
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);	// parent
		if ( CPrefs.userLevel > 2 )
			UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);	// parent

		numEntries1 = 1;
		numDisplay1 = 1;
		topEntry1 = 0;
		UA_InitPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
	}
}

#endif

/******** OpenPlayScreen() ********/

BOOL OpenPlayScreen(ULONG monitorID)
{
int depth, width;//, pos;
//struct Rectangle dclip;
ULONG modes;
//BOOL query;
//Tag ti_Tag;
//ULONG ti_Data;

	width = 640;
	depth = 1;
	modes = monitorID;

	if ( ModeNotAvailable(modes) )
	{
		if ( CPrefs.PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	//query = QueryOverscan(modes, &dclip, OSCAN_TEXT);
#if 0
	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		if ( (dclip.MaxX-dclip.MinX+1) > width )
		{
			pos = ((dclip.MaxX - dclip.MinX + 1) - width) / 2;
			dclip.MinX += pos;
			dclip.MaxX = dclip.MinX + width - 1; 
		}
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}
#endif

	playScreen = OpenScreenTags(		NULL,
																	SA_Depth, 		depth,
																	SA_DisplayID,	modes,
																	SA_ShowTitle,	FALSE,
																	SA_Behind,		TRUE,
																	SA_DetailPen,	0,
																	SA_BlockPen,	2,
																	SA_Type,			CUSTOMSCREEN,
																	SA_Title,			NULL,
																	SA_Quiet,			TRUE,
																	/*ti_Tag,				ti_Data,*/
																	TAG_END
																);
	if ( playScreen==NULL )
		return(FALSE);

#ifndef USED_FOR_PLAYER
	GenlockOff(playScreen);
#endif

#ifndef USED_FOR_PLAYER
	SetScreenToColorTable4(playScreen, &palettes[CPrefs.colorSet*8], 2);	// MP
	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		SetScreenToPartialCM(playScreen, CPrefs.PageCM, 0, 0);	// NEW
#else
	{
	//UWORD cols[3*4] = { 0,0,0, 0,0,0, };
	//SetScreenToColorTable4(playScreen, cols, 2);
		SetRGB4(&(playScreen->ViewPort),  0, 0,0,0);
		SetRGB4(&(playScreen->ViewPort),  1, 0,0,0);
		SetRGB4(&(playScreen->ViewPort), 17, 0,0,0);
		SetRGB4(&(playScreen->ViewPort), 18, 0,0,0);
		SetRGB4(&(playScreen->ViewPort), 19, 0,0,0);
	}
#endif

	/**** open window ****/

	NewWindowStructure.LeftEdge 		= 0;
	NewWindowStructure.TopEdge 			= 0;
	NewWindowStructure.Width 				= playScreen->Width;
	NewWindowStructure.Height 			= playScreen->Height;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 2;
	NewWindowStructure.IDCMPFlags 	= 0;
	NewWindowStructure.Flags				= WFLG_BACKDROP |	WFLG_RMBTRAP | WFLG_NOCAREREFRESH |
																		WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	=	NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= playScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	playWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (playWindow == NULL)
	{
		CloseScreen(playScreen);
		playScreen=NULL;
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** set the communications port up ****/

	playWindow->UserPort = capsPort;
	ModifyIDCMP(playWindow, IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS);

	/**** set font ****/

	SetFont(playWindow->RPort, smallFont);
	SetDrMd(playWindow->RPort,JAM1);

	ShowTitle(playScreen, FALSE);

	if ( emptySprite )
		SetPointer(playWindow,emptySprite,0,16,0,0);

	return(TRUE);
}

/******** ClosePlayScreen() ********/

void ClosePlayScreen(void)
{
	if ( playWindow )
		UA_CloseWindowSafely(playWindow);

	if ( playScreen )
		CloseScreen(playScreen);

	playWindow=NULL;
	playScreen=NULL;
}

/******** PlayScreenToFront() ********/

void PlayScreenToFront(void)
{
	ActivateWindow(playWindow);
	ScreenToFront(playScreen);
	if ( emptySprite )
		SetPointer(playWindow,emptySprite,0,16,0,0);
}

/******** WaitInPlayScreen() ********/

void WaitInPlayScreen(void)
{
struct IntuiMessage *message;
int i,len;

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	len = TextLength(	playWindow->RPort,msgs[Msg_PressAnyKey-1],
										strlen(msgs[Msg_PressAnyKey-1]));
	len = (playWindow->Width-len)/2;

	SetDrMd(playWindow->RPort,JAM1);

	for(i=0; i<3; i++)
	{
		SetAPen(playWindow->RPort,1);
		Move(playWindow->RPort,len,playWindow->Height/2);
		Text(playWindow->RPort,msgs[Msg_PressAnyKey-1],strlen(msgs[Msg_PressAnyKey-1]));
		Delay(4L);
		SetAPen(playWindow->RPort,0);
		Move(playWindow->RPort,len,playWindow->Height/2);
		Text(playWindow->RPort,msgs[Msg_PressAnyKey-1],strlen(msgs[Msg_PressAnyKey-1]));
		Delay(7L);
	}

	{	// wait for any key
	UBYTE key,newkey;
		key = *(UBYTE *)0xbfec01;
		key = -key;
		key >>= 1;
		newkey = key;
		while(newkey==key)
		{
			newkey = *(UBYTE *)0xbfec01;
			newkey = -newkey;
			newkey >>= 1;
		}
	}

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);
}

#ifndef USED_FOR_PLAYER

/******** IsWBThere() ********/

BOOL IsWBThere(void)
{
struct Screen *screen;
BOOL WB=FALSE;

	Forbid();
	screen = IntuitionBase->FirstScreen;
	while(1)
	{
		if (screen && screen->Title!=NULL && !strncmp((UBYTE *)"Workbench",(UBYTE *)screen->Title,9) )
		{
			WB=TRUE;
			break;
		}
		screen = screen->NextScreen;
		if (screen==NULL)
			break;
	}
	Permit();

	return(WB);
}

/******** OpenSmallScreen() ********/

struct Window *OpenSmallScreen(int height)
{
int width, depth, pos, md;
ULONG modes;
struct Rectangle dclip;
struct Screen *screen;
struct Window *window;
BOOL query;
Tag ti_Tag;
ULONG ti_Data;

	UnSetBit(&allocFlags, SPECIALS_WDW_FLAG);
	UnSetBit(&allocFlags, WINDEF_WDW_FLAG);
	UnSetBit(&allocFlags, STYLE_WDW_FLAG);

	if ( !smallWindow )
	{
		/**** choose font ****/

		small_textAttr.ta_Name = (UBYTE *)APPFONT;
		if ( CPrefs.PageScreenModes & LACE )
			small_textAttr.ta_YSize = 20;
		else
			small_textAttr.ta_YSize = 10;
		small_textAttr.ta_Style = FS_NORMAL;
		small_textAttr.ta_Flags = FPF_DESIGNED;

		width = 640;
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
		if ( md==-1 || md<4 )
		{
			if ( CPrefs.PagePalNtsc==PAL_MODE )
				modes = PAL_MONITOR_ID | HIRES;
			else
				modes = NTSC_MONITOR_ID | HIRES;
		}

		if ( CPrefs.PageScreenModes & LACE )
			modes |= LACE;

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

		if ( query )
		{
			ti_Tag = SA_DClip;
			ti_Data = (ULONG)&dclip;

			if ( (dclip.MaxX-dclip.MinX+1) > width )
			{
				pos = ((dclip.MaxX - dclip.MinX + 1) - width) / 2;
				dclip.MinX += pos;
				dclip.MaxX = dclip.MinX + width - 1; 
			}

			dclip.MinY = dclip.MaxY - height + 1;
		}
		else
		{
			ti_Tag = TAG_END;
			ti_Data = 0L;
		}

		screen = OpenScreenTags(	NULL,
															SA_Depth, 		depth,
															SA_Font,			&small_textAttr,
															SA_DisplayID,	modes,
															SA_ShowTitle,	FALSE,
															SA_Behind,		TRUE,
															SA_DetailPen,	0,
															SA_BlockPen,	1,
															SA_Type,			CUSTOMSCREEN,
															SA_Title,			(UBYTE *)special_char,
															SA_Quiet,			TRUE,
															SA_AutoScroll,TRUE,
															ti_Tag,				ti_Data,
															TAG_END
														);
		if (screen == NULL)
			return(NULL);

		GenlockOff(screen);
	
		screen->Title = (UBYTE *)special_char;
	
		/**** open window ****/
	
		NewWindowStructure.LeftEdge			= 0;
		NewWindowStructure.TopEdge			= (screen->Height  - height) / 2;
		NewWindowStructure.Width				= width;
		NewWindowStructure.Height				= height;
		if ( screen->RastPort.BitMap->Depth==1 )
			NewWindowStructure.DetailPen	= 0;
		else
			NewWindowStructure.DetailPen	= 7;
		NewWindowStructure.BlockPen			= 1;
		NewWindowStructure.IDCMPFlags		= 0;
		NewWindowStructure.Flags				= WFLG_BACKDROP |	WFLG_RMBTRAP |
																			WFLG_NOCAREREFRESH | WFLG_BORDERLESS;
		NewWindowStructure.FirstGadget	= NULL;
		NewWindowStructure.CheckMark		= NULL;
		NewWindowStructure.Title				= NULL;
		NewWindowStructure.Screen				= (struct Screen *)screen;
		NewWindowStructure.BitMap				= NULL;
		NewWindowStructure.MinWidth			= 0;
		NewWindowStructure.MinHeight		= 0;
		NewWindowStructure.MaxWidth			= 0;
		NewWindowStructure.MaxHeight		= 0;
		NewWindowStructure.Type					= CUSTOMSCREEN;
	
		window = (struct Window *)OpenWindow(&NewWindowStructure);
		if (window==NULL)
		{
			CloseScreen(screen);
			return(FALSE);
		}
	
		window->UserPort = capsPort;
		ModifyIDCMP(window, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_GADGETDOWN | IDCMP_GADGETUP);
	}
	else
	{
		window = smallWindow;
		screen = smallWindow->WScreen;

		if ( window->FirstGadget )
			RemoveGList(window,window->FirstGadget,-1);
	}
		
	/**** paint the wall ****/

	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, AREA_PEN);
	RectFill(	window->RPort,
						window->BorderLeft, window->BorderTop,
						window->Width  - window->BorderRight-1,
						window->Height - window->BorderBottom-1 );
	WaitBlit();

	/**** set font for this window ****/

	if ( CPrefs.PageScreenModes & LACE )
		SetFont(window->RPort, largeFont);
	else
		SetFont(window->RPort, smallFont);

	/**** set colors ****/

	SetScreenToColorTable4(screen, &palettes[CPrefs.colorSet*8], 8);
	SetScreenToPartialCM(screen, CPrefs.PageCM, 0, 0);	// NEW

	/**** remove title ****/

	ShowTitle(screen, FALSE);

	/**** show the user what we've made ****/

	ActivateWindow(window);

	return(window);
}

/******** CloseSmallScreen() ********/

void CloseSmallScreen(struct Window *window)
{
struct Screen *screen;

	if ( window && window->WScreen )
	{
		screen = window->WScreen;
		MyScreenToBack(screen);
		UA_CloseWindowSafely(window);
		CloseScreen(screen);
		smallWindow = NULL;
	}
}

/******** CloseSmallScrWdwStuff() ********/

void CloseSmallScrWdwStuff(void)
{
	if ( smallWindow )
	{
		if ( TestBit(allocFlags, SPECIALS_WDW_FLAG) )
			CloseSpecialsScreen();
		else if ( TestBit(allocFlags, WINDEF_WDW_FLAG) )
			CloseWinDefScreen();
		else if ( TestBit(allocFlags, STYLE_WDW_FLAG) )
			CloseStyleScreen();
	}
}

/******** GetMaxDepth() ********/

int GetMaxDepth(ULONG ID)
{
struct DimensionInfo diminfo;

	if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
		return( diminfo.MaxDepth );
	else
		return(-1);
}

/******** ModeHasThisWidth() ********/

BOOL ModeHasThisWidth(ULONG ID, WORD w)
{
struct DimensionInfo diminfo;

	if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
	{
		if ( (diminfo.Nominal.MaxX - diminfo.Nominal.MinX + 1) >= w )
			return(TRUE);
	}

	return(FALSE);
}

/******** DeleteAllPicsAndBackground() ********/

void DeleteAllPicsAndBackground(void)
{
int i;

	if ( backES.photoOpts & MOVE_PHOTO || backES.photoOpts & SIZE_PHOTO )
	{
		SetByteBit(&backES.photoOpts, REMAP_PHOTO);

		if ( backES.scaled_bm.Planes[0] )
			FreeFastBitMap( &backES.scaled_bm );
		backES.scaled_w = 0;
		backES.scaled_h = 0;

		if ( backES.remapped_bm.Planes[0] )
			FreeFastBitMap( &backES.remapped_bm );
		backES.remapped_w = 0;
		backES.remapped_h = 0;
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] && EditSupportList[i]->ori_bm.Planes[0] )
		{
			if (	EditSupportList[i]->photoOpts & MOVE_PHOTO ||
						EditSupportList[i]->photoOpts & SIZE_PHOTO )
			{
				SetByteBit(&EditSupportList[i]->photoOpts, REMAP_PHOTO);

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
	}
}

#endif

/******** E O F ********/
