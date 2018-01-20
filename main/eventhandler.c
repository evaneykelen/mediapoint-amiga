#include "nb:pre.h"

/**** externals ****/

extern ULONG allocFlags;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Screen *pageScreen;
extern struct Window *pageWindow;
extern struct Screen *scriptScreen;
extern struct Window *scriptWindow;
extern struct Document pageDoc;
extern struct Document scriptDoc;
extern struct GadgetRecord *kept_Script_GR;
extern ULONG kept_S_size;
extern struct Library *medialinkLibBase;
extern struct EditSupport backES;
extern UBYTE **msgs;   

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** HandleEvents() ********/

void HandleEvents(struct Window *spriteWindow)
{
int result=0;
TEXT volume[SIZE_PATH];
LONG size;

	EHI.activeScreen		= 0;
	//EHI.paletteVisible	= FALSE;
	EHI.thumbsVisible		= FALSE;

	pageDoc.title[0]		= '\0';
	pageDoc.modified		= FALSE;
	pageDoc.untitled		= TRUE;			// OBSOLETE?
	pageDoc.opened			= FALSE;
	pageDoc.path[0]			= '\0';

	scriptDoc.title[0]	= '\0';
	scriptDoc.modified	= FALSE;
	scriptDoc.untitled	= TRUE;			// OBSOLETE?
	scriptDoc.opened		= FALSE;
	scriptDoc.path[0]		= '\0';

	CopyMem(Script_GR, kept_Script_GR, kept_S_size);

	/**** open GUI ****/

	if ( OpenScriptScreen() )
	{
		if ( OpenScriptWindow() )
		{
			DrawScriptScreen();		// used to be part of OpenScriptWindow until
														// we closed/opened it after playing a script
			DrawScriptScreen_2();	// call functions that used to be part of DrawScriptScreen()
														// they we're separated because the script screen is now
														// opened & closed by prefs.

			/**** init page menus ****/

			InitPageEditMenus();

			/**** init script menus ****/

			InitScriptEditMenus();
			do_New(&scriptDoc);

			/**** init script screen ****/

			SetScriptUserLevel();

			if ( CPrefs.SystemTwo )
			{
				if ( !OpenMagicBroker() )
					UA_WarnUser(-1);
			}

			startClockTask();

			DrawObjectList(0, TRUE, TRUE);

			/**** init page screen ****/

			ClearBackWindow();	

			/**** suck the Transitions XaPP ****/

			if ( !GetInfoFromPageXaPP() )
				FreeAndExit();

			/**** preload picts dir ****/

			OpenDir(CPrefs.import_picture_Path, DIR_OPT_ILBM | DIR_OPT_THUMBS | DIR_OPT_NOINFO);
			CloseDir();	// doesn't close buffer

			/**** close the workbench ****/

			if ( !CPrefs.WorkBenchOn )
				CloseWorkBench();

			/**** warn user ****/

			if ( GetCurrentDirName(volume,SIZE_PATH-1) )
			{
				size = GetVolumeSize(volume);
				if ( size < 1024000 )
					Message(msgs[Msg_FreeOnDisk-1]);
			}

			/**** handle events ****/

			while(1)
			{
				EHI.activeScreen = STARTSCREEN_SCRIPT;
				result = HandleScriptEvents();
				if ( result==QUIT_MEDIALINK )
					break;

				EHI.activeScreen = STARTSCREEN_PAGE;
				result = HandlePageEvents();
				EHI.activeScreen = STARTSCREEN_SCRIPT;
				if (	result==QUIT_MEDIALINK && scriptDoc.opened &&
							do_Close(&scriptDoc,FALSE) )	// extra confirm!
					break;
				if ( result==QUIT_MEDIALINK && !scriptDoc.opened )
					break;
			}
		}
	}

	/**** leave the show ****/

	WBenchToFront();

	stopClockTask();

	if ( CPrefs.SystemTwo )
		CloseMagicBroker();

	CloseAllEditWindows();

	/**** START -- free back win ****/

	RemovePic24FromWindow(&backES, &backES.ori_bm);
	RemovePicFromWindow(&backES, &backES.scaled_bm);
	RemovePicFromWindow(&backES, &backES.remapped_bm);

	/**** free iff struct ****/

	if ( backES.iff )
		FreeMem(backES.iff, sizeof(struct IFF_FRAME));

	/**** free colormap ****/

	if ( backES.cm )
		FreeColorMap(backES.cm);

	ClearBackWindow();

	/**** END -- free back win ****/

	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
		UA_CloseWindowSafely(scriptWindow);

	if ( TestBit(allocFlags, SCRIPTSCREEN_FLAG) )
		CloseScreen(scriptScreen);

	if ( TestBit(allocFlags, PAGEWINDOW_FLAG) )
		UA_CloseWindowSafely(pageWindow);

	if ( TestBit(allocFlags, PAGESCREEN_FLAG) )
		CloseScreen(pageScreen);

	UnSetBit(&allocFlags, SCRIPTWINDOW_FLAG);
	UnSetBit(&allocFlags, SCRIPTSCREEN_FLAG);
	UnSetBit(&allocFlags, PAGEWINDOW_FLAG);
	UnSetBit(&allocFlags, PAGESCREEN_FLAG);

	/**** close palette screen and window ****/

	ClosePalette();
	CloseSmallScrWdwStuff();

	/**** free XaPPs ****/

	CloseToolIcons();
}

/******** GetVolumeSize() ********/

LONG GetVolumeSize(STRPTR path)
{
int error;
struct InfoData __aligned info;

	if ( !strnicmp(path,"ram",3) )	// this is RAM: or RAM DISK:
		return( (LONG)AvailMem(MEMF_PUBLIC) );
	else
	{
		error = getdfs(path,&info);	
		if ( error!=0 )
			return(0);
		else
			return( (info.id_NumBlocks-info.id_NumBlocksUsed) * info.id_BytesPerBlock ); 
	}
}

/******** E O F ********/
