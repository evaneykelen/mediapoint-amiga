// File		: preview.c
// Uses		:
//	Date		: 12 july 1994
// Author	: ing. C. Lieshout
// Desc.		: Preview a MIDI file
//

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include "midi_play.h"

#include "inthandler.h"

void Preview( char *filename )
{
	SMFInfo		smfi;
	OBJECT_INPUT_STRUCT oi;
	int read_err;

	init_smfi( &smfi );
	if( ( read_err = read_and_evaluate( filename, &smfi ) ) == 0 )
	{
		smfi.action = 0;
		smfi.mport_ptoc = 0;
		smfi.sig_xtox = 0;
		smfi.sig_ptoc = 0;
		smfi.mainsignal = 0;
		if( !SetupInputHandler( &oi ) )
		{
			AddInputHandler( &oi );
			smfi.quitsig = (1L << oi.SigNum_NO);
			play_midi( &smfi );
			RemoveInputHandler( &oi );
			FreeInputHandler( &oi );
		}
		release( &smfi ,0);
	}	// else error
}