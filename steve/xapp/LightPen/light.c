#include <exec/exec.h>
#include <exec/types.h>
#include <dos/dos.h> 
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include "gen:lightpen.h"

struct lp lp;

int init_mouse( struct lp *lp );
void set_mouse( struct lp *lp);
void free_mouse( struct lp *lp );

void main( int argc, char *argv[] )
{

	long stop,signal,oldx,oldy;

	stop = 0;
	oldx = 30;
	oldy = 0;

	if( argc > 1 )
		sscanf(argv[1],"%d",&oldx);
	if( argc > 2 )
		sscanf(argv[2],"%d",&oldy);

	lp.offset_x = oldx;
	lp.offset_y = oldy;

	oldx = oldy = 0;

	if( init_mouse( &lp ) )
	{
		setlightpen(&lp);

		while( !stop )
		{
			signal = Wait( SIGBREAKF_CTRL_C | lp.signal );
			if( signal & SIGBREAKF_CTRL_C )
				stop = 1;
			else
				if( signal & lp.signal )
					printf("%ld %ld\n",lp.x,lp.y);
					set_mouse( &lp );
					oldx = lp.x;
					oldy = lp.y;
		}
		removelightpen(&lp);
	}
	free_mouse( &lp );
}
