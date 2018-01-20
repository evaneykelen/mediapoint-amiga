#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Screen *pageScreen;
extern struct Window *pageWindow;
extern struct Window *paletteWindow;
extern struct Screen *scriptScreen;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct MenuRecord **page_MR;
extern struct Screen **DA_Screens;
extern struct EditWindow **Clipboard_WL;
extern struct EditWindow **Undo_WL;
extern int lastUndoableAction;
extern int lastUndoWindow;
extern ULONG allocFlags;
extern struct Process *process;
extern struct Document pageDoc;
extern struct ScriptNodeRecord *editSNR;
extern char *pageCommands[];
extern int HANDLESNIF;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;
extern struct EditWindow prefsEW;
extern struct FER FontEntryRecord;
extern struct ObjectInfo ObjectRecord;
extern TEXT MRO_Script[];
extern TEXT MRO_Page[];
extern struct Window *smallWindow;
extern BOOL save_as_iff;

/**** static globals ****/

static struct BitMap textUndoBM;
static struct RastPort textUndoRP;
static BOOL marqueeDrawn=FALSE;

/**** functions ****/

/******** HandlePageEvents() ********/
/*
 * output: 0 (also 0 if DO_OTHER), QUIT_MEDIALINK
 *
 */

int HandlePageEvents(void)
{
int mode, retVal;
ULONG signals;
struct Window *activeWindow, *panel;
int hitWdw;
WORD ascii;
struct IntuiMessage *message;
TEXT fullPath[SIZE_FULLPATH];
BOOL didASCII;

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	if ( !TestBit(allocFlags, PAGEWINDOW_FLAG) )	// PLS is out
	{
		if ( !ReopenPageScreen() )
			return(QUIT_MEDIALINK);
	}

	DisableMenu(page_MR[SCREEN_MENU], SCREEN_PAGE);
	EnableMenu(page_MR[SCREEN_MENU], SCREEN_SCRIPT);

	if ( editSNR == NULL )
		ScreenToFront(pageScreen);
	ActivateWindow(pageWindow);

	SetSpriteOfActWdw(SPRITE_BUSY);
#if 0
	Forbid();
	process->pr_WindowPtr = (APTR)pageWindow;
	Permit();
#endif
	SetPageUserLevel();

	if ( editSNR )
	{
		if ( editSNR->objectName[0] != '\0' )
		{
			strcpy(pageDoc.title, editSNR->objectName);
			strcpy(pageDoc.path, editSNR->objectPath);
		}
		else
		{
			stccpy(pageDoc.title, msgs[Msg_Untitled-1], SIZE_FILENAME);
			stccpy(pageDoc.path, CPrefs.document_Path, SIZE_FULLPATH);
		}

		if ( editSNR->objectName[0] != '\0' )
		{
			OpenDocumentProc(&pageDoc);
			ReadPage(pageDoc.path, pageDoc.title, pageCommands, FALSE);
		}

		if ( editSNR->objectName[0] != '\0' )
		{
			if ( editSNR->numericalArgs[15]==2 ) // IFF
			{
				stccpy(pageDoc.title, msgs[Msg_Untitled-1], SIZE_FILENAME);
				stccpy(pageDoc.path, CPrefs.document_Path, SIZE_FULLPATH);
			}
			else if ( editSNR->numericalArgs[15]==1 ) // PageTalk
			{
				strcpy(pageDoc.title, editSNR->objectName);
				strcpy(pageDoc.path, editSNR->objectPath);
			}
		}

		SetOpenedStatePageMenuItems();

		pageDoc.opened = TRUE;

		ScreenToFront(pageScreen);
	}

	stopClockTask();
	TempCloseScriptScreen();

	if ( !TestBit(allocFlags,FONTS_SCANNED_FLAG) )
	{
		panel = (struct Window *)UA_OpenMessagePanel(pageWindow, msgs[Msg_GettingFonts-1]);
		if ( panel )
		{
			SetSpriteOfActWdw(SPRITE_BUSY);
			if ( !UA_ScanFontsDir(&FontEntryRecord) )
				UA_WarnUser(218);
			else
				SetBit(&allocFlags, FONTS_SCANNED_FLAG);
		}
		Delay(15L);
		UA_CloseMessagePanel(panel);
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);

	/**** color table ****/

	AllocateTable();

	/**** show user the palette if nothing else in on screen ****/

	if ( NumEditWindows()==0 )
		TogglePaletteOnOff();

	/**** event handler ****/

	mode=0;
	while(mode==0)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(pageWindow,FALSE,NULL);

idcmp_to_examine:

			if (CED.Class == IDCMP_RAWKEY)
				ascii = RawKeyToASCII(CED.Code);

			if (CED.Class == IDCMP_MENUPICK)
			{
				if ( marqueeDrawn )
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}
				mode = CheckPageMenu();
				if ( mode==-1 ) mode=0;
			}
			else if (	( CED.Class == IDCMP_MOUSEBUTTONS ||
								CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
								 && pageDoc.opened )
			{
				activeWindow = GetActiveWindow();
				if (paletteWindow && activeWindow == paletteWindow && CED.Code==SELECTDOWN)
				{
					if ( marqueeDrawn )
					{
						marqueeDrawn = FALSE;
						DrawAllMarquees(1);
					}
					retVal = Monitor_ColorAdjust();
					ActivateWindow(pageWindow);
					if ( retVal == IDCMP_TO_EXAMINE )
						goto idcmp_to_examine;
				}
				else if (	smallWindow && activeWindow==smallWindow && ( CED.Code==SELECTDOWN ||
									CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP ) )
				{
					if ( marqueeDrawn )
					{
						marqueeDrawn = FALSE;
						DrawAllMarquees(1);
					}
					if ( TestBit(allocFlags,SPECIALS_WDW_FLAG) )
						retVal = MonitorSpecials();
					else if ( TestBit(allocFlags,WINDEF_WDW_FLAG) )
						retVal = MonitorWinDef();
					else if ( TestBit(allocFlags,STYLE_WDW_FLAG) )
						retVal = MonitorStyle();
					ActivateWindow(pageWindow);
					if ( retVal == IDCMP_TO_EXAMINE )
						goto idcmp_to_examine;
				}	
				else if (	activeWindow == pageWindow )
				{
					if ( CED.extraClass==DBLCLICKED )
					{
						if ( !CheckSmallFastButtons() )
						{
							if ( marqueeDrawn )
							{
								marqueeDrawn = FALSE;
								DrawAllMarquees(1);
							}
							if ( DetermineClickEvent(&hitWdw,FALSE) == DO_EW_SELECTED )
							{
								if (hitWdw!=-1)
								{
									if ( !(CED.Qualifier&IEQUALIFIER_LSHIFT ||
												CED.Qualifier&IEQUALIFIER_RSHIFT) )
									{
										if ( ProcessTextEdit(hitWdw) )
										{
											if (CED.Class==RAWKEY)
												CED.Class=0L;
											goto idcmp_to_examine;
										}
									}
									else
									{
										if ( EditSupportList[hitWdw]->Active )
										{
											EditSupportList[hitWdw]->Active = FALSE;
											DrawHandles(EditWindowList[hitWdw]->X,
																	EditWindowList[hitWdw]->Y,
																	EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1,
																	EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1);
										}
										else
										{
											EditSupportList[hitWdw]->Active = TRUE;
											DrawHandles(EditWindowList[hitWdw]->X,
																	EditWindowList[hitWdw]->Y,
																	EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1,
																	EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1);
										}
									}	
								}
							}
							else
								doEditWindows();
						}
					}
					else if ( CED.Code==SELECTDOWN )
					{
						doEditWindows();
					}
				}
			}
			else if (CED.Class == IDCMP_RAWKEY && pageDoc.opened )
			{
				if ( marqueeDrawn )
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}

				didASCII=FALSE;
				switch(ascii)
				{
					case 'e':	// specials aka extras
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_SPECIALS] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_SPECIALS;
							mode = CheckPageMenu();
						}
						break;

					case 't':		// transitions
					case 'T':		// transitions (shifted)
					case 0xfe:	// 't' transitions (alted)
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_TRANSITIONS] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_TRANSITIONS;
							mode = CheckPageMenu();
						}
						break;

					case 'y':		// style
						didASCII=TRUE;
						if ( !page_MR[FONT_MENU]->disabled[FONT_STYLE] )
						{
							CED.menuNum = FONT_MENU;
							CED.itemNum = FONT_STYLE;
							mode = CheckPageMenu();
						}
						break;

					case 'i':	// import
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_IMPORT] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_IMPORT;
							mode = CheckPageMenu();
						}
						break;

					case 'o':	// open
						didASCII=TRUE;
						if ( !page_MR[FILE_MENU]->disabled[FILE_OPEN] )
						{
							CED.menuNum = FILE_MENU;
							CED.itemNum = FILE_OPEN;
							mode = CheckPageMenu();
						}
						break;

					case 'k':	// locked state
						didASCII=TRUE;
 						if ( CED.Qualifier&IEQUALIFIER_RCOMMAND )
						{
							InvertLockedState();
							SetPageEditMenuItems();
						}
						break;

					case 'b':	// buttons
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_INTERACTIVE] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_INTERACTIVE;
							mode = CheckPageMenu();
						}
						break;

					case 'p':	// palette
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_PALETTE] )
							TogglePaletteOnOff();
						break;

					case 'd':	// define
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_DEFINE] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_DEFINE;
							mode = CheckPageMenu();
						}
						break;

					case 'u':	// undo
					case 'z':	// undo
						didASCII=TRUE;
						if ( !page_MR[EDIT_MENU]->disabled[EDIT_UNDO] )
						{
							CED.menuNum = EDIT_MENU;
							CED.itemNum = EDIT_UNDO;
							mode = CheckPageMenu();
						}
						break;

					case 'x':	// cut
						didASCII=TRUE;
						if ( !page_MR[EDIT_MENU]->disabled[EDIT_CUT] )
						{
							CED.menuNum = EDIT_MENU;
							CED.itemNum = EDIT_CUT;
							mode = CheckPageMenu();
							lastUndoableAction = 0;
						}
						break;

					case 'c':	// copy
						didASCII=TRUE;
						if ( !page_MR[EDIT_MENU]->disabled[EDIT_COPY] )
						{
							CED.menuNum = EDIT_MENU;
							CED.itemNum = EDIT_COPY;
							mode = CheckPageMenu();
							lastUndoableAction = 0;
						}
						break;

					case 'v':	// paste
						didASCII=TRUE;
						if ( !page_MR[EDIT_MENU]->disabled[EDIT_PASTE] )
						{
							CED.menuNum = EDIT_MENU;
							CED.itemNum = EDIT_PASTE;
							mode = CheckPageMenu();
							lastUndoableAction = 0;
						}
						break;

					case '0':	// prefs
						didASCII=TRUE;
						if ( !page_MR[SCREEN_MENU]->disabled[SCREEN_PREFS] )
						{
							CED.menuNum = SCREEN_MENU;
							CED.itemNum = SCREEN_PREFS;
							mode = CheckPageMenu();
						}
						lastUndoableAction = 0;
						break;

					case 'm':	// screen size
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_SCREENSIZE] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_SCREENSIZE;
							mode = CheckPageMenu();
						}
						lastUndoableAction = 0;
						break;

					case 'h':	// crawl
						didASCII=TRUE;
						if ( !page_MR[PMISC_MENU]->disabled[PMISC_LINK] )
						{
							CED.menuNum = PMISC_MENU;
							CED.itemNum = PMISC_LINK;
							mode = CheckPageMenu();
						}
						lastUndoableAction = 0;
						break;

					case '2':	// goto script
						didASCII=TRUE;
						if ( !page_MR[SCREEN_MENU]->disabled[SCREEN_SCRIPT] )
						{
							CED.menuNum = SCREEN_MENU;
							CED.itemNum = SCREEN_SCRIPT;
							mode = CheckPageMenu();
						}
						break;
				}

				if ( !didASCII )
				{
					switch(CED.Code)
					{
						case 0x00:			// raw ~ ( toggle script <==> page )
							if ( !page_MR[SCREEN_MENU]->disabled[SCREEN_SCRIPT] )
							{
								if (	(CED.Qualifier & IEQUALIFIER_LSHIFT ||
											CED.Qualifier & IEQUALIFIER_RSHIFT) &&
											!page_MR[FILE_MENU]->disabled[FILE_SAVEAS] )
								{
									CED.menuNum = FILE_MENU;
									CED.itemNum = FILE_SAVEAS;
									mode = CheckPageMenu();
								}
								if ( mode != -1 )
								{
									CED.menuNum = SCREEN_MENU;
									CED.itemNum = SCREEN_SCRIPT;
									mode = CheckPageMenu();
								}
								else	
									mode=0;
							}
							break;
	
						//case RAW_HELP:
							//PaletteToBack();
							//HelpWindow();
							//break;					
	
						case RAW_SPACE:
							CycleTruScreens();
							break;
	
						case RAW_F1:		// solid, pattern and transp.
						case RAW_F2:
						case RAW_F3:
							SetDrawMode(CED.Code-RAW_F1);	
							lastUndoableAction = 0;
							SetPageEditMenuItems();
							break;
	
						case RAW_F4:		// saved wdw settings
							if ( CED.Qualifier & IEQUALIFIER_LALT || CED.Qualifier & IEQUALIFIER_RALT )
							{
								if ( NumActiveEditWindows()==1 )
								{
									hitWdw = FirstActiveEditWindow();
									CopyMem(EditWindowList[hitWdw],&prefsEW,sizeof(struct EditWindow));
									WriteConfigFile();
								}
								else
									DisplayBeep(pageScreen);
							}
							else
								SetDrawMode(4);	// prefsEW.
							lastUndoableAction = 0;
							SetPageEditMenuItems();
							break;
	
						case RAW_F5:		// genlock
							if ( CED.Qualifier & IEQUALIFIER_LALT || CED.Qualifier & IEQUALIFIER_RALT )
								ShowBGM();							
							else if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
								GenlockOff(pageScreen);
							else
								GenlockOn(pageScreen);
							break;
	
						case RAW_F7:		// antialias
							SetDrawMode(5);
							lastUndoableAction = 0;
							SetPageEditMenuItems();
							break;
	
						case RAW_F8:		// define
							if ( !page_MR[PMISC_MENU]->disabled[PMISC_DEFINE] )
							{
								CED.menuNum = PMISC_MENU;
								CED.itemNum = PMISC_DEFINE;
								mode = CheckPageMenu();
							}
							lastUndoableAction = 0;
							break;
	
						case RAW_F9:		// scale on/off
							ToggleScaling();	// windef.c
							lastUndoableAction = 0;
							SetPageEditMenuItems();
							break;

						case RAW_F10:		// palette
						case RAW_ESCAPE:
						case RAW_RETURN:
							if ( !page_MR[PMISC_MENU]->disabled[PMISC_PALETTE] )
								TogglePaletteOnOff();
							break;
	
						case RAW_TAB:		// activate next window
							DoWdwTab();
							SetPageEditMenuItems();
							break;
					}
				}
	
				DoWdwMove();
	
				/**** control for margins ****/
	
				if ( CED.Qualifier & IEQUALIFIER_CONTROL )
				{
					if ( !marqueeDrawn )
					{
						marqueeDrawn = TRUE;
						DrawAllHandles(MAKE_INACTIVE);
						DrawAllMarquees(1);
					}
				}
				else if ( marqueeDrawn )					
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}

				/**** backspace ****/

				if ( NumActiveEditWindows()>0 )
				{
					if ( CED.Code==RAW_BACKSPACE )
					{
						if ( marqueeDrawn )
						{
							marqueeDrawn = FALSE;
							DrawAllMarquees(1);
						}
						ClearActiveWindows();
						SetPageEditMenuItems();
						pageDoc.modified=TRUE;
					}
					else if ( CED.Code==RAW_DELETE )
					{
						if ( marqueeDrawn )
						{
							marqueeDrawn = FALSE;
							DrawAllMarquees(1);
						}
						CutActiveWindows();
						SetPageEditMenuItems();
						pageDoc.modified=TRUE;
						lastUndoableAction = 0;
					}
				}
			}
		}
	}

	/**** color table ****/

	ReleaseTable();

	/**** reopen script ****/

	ReopenScriptScreen();
	startClockTask();

	ScreenToFront(scriptScreen);
	TempClosePageScreen();

	if ( editSNR )
	{
		SetByteBit(&editSNR->miscFlags, OBJ_SELECTED);
		SetByteBit(&editSNR->miscFlags, OBJ_NEEDS_REFRESH);
		DrawObjectList(-1, FALSE, TRUE);	// only redraw object name part
		if ( editSNR->objectName[0] != '\0' )
		{
			UA_MakeFullPath(editSNR->objectPath, editSNR->objectName, fullPath);
			ModifyLEInfo(editSNR,fullPath,&(ObjectRecord.scriptSIR));
			//UpdateLEInfo(editSNR, fullPath, &(ObjectRecord.scriptSIR));
		}
	}

	if (mode==DO_OTHER)
		return(0);
	else if (mode==DO_QUIT)
		return(QUIT_MEDIALINK);
	else
		return(0);
}

/******** CheckPageMenu() ********/
/*
 * return 0 (nakko), DO_QUIT, DO_OTHER, -1 (hit cancel during
 * ~, save etc.
 *
 */

int CheckPageMenu(void)
{
int mode=0;
BOOL optimize;

	if ( marqueeDrawn )
	{
		marqueeDrawn = FALSE;
		DrawAllMarquees(1);
	}

	switch(CED.menuNum)
	{
		case DA_MENU:
			switch(CED.itemNum)
			{
				case DA_ABOUT:
					ShowAbout();
					ActivateWindow(pageWindow);
					break;

				case 1: 
				case 2:
				case 3:
				case 4:
				case 5:
					if ( !ExecuteDA(pageWindow, CED.itemNum-1) )
					{
						ScreenToFront(pageScreen);
						ActivateWindow(pageWindow);
					}
					break;

				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					if ( MRO_Page[ (CED.itemNum-6)*SIZE_FULLPATH ] )
						do_Open(&pageDoc,TRUE,&MRO_Page[ (CED.itemNum-6)*SIZE_FULLPATH ]);
					break;
			}
			break;

		case FILE_MENU:
			switch(CED.itemNum)
			{
				case FILE_NEW:
					lastUndoableAction=0;
					PaletteToBack();
					do_New(&pageDoc);
					TogglePaletteOnOff();
					break;

				case FILE_OPEN:
					lastUndoableAction=0;
					do_Open(&pageDoc,FALSE,NULL);
					break;

				case FILE_CLOSE:
					lastUndoableAction=0;
					PaletteToBack();
					if ( do_Close(&pageDoc,TRUE) )
						DrawClosedPage();
					break;

				case FILE_SAVE:
					lastUndoableAction=0;
					do_Save(&pageDoc);
					if ( !save_as_iff && editSNR )
					{
						editSNR->numericalArgs[15] = 1;	// document
						strcpy(editSNR->objectName, pageDoc.title);
						strcpy(editSNR->objectPath, pageDoc.path);

						mode=DO_OTHER;
						SetSpriteOfActWdw(SPRITE_BUSY);
					}
					break;

				case FILE_SAVEAS:
					lastUndoableAction=0;
					if ( !do_SaveAs(&pageDoc,TRUE) )
						mode=-1;
					if ( save_as_iff )
						mode=-1;
					if ( mode!=-1 && editSNR )
					{
						editSNR->numericalArgs[15] = 1;	// document
						strcpy(editSNR->objectName, pageDoc.title);
						strcpy(editSNR->objectPath, pageDoc.path);

						mode=DO_OTHER;
						SetSpriteOfActWdw(SPRITE_BUSY);
					}
					break;

				case FILE_PAGESETUP:
					do_PageSetUp(&pageDoc);
					break;

				case FILE_PRINT:
					do_Print(&pageDoc);
					break;

				case FILE_QUIT:
					PaletteToBack();
					if ( do_Close(&pageDoc,TRUE) )
						mode=DO_QUIT;
					break;
			}
			break;

		case EDIT_MENU:
			switch(CED.itemNum)
			{
				case EDIT_UNDO:
					UndoEditWindowModification();
					break;

				case EDIT_CUT:
					CutActiveWindows();
					lastUndoableAction=0;
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
//						if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
//							CBCutCopy();
					break;

				case EDIT_COPY:
					CopyActiveWindows();
					lastUndoableAction=0;
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
//						if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
//							CBCutCopy();
					break;

				case EDIT_PASTE:
					DrawAllHandles(LEAVE_ACTIVE);
					PasteActiveWindows(0,0);
					DrawAllHandles(LEAVE_ACTIVE);
					lastUndoableAction=0;
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
//						if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
//							CBPaste();
					break;

				case EDIT_CLEAR:
					ClearActiveWindows();
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
					break;
		
				case EDIT_SELECTALL:
					SelectAllWindows();
					SetPageEditMenuItems();
					break;

				case EDIT_DISTRI:
					lastUndoableAction=0;
					PaletteToBack();
					Distributor();
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
					break;

				case EDIT_DUPLI:
					lastUndoableAction=0;
					PaletteToBack();
					Duplicator();
					SetPageEditMenuItems();
					pageDoc.modified=TRUE;
					break;
			}
			break;

		case FONT_MENU:
			switch(CED.itemNum)
			{
				case FONT_STYLE:
					lastUndoableAction=0;
					ToggleStyle();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					break;
			}
			break;

		case PMISC_MENU:
			switch(CED.itemNum)
			{
				case PMISC_IMPORT:
					lastUndoableAction=0;
					PaletteToBack();
					ProcessImport();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					DoSpecialsSettings(TRUE);
					break;

				case PMISC_DEFINE:
					lastUndoableAction=0;
					ToggleWinDef();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					break;

				case PMISC_PALETTE:
					TogglePaletteOnOff();
					break;

				case PMISC_SCREENSIZE:
					lastUndoableAction=0;
					PaletteToBack();
					if (MonitorScreenSize(&optimize,STDCOLORS,FALSE))
						OpenNewPageScreen(TRUE,TRUE,TRUE, optimize);
					pageDoc.modified=TRUE;
					InvalidateTable();
					break;

				case PMISC_LINK:	// crawl
					lastUndoableAction = 0;
					PaletteToBack();
					MonitorCrawl();
					pageDoc.modified=TRUE;
					break;

				case PMISC_REMAP:
					lastUndoableAction = 0;
					RemapAllPics();
					pageDoc.modified=TRUE;
					break;

				case PMISC_SPECIALS:
					lastUndoableAction=0;
					ToggleSpecials();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					break;

				case PMISC_TRANSITIONS:
					lastUndoableAction=0;
					PaletteToBack();
					SetWdwTransitions();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					break;

				case PMISC_INTERACTIVE:
					lastUndoableAction=0;
					PaletteToBack();
					MakeInteractiveButton();
					pageDoc.modified=TRUE;
					SetPageEditMenuItems();
					break;
			}
			break;

		case SCREEN_MENU:
			switch(CED.itemNum)
			{
				case SCREEN_SCRIPT:
					PaletteToBack();
					if ( do_Close(&pageDoc,TRUE) )
					{
						mode=DO_OTHER;
						SetSpriteOfActWdw(SPRITE_BUSY);
					}
					break;

				case SCREEN_PREFS:
					ShowPrefs();
					ActivateWindow(pageWindow);
					SetPageEditMenuItems();
					break;

				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					Fill_DA_Menu(TRUE);
					if ( DA_Screens[CED.itemNum-3] != NULL )
					{
						if ( DA_Screens[CED.itemNum-3]->FirstWindow != NULL )
							ActivateWindow( DA_Screens[CED.itemNum-3]->FirstWindow );
						ScreenToFront( DA_Screens[CED.itemNum-3] );
					}
					break;
			}
			break;
	}

	return(mode);
}

/******** doEditWindows() ********/

void doEditWindows(void)
{
int clickEvent, hitWdw, myWdw, i;
WORD oldX, oldY, oldW, oldH, my;
BOOL doit=TRUE, didIt=FALSE;

	clickEvent = DetermineClickEvent(&hitWdw, TRUE);	// hitWdw may be -1!

	switch(clickEvent)
	{
		case DO_EW_SELECTED:
			if (hitWdw != -1)
			{
				if ( CED.Qualifier & IEQUALIFIER_NUMERICPAD )
				{
					if ( marqueeDrawn && (EditWindowList[hitWdw]->flags & EW_LOCKED) )
					{
						marqueeDrawn = FALSE;
						DrawAllMarquees(1);
					}
					MovePicture(hitWdw);
					doit=FALSE;
				}
			}
			if ( doit )
				CreateEditWindow();	//didIt=CreateEditWindow();
			pageDoc.modified=TRUE;
			break;

		case DO_EW_OPENWDW:
			if ( !CheckSmallFastButtons() )
			{
				if ( marqueeDrawn )
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}
				if ( CED.Qualifier & IEQUALIFIER_CONTROL )
				{
					myWdw = -1;
					if (	CED.Qualifier&IEQUALIFIER_LALT ||
								CED.Qualifier&IEQUALIFIER_RALT )
						i=0;
					else
						i=MAXEDITWINDOWS-1;
					while(1)
					{
						my = MassageY(pageWindow->MouseY);
						if (	EditWindowList[i]!=NULL &&
									(pageWindow->MouseX > EditWindowList[i]->X - HANDLESNIF) &&
									(pageWindow->MouseX < (EditWindowList[i]->X + EditWindowList[i]->Width + HANDLESNIF)) &&
									(my > EditWindowList[i]->Y - HANDLESNIF) &&
									(my < (EditWindowList[i]->Y + EditWindowList[i]->Height + HANDLESNIF)) )
						{
							myWdw=i;
							break;
						}
						if (	CED.Qualifier&IEQUALIFIER_LALT ||
									CED.Qualifier&IEQUALIFIER_RALT )
							i++;
						else
							i--;
						if (i<0 || i==MAXEDITWINDOWS)
							break;
					}
					if (myWdw != -1)
					{
						oldX = EditWindowList[myWdw]->X;
						oldY = EditWindowList[myWdw]->Y;
						oldW = EditWindowList[myWdw]->Width;
						oldH = EditWindowList[myWdw]->Height;
						EditWindowList[myWdw]->X += EditWindowList[myWdw]->LeftMargin;
						EditWindowList[myWdw]->Y += EditWindowList[myWdw]->TopMargin;
						EditWindowList[myWdw]->Width -=
							(EditWindowList[myWdw]->LeftMargin + 
							EditWindowList[myWdw]->RightMargin);
						EditWindowList[myWdw]->Height -=
							(EditWindowList[myWdw]->TopMargin + 
							EditWindowList[myWdw]->BottomMargin);
						CED.Qualifier=0;
						clickEvent = DetermineClickEvent(&hitWdw,FALSE);	// hitWdw may be -1!
						if (hitWdw==-1)
							clickEvent = -1;
						EditWindowList[myWdw]->X			= oldX;
						EditWindowList[myWdw]->Y			= oldY;
						EditWindowList[myWdw]->Width	= oldW;
						EditWindowList[myWdw]->Height	= oldH;
						if (clickEvent>=DO_EW_SIZE1 && clickEvent<=DO_EW_DRAG)
						{
							if (EditSupportList[myWdw]->Active)
							{
								EditSupportList[myWdw]->Active = FALSE;
								DrawHandles(EditWindowList[myWdw]->X,
														EditWindowList[myWdw]->Y,
														EditWindowList[myWdw]->X + EditWindowList[myWdw]->Width - 1,
														EditWindowList[myWdw]->Y + EditWindowList[myWdw]->Height - 1);
							}
							SetMargins(myWdw);
							pageDoc.modified=TRUE;
						}
						else
						{
							if (EditSupportList[myWdw]->Active)
							{
								EditSupportList[myWdw]->Active = FALSE;
								DrawHandles(EditWindowList[myWdw]->X,
														EditWindowList[myWdw]->Y,
														EditWindowList[myWdw]->X + EditWindowList[myWdw]->Width - 1,
														EditWindowList[myWdw]->Y + EditWindowList[myWdw]->Height - 1);
							}
							didIt = CreateEditWindow();
							pageDoc.modified=TRUE;
						}
					}
					else
					{
						didIt = CreateEditWindow();
						pageDoc.modified=TRUE;
					}
				}
				else
				{
					didIt = CreateEditWindow();
					if ( hitWdw!=-1 || (CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT) )
						didIt=TRUE;	// prevent already selected windows from being de-selected
					pageDoc.modified=TRUE;
				}
				if (!didIt)
					DrawAllHandles(MAKE_INACTIVE);
			}
			break;

		case DO_EW_SIZE1:
		case DO_EW_SIZE2:
		case DO_EW_SIZE3:
		case DO_EW_SIZE4:
		case DO_EW_SIZE5:
		case DO_EW_SIZE6:
		case DO_EW_SIZE7:
		case DO_EW_SIZE8:
			if (hitWdw!=-1)
			{
				if ( marqueeDrawn )
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}
				SizeEditWindow(clickEvent, hitWdw);
				pageDoc.modified=TRUE;
			}
			break;
	
		case DO_EW_DRAG:
			if (hitWdw != -1)
			{
				if ( marqueeDrawn )
				{
					marqueeDrawn = FALSE;
					DrawAllMarquees(1);
				}
				DrawAllHandles(LEAVE_ACTIVE);
				DragEditWindow(clickEvent, hitWdw);
				DrawAllHandles(LEAVE_ACTIVE);
				pageDoc.modified=TRUE;
			}
			break;
	}

	SetPageEditMenuItems();
}

/******** SetPageEditMenuItems() ********/

void SetPageEditMenuItems(void)
{
int numActive, i;

	numActive = NumActiveEditWindows();

	/**** undo ****/

	if ( lastUndoableAction != 0 )
		EnableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
	else
		DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);

	/**** cut, copy, clear, dupli, selectall ****/

	if (numActive > 0)
	{
		EnableMenu(page_MR[EDIT_MENU], EDIT_CUT);
		EnableMenu(page_MR[EDIT_MENU], EDIT_COPY);
		EnableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
		EnableMenu(page_MR[EDIT_MENU], EDIT_DUPLI);
	}
	else
	{
		DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
		DisableMenu(page_MR[EDIT_MENU], EDIT_DUPLI);
	}

	/**** distribute ****/

	if (numActive >= 1)
		EnableMenu(page_MR[EDIT_MENU], EDIT_DISTRI);
	else
		DisableMenu(page_MR[EDIT_MENU], EDIT_DISTRI);

	/**** paste ****/

	if ( Clipboard_WL[0] )
		EnableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
	else
		DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);

	/**** menus which need at least 1 window (active or non-active) ****/

	DisableMenu(page_MR[EDIT_MENU], EDIT_SELECTALL);
	DisableMenu(page_MR[PMISC_MENU], PMISC_TRANSITIONS);
	DisableMenu(page_MR[PMISC_MENU], PMISC_INTERACTIVE);
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			EnableMenu(page_MR[EDIT_MENU], EDIT_SELECTALL);
			EnableMenu(page_MR[PMISC_MENU], PMISC_TRANSITIONS);
			EnableMenu(page_MR[PMISC_MENU], PMISC_INTERACTIVE);
			break;
		}
	}

	/**** MISC MENU ****/

	DisableMenu(page_MR[PMISC_MENU], PMISC_REMAP);
	if (numActive > 0)
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (EditSupportList[i] && EditSupportList[i]->Active &&
					EditSupportList[i]->ori_bm.Planes[0] )
			{
				EnableMenu(page_MR[PMISC_MENU], PMISC_REMAP);
				break;
			}
		}
	}

	DisableMenu(page_MR[PMISC_MENU], PMISC_LINK);	// crawl, formerly link
	if (numActive == 1)
		EnableMenu(page_MR[PMISC_MENU], PMISC_LINK);

	if ( TestBit(allocFlags,SPECIALS_WDW_FLAG) )
		DoSpecialsSettings(FALSE);
	else if ( TestBit(allocFlags,WINDEF_WDW_FLAG) )
		DoWinDefSettings();
	else if ( TestBit(allocFlags,STYLE_WDW_FLAG) )
		DoStyleSettings();
}

/******** DrawAllMarquees() ********/
/*
 * mode = 1 --> draw everything
 * mode = 2 --> don't draw lines and numbers
 *
 */
 
void DrawAllMarquees(BYTE mode)
{
int i;
WORD x,y,w,h,hw,hh;
struct RastPort *rp;
TEXT dsn[5];
BOOL depth=FALSE;

	x=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] != NULL )
			x++;

	if ( x>=2 )
		depth=TRUE;

	rp = pageWindow->RPort;
	SetDrMd(rp, JAM1 | COMPLEMENT);
	//SafeSetWriteMask(rp, 0x3);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (EditWindowList[i] != NULL)
		{
			/**** draw 'close' and 'depth arrange' gadgets ****/

			x = EditWindowList[i]->X;
			y = EditWindowList[i]->Y;
			w = EditWindowList[i]->Width;
			h = EditWindowList[i]->Height;

			if (w>=28 && h>=14)
			{
				/**** draw close gadget ****/
				DrawSimpleBox(rp, x+2, y+2, x+12, y+11);
				DrawSimpleBox(rp, x+5, y+5, x+9, y+8);

				/**** draw depth gadget ****/
				if ( depth )
				{
					DrawSimpleBox(rp, x+w-13, y+2, x+w-3, y+11);
					DrawSimpleBox(rp, x+w-11, y+4, x+w-7, y+7);
					Move(rp, x+w-9, y+6);
					Draw(rp, x+w-8, y+6);
					RectFill(rp, x+w-6, y+6, x+w-5, y+9);
					RectFill(rp, x+w-9, y+8, x+w-7, y+9);
				}
			}

			/**** draw margins ****/

			SetDrPt(rp, 0x3333);	// 0011001100110011
			DrawSimpleBox(rp,
										x + EditWindowList[i]->LeftMargin,
										y + EditWindowList[i]->TopMargin,
										x + w - 1 - EditWindowList[i]->RightMargin,
										y + h - 1 - EditWindowList[i]->BottomMargin );
			SetDrPt(rp, 0xffff);

			/**** print DSN ****/
			
			if ( mode!=2 && w>=38 && h>=14 )
			{
				sprintf(dsn,"%d",EditWindowList[i]->DrawSeqNum);
				Move(rp,x+14,y+10);
				Text(rp,dsn,strlen(dsn));
			}

			hw = w/2;
			hh = h/2;
			if (w>=28 && h>=14)
				DrawSimpleBox(rp, x+hw-5, y+hh-5, x+hw+5, y+hh+5);
		}
	}

	/**** draw move vectors ****/

	if ( mode!=2 )
		DrawTrack();

	SetDrMd(rp, JAM1);
	//SafeSetWriteMask(rp, 0xff);

	SetPageEditMenuItems();
	pageDoc.modified=TRUE;
}

/******** DrawSimpleBox() ********/

void DrawSimpleBox(struct RastPort *rp, UWORD x1, UWORD y1, UWORD x2, UWORD y2)
{
	Move(rp, (LONG)x1+1, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y1);
}

/******** SetMargins() ********/

void SetMargins(int hitWdw)
{
ULONG signals;
BOOL loop=TRUE, mouseMoved=FALSE;
WORD start_x, start_y, x1, y1, x2, y2, margin, min, max, x,y,w,h;
struct IntuiMessage *message;
struct Window *aWin;
struct RastPort *rp;

	rp = pageWindow->RPort;

	/**** check which margin where going to set ****/

	start_x	= pageWindow->MouseX;
	start_y	= MassageY(pageWindow->MouseY);

	x1 = EditWindowList[hitWdw]->X;
	y1 = EditWindowList[hitWdw]->Y;
	x2 = x1 + EditWindowList[hitWdw]->Width;
	y2 = y1 + EditWindowList[hitWdw]->Height;

	x1 += EditWindowList[hitWdw]->LeftMargin;
	y1 += EditWindowList[hitWdw]->TopMargin;
	x2 -= EditWindowList[hitWdw]->RightMargin;
	y2 -= EditWindowList[hitWdw]->BottomMargin;

	margin = 0;	// 1=top, 2=right, 3=bottom, 4=left

	if ( start_y > y1-HANDLESNIF && start_y < y1+HANDLESNIF )
		margin = 1;
	else if ( start_x > x2-HANDLESNIF && start_x < x2+HANDLESNIF )
		margin = 2;
	else if (	start_y > y2-HANDLESNIF && start_y < y2+HANDLESNIF )
		margin = 3;
	else if (	start_x > x1-HANDLESNIF && start_x < x1+HANDLESNIF )
		margin = 4;

	if (margin==0)
		return;

	aWin = GetActiEditWin();
	if (aWin!=NULL)
		UA_SwitchMouseMoveOn(aWin);

	if (margin==1)	// top
	{
		x1 = EditWindowList[hitWdw]->X;
		y1 = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->TopMargin;
		x2 = x1 + EditWindowList[hitWdw]->Width - 1;
		y2 = y1;
		min = EditWindowList[hitWdw]->Y;
		max = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1;
	}
	else if (margin==2)	// right
	{
		x1 = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1 - EditWindowList[hitWdw]->RightMargin;
		y1 = EditWindowList[hitWdw]->Y;
		x2 = x1;
		y2 = y1 + EditWindowList[hitWdw]->Height - 1;
		min = EditWindowList[hitWdw]->X;
		max = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1;
	}
	else if (margin==3)	// bottom
	{
		x1 = EditWindowList[hitWdw]->X;
		y1 = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1 - EditWindowList[hitWdw]->BottomMargin;
		x2 = x1 + EditWindowList[hitWdw]->Width - 1;
		y2 = y1;
		min = EditWindowList[hitWdw]->Y;
		max = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1;
	}
	else // left
	{
		x1 = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->LeftMargin;
		y1 = EditWindowList[hitWdw]->Y;
		x2 = x1;
		y2 = y1 + EditWindowList[hitWdw]->Height - 1;
		min = EditWindowList[hitWdw]->X;
		max = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1;
	}

	SetDrMd(rp, JAM1 | COMPLEMENT);
	SetDrPt(rp, 0x3333);	// 0011001100110011
	//SafeSetWriteMask(rp, 0x3);

	x = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->LeftMargin;
	y = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->TopMargin;
	w = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1 - 
			EditWindowList[hitWdw]->RightMargin;
	h = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1 - 
			EditWindowList[hitWdw]->BottomMargin;
	DrawSimpleBox(rp, x, y, w, h);

	Move(rp, x1, y1);
	Draw(rp, x2, y2);	

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class		= message->Class;
				CED.Code		= message->Code;
				CED.MouseX	= pageWindow->MouseX;
				CED.MouseY	= MassageY(pageWindow->MouseY);
				ReplyMsg((struct Message *)message);
				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}
			if (mouseMoved)
			{
				Move(rp, x1, y1);
				Draw(rp, x2, y2);	
				if (margin==1 || margin==3)	// top and bottom
				{
					if ( CED.MouseY < min )
						CED.MouseY = min;
					else if ( CED.MouseY > max )
						CED.MouseY = max;
					y1 = CED.MouseY;
					y2 = y1;
				}
				else
				{
					if ( CED.MouseX < min )
						CED.MouseX = min;
					else if ( CED.MouseX > max )
						CED.MouseX = max;
					x1 = CED.MouseX;
					x2 = x1;
				}
				Move(rp, x1, y1);
				Draw(rp, x2, y2);	
			}
		}
	}

	if (aWin!=NULL)
		UA_SwitchMouseMoveOff(aWin);

	/**** remove line ****/

	Move(rp, x1, y1);
	Draw(rp, x2, y2);	

	/**** use margins ****/

	if (margin==1)	// top
	{
		min = y1;
		max = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height -
					EditWindowList[hitWdw]->BottomMargin;
		if (min>=max-6)
			DisplayBeep(NULL);
		else
			EditWindowList[hitWdw]->TopMargin = y1 - EditWindowList[hitWdw]->Y;
	}
	else if (margin==2)	// right
	{
		min = x1;
		max = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->LeftMargin;
		if (min<=max+6)
			DisplayBeep(NULL);
		else
		{
			EditWindowList[hitWdw]->RightMargin =
				EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width - 1 - x1;
		}
	}
	else if (margin==3)	// bottom
	{
		min = y1;
		max = EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->TopMargin;
		if (min<=max+6)
			DisplayBeep(NULL);
		else
		{
			EditWindowList[hitWdw]->BottomMargin =
				EditWindowList[hitWdw]->Y + EditWindowList[hitWdw]->Height - 1 - y1;
		}
	}
	else if (margin==4)	// left
	{
		min = x1;
		max = EditWindowList[hitWdw]->X + EditWindowList[hitWdw]->Width -
						EditWindowList[hitWdw]->RightMargin;
		if (min>=max-6)
			DisplayBeep(NULL);
		else
			EditWindowList[hitWdw]->LeftMargin = x1 - EditWindowList[hitWdw]->X;
	}

	DrawSimpleBox(rp, x, y, w, h);	// remove old margins
	SetDrPt(rp, 0xffff);

	SetDrMd(rp, JAM1);
	//SafeSetWriteMask(rp, 0xff);

	RedrawAllOverlapWdw(	EditWindowList[hitWdw]->X, EditWindowList[hitWdw]->Y, 
												EditWindowList[hitWdw]->Width, EditWindowList[hitWdw]->Height,
												EditWindowList[hitWdw]->X, EditWindowList[hitWdw]->Y,
												EditWindowList[hitWdw]->Width, EditWindowList[hitWdw]->Height,
												hitWdw, TRUE, TRUE );

	if (!marqueeDrawn)
	{
		marqueeDrawn = TRUE;
		DrawAllMarquees(1);
	}
}

/******** FastCloseEW() ********/

void FastCloseEW(int myWdw)
{
WORD x,y;
struct RastPort *rp;

	rp = pageWindow->RPort;

	/**** hilite close gadget ****/

	x = EditWindowList[myWdw]->X;
	y = EditWindowList[myWdw]->Y;
	SetDrMd(rp, JAM2 | COMPLEMENT);
	RectFill(rp, x+2, y+2, x+12, y+11);
	Delay(4L);
	RectFill(rp, x+2, y+2, x+12, y+11);
	SetDrMd(rp, JAM1);

	if ( marqueeDrawn )
		DrawAllMarquees(1);

	EditSupportList[myWdw]->Active=TRUE;
	DrawAllHandles(LEAVE_ACTIVE);
	ClearActiveWindows();

	if ( marqueeDrawn )
		DrawAllMarquees(1);
}

/******** FastDepthEW() ********/

void FastDepthEW(int myWdw)
{
int dsn,i,slot;
WORD x,y,w,h;
struct RastPort *rp;

	rp = pageWindow->RPort;

	/**** hilite depth gadget ****/

	x = EditWindowList[myWdw]->X;
	y = EditWindowList[myWdw]->Y;
	w = EditWindowList[myWdw]->Width;
	h = EditWindowList[myWdw]->Height;
	SetDrMd(rp, JAM2 | COMPLEMENT);
	RectFill(rp, x+w-13, y+2, x+w-3, y+11);
	Delay(4L);
	RectFill(rp, x+w-13, y+2, x+w-3, y+11);
	SetDrMd(rp, JAM1);
	
	if ( marqueeDrawn )
		DrawAllMarquees(1);

	dsn = GetNewDrawSeqNum();

	if ( EditWindowList[myWdw]->DrawSeqNum==dsn-1 )	// this is the top window
	{																								// move it to the bottom.
		/*
		 *	1		2  
		 *	2		3
		 * >3		1
		 *	4		4
		 *	5		5
		 */

		for(i=0; i<MAXEDITWINDOWS; i++)
			if (	EditWindowList[i] && i!=myWdw &&
						EditWindowList[i]->DrawSeqNum < EditWindowList[myWdw]->DrawSeqNum )
				EditWindowList[i]->DrawSeqNum++;
		EditWindowList[myWdw]->DrawSeqNum = 1;
	}
	else	// this is *not* the top window, move it to the top
	{
		/*
		 *	1		1  
		 *	2		2
		 * >3		3
		 *	4		4
		 *	5		5
		 */

		slot=1;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && i!=myWdw )
			{
				EditWindowList[i]->DrawSeqNum = slot;
				slot++;
			}
		}
		EditWindowList[myWdw]->DrawSeqNum = dsn-1;
	}

	for(i=MAXEDITWINDOWS-1; i>=0; i--)
		if ( EditWindowList[i] )
			RestoreBack( EditWindowList[i], EditSupportList[i] );

	SortEditWindowLists(0);

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] )
			DrawEditWindow( EditWindowList[i], EditSupportList[i] );

	if ( marqueeDrawn )
		DrawAllMarquees(1);
}

/******** CheckSmallFastButtons() ********/

BOOL CheckSmallFastButtons(void)
{
int i;
WORD x,y,w,h,hw,hh,my;
BOOL depth=FALSE;

	if ( !(CED.Qualifier & IEQUALIFIER_CONTROL) )
		return(FALSE);

	x=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] != NULL )
			x++;

	if ( x>=2 )
		depth=TRUE;	// 2 or more windows, depth is possible

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] != NULL )
		{
			x = EditWindowList[i]->X;
			y = EditWindowList[i]->Y;
			w = EditWindowList[i]->Width;
			h = EditWindowList[i]->Height;
			my = MassageY(pageWindow->MouseY);

			if (	pageWindow->MouseX > x && pageWindow->MouseX < (x+13) &&
						my > y && my < (y+12) )
			{
				FastCloseEW(i);
				return(TRUE);
			}

			if ( depth )
			{
				if (	pageWindow->MouseX > (x+w-14) && pageWindow->MouseX < (x+w) &&
							my > y && my < (y+12) )
				{
					FastDepthEW(i);
					return(TRUE);
				}

				hw = w/2;
				hh = h/2;
				if (	pageWindow->MouseX > (x+hw-5) && pageWindow->MouseX < (x+hw+5) &&
							my > (y+hh-5) && my < (y+hh+5) )
				{
					ChangeWindowStacking(i);
					return(TRUE);
				}
			}
		}
	}

	return(FALSE);
}

/******** ChangeWindowStacking() ********/

void ChangeWindowStacking(int wdw)
{
ULONG signals;
BOOL loop=TRUE;
struct IntuiMessage *message;
WORD x,y,w,h,hw,hh;
int dsn, hitWdw, i;
BOOL drewTrack=FALSE;
USHORT Qualifier;
UWORD undo_DrawSeqNums[MAXEDITWINDOWS];
struct RastPort *rp;

	rp = pageWindow->RPort;

	/**** hilite stack gadget ****/

	x = EditWindowList[wdw]->X;
	y = EditWindowList[wdw]->Y;
	w = EditWindowList[wdw]->Width;
	h = EditWindowList[wdw]->Height;
	hw = w/2;
	hh = h/2;
	SetDrMd(rp, JAM2 | COMPLEMENT);
	RectFill(rp, x+hw-5, y+hh-5, x+hw+5, y+hh+5);
	Delay(4L);
	RectFill(rp, x+hw-5, y+hh-5, x+hw+5, y+hh+5);
	SetDrMd(rp, JAM1);

	if ( marqueeDrawn )
	{
		marqueeDrawn = FALSE;
		DrawAllMarquees(1);
	}

	DrawAllMarquees(2);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			undo_DrawSeqNums[i] = EditWindowList[i]->DrawSeqNum;
			EditWindowList[i]->DrawSeqNum = 999;	// magic number
		}
	}

	EditWindowList[wdw]->DrawSeqNum = 1; 
	dsn = 2;

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			if ( !(CED.Qualifier & IEQUALIFIER_CONTROL) )
				loop=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class			= message->Class;
				CED.Code			= message->Code;
				CED.MouseX		= message->MouseX;
				CED.MouseY		= MassageY(message->MouseY);
				CED.Qualifier	= message->Qualifier;
				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTDOWN)
						{
							Qualifier = CED.Qualifier;
							CED.Qualifier = 0;
							DetermineClickEvent(&hitWdw,FALSE);
							CED.Qualifier = Qualifier;
							if (hitWdw!=-1)
							{
								/**** hilite stack gadget ****/
								x = EditWindowList[hitWdw]->X;
								y = EditWindowList[hitWdw]->Y;
								w = EditWindowList[hitWdw]->Width;
								h = EditWindowList[hitWdw]->Height;
								hw = w/2;
								hh = h/2;
								SetDrMd(rp, JAM2 | COMPLEMENT);
								RectFill(rp, x+hw-5, y+hh-5, x+hw+5, y+hh+5);
								Delay(4L);
								RectFill(rp, x+hw-5, y+hh-5, x+hw+5, y+hh+5);
								SetDrMd(rp, JAM1);
								if ( EditWindowList[hitWdw]->DrawSeqNum > (dsn-1) )	// last clicked
								{
									drewTrack=TRUE;
									DrawTrack();
									EditWindowList[hitWdw]->DrawSeqNum = dsn;
									dsn++;
									DrawTrack();
								}
							}
						}
						break;
					case IDCMP_RAWKEY:
						loop=FALSE;
						break;
				}
			}
		}
	}

	DrawAllMarquees(2);

	if (drewTrack)
	{
		DrawTrack();

		/**** get highest DSN ****/

		dsn=0;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (	EditWindowList[i] &&
						EditWindowList[i]->DrawSeqNum != 999 &&
						EditWindowList[i]->DrawSeqNum	> dsn )
				dsn = EditWindowList[i]->DrawSeqNum;
		}

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && EditWindowList[i]->DrawSeqNum == 999 )
			{
				EditWindowList[i]->DrawSeqNum = dsn+1;
				dsn++;
			}
		}

		for(i=MAXEDITWINDOWS-1; i>=0; i--)
			if ( EditWindowList[i] )
				RestoreBack( EditWindowList[i], EditSupportList[i] );

		SortEditWindowLists(0);

		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] )
				DrawEditWindow( EditWindowList[i], EditSupportList[i] );
	}
	else
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
			if ( EditWindowList[i] )
				EditWindowList[i]->DrawSeqNum = undo_DrawSeqNums[i];
	}
}

/******* DrawTrack() ********/

void DrawTrack(void)
{
int i,j,k;
BYTE movedraw=1;
struct RastPort *rp;
WORD x,y;

	rp = pageWindow->RPort;
	SetDrMd(rp, JAM1 | COMPLEMENT);
	SetDrPt(rp, 0xffff);	//0x3333);	// 0011001100110011
	//SafeSetWriteMask(rp, 0x3);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		for(j=0; j<MAXEDITWINDOWS; j++)
		{
			if (	EditWindowList[j] &&
						EditWindowList[j]->DrawSeqNum==(i+1) &&
						EditWindowList[j]->DrawSeqNum!=999 )
			{
				if ( movedraw==1 )
				{
					movedraw=2;
					Move(	rp,
								EditWindowList[j]->X+EditWindowList[j]->Width/2,
								EditWindowList[j]->Y+EditWindowList[j]->Height/2 );
					x = EditWindowList[j]->X+EditWindowList[j]->Width/2;
					y = EditWindowList[j]->Y+EditWindowList[j]->Height/2;
				}
				else
				{
					for(k=0;k<4;k++)
					{
						Move(rp,x+k,y+k);
						Draw(	rp,
									EditWindowList[j]->X+EditWindowList[j]->Width/2+k,
									EditWindowList[j]->Y+EditWindowList[j]->Height/2+k );
					}
					x = EditWindowList[j]->X+EditWindowList[j]->Width/2;
					y = EditWindowList[j]->Y+EditWindowList[j]->Height/2;
				}
				break;
			}
		}
	}

	SetDrPt(rp, 0xffff);
	SetDrMd(rp, JAM1);
	//SafeSetWriteMask(rp, 0xff);
}

/******** DoWdwMove() ********/

void DoWdwMove(void)
{
WORD x,y,w,h,dx,dy;
int i;
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
BOOL list[MAXEDITWINDOWS];
struct RasInfo *RI;
BOOL go_on=TRUE;

	if ( IntuitionBase->FirstScreen != pageScreen )
	{
		if ( CED.Code==RAW_CURSORUP || CED.Code==RAW_CURSORDOWN )
		{
			RI = pageScreen->ViewPort.RasInfo;
			if ( CED.Code==RAW_CURSORUP )
				RI->RyOffset = 0;
			else if ( CED.Code==RAW_CURSORDOWN )
				RI->RyOffset = IntuitionBase->FirstScreen->Height;
			//ScrollVPort( &pageScreen->ViewPort );
			MakeScreen( pageScreen);
			RethinkDisplay();
			go_on=FALSE;
		}
	}

	if (go_on)
	{
		PrepareRedrawAll(prevX,prevY,prevW,prevH,newX,newY,newW,newH,list);

		dx = 0;
		dy = 0;

		switch(CED.Code)
		{
			case RAW_CURSORLEFT:	CED.Code=0; dx = -1; break;
			case RAW_CURSORRIGHT:	CED.Code=0; dx =  1; break;
			case RAW_CURSORUP:		CED.Code=0; dy = -1; break;
			case RAW_CURSORDOWN:	CED.Code=0; dy =  1; break;
		}

		if ( dx!=0 || dy!=0 )
		{
			DrawAllHandles(LEAVE_ACTIVE);

			if ( (CED.Qualifier&IEQUALIFIER_LSHIFT) || (CED.Qualifier&IEQUALIFIER_RSHIFT) )
			{
				dx *= 30;
				dy *= 30;
			}

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] && EditSupportList[i]->Active )
				{
					x = EditWindowList[i]->X;
					y = EditWindowList[i]->Y;
					w = EditWindowList[i]->Width;
					h = EditWindowList[i]->Height;
					x += dx;
					y += dy;
	
					if (x < 0)
						x = 0;
					if (y < 0)
						y = 0;
					if ( (x+w) >= CPrefs.PageScreenWidth )
						x = CPrefs.PageScreenWidth-w;
					if ( (y+h) >= CPrefs.PageScreenHeight )
						y = CPrefs.PageScreenHeight-h;

					list[i] = TRUE;

					prevX[i] = EditWindowList[i]->X;
					prevY[i] = EditWindowList[i]->Y;
					prevW[i] = EditWindowList[i]->Width;
					prevH[i] = EditWindowList[i]->Height;

					newX[i] = x;
					newY[i] = y;
					newW[i] = w;
					newH[i] = h;
				}
			}

			RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);

			DrawAllHandles(LEAVE_ACTIVE);
		}
	}
}

/******** DoWdwTab() ********/

void DoWdwTab(void)
{
int i,win=-1,num=0;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			num++;
			if ( win==-1 && EditSupportList[i]->Active )
				win=i;
		}
	}

	DrawAllHandles(MAKE_INACTIVE);

	if ( win==-1 || num==1 )	// make first found active
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] )
			{
				EditSupportList[i]->Active = TRUE;
				DrawAllHandles(LEAVE_ACTIVE);
				return;
			}
		}
	}

	if ( (CED.Qualifier&IEQUALIFIER_LSHIFT) || (CED.Qualifier&IEQUALIFIER_RSHIFT) )
	{
		// cycle back

		for(i=MAXEDITWINDOWS-1; i>=0; i--)
		{
			if ( EditWindowList[i] && i<win )
			{
				EditSupportList[i]->Active = TRUE;
				DrawAllHandles(LEAVE_ACTIVE);
				return;
			}
		}

		/**** no window found lower ****/
		for(i=MAXEDITWINDOWS-1; i>=0; i--)
		{
			if ( EditWindowList[i] )
			{
				EditSupportList[i]->Active = TRUE;
				DrawAllHandles(LEAVE_ACTIVE);
				return;
			}
		}
	}
	else
	{
		// cycle forward

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && i>win )
			{
				EditSupportList[i]->Active = TRUE;
				DrawAllHandles(LEAVE_ACTIVE);
				return;
			}
		}

		/**** no window found higher ****/
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] )
			{
				EditSupportList[i]->Active = TRUE;
				DrawAllHandles(LEAVE_ACTIVE);
				return;
			}
		}
	}
}

/******** CycleTruScreens() ********/

void CycleTruScreens(void)
{
int next=0;

	if ( IntuitionBase->FirstScreen == pageScreen )
		next=1;	// next should be palette
	else if ( !paletteWindow || IntuitionBase->FirstScreen == paletteWindow->WScreen )
		next=2;	// next should be specials/extras
	else if ( !smallWindow || ((IntuitionBase->FirstScreen == smallWindow->WScreen) &&
						(allocFlags & SPECIALS_WDW_FLAG)) )
		next=3;	// next should be define
	else if ( !smallWindow || ((IntuitionBase->FirstScreen == smallWindow->WScreen) &&
						(allocFlags & WINDEF_WDW_FLAG)) )
		next=4;	// next should be style
	else if ( !smallWindow || ((IntuitionBase->FirstScreen == smallWindow->WScreen) &&
						(allocFlags & STYLE_WDW_FLAG)) )
		next=5;	// next should be only the page editor

	switch(next)
	{
		case 1:
			TogglePaletteOnOff();
			break;
		case 2:
			ToggleSpecials();
			break;
		case 3:
			ToggleWinDef();
			break;
		case 4:
		case 5:
			ToggleStyle();
			break;
	}

	lastUndoableAction=0;
	pageDoc.modified=TRUE;
	SetPageEditMenuItems();

	if (next==5)
		ScreenAtNormalPos();
}

/******** ScreenAtNormalPos() ********/

void ScreenAtNormalPos(void)
{
struct RasInfo *RI;

	if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )
	{
		RI = pageScreen->ViewPort.RasInfo;
		RI->RyOffset = 0;
		//ScrollVPort( &pageScreen->ViewPort );
		MakeScreen( pageScreen);
		RethinkDisplay();
	}
}

/******** MassageY() ********/

int MassageY(int y)
{
struct RasInfo *RI;

	if ( TestBit(allocFlags,PAGEWINDOW_FLAG) )
	{
		RI = pageScreen->ViewPort.RasInfo;
		return( y + RI->RyOffset );
	}
	return(y);
}

/******** MyScreenToBack() ********/

void MyScreenToBack(struct Screen *screen)
{
struct RasInfo *RI;

	if ( screen )
	{
		if ( TestBit(allocFlags, PAGESCREEN_FLAG) )
		{
			RI = screen->ViewPort.RasInfo;
			RI->RyOffset = 0;
			MakeScreen(pageScreen);
			RethinkDisplay();
		}
		ScreenToBack(screen);
	}
}

/******** E O F ********/
