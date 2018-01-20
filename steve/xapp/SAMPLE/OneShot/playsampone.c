// File		: playsampone.c
// Uses		: sampleone.h
//	Date		: 20-1-1994
// Author	: ing. C. Lieshout
// Desc.		: Play a Sample from chipmem
//

//#include <stdio.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include	<exec/libraries.h>
#include	"sampleone.h"
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

int loadsoundfile( SoundInfoOne *sinfo, char *filename );
void freesound( SoundInfoOne *sinfo );
void exit_sound( SoundInfoOne *sinfo );
void play_sound( SoundInfoOne *sinfo );

int test( void )
{
	SoundInfoOne si;
	int ret;
	ret = loadsoundfile( &si, "mp:sounds/samples/mp_woman" );
	if( ret == 1 )
	{
		play_sound( &si );
		Wait( si.audiosig );
		Wait( si.audiosig );
		exit_sound( &si );
	}
	freesound( &si );
	return 0;
}
