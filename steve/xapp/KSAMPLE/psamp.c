// File		: psamp.c
// Uses		:
//	Date		: 2 february 1993
// Author	: ing. C. Lieshout
// Desc.		: Sample load and play routines
//

#define FREQ 3546895

#include <stdio.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>

#include "iff_fsound.h"
#include "sample.h"
#include	"loadsamp.h"

void fade_in( SoundInfo *sinfo );
void remove_fade( SoundInfo *sinfo );
void exit_sound( SoundInfo *sinfo );
void play_sound( SoundInfo *sinfo );
void change_sound( SoundInfo *sinfo );
void AudioBoxOn( void );
void AudioBoxOff( void );

int OtherSample( void );
void QuitOtherSample( void );

//	SoundInfo si;

void main( int argc, char *argv[] )
{
	SoundInfo si;
	struct MsgPort *sampleport;
	struct Message	*Msg;

	int err;
	ULONG sig,wsig,portmask;
	int freq = 0;
	int filter = 0;
	struct Task *task;
	long oldpri = 0;
	int quit=0;

	setmem(&si, sizeof(SoundInfo), 0);

	if( OtherSample() )
	{
//		printf("quiting other sample\n");
		QuitOtherSample();
	}

	task = FindTask( 0 );

	si.channel = 1;
	si.filter = 1;

	sampleport = CreatePort("SamplePort", 0);
	if( sampleport == NULL )
	{
		printf("Can't open SamplePort\n");
		return( 3 );
	}

	if( argc >= 2 )
	{
		si.type = SI_DISK;
		if( argc >=3 && (argv[2][0] == 'm' || argv[2][0] == 'M' ) )
			si.type = SI_NORMAL;

//		si.type |= SI_LOOPING;
		si.sigtest = 1;
		si.loops = 1;
		si.loop = 0;
		if( argc >=4 && argv[3][0] == '2' )
			si.channel = 2;

		err = loadsoundfile( &si, argv[1], FALSE, 0L );
		if( argc >=5 )
		{
			sscanf( argv[4],"%d",&filter );
			si.filter = (WORD)filter;
		}

		if( argc >=6 )
		{
			sscanf( argv[5],"%d",&freq );
			si.period = (WORD)( 3579546 / freq);
		}
		if( err == 1 && get_chipmem( &si ) )
		{

			set_volume( &si, 64, 0 );
			AudioBoxOff();
			oldpri = SetTaskPri(task, 21 );
			play_sound( &si );

			portmask = 1L << sampleport->mp_SigBit;

			wsig = si.audiosig | portmask;
			while( si.end == 0 )
			{
				sig = Wait( wsig );
				if( sig & si.audiosig )
				{
					load_sound_frame( &si );
				}
				else
					if( sig & si.fadesig )
					{
						remove_fade( &si );
						wsig = si.audiosig;
						printf("fade removed\n");
					}
				if( sig & portmask )
				{
					while ( Msg = (struct Message *)GetMsg((struct MsgPort *)sampleport))
						ReplyMsg( (struct Message *)Msg );
					quit = 1;
					break;
				}
			}
			if( quit == 0 )
				Wait( si.audiosig );					// wait for the sound to finish
		}
		if( sampleport )
			DeletePort( (struct MsgPort *)sampleport );

		set_volume( &si, 0, 0 );	// sound off ?
		change_sound( &si );
		AudioBoxOn();
		exit_sound( &si );
		freesound( &si );
		SetTaskPri( task,oldpri );
		
	}
	else
		printf("use %s samplename M(emory) 1(channel) filter frequency\n",argv[0] );
}

int OtherSample( void )
{
	int ret = 0;

	Forbid();
	if( FindPort("SamplePort") )
		ret = 1;
	Permit();

	return( ret );
}

int SafePutToPortQuit( struct Message *message, STRPTR portname)
{
	struct MsgPort *port;

	Forbid();
	port = FindPort(portname);
	if (port)
		PutMsg(port,message);
	Permit();
	return( (BOOL)port ); /* If zero, the port has gone away */
}

void QuitOtherSample( void )
{
	struct MsgPort *replyport=0;
	struct Message msg;

	replyport = CreatePort(0,0);

	if( replyport )
	{
		msg.mn_Node.ln_Type = NT_MESSAGE;
		msg.mn_Length = sizeof( struct Message );
		msg.mn_ReplyPort = replyport;
		if( SafePutToPortQuit( &msg, "SamplePort") )
		{
			WaitPort( replyport );
			GetMsg( replyport );
		}
		DeletePort( replyport );
	}

}
