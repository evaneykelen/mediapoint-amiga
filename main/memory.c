#include "nb:pre.h"

/**** externals ****/

extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** functions ****/

#if 0
/******** CheckMemStart() ********/

BOOL CheckMemStart(void)
{
ULONG fast_mem, chip_mem;

	/**** after ML is started around 450 k CHIP is gone and 110 k FAST ****/

	fast_mem = AvailMem(MEMF_FAST);

	chip_mem = 450000;

	if (fast_mem<110000)
		chip_mem += 110000;

	if ( AvailMem(MEMF_CHIP) < chip_mem )
		return(FALSE);

	if ( AvailMem(MEMF_CHIP | MEMF_LARGEST) < 200000 )
		return(FALSE);

	return(TRUE);
}

/******** CheckMemRunning() ********/

BOOL CheckMemRunning(struct Window *window)
{
	if ( AvailMem(MEMF_CHIP) < 100000 )
	{
		if ( UA_OpenGenericWindow(	window, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																QUESTION_ICON, msgs[Msg_LowOnMem-1], FALSE, NULL ) )
		{
			CloseAllClipboardWindows();
			CloseAllUndoWindows();
		}
	}

	if ( AvailMem(MEMF_CHIP) < 100000 )
		return(FALSE);	// still too less chip mem!

	return(TRUE);
}
#endif

/******** FlushTheSucker() ********/

void FlushTheSucker(void)
{
int i;
UBYTE *ptr;

	for(i=0; i<10; i++)
	{
		ptr = (UBYTE *)AllocMem(0x7ffffff0,MEMF_PUBLIC);
		if (ptr)
			FreeMem(ptr,0x7ffffff0);
	}
}

/******** E O F ********/
