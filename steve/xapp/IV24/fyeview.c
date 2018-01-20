#include <stdio.h>
#include <fye/fye.h>
#include <fye/fyebase.h>
#include <clib/fye_protos.h>
#include <pragmas/fye.h>
#include "iff.h"
#include <workbench/startup.h>
#include <clib/wb_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#define	GfxBase	(FyeBase->ml_GfxBase)
#include <graphics/gfxmacros.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#define	DOSBase (FyeBase->ml_DOSBase)
#include <clib/exec_protos.h>
#include <dos/dos.h>
#include <pragmas/dos_pragmas.h>
#include <exec/memory.h>
#include <pragmas/exec_pragmas.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include "fyeview_protos.h"
#include "nb:parser.h"
#include "nb:capsdefines.h"
#include "nb:newdefines.h"
#include "pascal:include/toolslib.h"
#include "nb:capsstructs.h"
#include "mllib:MediaLinkLib_proto.h"
#include "mllib:MediaLinkLib_pragma.h"
#include "demo:gen/support_protos.h"
#include "nb:gui_texts.h"
#include <dos.h>

/**** externals ****/

extern struct Library *medialinkLibBase;
extern UBYTE **msgs;
extern struct RendezVousRecord *rvrec;

/**** functions ****/

/******** PickPicture() ********/

BOOL PickPicture(struct Window *window, STRPTR picpath, UWORD *mypattern1)
{
TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME];
BOOL retval=FALSE;
struct FileReqRecord FRR;

	FRR.path							= path;
	FRR.fileName					= filename;
	FRR.title							= msgs[Msg_IV_MISC_4-1]; //"Choose a 24 bits picture:";
	FRR.opts							= DIR_OPT_ILBM | DIR_OPT_NOINFO;
	FRR.multiple					= FALSE;

	if ( picpath[0] != '\0' )
		UA_SplitFullPath(picpath, path, filename);
	else
	{
		strcpy(path, rvrec->capsprefs->import_picture_Path);
		filename[0] = '\0';
	}

	UA_SetSprite(window, SPRITE_BUSY);
	retval = UA_OpenAFile(window, &FRR, mypattern1);
	if (retval)
		UA_MakeFullPath(path, filename, picpath);
	UA_SetSprite(window, SPRITE_NORMAL);

	return(retval);
}

/******** DisplayFye() ********/

BOOL DisplayFye(char *FileName, int delay)
{
int i;
short dx, dy;
PLANEPTR bitmap[24] = { 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };
PLANEPTR bitmap2[4] = { 0,0,0,0 };
struct PictHeader *ph;
ULONG file, FyeBoard, OneMemoryBlock, error;
struct FyeScreen *fs;
struct FyeBase *FyeBase = NULL;

	/**** open lib ****/

 	if ((FyeBase=(struct FyeBase *)OpenLibrary("fye.library",0))==NULL)
		return(FALSE);

	/**** init vars ****/

	dx = 187;
	dy = 80;
	fs = NULL;
	ph = NULL;
	FyeBoard = OneMemoryBlock = TRUE;
	for (i=0; i<24; i++)
		bitmap[i] = NULL;
	for (i=0; i<4; i++)
		bitmap2[i] = NULL;

	/**** open the file from disk ****/

	file = Open(FileName, MODE_OLDFILE);
	if (file==NULL)
	{
//KPrintF("error 1\n");
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	if (error = FyeReadIffPictHeader(file, &ph, DOSBase))
	{
//KPrintF("error 2\n");
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	bitmap2[0] = AllocMem(ph->bitplanesize * 4, MEMF_CHIP | MEMF_CLEAR);
	if (bitmap2[0])
	{
		for (i=1;i<4; i++)
			bitmap2[i]=bitmap2[i-1]+ph->bitplanesize;
	}
	else
	{
//KPrintF("error 3\n");
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	bitmap[0] = AllocMem(ph->bitplanesize * 24, MEMF_ANY | MEMF_CLEAR);
	if (bitmap[0])
	{
		for (i=1;i<24; i++)
			bitmap[i]=bitmap[i-1]+ph->bitplanesize;
	}
	else
	{
//KPrintF("error 4\n");
		if (bitmap2[0])
			FreeMem(bitmap2[0], ph->bitplanesize * 4);
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	if (error=FyeReadIffPictBody(ph, bitmap, 0, 24, CONTINOUS_BITPLANES, DOSBase))
	{
//KPrintF("error 5\n");
		if (bitmap[0])
			FreeMem(bitmap[0], ph->bitplanesize * 24);
		if (bitmap2[0])
			FreeMem(bitmap2[0], ph->bitplanesize * 4);
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	if (ph->bmhd.nplanes!=24)
	{
//KPrintF("error 6\n");
		if (bitmap[0])
			FreeMem(bitmap[0], ph->bitplanesize * 24);
		if (bitmap2[0])
			FreeMem(bitmap2[0], ph->bitplanesize * 4);
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	ph->vpdx = dx;
	ph->vpdy = dy;

	if (error=FyeCreateScreen(&fs, ph, bitmap2, NULL, FyeBase))
	{
//KPrintF("error 7\n");
		if (bitmap[0])
			FreeMem(bitmap[0], ph->bitplanesize * 24);
		if (bitmap2[0])
			FreeMem(bitmap2[0], ph->bitplanesize * 4);
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	LoadView(&(fs->v));

	if (FyeBoard=error=ObtainFyeBoard())
	{
//KPrintF("error 8\n");
		fcs_cleanup(0,fs,FyeBase);
		if (bitmap[0])
			FreeMem(bitmap[0], ph->bitplanesize * 24);
		if (bitmap2[0])
			FreeMem(bitmap2[0], ph->bitplanesize * 4);
		FyeCleanupReadIff(ph);
		Close(file);
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	if(SendFyeCmd (FYE_MODE_16M))
		goto quit;

	if(SendFyeCmd (FYE_FREEZE))
		goto quit;

	FyeDisplayPicture(&(fs->vp), bitmap, ph->bmhd.w, ph->bmhd.h, 0, 0,
							 			MODE_24BP_FAST | INTERLACEMODE | CLEARSPRITE);

	if (delay>0 && delay<100)
		Delay(delay*50);

quit:
//KPrintF("error 9\n");
	ReleaseFyeBoard();
	fcs_cleanup(0,fs,FyeBase);
	if (bitmap[0])
		FreeMem(bitmap[0], ph->bitplanesize * 24);
	if (bitmap2[0])
		FreeMem(bitmap2[0], ph->bitplanesize * 4);
	FyeCleanupReadIff(ph);
	Close(file);

	ObtainFyeBoard();
	SendFyeCmd(31);
	ReleaseFyeBoard();

	CloseLibrary((struct Library *)FyeBase);

	return(TRUE);
}

/******** E O F ********/
