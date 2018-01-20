#include "nb:pre.h"

/**** externals ****/

extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern UWORD palettes[];
extern struct CapsPrefs CPrefs;
extern ULONG prefsPageModes;
extern ULONG prefsScriptModes;
extern struct ObjectInfo ObjectRecord;
extern UWORD chip gui_pattern[];
extern struct Library *medialinkLibBase;
extern struct TextFont *largeFont;
extern struct TextFont *smallFont;
extern UBYTE **msgs;   
extern struct Document pageDoc;
extern struct Document scriptDoc;
extern ULONG languagesAvailable;
extern struct RendezVousRecord rvrec;
extern ULONG allocFlags;
extern struct Locale *Locale;
extern ULONG numEntries1, numDisplay1;
extern ULONG numEntries2, numDisplay2;
extern LONG topEntry1, topEntry2;
extern int xappWdwHeight;
extern struct Gadget ScriptSlider1;
extern struct Gadget ScriptSlider2;
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern TEXT path1[SIZE_FULLPATH];
extern TEXT path2[SIZE_FULLPATH];
extern TEXT path3[SIZE_FULLPATH];
extern TEXT path4[SIZE_FULLPATH];
extern TEXT path5[SIZE_FULLPATH];
extern TEXT path6[SIZE_FULLPATH];
extern TEXT path7[SIZE_FULLPATH];
extern BOOL RA_Installed;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];
extern struct GadgetRecord ColorAdjust_GR[];

/**** functions ****/

/******** ShowPrefs() ********/

void ShowPrefs(void)
{
BOOL retval, wb=FALSE, okClose=FALSE, reopen, optimize=FALSE;
struct CapsPrefs *copy_CPrefs;
WORD w,h,d;
struct Window *activeWindow;
BOOL MeddledWithPaths=FALSE;
TEXT path1b[SIZE_FULLPATH];
TEXT path2b[SIZE_FULLPATH];
TEXT path3b[SIZE_FULLPATH];
TEXT path4b[SIZE_FULLPATH];
TEXT path5b[SIZE_FULLPATH];
TEXT path6b[SIZE_FULLPATH];
TEXT path7b[SIZE_FULLPATH];
struct DisplayInfo dispinfo;

	/**** allocate memory for undo ****/

	copy_CPrefs = (struct CapsPrefs *)AllocMem(sizeof(struct CapsPrefs), MEMF_ANY);
	if (copy_CPrefs==NULL)
	{
		UA_WarnUser(152);
		return;
	}
	CopyMem(&CPrefs, copy_CPrefs, sizeof(struct CapsPrefs));

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		PaletteToBack();

	/**** load prefs coding ****/

	rvrec.aPtr = (APTR)&languagesAvailable;
	rvrec.aPtrTwo = (APTR)&MeddledWithPaths;

	if ( !LoadModule("Prefs", &retval) )
		UA_WarnUser(153);

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( retval )
	{
		/**** check language availability, modify changed if needed ****/

		CheckLanguage(CPrefs.lanCode, copy_CPrefs->lanCode);

		if ( !MeddledWithPaths )
		{
			strcpy(path1b, CPrefs.import_picture_Path);	// copy 'run time' paths
			strcpy(path2b, CPrefs.import_text_Path);
			strcpy(path3b, CPrefs.document_Path);
			strcpy(path4b, CPrefs.script_Path);
			strcpy(path5b, CPrefs.anim_Path);
			strcpy(path6b, CPrefs.music_Path);
			strcpy(path7b, CPrefs.sample_Path);

			strcpy(CPrefs.import_picture_Path, path1);	// copy path as loaded by config.c
			strcpy(CPrefs.import_text_Path,	path2);
			strcpy(CPrefs.document_Path, path3);
			strcpy(CPrefs.script_Path, path4);
			strcpy(CPrefs.anim_Path, path5);
			strcpy(CPrefs.music_Path, path6);
			strcpy(CPrefs.sample_Path, path7);
		}

		/**** START - REOPEN SCRIPT OR PAGE SCREEN ****/

		reopen = FALSE;

		if ( CPrefs.lanCode != copy_CPrefs->lanCode )
			reopen=TRUE;

		if (	EHI.activeScreen == STARTSCREEN_PAGE &&
					CPrefs.pageMonitorID != copy_CPrefs->pageMonitorID )
			reopen=TRUE;

		if (	EHI.activeScreen == STARTSCREEN_SCRIPT &&
					CPrefs.scriptMonitorID != copy_CPrefs->scriptMonitorID )
			reopen=TRUE;

		if ( reopen )
		{
			/**** exchange languages ****/

			MakeLanExt(CPrefs.lanCode,CPrefs.lanExtension);
			UnLoadTranslationFile();
			if ( !TranslateApp(FALSE) )
				UA_WarnUser(-1);

			/**** restore GUI ****/

			SetMenuTitles();
		}

		// START - ADJUST SCRIPT AND THUMB SCREEN DIMENSIONS

		if ( CPrefs.scriptMonitorID != copy_CPrefs->scriptMonitorID )
		{
			if ( GetInfoOnModeID(CPrefs.scriptMonitorID,&w,&h,&d,0) )
			{
				CPrefs.ScriptScreenWidth = w;
				CPrefs.ScriptScreenHeight = h;

				if ( CPrefs.ScriptScreenDepth < 3 )			// preferred depth
					CPrefs.ScriptScreenDepth = 3;
				if ( CPrefs.ScriptScreenDepth > d )
					CPrefs.ScriptScreenDepth = d;

				if ( CPrefs.ThumbnailScreenDepth < 4 )	// preferred depth
					CPrefs.ThumbnailScreenDepth = 4;
				if ( CPrefs.ThumbnailScreenDepth > d )
					CPrefs.ThumbnailScreenDepth = d;

				if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),
																DTAG_DISP,CPrefs.scriptMonitorID) )
				{
					if ( dispinfo.PropertyFlags & DIPF_IS_LACE )
						CPrefs.ScriptScreenModes |= LACE;
					else
						CPrefs.ScriptScreenModes &= ~LACE;
				}

				if ( CPrefs.ScriptScreenHeight >= 400 )
					CPrefs.ScriptScreenModes |= LACE;				

				CPrefs.ThumbnailScreenWidth = CPrefs.ScriptScreenWidth;
				CPrefs.ThumbnailScreenHeight = CPrefs.ScriptScreenHeight;
				CPrefs.ThumbnailScreenModes = CPrefs.ScriptScreenModes;
			}
			else
			{
				CPrefs.scriptMonitorID = copy_CPrefs->scriptMonitorID;
				CPrefs.ScriptScreenModes = copy_CPrefs->ScriptScreenModes;
			}
		}

		// END - ADJUST SCRIPT AND THUMB SCREEN DIMENSIONS

		// START - ADJUST PAGE SCREEN DIMENSIONS

		if ( CPrefs.pageMonitorID != copy_CPrefs->pageMonitorID )
		{
			if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )	// Present screen format requester
			{
				if ( MonitorScreenSize(&optimize,STDCOLORS,TRUE) )
				{
					pageDoc.modified=TRUE;
					InvalidateTable();
				}
				else
				{
					CPrefs.pageMonitorID = copy_CPrefs->pageMonitorID;
					CPrefs.PageScreenModes = copy_CPrefs->PageScreenModes;
				}
			}
			else	// Page monitor changed in Script Editor
			{
				if ( GetInfoOnModeID(CPrefs.pageMonitorID,&w,&h,&d,0) )
				{
					CPrefs.PageScreenWidth = w;
					CPrefs.PageScreenHeight = h;
					if ( CPrefs.PageScreenDepth > d )
						CPrefs.PageScreenDepth = d;

					if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),
																	DTAG_DISP,CPrefs.pageMonitorID) )
					{
						if ( dispinfo.PropertyFlags & DIPF_IS_LACE )
							CPrefs.PageScreenModes |= LACE;
						else
							CPrefs.PageScreenModes &= ~LACE;
					}

					if ( CPrefs.PageScreenHeight >= 400 )
						CPrefs.PageScreenModes |= LACE;				
				}
				else
				{
					CPrefs.pageMonitorID = copy_CPrefs->pageMonitorID;
					CPrefs.PageScreenModes = copy_CPrefs->PageScreenModes;
				}
			}
		}

		// END - ADJUST PAGE SCREEN DIMENSIONS

		if ( reopen )
		{
			if ( EHI.activeScreen == STARTSCREEN_PAGE )
				OpenPlayScreen(CPrefs.pageMonitorID);
			else
				OpenPlayScreen(CPrefs.scriptMonitorID);
			PlayScreenToFront();

			if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
			{
				stopClockTask();
				TempCloseScriptScreen();
				ReopenScriptScreen();
				startClockTask();
				if ( !scriptDoc.opened )
					DrawClosedScr();

				if (CPrefs.ScriptScreenModes & LACE)
					DoubleEffBM(TRUE);
				else
					DoubleEffBM(FALSE);
			}

			if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )
				OpenNewPageScreen(TRUE,FALSE,FALSE,optimize);

			if ( EHI.activeScreen == STARTSCREEN_PAGE )
				ScreenToFront(pageScreen);
			else
				ScreenToFront(scriptScreen);

			SetSpriteOfActWdw(SPRITE_BUSY);

			ClosePlayScreen();
		}

		/**** END - REOPEN SCRIPT OR PAGE SCREEN ****/

		/**** write new config file ****/

		WriteConfigFile();

		if ( !MeddledWithPaths )
		{
			strcpy(CPrefs.import_picture_Path, path1b);	// copy back 'run time' paths
			strcpy(CPrefs.import_text_Path, path2b);
			strcpy(CPrefs.document_Path, path3b);
			strcpy(CPrefs.script_Path, path4b);
			strcpy(CPrefs.anim_Path, path5b);
			strcpy(CPrefs.music_Path, path6b);
			strcpy(CPrefs.sample_Path, path7b);
		}

		/**** do workbench ****/

		wb = IsWBThere();
		if ( wb && !CPrefs.WorkBenchOn )			// WB is around and user doesn't want that
			CloseWorkBench();
		else if ( !wb && CPrefs.WorkBenchOn )	// WB is not around but user wants that
		{
			if ( CPrefs.WorkBenchOn != copy_CPrefs->WorkBenchOn )
			{
				OpenWorkBench();
				Delay(25L);	// show the user that the WB was opened
			}
		}

		/**** bring screens to front ****/

		if ( EHI.activeScreen == STARTSCREEN_PAGE )
		{
			ScreenToFront(pageScreen);
			ActivateWindow(pageWindow);
			activeWindow = pageWindow;
		}
		else
		{
			ScreenToFront(scriptScreen);
			ActivateWindow(scriptWindow);
			activeWindow = scriptWindow;
		}

		SetSpriteOfActWdw(SPRITE_BUSY);

		/**** change user level ****/

		if ( (CPrefs.userLevel < copy_CPrefs->userLevel) && scriptDoc.opened )
		{
			if (	ObjectRecord.scriptSIR.allLists[1]->lh_TailPred ==
						(struct Node *)ObjectRecord.scriptSIR.allLists[1] )
				okClose=TRUE;

			if ( !okClose )
			{
				okClose = UA_OpenGenericWindow(	activeWindow, TRUE, TRUE,
																				msgs[Msg_Ignore-1], msgs[Msg_Close-1],
																				QUESTION_ICON, msgs[Msg_UserLevelConflict-1],
																				FALSE, NULL);
				if ( !okClose )	// UA_Open... returns FALSE if OK to close.
					okClose=TRUE;
				else
					okClose=FALSE;
			}

			if ( okClose )
				CloseDocumentProc(&scriptDoc);
			else
				CPrefs.userLevel = copy_CPrefs->userLevel;		// ignore chosen user level
		}

		if (	(CPrefs.scriptMonitorID != copy_CPrefs->scriptMonitorID) ||
					(CPrefs.userLevel != copy_CPrefs->userLevel) ||
					(CPrefs.lanCode != copy_CPrefs->lanCode) )
			ChangeUserLevel();
	}

	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) && scriptDoc.opened )
	{
		ClearBetweenLines();
		DrawObjectList(-1, TRUE, TRUE);
		topEntry2=0L;
		numEntries2=(LONG)xappWdwHeight;
		numDisplay2=(LONG)Script_GR[2].y2-Script_GR[2].y1-2;
		if ( AbsInt(numDisplay2,numEntries2) < 5 )
			numEntries2 = numDisplay2;
		UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
		if ( reopen )
		{
			topEntry1 = 0;
			ObjectRecord.maxObjects = (Script_GR[0].y2-Script_GR[0].y1)/20;
			numDisplay1 = ObjectRecord.maxObjects;
			UA_InitPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
		}
	}

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		SetScreenToCM(pageScreen,CPrefs.PageCM);

	if ( ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
	{
		if (CPrefs.showDays)
			SetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
		else
			UnsetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
		if ( !scriptDoc.opened )
			DisableMenu(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	}

	if ( CPrefs.lanCode != copy_CPrefs->lanCode )
		HintUserLocale();

	FreeMem(copy_CPrefs, sizeof(struct CapsPrefs));

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** SetScriptUserLevel() ********/

void SetScriptUserLevel(void)
{
	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
	{
		if ( CPrefs.userLevel < 3 )
		{
			Script_GR[5].type = INVISIBLE_GADGET;
			UA_ClearButton(scriptWindow, &Script_GR[5], BGND_PEN);	// parent
		}
		else
		{
			if (CPrefs.ScriptScreenModes & LACE)
				SetFont(scriptWindow->RPort, largeFont);

			Script_GR[5].type = BUTTON_GADGET;
			UA_DrawGadget(scriptWindow, &Script_GR[5]);
			if ( !scriptDoc.opened || ObjectRecord.scriptSIR.allLists[0] == ObjectRecord.objList )
				UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);

			SetFont(scriptWindow->RPort, smallFont);
		}
	}

	/**** menus ****/

	if ( CPrefs.userLevel == 1 )
	{
		page_MR[DA_MENU]->height				= (1*MHEIGHT)+2;
		page_MR[EDIT_MENU]->height			= (6*MHEIGHT)+2;
		page_MR[FONT_MENU]->height			= 0;
		page_MR[PMISC_MENU]->height			= (4*MHEIGHT)+2;
		page_MR[SCREEN_MENU]->height		= (3*MHEIGHT)+2;

		script_MR[DA_MENU]->height			= (1*MHEIGHT)+2;
		script_MR[SMISC_MENU]->height		= 0;
		script_MR[SCREEN_MENU]->height	= (3*MHEIGHT)+2;
	}

	if ( !RA_Installed )
		script_MR[XFER_MENU]->height		= 0;
	else
		script_MR[XFER_MENU]->height		= (1*MHEIGHT)+2;

	if ( CPrefs.userLevel > 1 )
	{
		page_MR[DA_MENU]->height				= (11*MHEIGHT)+2;
		page_MR[EDIT_MENU]->height			= (8*MHEIGHT)+2;
		page_MR[FONT_MENU]->height			= (7*MHEIGHT)+2;
		page_MR[PMISC_MENU]->height			= (9*MHEIGHT)+2;
		page_MR[SCREEN_MENU]->height		= (11*MHEIGHT)+2;

		script_MR[DA_MENU]->height			= (11*MHEIGHT)+2;
		script_MR[SMISC_MENU]->height		= (5*MHEIGHT)+2;
		script_MR[SCREEN_MENU]->height 	= (11*MHEIGHT)+2;
	}

	ReRenderMenus();
}

/******** SetPageUserLevel() ********/

void SetPageUserLevel(void)
{
	if ( CPrefs.userLevel == 1 )
	{
		ColorAdjust_GR[18].type = INVISIBLE_GADGET;	// load
		ColorAdjust_GR[19].type = INVISIBLE_GADGET;	// save
		ColorAdjust_GR[20].type = INVISIBLE_GADGET;	// harmonize
	}
	else
	{
		ColorAdjust_GR[18].type = BUTTON_GADGET;		// load
		ColorAdjust_GR[19].type = BUTTON_GADGET;		// save
		ColorAdjust_GR[20].type = BUTTON_GADGET;		// harmonize
	}
}

/******** ChangeUserLevel() ********/

void ChangeUserLevel(void)
{
	/**** reload xapps ****/

	if (!ReloadXapps())
		UA_WarnUser(154);

	/**** show or remove the parent button ****/

	SetScriptUserLevel();

	SetPageUserLevel();
}

/******** CheckLanguage() ********/

void CheckLanguage(int newLan, int oldLan)
{
int lan;
TEXT lan1[50], lan2[50];

	lan = -1;

	if ( newLan==1 && (languagesAvailable & LAN_English) )
		lan = newLan;	
	else if ( newLan==2 && (languagesAvailable & LAN_Nederlands) )
		lan = newLan;	
	else if ( newLan==3 && (languagesAvailable & LAN_Deutsch) )
		lan = newLan;	
	else if ( newLan==4 && (languagesAvailable & LAN_Français) )
		lan = newLan;	
	else if ( newLan==5 && (languagesAvailable & LAN_Español) )
		lan = newLan;	
	else if ( newLan==6 && (languagesAvailable & LAN_Italiano) )
		lan = newLan;	
	else if ( newLan==7 && (languagesAvailable & LAN_Português) )
		lan = newLan;	
	else if ( newLan==8 && (languagesAvailable & LAN_Dansk) )
		lan = newLan;	
	else if ( newLan==9 && (languagesAvailable & LAN_Svenska) )
		lan = newLan;	
	else if ( newLan==10 && (languagesAvailable & LAN_Norsk) )
		lan = newLan;	

	if ( lan==-1 )
	{
		MakeLanExt(newLan, lan1);
		MakeLanExt(oldLan, lan2);
		Message(msgs[Msg_LanNotAvailable-1], lan1, lan2);
		lan = oldLan;
	}

	CPrefs.lanCode = lan;
}

/******** HintUserLocale() ********/

void HintUserLocale(void)
{
TEXT lanext[50], lanname[50];
int len;

	if ( CPrefs.locale )
	{
		/**** get OS Locale language name ****/

		strcpy(lanname,Locale->loc_LanguageName);
		len = strlen(lanname);
		if ( len > 9 )
			lanname[ len-9 ] = '\0';	// chop off '.language'

		/**** get MP language name ****/

		MakeLanExt(CPrefs.lanCode, lanext);

		if ( strcmpi(lanext, lanname)!=0 )
			Message( msgs[Msg_LocaleHint-1], lanext);
	}
}

/******** E O F ********/
