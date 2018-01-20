/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include "spritedata.h"
#include "nb:mpplayer/dongle/dongle_protos.h"

//TEXT codedStr[] = { "­š–‘—ž›ß¬–ŒŒšßÒß¯’‹š“ßŒÑÑ“Ñ" };	// for demomsg.c

// "Chis Palmer Circle Computer "
TEXT codedStr[] = { "¼—–Œß¯ž“’šßÒß¼–œ“šß¼’Š‹šŒ" };	// for demomsg.c

// "OFFCAM - Johan Shirren - Lorenczendam 16 - 24103 Kiel - Germany"
// [°¹¹¼¾²ßÒßµ—ž‘ß¬—–š‘ßÒß³š‘œ…š‘›ž’ßÎÉßÒßÍËÎÏÌß´–š“ßÒß¸š’ž‘†]

// "Amiga Special - Jens Tillack - Waldweg 5 - 88175 Scheidegg - Germany" 
// [¾’–˜žß¬šœ–ž“ßÒßµš‘Œß«–““žœ”ßÒß¨ž“›ˆš˜ßÊßÒßÇÇÎÈÊß¬œ—š–›š˜˜ßÒß¸š’ž‘†]

// "Screen multimedia - Grosse Elbstrasse 277 - 22767 Hamburg - Germany"
// [¬œšš‘ß’Š“‹–’š›–žßÒß¸ŒŒšßº“Œ‹žŒŒšßÍÈÈßÒßÍÍÈÉÈß·ž’Š˜ßÒß¸š’ž‘†]

// "Frank Hoen - Eureka"
// [¹ž‘”ß·š‘ßÒßºŠš”ž]

// "Reinhard Spisser - Promotel s.r.l."
// [­š–‘—ž›ß¬–ŒŒšßÒß¯’‹š“ßŒÑÑ“Ñ]

extern void FakeRun(void);	// wm:fakerun.o

/**** GLOBAL POINTERS ****/

struct IntuitionBase *IntuitionBase				= NULL;
struct GfxBase *GfxBase										= NULL;
struct Library *DiskfontBase							= NULL;
struct Library *LocaleBase								= NULL;
struct Library *medialinkLibBase					= NULL;
struct Library *MLMMULibBase							= NULL;
struct LayersBase *LayersBase							= NULL;
struct Library *KeymapBase								= NULL;
struct Process *process										= NULL;
APTR tempWdwPtr														= NULL;
struct Locale *Locale											= NULL;
struct ColorMap *undoCM										= NULL;
struct FileInfoBlock *FIB									= NULL;
struct MsgPort *capsPort									= NULL;
struct Library *ConsoleDevice							= NULL;
struct List **clipLists										= NULL;
struct List **undoLists										= NULL;
int *arrayPos															= NULL;
struct List **newPtrs											= NULL;
struct MenuRecord **page_MR								= NULL;
struct MenuRecord **script_MR							= NULL;
struct Screen **DA_Screens								= NULL;
struct EditWindow **EditWindowList				= NULL;
struct EditSupport **EditSupportList			= NULL;
struct EditWindow  **Clipboard_WL					= NULL;
struct EditSupport **Clipboard_SL					= NULL;
struct EditWindow  **Undo_WL							= NULL;
struct EditSupport **Undo_SL							= NULL;
struct TextFont *systemFont								= NULL;
struct TextFont *smallFont								= NULL;
struct TextFont *largeFont								= NULL;
struct TextFont *textFont									= NULL;
struct TextFont *tiny_smallFont						= NULL;
struct TextFont *tiny_largeFont						= NULL;
struct List *fontList											= NULL;
struct MsgPort *ML_Port										= NULL;
struct Node *ML_Node											= NULL;
int *SNRlist															= NULL;
struct List *undoListPtr									= NULL;
struct FontEntry **FontEntryList					= NULL;
struct Screen *pageScreen									= NULL;
struct Window *pageWindow									= NULL;
struct Screen *scriptScreen								= NULL;
struct Window *scriptWindow								= NULL;
struct Screen *thumbScreen								= NULL;
struct Window *thumbWindow								= NULL;
struct Window *paletteWindow							= NULL;
struct Screen *paletteScreen							= NULL;
TEXT *dir_xapps														= NULL;
TEXT *dir_system													= NULL;
TEXT *dir_scripts													= NULL;
UBYTE *homeDirs														= NULL;												
UBYTE *homePaths													= NULL;												
ULONG *LoadRGB32Table											= NULL;
struct ScriptNodeRecord *editSNR					= NULL;
struct GadgetRecord *kept_Script_GR				= NULL;
struct IOClipReq *clipboard								= NULL;
struct Window *smallWindow								= NULL;
UWORD *emptySprite												= NULL;

/**** GLOBAL STRUCTURES ****/

struct CapsPrefs CPrefs;
struct EventData CED;
struct eventHandlerInfo EHI;
struct BitMap scratchBM;
struct RastPort scratchRP;
struct BitMap gfxBitMap;
struct RastPort gfxRP;
struct BitMap mbarBM;
struct RastPort mbarRP;
struct BitMap sharedBM;
struct RastPort sharedRP;
struct BitMap transpBM;
struct RastPort transpRP;
struct BitMap backBM;
struct RastPort backRP;
struct BitMap iconBM;
struct IOStdReq ioreq;
struct RendezVousRecord rvrec;
struct FileListInfo FLI;
struct FileListInfo buffer_FLI;
struct FER FontEntryRecord;
struct Document pageDoc;
struct Document scriptDoc;
struct NewWindow NewWindowStructure;
struct TextAttr page_textAttr;
struct TextAttr script_textAttr;
struct RastPort xappRP;
struct RastPort xappRP_2;
struct ObjectInfo ObjectRecord;
struct EditWindow undoEW;
struct EditSupport undoES;
struct BitMap effBM;
struct BitMap dragBM;
struct RastPort dragRP;
struct BitMap dragBM2;
struct RastPort dragRP2;
struct List usedxapplist;
struct EditWindow prefsEW;
struct EditWindow backEW;
struct EditSupport backES;
struct MenuRecord fast_script_MR;
struct MenuRecord fast_page_MR;

/**** GLOBAL ARRAYS ****/

UWORD chip gui_pattern[] = { 0x5555, 0xaaaa };
UWORD chip gui_pattern_lace[] = { 0x5555, 0x5555, 0xaaaa, 0xaaaa };
UBYTE *daPathList[10];
UBYTE *daDescList[10];
BOOL daUsedList[10];
UBYTE *objectNameList[MAXTOOLS];
int objectTypeList[MAXTOOLS];
int objectXPosList[MAXTOOLS];
int objectYPosList[MAXTOOLS];
BOOL IconEnabledList[MAXTOOLS];	// index is index in tool bar
BOOL ToolEnabledList[MAXTOOLS];	// index is type number
int standardXPosList[MAXTOOLS];
int standardYPosList[MAXTOOLS];
TEXT pageScreenTitle[255];
TEXT scriptScreenTitle[255];
BOOL EditMenuStates[8] = {	FALSE, FALSE, FALSE, FALSE,
														FALSE, FALSE, FALSE, FALSE };
TEXT newScript[100];
ULONG memsizes[4] = { 0,0,0,0 };	// used by mem watcher
TEXT MRO_Script[5*SIZE_FULLPATH];
TEXT MRO_Page[5*SIZE_FULLPATH];
TEXT mainName[40];

/**** GLOBALS VARIABLES ****/

ULONG allocFlags = 0L;
int selectedWell=1;
ULONG SNRlistSize=0;
ULONG FEL_size;
int kept_totalHeight;
int xappWdwHeight;
int numLevelTools;
ULONG numEntries1, numDisplay1;
LONG topEntry1;
ULONG numEntries2, numDisplay2;
LONG topEntry2;
int lastUndoableAction=0;
int lastUndoWindow;
int HANDLESNIF=9;
ULONG resourceFlags=0;
ULONG kept_S_size=0L;
BOOL alt_ctrl_esc_pressed = FALSE;
BOOL RA_Installed = FALSE;

/**** STATIC GLOBALS ****/

TEXT startuppath[SIZE_FULLPATH];

/**** FUNCTIONS ****/

/******** StartUpFuncs() ********/

BOOL StartUpFuncs(BOOL fromPlayer)
{
struct FileLock *lock;

	/**** init music shit ****/

	FakeRun();

	/**** init vars and arrays ****/

	SetGlobalVars();
	SetGlobalArrays();

	/**** open libraries ****/

	if ( !OpenLibraries() )
		exit(0);

	/**** set sprites ****/

	UA_SetSpritePtrs(WaitPointer,colorPicker,toSprite,hand);

	/**** get startup path ****/

	FIB = (struct FileInfoBlock *)AllocMem(
													(LONG)sizeof(struct FileInfoBlock), MEMF_ANY | MEMF_CLEAR);
	if ( FIB==NULL )
	{
		CloseLibraries();
		exit(0);
	}
	if (GfxBase->LibNode.lib_Version >= 36)
		lock = (struct FileLock *)GetProgramDir();
	else
		lock = (struct FileLock *)Lock("",(LONG)ACCESS_READ);
	if (lock==NULL)
	{
		FreeMem(FIB,(LONG)sizeof(struct FileInfoBlock));
		CloseLibraries();
		exit(0);
	}
	findFullPath(lock, startuppath);
	UA_ValidatePath(startuppath);
	if (GfxBase->LibNode.lib_Version < 36)
		UnLock((BPTR)lock);

	/**** check if vital files are present ****/

	if ( !CheckCoreFiles() )
	{
		FreeMem(FIB,(LONG)sizeof(struct FileInfoBlock));
		CloseLibraries();
		exit(0);
	}

	/**** get hold of CLI window pointer ****/

	Forbid();
	process = (struct Process *)FindTask(NULL);
	Permit();
	if (process==NULL)
		UA_WarnUser(59);
	tempWdwPtr = process->pr_WindowPtr;	// can be NULL but that don't matter
	process->pr_WindowPtr = -1;

	/**** check OS version and chip-set related stuff ****/

	DoReleaseTwoFirstPart();

	/**** do small global allocs ****/

	if ( !GlobalAllocs(fromPlayer) )
		return(FALSE);

	/**** open and init fonts ****/

	if ( !OpenAppFonts("fonts:"))	// startuppath) ) 
		return(FALSE);

#ifndef USED_FOR_PLAYER
	if ( !InitFontList() )
		return(FALSE);
#endif

	/**** set-up IPC port ****/

	if ( !setupRendezVous() )
		return(FALSE);

	return(TRUE);
}

/******** OpenLibraries() ********/

BOOL OpenLibraries(void)
{
TEXT path[SIZE_FULLPATH], str[100];

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
	if (IntuitionBase == NULL)
		return(FALSE);

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
	if (GfxBase == NULL)
		return(FALSE);

	DiskfontBase = (struct Library *)OpenLibrary("diskfont.library", 0L);
	if (DiskfontBase == NULL)
	{
		Early_WarnUser("The 'diskfont.library' can't be opened");
		return(FALSE);
	}

	IconBase = (struct Library *)OpenLibrary("icon.library", 0L);
	if (IconBase == NULL)
	{
		Early_WarnUser("The 'icon.library' can't be opened");
		return(FALSE);
	}

	LayersBase = (struct LayersBase *)OpenLibrary("layers.library", 0L);
	if (LayersBase == NULL)
	{
		Early_WarnUser("The 'layers.library' can't be opened");
		return(FALSE);
	}

	KeymapBase = (struct Library *)OpenLibrary("keymap.library", 0L);
	if (KeymapBase==NULL)
	{
		Early_WarnUser("The 'keymap.library' can't be opened");
		return(FALSE);
	}

	SetBit(&resourceFlags,OPENED_SYSTEMLIBS);

	/* Look for libs in:
	 * xapps/system/mediapoint.library
	 * libs:mediapoint.library
	 * mediapoint.library
	 */

	sprintf(path, "system/%s",ML_LIBRARY_1);
	medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
	if (medialinkLibBase == NULL)
	{
		sprintf(path, "libs:%s", ML_LIBRARY_1);
		medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
		if (medialinkLibBase == NULL)
		{
			sprintf(path, "%s", ML_LIBRARY_1);
			medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
			if (medialinkLibBase == NULL)
			{
				sprintf(str,"The '%s' can't be opened", ML_LIBRARY_1);
				Early_WarnUser(str);
				return(FALSE);
			}
		}
	}
	if ( !UA_Open_ML_Lib() )
	{
		sprintf(str,"The '%s' can't be initialised", ML_LIBRARY_1);
		Early_WarnUser(str);
		return(FALSE);
	}

	sprintf(path, "system/%s",ML_LIBRARY_2);
	MLMMULibBase = (struct Library *)OpenLibrary(path, 0L);
	if (MLMMULibBase == NULL)
	{
		sprintf(path, "libs:%s", ML_LIBRARY_2);
		MLMMULibBase = (struct Library *)OpenLibrary(path, 0L);
		if (MLMMULibBase == NULL)
		{
			sprintf(path, "%s", ML_LIBRARY_2);
			MLMMULibBase = (struct Library *)OpenLibrary(path, 0L);
			if (MLMMULibBase == NULL)
			{
				sprintf(str,"The '%s' can't be opened", ML_LIBRARY_2);
				Early_WarnUser(str);
				return(FALSE);
			}
		}
	}

	SetBit(&resourceFlags,OPENED_APPLIBS);

	return(TRUE);
}

/******** CloseLibraries() ********/

void CloseLibraries(void)
{
	if (IntuitionBase != NULL)
		CloseLibrary((struct Library *)IntuitionBase);

	if (GfxBase != NULL)
		CloseLibrary((struct Library *)GfxBase);

	if (DiskfontBase != NULL)
		CloseLibrary((struct Library *)DiskfontBase);

	if (IconBase != NULL)
		CloseLibrary((struct Library *)IconBase);

	UA_Close_ML_Lib();
	if (medialinkLibBase != NULL)
		CloseLibrary((struct Library *)medialinkLibBase);

	if (MLMMULibBase != NULL)
		CloseLibrary((struct Library *)MLMMULibBase);

	if (LayersBase != NULL)
		CloseLibrary((struct Library *)LayersBase);

	if (KeymapBase != NULL)
		CloseLibrary((struct Library *)KeymapBase);

	/**** not opened in initglobals.c but in releasetwo.c (if 2.0 available) ****/

	if (LocaleBase != NULL)
		CloseLibrary((struct Library *)LocaleBase);
}

/******** SetGlobalVars() ********/

void SetGlobalVars(void)
{
	/**** Event Handler Info ****/

	EHI.activeScreen		= 0;
	//EHI.paletteVisible	= FALSE;
	EHI.thumbsVisible		= FALSE;

	/**** CPrefs ****/

	CPrefs.PageCM			= NULL;
	CPrefs.appdirLock = NULL;
	CPrefs.locale			= FALSE;

	/**** buffered IO ****/

	buffer_FLI.fileList = NULL;
	buffer_FLI.numFiles = 0;

	buffer_FLI.selectionList = NULL;

	buffer_FLI.assignList = NULL;
	buffer_FLI.numAssigns = 0;

	buffer_FLI.deviceList = NULL;
	buffer_FLI.numDevices = 0;

	strcpy(mainName,"Main");
}

/******** SetGlobalArrays() ********/

void SetGlobalArrays(void)
{
int i;

	for(i=0; i<10; i++)
	{
		daPathList[i] = NULL;
		daDescList[i] = NULL;
		daUsedList[i] = FALSE;
	}

	for(i=0; i<MAXTOOLS; i++)
	{
		objectNameList[i]		= NULL;
		objectTypeList[i]		= -1;
		objectXPosList[i]		= -1;
		objectYPosList[i]		= -1;
		IconEnabledList[i]	= FALSE;
		ToolEnabledList[i]	= FALSE;
		standardXPosList[i]	= -1;
		standardYPosList[i]	= -1;
	}

	pageScreenTitle[0]		= '\0';
	scriptScreenTitle[0]	= '\0';

	/**** zero bitmap planes ****/

	for(i=0; i<8; i++)
	{
		scratchBM.Planes[i]	= NULL;
		gfxBitMap.Planes[i]	= NULL;
		mbarBM.Planes[i]		= NULL;
		sharedBM.Planes[i]	= NULL;
		transpBM.Planes[i]	= NULL;
		iconBM.Planes[i]		= NULL;
		backBM.Planes[i]		= NULL;
	}
}

/******** CheckCoreFiles() ********/

BOOL CheckCoreFiles(void)
{
TEXT path[SIZE_FULLPATH];
struct FileLock *lock;

	if ( !CheckDongle5() )
	{
		Early_WarnUser("The protection key is not present.\nInsert it into the mouse port or game port.");
		return(FALSE);
	}

	/**** check OS version: <= 36 sucks ****/

	if (GfxBase->LibNode.lib_Version <= 36)
	{
		Early_WarnUser("Sorry: Kickstart 2.0 or better is required.");
		return(FALSE);
	}

	/**** check critical drawers ****/

	sprintf(path,"%sSYSTEM",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The drawer 'System' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sXAPPS",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The drawer 'Xapps' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

#ifndef USED_FOR_PLAYER
	sprintf(path,"%sSCRIPTS",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The drawer 'Scripts' is missing"); return(FALSE); }
	UnLock((BPTR)lock);
#endif

	/**** check timers ****/

	sprintf(path,"%sSYSTEM/HHMMSST_Timer",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/HHMMSST_Timer' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sSYSTEM/HHMMSSFF_Timer",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/HHMMSSFF_Timer' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sSYSTEM/MIDI_Timer",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/MIDI_Timer' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	/**** check language file ****/

#ifndef USED_FOR_PLAYER
	sprintf(path,"%sSYSTEM/texts.English",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/texts.English' is missing"); return(FALSE); }
	UnLock((BPTR)lock);
#endif

	/**** check system files ****/

	sprintf(path,"%sSYSTEM/Animation",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/Animation' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sSYSTEM/Transitions",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/Transitions' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sSYSTEM/Input",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/Input' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	sprintf(path,"%sSYSTEM/Music",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock==NULL) { Early_WarnUser("The file 'System/Music' is missing"); return(FALSE); }
	UnLock((BPTR)lock);

	/**** NEW: check for presence of Remote Access software ****/

	sprintf(path,"%sSYSTEM/RemoteAccess",startuppath);
	lock = (struct FileLock *)Lock(path,(LONG)ACCESS_READ);
	if (lock)
	{
		RA_Installed = TRUE;
		UnLock((BPTR)lock);
	}
	else
		RA_Installed = FALSE;

	return(TRUE);
}

/******** MemWatcher() ********/

void MemWatcher(void)
{
TEXT str1[100], str2[100];

	sprintf(str1, "C=%08ld  F=%08ld  LC=%08ld  LF=%08ld\n",
					AvailMem(MEMF_CHIP),
					AvailMem(MEMF_FAST),
					AvailMem(MEMF_CHIP|MEMF_LARGEST),
					AvailMem(MEMF_FAST|MEMF_LARGEST) );

	if ( memsizes[0]+memsizes[1]+memsizes[2]+memsizes[3] != 0 )
		sprintf(str2, "DC=%08ld  DF=%08ld  DLC=%08ld  DLF=%08ld\n",
						memsizes[0]-AvailMem(MEMF_CHIP),
						memsizes[1]-AvailMem(MEMF_FAST),
						memsizes[2]-AvailMem(MEMF_CHIP|MEMF_LARGEST),
						memsizes[3]-AvailMem(MEMF_FAST|MEMF_LARGEST) );
	else
		sprintf(str2,"\n");

	memsizes[0]=AvailMem(MEMF_CHIP);
	memsizes[1]=AvailMem(MEMF_FAST);
	memsizes[2]=AvailMem(MEMF_CHIP|MEMF_LARGEST);
	memsizes[3]=AvailMem(MEMF_FAST|MEMF_LARGEST);

	PrintSer(str1);
	PrintSer(str2);
}

/******** E O F ********/
