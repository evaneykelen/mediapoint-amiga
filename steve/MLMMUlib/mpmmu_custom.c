/*************************************************************
*Desc : Initialize and free anything special used by this library
*
*
*/
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/semaphores.h>

#include <clib/exec_protos.h>

#include "myproto/exec.h"

#include "mlmmu.h"
#include "mpmmu_custom.h"
#include "mpmmu_ext.h"

struct ExecBase *SysBase = NULL;

struct SignalSemaphore MMUSemaphore;

struct List *MTList;		// List of Memtag structs

extern ULONG __saveds __asm LIBMLMMU_ReallyFlushMem(register __d1 ULONG);

/***************************************************
*Func : General init called once upon opening of the 
*		library.
*in   : libbase -> pointer to this library
*out  : TRUE -> library succesfully opened
*		FALSE -> error on open
*/
BOOL __saveds __asm CustomLibInit(register __a6 struct Library *libbase)
{
	SysBase=(struct ExecBase *)(* ((ULONG *)4) );

	InitSemaphore(&MMUSemaphore);

	if( (MTList = (struct List *)AllocMem(sizeof(struct List),MEMF_CLEAR|MEMF_PUBLIC)) == NULL)
		return(FALSE);

 	NewList(MTList);

#if 0
	MemRestrict = 0x7fffffff;		// at startup there is no memory limit
	MemUsed = 0;
	MemReside = TRUE;
#endif

	return(TRUE);
}

/************************************************************
*Func : Clean up all which is of general use for this library
*		Clean up all memory areas allocated via this library
*in   : libbase -> pointer to this library
*out  : -
*/
void __saveds __asm CustomLibExpunge(register __a6 struct Library *libbase)
{
	//LIBMLMMU_FlushMem(MEMF_ALL);			// free all
	LIBMLMMU_ReallyFlushMem(MEMF_USED);
	FreeMem(MTList,sizeof(struct List));
	MTList = NULL;
}
