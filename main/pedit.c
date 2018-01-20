#include "nb:pre.h"

/**** defines ****/

#define DO_DONTSAVE 0
#define DO_CANCEL 	1
#define DO_SAVE 		2

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Document pageDoc;
extern struct Document scriptDoc;
extern UWORD chip gui_pattern[];
extern UWORD chip gui_pattern_lace[];
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern struct eventHandlerInfo EHI;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern char *pageCommands[];
extern char *scriptCommands[];
extern struct ObjectInfo ObjectRecord;
extern TEXT *dir_scripts;
extern struct EditWindow **Clipboard_WL;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct List **clipLists;
extern int lastUndoableAction;
extern int lastUndoWindow;
extern UBYTE **msgs;   
extern struct EditSupport backES;
extern TEXT MRO_Script[];
extern TEXT MRO_Page[];
extern struct ScriptNodeRecord *editSNR;
extern BOOL save_as_iff;

/**** gadgets ****/

extern struct GadgetRecord DontSaveRequester_GR[];

/**** functions ****/

/******** do_New() ********/

void do_New(struct Document *doc)
{
	if ( doc->opened && !do_Close(doc,TRUE) )
		return;

	OpenDocumentProc(doc);	// equal for page and script

	stccpy(doc->title, msgs[Msg_Untitled-1], SIZE_FILENAME);

	if (doc==&pageDoc)
	{
		doc->opened = TRUE;
		stccpy(doc->path, CPrefs.document_Path, SIZE_FULLPATH);
	}
	else if (doc==&scriptDoc)
	{
		doc->opened = TRUE;
		//stccpy(doc->path, CPrefs.script_Path, SIZE_FULLPATH);
		stccpy(doc->path, dir_scripts, SIZE_FULLPATH);

		SetSpriteOfActWdw(SPRITE_BUSY);

		if (!ReadScript(dir_scripts, msgs[Msg_Untitled-1], scriptCommands))
		{
			if (CreateEmptyScript())
			{
				if (!ReadScript(dir_scripts, msgs[Msg_Untitled-1], scriptCommands))
					UA_WarnUser(223);
			}
		}
		SetOpenedStateScriptMenuItems();
	
		SetSpriteOfActWdw(SPRITE_NORMAL);		
	}
}

/******** do_Open() ********/

BOOL do_Open(struct Document *doc, BOOL MRO, STRPTR MRO_path)
{
TEXT title[SIZE_FILENAME+10];
TEXT keep1[SIZE_FULLPATH];
TEXT keep2[SIZE_FULLPATH];
BOOL opened=FALSE;

	strcpy(title, doc->title);

	if (doc==&pageDoc)
	{
		if ( MRO )
		{
			UA_SplitFullPath(MRO_path,CPrefs.document_Path,title);
			goto fast1;
		}

		if (OpenAFile(CPrefs.document_Path, title,
									msgs[Msg_SelectADocOrAPic-1], pageWindow,
									DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_THUMBS | DIR_OPT_NOINFO, FALSE ))
		{
fast1:
			strcpy(keep1,CPrefs.document_Path);
			strcpy(keep2,title);

			if ( doc->opened && !do_Close(doc,FALSE) )
				return(opened);

			strcpy(CPrefs.document_Path,keep1);
			strcpy(title,keep2);

			strcpy(doc->title, title);
			stccpy(doc->path, CPrefs.document_Path, SIZE_FULLPATH);
			OpenDocumentProc(doc);
			ReadPage(doc->path, doc->title, pageCommands, TRUE);
			SetOpenedStatePageMenuItems();
			doc->opened = TRUE;
			opened=TRUE;

			UA_MakeFullPath(doc->path, doc->title, keep1);
			AddMRO( MRO_Page, keep1 );
		}
	}
	else if (doc==&scriptDoc)
	{
		if ( MRO )
		{
			UA_SplitFullPath(MRO_path,CPrefs.script_Path,title);
			goto fast2;
		}

		if (OpenAFile(CPrefs.script_Path, title,
									msgs[Msg_SelectAScript-1], scriptWindow,
									DIR_OPT_SCRIPTS | DIR_OPT_NOINFO, FALSE ))
		{	
fast2:
			strcpy(keep1,CPrefs.script_Path);
			strcpy(keep2,title);

			if ( doc->opened && !do_Close(doc,FALSE) )
				return(opened);

			strcpy(CPrefs.script_Path,keep1);
			strcpy(title,keep2);

			SetSpriteOfActWdw(SPRITE_BUSY);

			strcpy(doc->title, title);
			stccpy(doc->path, CPrefs.script_Path, SIZE_FULLPATH);
			OpenDocumentProc(doc);	// equal for page and script
			ReadScript(doc->path, doc->title, scriptCommands);
			SetOpenedStateScriptMenuItems();
			doc->opened = TRUE;
			opened=TRUE;

			UA_MakeFullPath(doc->path, doc->title, keep1);
			AddMRO( MRO_Script, keep1 );

			SetSpriteOfActWdw(SPRITE_NORMAL);
		}
	}

	return(opened);
}

/******** do_Close() ********/

BOOL do_Close(struct Document *doc, BOOL setColors)
{
int retVal;

	if (doc->modified)
		retVal = AskDontSaveCancelSave(doc, msgs[Msg_SaveChanges-1], TRUE);	// equal for page and script
	else
		retVal = DO_DONTSAVE;	// doc is not modified (e.g. just saved)

	if (retVal == DO_DONTSAVE)
	{
		CloseDocumentProc(doc);
		return(TRUE);
	}
	else if (retVal == DO_SAVE)
	{
		if ( SaveDocumentProc(doc,setColors) )	// yes, save and close
		{
			CloseDocumentProc(doc);
			return(TRUE);
		}
	}

	return(FALSE);		// doc not closed
}

/******** do_Save() ********/

void do_Save(struct Document *doc)
{
	if (doc==&pageDoc)
	{
		DrawAllHandles(LEAVE_ACTIVE);

		SetSpriteOfActWdw(SPRITE_BUSY);

		if (WritePage(doc->path, doc->title, pageCommands))
		{
			doc->modified = FALSE;
			if ( editSNR )
			{
				strcpy(editSNR->objectName, doc->title);
				strcpy(editSNR->objectPath, doc->path);
				if ( save_as_iff )
					editSNR->numericalArgs[15] = 2;	// IFF
				else
					editSNR->numericalArgs[15] = 1;	// pagetalk
			}
		}

		SetSpriteOfActWdw(SPRITE_NORMAL);

		DrawAllHandles(LEAVE_ACTIVE);
	}
	else if (doc==&scriptDoc)
	{
		SetSpriteOfActWdw(SPRITE_BUSY);

		if (WriteScript(doc->path, doc->title, &(ObjectRecord.scriptSIR), scriptCommands))
			doc->modified = FALSE;

		SetSpriteOfActWdw(SPRITE_NORMAL);
	}
}

/******** do_SaveAs() ********/

BOOL do_SaveAs(struct Document *doc, BOOL setColors)
{
	return( SaveDocumentProc(doc, setColors) );
}

/******** do_PageSetUp() ********/

void do_PageSetUp(struct Document *doc)
{
	ShowPageSetUp();
	if (doc==&pageDoc)
		ActivateWindow(pageWindow);
	else
		ActivateWindow(scriptWindow);
}

/******** do_Print() ********/

void do_Print(struct Document *doc)
{
	if (doc==&pageDoc)
	{
		ShowPrint(2);
		ActivateWindow(pageWindow);
	}
	else
	{
		ShowPrint(1);
		ActivateWindow(scriptWindow);
	}
}

/******** OpenDocumentProc() ********/

void OpenDocumentProc(struct Document *doc)
{
	if (doc==&pageDoc)
	{
		SetOpenedStatePageMenuItems();
		doc->modified = FALSE;
		DrawOpenedPage();
	}
	else if (doc==&scriptDoc)
	{
		SetOpenedStateScriptMenuItems();
		doc->modified = FALSE;
		DrawOpenedScript();
	}
}

/******** CloseDocumentProc() ********/

void CloseDocumentProc(struct Document *doc)
{
int i;

	if (doc==&pageDoc)
	{
		doc->modified = FALSE;
		doc->opened = FALSE;
		doc->title[0]='\0';

		SetClosedStatePageMenuItems();

		ClosePalette();
		CloseSmallScrWdwStuff();

		lastUndoableAction=0;

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (EditWindowList[i] != NULL)
			{
				CloseEditWindow(EditWindowList[i], EditSupportList[i]);
				EditWindowList[i] = NULL;
				EditSupportList[i] = NULL;
			}
		}

		/**** START -- free back win ****/

		RemovePic24FromWindow(&backES, &backES.ori_bm);
		ClearBitMap24(&backES.ori_bm);
		RemovePicFromWindow(&backES, &backES.scaled_bm);
		ClearBitMap(&backES.scaled_bm);
		RemovePicFromWindow(&backES, &backES.remapped_bm);
		ClearBitMap(&backES.remapped_bm);

		/**** free iff struct ****/

		if ( backES.iff )
			FreeMem(backES.iff, sizeof(struct IFF_FRAME));

		/**** free colormap ****/

		if ( backES.cm )
			FreeColorMap(backES.cm);
		backES.cm = NULL;

		ClearBackWindow();

		/**** END -- free back win ****/

		Move(pageWindow->RPort,0,0);
		WaitTOF();
		SetRast(pageWindow->RPort,0);
		WaitBlit();
	}
	else if (doc==&scriptDoc)
	{
		doc->modified = FALSE;
		doc->opened = FALSE;
		doc->title[0]='\0';

		ExpungeScript();

		SetClosedStateScriptMenuItems();
	}
}

/********	SaveDocumentProc() ********/

BOOL SaveDocumentProc(struct Document *doc, BOOL setColors)
{
TEXT path[SIZE_FULLPATH];

	if (doc==&pageDoc)
	{
		if ( !strcmpi(doc->title,msgs[Msg_Untitled-1]) )
		{
			strcat(doc->title, "_");
			strcat(doc->title, msgs[ Msg_Document-1 ]);
		}

		if (SaveAFile(doc->path, doc->title, msgs[Msg_SaveThisDocAs-1],
									pageWindow,
									DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_THUMBS | DIR_OPT_NOINFO))
		{
			DrawAllHandles(LEAVE_ACTIVE);
			/******* WATCH OUT FOR THAT SNEAKY RETURN() ! ****/

			SetSpriteOfActWdw(SPRITE_BUSY);

			if ( WritePage(doc->path, doc->title, pageCommands) )
			{
				if ( editSNR )
				{
					strcpy(editSNR->objectName, doc->title);
					strcpy(editSNR->objectPath, doc->path);
					if ( save_as_iff )
						editSNR->numericalArgs[15] = 2;	// IFF
					else
						editSNR->numericalArgs[15] = 1;	// pagetalk
				}

				doc->modified = FALSE;
				EnableMenu(page_MR[FILE_MENU], FILE_SAVE);
				DrawAllHandles(LEAVE_ACTIVE);

				SetSpriteOfActWdw(SPRITE_NORMAL);

				UA_MakeFullPath(doc->path, doc->title, path);
				AddMRO( MRO_Page, path );

				return(TRUE);
			}

			SetSpriteOfActWdw(SPRITE_NORMAL);

			DrawAllHandles(LEAVE_ACTIVE);
		}
	}
	else if (doc==&scriptDoc)
	{
		if ( !strcmpi(doc->title,msgs[Msg_Untitled-1]) )
		{
			strcat(doc->title, "_");
			strcat(doc->title, msgs[ Msg_Script-1 ]);
		}

		if (SaveAFile(CPrefs.script_Path, doc->title, msgs[Msg_SaveThisScriptAs-1],
									scriptWindow,
									DIR_OPT_SCRIPTS | DIR_OPT_NOINFO))
		{
			stccpy(doc->path, CPrefs.script_Path, SIZE_FULLPATH);
			/******* WATCH OUT FOR THAT SNEAKY RETURN() ! ****/

			SetSpriteOfActWdw(SPRITE_BUSY);

			if (WriteScript(doc->path, doc->title, &(ObjectRecord.scriptSIR), scriptCommands))
			{
				doc->modified = FALSE;
				EnableMenu(script_MR[FILE_MENU], FILE_SAVE);
				SetSpriteOfActWdw(SPRITE_NORMAL);

				UA_MakeFullPath(doc->path, doc->title, path);
				AddMRO( MRO_Script, path );

				return(TRUE);
			}

			SetSpriteOfActWdw(SPRITE_NORMAL);
		}
	}

	return(FALSE);
}

/******** DrawOpenedPage() ********/

void DrawOpenedPage(void)
{
	SetAPen(pageWindow->RPort, BGND_PEN);
	SetDrMd(pageWindow->RPort, JAM1);
	RectFill(pageWindow->RPort, 0L, 0L, pageWindow->Width-1, pageWindow->Height-1);
	WaitBlit();
}

/******** DrawOpenedScript() ********/

void DrawOpenedScript(void)
{
	InitObjectArea();
}

/******** DrawClosedPage ********/

void DrawClosedPage(void)
{
	SetAPen(pageWindow->RPort, LO_PEN);
	SetBPen(pageWindow->RPort, 0L);
	SetDrMd(pageWindow->RPort, JAM2);

	if ( CPrefs.PageScreenModes & LACE )
	{
		SetAfPt(pageWindow->RPort, gui_pattern_lace, 2);
	}
	else
	{
		SetAfPt(pageWindow->RPort, gui_pattern, 1);
	}

	RectFill(pageWindow->RPort, 0L, 0L, pageWindow->Width-1, pageWindow->Height-1);
	SetAfPt(pageWindow->RPort, NULL, 0);
	SetDrMd(pageWindow->RPort, JAM1);
	WaitBlit();
}

/******** AskDontSaveCancelSave() ********/
/* 0 ==> don't save
 * 1 ==> cancel
 * 2 ==> save
 */

int AskDontSaveCancelSave(struct Document *doc, STRPTR askStr, BOOL cancel)
{
struct Window *window;
BOOL loop=TRUE;
int ID, retval;
TEXT str[512];
WORD ascii;

	/**** open a window ****/

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		window = pageWindow;
	else
		window = scriptWindow;

	window = UA_OpenRequesterWindow(window,DontSaveRequester_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(125);
		return(DO_SAVE); // at least user doesn't lose its doc (hopefully...)
	}

	/**** render gadget ****/

	if ( !cancel )
		DontSaveRequester_GR[1].type = INVISIBLE_GADGET;

	UA_DrawGadgetList(window, DontSaveRequester_GR);

	if ( !cancel )
		UA_DisableButton(window, &DontSaveRequester_GR[1], gui_pattern);	// dont save

	UA_DrawDefaultButton(window,&DontSaveRequester_GR[3]);

	/**** render questionmark ****/

	if ( window->WScreen->ViewPort.Modes & LACE )
		PutImageInRastPort(	GFX_LARGE_QUEST_X, GFX_LARGE_QUEST_Y,
												window->RPort, 10, 5, GFX_LARGE_QUEST_W, GFX_LARGE_QUEST_H);
	else
		PutImageInRastPort(	GFX_SMALL_QUEST_X, GFX_SMALL_QUEST_Y,
												window->RPort, 10, 5, GFX_SMALL_QUEST_W, GFX_SMALL_QUEST_H);

	/**** print question ****/

	if ( msgs[Msg_SaveChanges-1] == askStr )
	{
		if (doc==&pageDoc)
			sprintf(str, askStr, msgs[Msg_Document-1], doc->title);
		else
			sprintf(str, askStr, msgs[Msg_Script-1], doc->title);
	}
	else
		sprintf(str, askStr, doc->title);

	UA_printSeveralLines(	window, &DontSaveRequester_GR[0],
												57, window->RPort->TxBaseline+5,
												DontSaveRequester_GR[0].x2-37,
												DontSaveRequester_GR[0].y2-6, str);

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, DontSaveRequester_GR, &CED);
			if (ID>=1 && ID<=3)
			{
				UA_HiliteButton(window, &DontSaveRequester_GR[ID]);
				loop=FALSE;
				retval=ID-1;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			ascii = RawKeyToASCII(CED.Code);

			if (ascii=='d' || ascii=='n')
			{
				UA_HiliteButton(window, &DontSaveRequester_GR[1]);
				loop=FALSE;
				retval=DO_DONTSAVE;
			}
			else if (CED.Code==RAW_ESCAPE)
			{
				UA_HiliteButton(window, &DontSaveRequester_GR[2]);
				loop=FALSE;
				retval=DO_CANCEL;
			}
			else if (CED.Code==RAW_RETURN)
			{
				UA_HiliteButton(window, &DontSaveRequester_GR[3]);
				loop=FALSE;
				retval=DO_SAVE;
			}
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	DontSaveRequester_GR[1].type = BUTTON_GADGET;

	return(retval);
}

/******** SetClosedStatePageMenuItems() ********/

void SetClosedStatePageMenuItems(void)
{
	/**** project ****/

	DisableMenu(page_MR[FILE_MENU], FILE_CLOSE);
	DisableMenu(page_MR[FILE_MENU], FILE_SAVE);
	DisableMenu(page_MR[FILE_MENU], FILE_SAVEAS);
	DisableMenu(page_MR[FILE_MENU], FILE_PAGESETUP);
	DisableMenu(page_MR[FILE_MENU], FILE_PRINT);

	/**** edit ****/

	DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
	DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
	DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);
	DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
	DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
	DisableMenu(page_MR[EDIT_MENU], EDIT_SELECTALL);
	DisableMenu(page_MR[EDIT_MENU], EDIT_DISTRI);
	DisableMenu(page_MR[EDIT_MENU], EDIT_DUPLI);

	/**** font ****/

	DisableMenu(page_MR[FONT_MENU], FONT_TYPE);
	DisableMenu(page_MR[FONT_MENU], FONT_STYLE);
	DisableMenu(page_MR[FONT_MENU], FONT_COLOR);
	DisableMenu(page_MR[FONT_MENU], FONT_PLAIN);
	DisableMenu(page_MR[FONT_MENU], FONT_BOLD);
	DisableMenu(page_MR[FONT_MENU], FONT_ITALIC);
	DisableMenu(page_MR[FONT_MENU], FONT_UNDERLINE);

	/**** misc ****/

	DisableMenu(page_MR[PMISC_MENU], PMISC_IMPORT);
	DisableMenu(page_MR[PMISC_MENU], PMISC_DEFINE);
	DisableMenu(page_MR[PMISC_MENU], PMISC_PALETTE);
	DisableMenu(page_MR[PMISC_MENU], PMISC_SCREENSIZE);
	DisableMenu(page_MR[PMISC_MENU], PMISC_LINK);
	DisableMenu(page_MR[PMISC_MENU], PMISC_REMAP);
	DisableMenu(page_MR[PMISC_MENU], PMISC_SPECIALS);
	DisableMenu(page_MR[PMISC_MENU], PMISC_TRANSITIONS);
	DisableMenu(page_MR[PMISC_MENU], PMISC_INTERACTIVE);
}

/******** SetOpenedStatePageMenuItems() ********/

void SetOpenedStatePageMenuItems(void)
{
int i;

	/**** project ****/

	EnableMenu(page_MR[FILE_MENU], FILE_CLOSE);

	if ( CPrefs.userLevel>=3 && pageDoc.title[0]!='\0' && stricmp(pageDoc.title,msgs[Msg_Untitled-1]) )
		EnableMenu(page_MR[FILE_MENU], FILE_SAVE);	// multimedia & expert mode and NOT untitled
	else	// all other user levels
		DisableMenu(page_MR[FILE_MENU], FILE_SAVE);

	EnableMenu(page_MR[FILE_MENU], FILE_SAVEAS);
	EnableMenu(page_MR[FILE_MENU], FILE_PAGESETUP);
	EnableMenu(page_MR[FILE_MENU], FILE_PRINT);

	/**** edit ****/

	DisableMenu(page_MR[EDIT_MENU], EDIT_UNDO);
	DisableMenu(page_MR[EDIT_MENU], EDIT_CUT);
	DisableMenu(page_MR[EDIT_MENU], EDIT_COPY);

	DisableMenu(page_MR[EDIT_MENU], EDIT_PASTE);
	if ( Clipboard_WL[0] )
		EnableMenu(page_MR[EDIT_MENU], EDIT_PASTE);

	DisableMenu(page_MR[EDIT_MENU], EDIT_CLEAR);
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] != NULL )	// there are windows
		{
			EnableMenu(page_MR[EDIT_MENU], EDIT_SELECTALL);
			EnableMenu(page_MR[PMISC_MENU], PMISC_TRANSITIONS);
			EnableMenu(page_MR[PMISC_MENU], PMISC_INTERACTIVE);
			break;
		}
	}

	DisableMenu(page_MR[EDIT_MENU], EDIT_DISTRI);
	DisableMenu(page_MR[EDIT_MENU], EDIT_DUPLI);

	/**** font ****/

	DisableMenu(page_MR[FONT_MENU], FONT_TYPE);
	EnableMenu(page_MR[FONT_MENU], FONT_STYLE);
	DisableMenu(page_MR[FONT_MENU], FONT_COLOR);
	DisableMenu(page_MR[FONT_MENU], FONT_PLAIN);
	DisableMenu(page_MR[FONT_MENU], FONT_BOLD);
	DisableMenu(page_MR[FONT_MENU], FONT_ITALIC);
	DisableMenu(page_MR[FONT_MENU], FONT_UNDERLINE);

	/**** misc ****/

	EnableMenu(page_MR[PMISC_MENU], PMISC_IMPORT);
	EnableMenu(page_MR[PMISC_MENU], PMISC_DEFINE);
	EnableMenu(page_MR[PMISC_MENU], PMISC_PALETTE);
	EnableMenu(page_MR[PMISC_MENU], PMISC_SCREENSIZE);

	DisableMenu(page_MR[PMISC_MENU], PMISC_LINK);
	DisableMenu(page_MR[PMISC_MENU], PMISC_REMAP);
	EnableMenu(page_MR[PMISC_MENU], PMISC_SPECIALS);
}

/******** SetClosedStateScriptMenuItems() ********/

void SetClosedStateScriptMenuItems(void)
{
	/**** project ****/

	DisableMenu(script_MR[FILE_MENU], FILE_CLOSE);
	DisableMenu(script_MR[FILE_MENU], FILE_SAVE);
	DisableMenu(script_MR[FILE_MENU], FILE_SAVEAS);
	DisableMenu(script_MR[FILE_MENU], FILE_PAGESETUP);
	DisableMenu(script_MR[FILE_MENU], FILE_PRINT);

	/**** edit ****/

	DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
	DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
	DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
	DisableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
	DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);

	/**** xfer ****/

	DisableMenu(script_MR[XFER_MENU], XFER_UPLOAD);
	//DisableMenu(script_MR[XFER_MENU], XFER_DOWNLOAD);

	/**** misc ****/

	DisableMenu(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	DisableMenu(script_MR[SMISC_MENU], SMISC_LOCALEVENTS);
	DisableMenu(script_MR[SMISC_MENU], SMISC_TWEAKER);
	DisableMenu(script_MR[SMISC_MENU], SMISC_SCRIPTMANAGER);
	DisableMenu(script_MR[SMISC_MENU], SMISC_VARPATH);

	ScriptGadgetsOff();
}

/******** SetOpenedStateScriptMenuItems() ********/

void SetOpenedStateScriptMenuItems(void)
{
int i;

	/**** project ****/

	EnableMenu(script_MR[FILE_MENU], FILE_CLOSE);

	if ( CPrefs.userLevel>=3 && scriptDoc.title[0]!='\0' && stricmp(scriptDoc.title,msgs[Msg_Untitled-1]) )
		EnableMenu(script_MR[FILE_MENU], FILE_SAVE);	// multimedia & expert mode and NOT untitled
	else	// all other user levels
		DisableMenu(script_MR[FILE_MENU], FILE_SAVE);

	EnableMenu(script_MR[FILE_MENU], FILE_SAVEAS);
	EnableMenu(script_MR[FILE_MENU], FILE_PAGESETUP);
	EnableMenu(script_MR[FILE_MENU], FILE_PRINT);

	/**** edit ****/

	DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
	DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
	DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
	DisableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( clipLists[i] != NULL )
		{
			EnableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
			break;
		}
	}
	DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);

	/**** xfer ****/

	EnableMenu(script_MR[XFER_MENU], XFER_UPLOAD);
	//EnableMenu(script_MR[XFER_MENU], XFER_DOWNLOAD);

	/**** smisc ****/

	EnableMenu(script_MR[SMISC_MENU], SMISC_LOCALEVENTS);

	if (	ObjectRecord.scriptSIR.timeCodeFormat!=TIMEFORMAT_HHMMSS ||
				ObjectRecord.scriptSIR.listType == TALK_STARTPAR )
		EnableMenu(script_MR[SMISC_MENU], SMISC_TWEAKER);
	else
		DisableMenu(script_MR[SMISC_MENU], SMISC_TWEAKER);

	EnableMenu(script_MR[SMISC_MENU], SMISC_SCRIPTMANAGER);

	EnableMenu(script_MR[SMISC_MENU], SMISC_VARPATH);

	ScriptGadgetsOn();
}

/******** E O F ********/
