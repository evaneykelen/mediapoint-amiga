// File		: preview.c
// Uses		:
//	Date		: 9 december 1993
// Author	: ing. C. Lieshout
// Desc.		: Sample preview module
//
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

//#include "pascal:include/textedit.h"
//#include "pascal:include/textstyles.h"
#include "nb:capsdefines.h"
#include "nb:newdefines.h"
#include "nb:parser.h"
#include "pascal:include/toolslib.h"
#include "nb:capsstructs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#include "iff_fsound.h"
#include "sample.h"
#include "loadsamp.h"
#include "protos.h"
#include "structs.h"

#include "mllib:medialinklib_proto.h"
#include "mllib:medialinklib_pragma.h"
 
/**** diskplay.o ****/

void fade_in( SoundInfo *sinfo );
void remove_fade( SoundInfo *sinfo );
void exit_sound( SoundInfo *sinfo );
void play_sound( SoundInfo *sinfo );
void change_sound( SoundInfo *sinfo );

#include <stdlib.h>
#include <stdio.h>

extern struct Library *medialinkLibBase;

void set_sound_vars( SoundInfo *sinfo, struct Sample_record *sr );

void preview_sample(	struct Sample_record *sr, struct GadgetRecord *GR,
											struct Window *waitWindow, struct EventData *CED,
											ULONG sigbit )
{
	//char tt[250];		// DEBUG

	long sigs,lsigs,B_Last,B_Fading;
	int i,load_err,stop,fromdisk;
	int pressed;
	//struct Library *medialinkLibBase;
	SoundInfo sinfo;
	BYTE *cp;

	cp = ( BYTE * )&sinfo;
	for( i = 0; i < sizeof( SoundInfo ); i++ )*cp++ = 0;		// clear the struct

	sigs = 0;
	fromdisk = sr->playFromDisk;			// remember the play from disk variable
	sr->playFromDisk = 1;
	pressed = 0;

	//medialinkLibBase = (struct Library *)rvrec->medialink;

	set_sound_vars( &sinfo, sr ); 		// copy the data from sample_rec to sinfo struct

	load_err = loadsoundfile( &sinfo, sr->filename,FALSE, sigbit );

	if( load_err && get_chipmem( &sinfo ) )
	{
		B_Fading = FALSE;
		set_sound_vars( &sinfo, sr );		// set frequence in sinfo struct

		if( sr->playFadeIn > 0 )
		{	
			fade_it_in( &sinfo, sr->playFadeIn );
			B_Fading = TRUE;
			play_sound( &sinfo );
			sinfo.vol_right = sinfo.vol_temp_right;
			sinfo.vol_left = sinfo.vol_temp_left;
			sinfo.vol_temp_left = 0;
			sinfo.vol_temp_right = 0;
			sigs |= sinfo.fadesig;
		}
		else
		{
//			set_sound_vars( &sinfo, sr );
			reset_sound( &sinfo );
			play_sound( &sinfo );
		}
		sigs |= sinfo.audiosig;
		B_Last = FALSE;
		stop = 0;

		while( !stop )
		{
//			lsigs = Wait( sigs );

			CED->Class = 0L;
			lsigs = UA_doStandardWaitExtra(	waitWindow, CED, sigs );

			// Do some button check here

			if (CED->Class==MOUSEBUTTONS && CED->Code==SELECTDOWN)
			{
				if ( UA_CheckGadgetList(waitWindow, GR, CED) == 8 )
				{
					UA_HiliteButton(waitWindow, &GR[8]);
					stop = 1;
				}
			}

			if( lsigs & sinfo.audiosig )
			{
				if( sinfo.end == 1 )
					if( !B_Last )
					{
						B_Last = TRUE;
					}
					else
					{
//		KPrintF("Set volume at 0\n");
						set_volume( &sinfo, 0, 0 );	// sound off ?
						change_sound( &sinfo );
						sinfo.sigtest = 0;				// swith off signalling
						stop = 1;
					}
				load_sound_frame( &sinfo );
			}

			if( lsigs & sinfo.fadesig )
			{
//				KPrintF("remove fade\n");
				remove_fade( &sinfo );
				sigs &= ~sinfo.fadesig;
				B_Fading = FALSE;
			}
		}
		if( B_Fading )
			remove_fade( &sinfo );
		exit_sound( &sinfo );
		free_chipmem( &sinfo );
	}
//	else
//		KPrintF("load err\n");

	freesound( &sinfo );
	sr->playFromDisk = fromdisk;
}

