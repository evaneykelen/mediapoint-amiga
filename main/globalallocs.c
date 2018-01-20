/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include "scriptimages.h"

struct MsgPort *MyCreatePort(UBYTE *name, LONG pri);

/**** defines ****/

#define SCRATCH_WIDTH		180L	// see also initmenus.c
#define SCRATCH_HEIGHT	134L

#define MBAR_WIDTH			1280L
#define MBAR_HEIGHT			MHEIGHT	// 12

#define ICON_WIDTH			80	// see also icon.c and filethumbs.c !!!!!!!!!!!!!!!!!!!
#define ICON_HEIGHT			80
#define ICON_DEPTH			4

#define MAXNUMLISTS			512

#define MAXARGSTRLEN		512	// see also errors.c

#define DRAG_WIDTH			432
#define DRAG_HEIGHT			22

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct ColorMap *undoCM;
extern struct BitMap scratchBM;
extern struct RastPort scratchRP;
extern struct BitMap gfxBitMap;
extern struct RastPort gfxRP;
extern struct BitMap mbarBM;
extern struct RastPort mbarRP;
extern struct BitMap iconBM;
extern struct FileInfoBlock *FIB;
extern struct MsgPort *capsPort;
extern struct Library *ConsoleDevice;
extern struct IOStdReq ioreq;
extern struct List **clipLists;
extern struct List **undoLists;
extern int *arrayPos;
extern struct List **newPtrs;
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern struct Screen **DA_Screens;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditWindow **Clipboard_WL;
extern struct EditSupport **Clipboard_SL;
extern struct EditWindow **Undo_WL;
extern struct EditSupport **Undo_SL;
extern TEXT *dir_xapps;
extern TEXT *dir_system;
extern TEXT *dir_scripts;
extern UBYTE *daPathList[];
extern UBYTE *daDescList[];
extern struct Locale *Locale;
extern int *SNRlist;
extern ULONG SNRlistSize;
extern allocFlags;
extern struct FileListInfo buffer_FLI;
extern UBYTE *homeDirs;
extern UBYTE *homePaths;
extern struct RendezVousRecord rvrec;
extern ULONG *LoadRGB32Table;
extern struct BitMap dragBM;
extern struct RastPort dragRP;
extern struct BitMap dragBM2;
extern struct RastPort dragRP2;
extern struct GadgetRecord *kept_Script_GR;
extern struct GadgetRecord Script_GR[];
extern ULONG kept_S_size;
extern UWORD *emptySprite;

/**** functions ****/

/******** GlobalAllocs() ********/

BOOL GlobalAllocs(BOOL fromPlayer)
{
int i;

	kept_S_size = sizeof(struct GadgetRecord)*11;

	kept_Script_GR = (struct GadgetRecord *)AllocMem(kept_S_size, MEMF_ANY | MEMF_CLEAR);
	if ( !kept_Script_GR )
			return(FALSE);
	kept_Script_GR[9].ptr = NULL;

	if ( !fromPlayer )
	{
		LoadRGB32Table = (ULONG *)AllocMem(4096L, MEMF_ANY | MEMF_CLEAR);
		if ( LoadRGB32Table==NULL )
			return(FALSE);
	}

	homeDirs = (UBYTE *)AllocMem(10*SIZE_FILENAME, MEMF_ANY | MEMF_CLEAR);
	if (homeDirs==NULL)
		return(FALSE);

	homePaths = (UBYTE *)AllocMem(10*SIZE_FULLPATH, MEMF_ANY | MEMF_CLEAR);
	if (homePaths==NULL)
		return(FALSE);

	rvrec.homeDirs	= homeDirs;
	rvrec.homePaths	= homePaths;

	/**** sprite ****/

	emptySprite = (UWORD *)AllocMem(20L,MEMF_CHIP|MEMF_CLEAR);
	if ( !emptySprite )
		return(FALSE);

	/**** colormaps ****/

	if (GfxBase->LibNode.lib_Version >= 39)
		i=256;
	else
		i=32;

	CPrefs.PageCM = GetColorMap(i);
	if (CPrefs.PageCM==NULL)
		return(FALSE);

	undoCM = GetColorMap(i);
	if (undoCM==NULL)
		return(FALSE);

	/**** bitmaps ****/

	if ( !fromPlayer )
	{
		if ( !AllocEasyBitmaps() )
			return(FALSE);
	}

	/**** prepare bitmap which holds e.g. time/date buttons ****/

	InitBitMap(&gfxBitMap, 3, GFX_W, GFX_H);
	gfxBitMap.Planes[0] = (PLANEPTR)ImageData_pl_1; 
	gfxBitMap.Planes[1] = (PLANEPTR)ImageData_pl_2;
	gfxBitMap.Planes[2] = (PLANEPTR)ImageData_pl_3;
	InitRastPort(&gfxRP);
	gfxRP.BitMap=&gfxBitMap;

	/**** iconBM is used to save icon representing docs ****/

	if ( !fromPlayer )
	{
		//InitBitMap(&iconBM, ICON_DEPTH, ICON_WIDTH, ICON_HEIGHT);

		iconBM.Planes[0] = AllocMem(RASSIZE(ICON_WIDTH,ICON_HEIGHT)*ICON_DEPTH,
																MEMF_ANY | MEMF_CLEAR);
		if (iconBM.Planes[0] == NULL)
			return(FALSE);

		for(i=0; i<ICON_DEPTH; i++)
			iconBM.Planes[i] = iconBM.Planes[0] + i*RASSIZE(ICON_WIDTH,ICON_HEIGHT);
	}

	/**** set-up caps port ****/

	capsPort = (struct MsgPort *)MyCreatePort(MEDIALINKPORT, 0);
	if (capsPort == NULL)
		return(FALSE);

	/**** open the console device e.g. for RawKeyConvert ****/

	if (OpenDevice("console.device", -1L, (struct IORequest *)&ioreq, 0L))
		return(FALSE);
	else
		ConsoleDevice = (struct Library *)ioreq.io_Device;

	/**** newPtrs ****/

	CPrefs.MaxNumLists = SCRIPTSIZE_SMALL;	// this is 512 == MAXNUMLISTS

	newPtrs = (struct List **)AllocMem(	(sizeof(struct List *) * MAXNUMLISTS),
																			MEMF_ANY | MEMF_CLEAR);
	if (newPtrs==NULL)
		return(FALSE);

	/**** arrayPos ****/

	arrayPos = (int *)AllocMem(	sizeof(int) * MAXNUMLISTS, MEMF_ANY | MEMF_CLEAR);
	if (arrayPos==NULL)
		return(FALSE);

	/**** clipLists - allocate space for array of pointers ****/

	clipLists = (struct List **)AllocMem(	sizeof(struct List *) * MAXNUMLISTS,
																				MEMF_ANY | MEMF_CLEAR);
	if (clipLists==NULL)
		return(FALSE);

	/**** undoLists - allocate space for array of pointers ****/

	undoLists = (struct List **)AllocMem(	sizeof(struct List *) * MAXNUMLISTS,
																				MEMF_ANY | MEMF_CLEAR);
	if (undoLists==NULL)
		return(FALSE);

	/**** menu record ****/

	if ( !fromPlayer )
	{
		page_MR = (struct MenuRecord **)AllocMem(sizeof(struct MenuRecord *)*NUMMENUS, MEMF_ANY|MEMF_CLEAR);
		if (page_MR==NULL)
			return(FALSE);

		for(i=0; i<NUMMENUS; i++)
		{
			page_MR[i] = (struct MenuRecord *)AllocMem(sizeof(struct MenuRecord), MEMF_ANY|MEMF_CLEAR);
			if ( page_MR[i]==NULL )
				return(FALSE);
		}

		script_MR = (struct MenuRecord **)AllocMem(sizeof(struct MenuRecord *)*NUMMENUS, MEMF_ANY|MEMF_CLEAR);
		if (script_MR==NULL)
			return(FALSE);

		for(i=0; i<NUMMENUS; i++)
		{
			script_MR[i] = (struct MenuRecord *)AllocMem(sizeof(struct MenuRecord), MEMF_ANY|MEMF_CLEAR);
			if ( script_MR[i]==NULL )
				return(FALSE);
		}

		/**** da screens ****/
	
		DA_Screens = (struct Screen **)AllocMem(sizeof(struct Screen *)*10, MEMF_ANY|MEMF_CLEAR);
		if (DA_Screens==NULL)
			return(FALSE);

		/**** edit window list ****/

		EditWindowList = (struct EditWindow **)
												AllocMem(	sizeof(struct EditWindow *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (EditWindowList==NULL)
			return(FALSE);

		/**** edit support list ****/

		EditSupportList = (struct EditSupport **)
												AllocMem(	sizeof(struct EditSupport *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (EditSupportList==NULL)
			return(FALSE);

		/**** clipboard wl list ****/

		Clipboard_WL = (struct EditWindow **)
												AllocMem(	sizeof(struct EditWindow *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (Clipboard_WL==NULL)
			return(FALSE);

		/**** clipboard sl list ****/

		Clipboard_SL = (struct EditSupport **)
												AllocMem(	sizeof(struct EditSupport *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (Clipboard_SL==NULL)
			return(FALSE);

		/**** undo wl list ****/

		Undo_WL = (struct EditWindow **)
												AllocMem(	sizeof(struct EditWindow *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (Undo_WL==NULL)
			return(FALSE);

		/**** clipboard sl list ****/

		Undo_SL = (struct EditSupport **)
												AllocMem(	sizeof(struct EditSupport *)*MAXEDITWINDOWS,
																	MEMF_ANY|MEMF_CLEAR);
		if (Undo_SL==NULL)
			return(FALSE);
	}

	/**** strings ****/

	dir_xapps = (TEXT *)AllocMem(SIZE_FULLPATH, MEMF_ANY|MEMF_CLEAR);
	if (dir_xapps==NULL)
		return(FALSE);

	dir_system = (TEXT *)AllocMem(SIZE_FULLPATH, MEMF_ANY|MEMF_CLEAR);
	if (dir_system==NULL)
		return(FALSE);

	dir_scripts = (TEXT *)AllocMem(SIZE_FULLPATH, MEMF_ANY|MEMF_CLEAR);
	if (dir_scripts==NULL)
		return(FALSE);

	/**** DA stuff ****/

	for(i=0; i<10; i++)
	{
		daPathList[i] = (UBYTE *)AllocMem(SIZE_FULLPATH, MEMF_ANY | MEMF_CLEAR);
		if (daPathList[i] == NULL)
			return(FALSE);

		daDescList[i] = (UBYTE *)AllocMem(SIZE_FILENAME, MEMF_ANY | MEMF_CLEAR);
		if (daDescList[i] == NULL)
			return(FALSE);
	}

	return(TRUE);
}

/******** FreeGlobalAllocs() ********/

void FreeGlobalAllocs(BOOL fromPlayer)
{
int i;

	if ( kept_Script_GR )
		FreeMem(kept_Script_GR, kept_S_size);

	if ( !fromPlayer )
	{
		if ( LoadRGB32Table!=NULL )
			FreeMem(LoadRGB32Table, 4096L);
	}

	if (homeDirs!=NULL)
		FreeMem(homeDirs, 10*SIZE_FILENAME);

	if (homePaths!=NULL)
		FreeMem(homePaths, 10*SIZE_FULLPATH);

	/**** buffered IO ****/

	if ( buffer_FLI.fileList != NULL )
		FreeMem(buffer_FLI.fileList, (buffer_FLI.numFiles+1)*SIZE_FILENAME);

	/**** sprite ****/

	if ( emptySprite )
		FreeMem(emptySprite,20L);

	/**** colormaps ****/

	if (CPrefs.PageCM!=NULL)
		FreeColorMap(CPrefs.PageCM);

	if (undoCM!=NULL)
		FreeColorMap(undoCM);

	/**** bitmaps ****/

	if ( !fromPlayer )
		FreeEasyBitmaps();

#ifndef USED_FOR_PLAYER
	FreeSharedBM();
#endif

	if ( !fromPlayer )
	{
		if (iconBM.Planes[0] != NULL)
			FreeMem(iconBM.Planes[0], RASSIZE(ICON_WIDTH,ICON_HEIGHT)*ICON_DEPTH);
	}

	/**** free File Info Block ****/

	if ( FIB!=NULL)
		FreeMem(FIB, (LONG)sizeof(struct FileInfoBlock));

	/**** close caps port ****/

	if (capsPort != NULL)
		DeletePort((struct MsgPort *)capsPort);

	if (ConsoleDevice != NULL)
		CloseDevice((struct IORequest *)&ioreq);

	/**** close Locale ****/

	if (Locale!=NULL)
		CloseLocale(Locale);

	if ( !CPrefs.SystemTwo )
		UnLock((BPTR)CPrefs.appdirLock);

	/**** newPtrs ****/

	if ( newPtrs != NULL )
		FreeMem(newPtrs, sizeof(struct List *) * MAXNUMLISTS);

	/**** arrayPos ****/

	if ( arrayPos != NULL )
		FreeMem(arrayPos, sizeof(int) * MAXNUMLISTS);

	/**** clipLists ****/

#ifndef USED_FOR_PLAYER
	FreeClipUndoLists(clipLists);
#endif

	if ( clipLists != NULL )
		FreeMem(clipLists, sizeof(struct List *) * MAXNUMLISTS);

	/**** undoLists ****/

#ifndef USED_FOR_PLAYER
	FreeClipUndoLists(undoLists);
#endif

	if ( undoLists != NULL )
		FreeMem(undoLists, sizeof(struct List *) * MAXNUMLISTS);

	if ( TestBit(allocFlags, SMALLNODES_FLAG) )
		FreeMem(SNRlist, SNRlistSize);

	/**** menu record ****/

	if ( !fromPlayer )
	{
		for(i=0; i<NUMMENUS; i++)
		{
			if ( page_MR[i] != NULL )
				FreeMem(page_MR[i], sizeof(struct MenuRecord));
		}

		if ( page_MR != NULL )
			FreeMem(page_MR, sizeof(struct MenuRecord *) * NUMMENUS);

		for(i=0; i<NUMMENUS; i++)
		{
			if ( script_MR[i] != NULL )
				FreeMem(script_MR[i], sizeof(struct MenuRecord));
		}

		if ( script_MR != NULL )
			FreeMem(script_MR, sizeof(struct MenuRecord *) * NUMMENUS);

		/**** da screens ****/

		if (DA_Screens!=NULL)
			FreeMem(DA_Screens, sizeof(struct Screen *)*10);

		/**** edit window list ****/

		if (EditWindowList!=NULL)
			FreeMem(EditWindowList, sizeof(struct EditWindow *)*MAXEDITWINDOWS);

		/**** edit support list ****/
	
		if (EditSupportList!=NULL)
			FreeMem(EditSupportList, sizeof(struct EditSupport *)*MAXEDITWINDOWS);

		/**** clipboard wl list ****/

		if (Clipboard_WL!=NULL)
			FreeMem(Clipboard_WL, sizeof(struct EditWindow *)*MAXEDITWINDOWS);

		/**** clipboard sl list ****/

		if (Clipboard_SL!=NULL)
			FreeMem(Clipboard_SL, sizeof(struct EditSupport *)*MAXEDITWINDOWS);

		/**** undo wl list ****/

		if (Undo_WL!=NULL)
			FreeMem(Undo_WL, sizeof(struct EditWindow *)*MAXEDITWINDOWS);

		/**** clipboard sl list ****/

		if (Undo_SL!=NULL)
			FreeMem(Undo_SL, sizeof(struct EditSupport *)*MAXEDITWINDOWS);
	}

	/**** strings ****/

	if (dir_xapps!=NULL)
		FreeMem(dir_xapps, SIZE_FULLPATH);

	if (dir_system!=NULL)
		FreeMem(dir_system, SIZE_FULLPATH);

	if (dir_scripts!=NULL)
		FreeMem(dir_scripts, SIZE_FULLPATH);

	/**** DA stuff ****/

	for(i=0; i<10; i++)
	{
		if (daPathList[i] != NULL)
			FreeMem(daPathList[i], SIZE_FULLPATH);

		if (daDescList[i] != NULL)
			FreeMem(daDescList[i], SIZE_FILENAME);
	}
}

/******** AllocEasyBitmaps() ********/
/* 
 * These are bitmaps which are alloced/freed when playing a script
 *
 */

BOOL AllocEasyBitmaps(void)
{
int i;

	/**** scratchBM is used for restoring menu background ****/

	if ( CPrefs.AA_available )
		InitBitMap(&scratchBM, 8, SCRATCH_WIDTH, SCRATCH_HEIGHT);
	else
		InitBitMap(&scratchBM, 6, SCRATCH_WIDTH, SCRATCH_HEIGHT);

	for(i=0; i<8; i++)
		scratchBM.Planes[i] = NULL;

	for(i=0; i<scratchBM.Depth; i++)
	{
		scratchBM.Planes[i] = (PLANEPTR)AllocRaster(SCRATCH_WIDTH, SCRATCH_HEIGHT);
		if (scratchBM.Planes[i]==NULL)
			return(FALSE);
	}

	InitRastPort(&scratchRP);
	scratchRP.BitMap=&scratchBM;

	/**** mbarBM is used to save the underlying screen ****/

	if ( CPrefs.AA_available )
		InitBitMap(&mbarBM, 8, MBAR_WIDTH, MBAR_HEIGHT);
	else
		InitBitMap(&mbarBM, 6, MBAR_WIDTH, MBAR_HEIGHT);

	for(i=0; i<8; i++)
		mbarBM.Planes[i] = NULL;

	mbarBM.Planes[0] = (PLANEPTR)AllocMem(RASSIZE(MBAR_WIDTH,MBAR_HEIGHT)*mbarBM.Depth,MEMF_CHIP|MEMF_CLEAR);
	if ( !mbarBM.Planes[0] )
		return(FALSE);

	for(i=0; i<mbarBM.Depth; i++)
		mbarBM.Planes[i] = mbarBM.Planes[0] + i*RASSIZE(MBAR_WIDTH,MBAR_HEIGHT);

/*
	for(i=0; i<mbarBM.Depth; i++)
	{
		mbarBM.Planes[i] = (PLANEPTR)AllocRaster(MBAR_WIDTH, MBAR_HEIGHT);
		if (mbarBM.Planes[i]==NULL)
			return(FALSE);
	}
*/

	InitRastPort(&mbarRP);
	mbarRP.BitMap=&mbarBM;

	/**** drag is used to store underlying script screen while dragging object ****/

	InitBitMap(&dragBM, 3, DRAG_WIDTH, DRAG_HEIGHT);

	for(i=0; i<8; i++)
		dragBM.Planes[i] = NULL;

	for(i=0; i<dragBM.Depth; i++)
	{
		dragBM.Planes[i] = (PLANEPTR)AllocRaster(DRAG_WIDTH, DRAG_HEIGHT);
		if (dragBM.Planes[i]==NULL)
			return(FALSE);
	}

	InitRastPort(&dragRP);
	dragRP.BitMap=&dragBM;

	/**** drag 2 ****/

	InitBitMap(&dragBM2, 3, DRAG_WIDTH, DRAG_HEIGHT);

	for(i=0; i<8; i++)
		dragBM2.Planes[i] = NULL;

	for(i=0; i<dragBM2.Depth; i++)
	{
		dragBM2.Planes[i] = (PLANEPTR)AllocRaster(DRAG_WIDTH, DRAG_HEIGHT);
		if (dragBM2.Planes[i]==NULL)
			return(FALSE);
	}

	InitRastPort(&dragRP2);
	dragRP2.BitMap=&dragBM2;
}

/******** FreeEasyBitmaps() ********/
/* 
 * These are bitmaps which are alloced/freed when playing a script
 *
 */

void FreeEasyBitmaps(void)
{
int i;

	WaitBlit();
	for(i=0; i<8; i++)
	{
		if ( scratchBM.Planes[i] != NULL )
			FreeRaster(scratchBM.Planes[i], SCRATCH_WIDTH, SCRATCH_HEIGHT);
		scratchBM.Planes[i] = NULL;

/*
		if (mbarBM.Planes[i] !=NULL )
			FreeRaster(mbarBM.Planes[i], MBAR_WIDTH, MBAR_HEIGHT);
		mbarBM.Planes[i] = NULL;
*/

		if (dragBM.Planes[i] !=NULL )
			FreeRaster(dragBM.Planes[i], DRAG_WIDTH, DRAG_HEIGHT);
		dragBM.Planes[i] = NULL;

		if (dragBM2.Planes[i] !=NULL )
			FreeRaster(dragBM2.Planes[i], DRAG_WIDTH, DRAG_HEIGHT);
		dragBM2.Planes[i] = NULL;
	}

	FreeMem(mbarBM.Planes[0], RASSIZE(MBAR_WIDTH,MBAR_HEIGHT)*mbarBM.Depth);
	mbarBM.Planes[0] = NULL;
}

/******** MyCreatePort() ********/

struct MsgPort *MyCreatePort(UBYTE *name, LONG pri)
{
LONG sigBit;
struct MsgPort *mp;
int sig;

	sigBit = -1;
	for(sig=0; sig<32; sig++)
	{
		sigBit = AllocSignal( sig );
		if ( sigBit != -1 )
			break;
	}
	if ( sigBit==-1 )
		return(NULL);

	mp = (struct MsgPort *)AllocMem((ULONG)sizeof(struct MsgPort),(ULONG)MEMF_PUBLIC|MEMF_CLEAR);
	if ( !mp )
	{
		FreeSignal(sigBit);
		return(NULL);
	}

	mp->mp_Node.ln_Name		= name;
	mp->mp_Node.ln_Pri		= pri;
	mp->mp_Node.ln_Type		= NT_MSGPORT;
	mp->mp_Flags					= PA_SIGNAL;
	mp->mp_SigBit					= sigBit;
	mp->mp_SigTask				= (struct Task *)FindTask(0L);

	if (name)
		AddPort(mp);
	else
		NewList( &(mp->mp_MsgList) );

	return( mp );
}

/******** E O F ********/
