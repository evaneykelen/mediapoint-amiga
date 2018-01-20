// File		: preview.c
// Uses		:
//	Date		: 24 sept 1994
// Author	: ing. C. Lieshout
// Desc.		: Preview a MPEG file
//

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>

#include <intuition/intuition.h>
#include <intuition/screens.h>

#include "workpeg.h"

#include "inthandler.h"

void Preview( char *filename )
{
	struct Library *IntuitionBase;
	struct MPEG_record *mp = NULL;
	OBJECT_INPUT_STRUCT oi;
	int read_err;
	struct Screen *play_screen;
	struct TagItem play_screen_tags[4];
	struct ColorSpec cp[2];

	cp[0].ColorIndex = 0;
	cp[0].Red = 0;
	cp[0].Green = 0;
	cp[0].Blue = 0;
	cp[1].ColorIndex = -1;

	play_screen_tags[0].ti_Tag = SA_Depth;
	play_screen_tags[0].ti_Data = 1;
	play_screen_tags[1].ti_Tag = SA_Quiet;
	play_screen_tags[1].ti_Data = 1;

	play_screen_tags[2].ti_Tag = SA_Colors;
	play_screen_tags[2].ti_Data = cp;

	play_screen_tags[3].ti_Tag = TAG_DONE;

	if( ( IntuitionBase = OpenLibrary( "intuition.library",0 ) ) )
	{
		if( ( mp = AllocMPEG() ) )
		{
			strcpy( mp->filename, filename );
			mp->action = 0;
			mp->mport_ptoc = 0;
			mp->sig_xtox = 0;
			mp->sig_ptoc = 0;
			mp->mainsignal = 0;
			if( !SetupInputHandler( &oi ) )
			{
				AddInputHandler( &oi );
				mp->quitsig = (1L << oi.SigNum_NO);

				if( (play_screen =OpenScreenTagList( NULL, &play_screen_tags ) ) )
				{
					play_mpeg( mp );
					CloseScreen( play_screen );
				}

				RemoveInputHandler( &oi );
				FreeInputHandler( &oi );
			}
			FreeMPEG( mp );
		}
		CloseLibrary( IntuitionBase );
	}
}