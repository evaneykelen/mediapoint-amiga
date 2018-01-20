// File		: preview.c
// Uses		:
//	Date		: 12 july 1994
// Author	: ing. C. Lieshout
// Desc.		: Preview a credit roll
//

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

#include "inthandler.h"

int get_varsize( void );
void preview( LONG MonID , UBYTE *datablock, char *filename, long quitsig );

void Preview_roll( LONG MonID, char *filename )
{

	OBJECT_INPUT_STRUCT oi;
	UBYTE	*VSCDataSegment;

	VSCDataSegment = (UBYTE *)AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR );

	if( VSCDataSegment )
	{
		if( !SetupInputHandler( &oi ) )
		{
			AddInputHandler( &oi );
			preview( MonID, VSCDataSegment, filename, 1L << oi.SigNum_NO );
			RemoveInputHandler( &oi );
			FreeInputHandler( &oi );
		}
		FreeMem(VSCDataSegment,get_varsize());
	}
}
