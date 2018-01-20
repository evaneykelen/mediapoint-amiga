#include "mllib_includes.h"

/**** globals ****/

struct ExecBase *SysBase						= NULL;
struct GfxBase *GfxBase							= NULL;
struct IntuitionBase *IntuitionBase	= NULL;
struct DosLibrary *DOSBase					= NULL;
struct Library *ConsoleDevice				= NULL;
struct IOStdReq ioreq;
struct Library *DiskfontBase				= NULL;
struct LayersBase *LayersBase 			= NULL;
struct Library *RexxSysBase					= NULL;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** LIBUA_Open_ML_Lib() ********/

BOOL __saveds __asm LIBUA_Open_ML_Lib(void)
{
	SysBase=(struct ExecBase *)(* ((ULONG *)4) );

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
	if (!GfxBase)
		return(FALSE);

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
	if (!IntuitionBase)
		return(FALSE);

	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0L);
	if (!DOSBase)
		return(FALSE);

	if (OpenDevice("console.device", -1L, (struct IORequest *)&ioreq, 0L))
		return(FALSE);
	else
		ConsoleDevice = (struct Library *)ioreq.io_Device;

	DiskfontBase = (struct Library *)OpenLibrary("diskfont.library", 0L);
	if (!DiskfontBase)
		return(FALSE);

	LayersBase = (struct LayersBase *)OpenLibrary("layers.library", 0L);
	if (!LayersBase)
		return(FALSE);

	RexxSysBase = OpenLibrary("rexxsyslib.library",0L);
	if (!RexxSysBase)
		return(FALSE);

	return(TRUE);
}

/******** LIBUA_Close_ML_Lib() ********/

void __saveds __asm LIBUA_Close_ML_Lib(void)
{
	if (RexxSysBase)
	{
		CloseLibrary((struct Library *)RexxSysBase);
		RexxSysBase=NULL;
	}

	if (LayersBase)
	{
		CloseLibrary((struct Library *)LayersBase);
		LayersBase=NULL;
	}

	if (DiskfontBase)
	{
		CloseLibrary((struct Library *)DiskfontBase);
		DiskfontBase=NULL;
	}

	CloseDevice((struct IORequest *)&ioreq);

	if (DOSBase)
	{
		CloseLibrary((struct Library *)DOSBase);
		DOSBase=NULL;
	}

	if (IntuitionBase)
	{
		CloseLibrary((struct Library *)IntuitionBase);
		IntuitionBase=NULL;
	}

	if (GfxBase)
	{
		CloseLibrary((struct Library *)GfxBase);
		GfxBase=NULL;
	}
}

/******** E O F ********/
