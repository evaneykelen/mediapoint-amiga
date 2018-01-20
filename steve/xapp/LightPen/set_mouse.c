//	File		:	set_mouse.c
//	Uses		:
//	Date		:	26-04-93
//	Autor		:	Devices ( P. 111 )
//	Desc.		:	Set the mouse to a certain position using the input device
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "lightpen.h"

#include <stdio.h>

#ifdef LATTICE
int CXBRK( void ) { return( 0 ); }
int ckabort( void ) { return( 0 );}
#endif

void set_mouse( struct lp *lp )
{
	struct InputEvent	*FakeEvent;
	FakeEvent = lp->FakeEvent;

	FakeEvent->ie_EventAddress = NULL;
	FakeEvent->ie_NextEvent = NULL;
	FakeEvent->ie_X = lp->x;
	FakeEvent->ie_Y = lp->y;
	FakeEvent->ie_Class = IECLASS_POINTERPOS;
	FakeEvent->ie_SubClass = 0;
	FakeEvent->ie_Code = IECODE_NOBUTTON;
	FakeEvent->ie_Qualifier = NULL;

	lp->InputIO->io_Data = (APTR)FakeEvent;
	lp->InputIO->io_Length = sizeof( struct InputEvent );
	lp->InputIO->io_Command = IND_WRITEEVENT;
	DoIO( ( struct IORequest *)lp->InputIO );
}

int init_mouse( struct lp *lp )

{
	lp->InputMP = NULL;
	lp->FakeEvent = NULL;
	lp->InputIO = NULL;
	lp->dev = 0;
	lp->signum = -1;

	if( ! (lp->InputMP = CreateMsgPort() ) )
		return 0;

	if( !( lp->FakeEvent = (struct InputEvent *)AllocMem( sizeof( struct InputEvent ),MEMF_PUBLIC )))
		return 0;

	if( !(lp->InputIO = CreateIORequest( lp->InputMP,sizeof( struct IOStdReq ))))
		return 0;

	if( (lp->dev = OpenDevice("input.device",NULL,(struct IORequest *)lp->InputIO, NULL ) ) )
		return 0;

	if( ( lp->signum = AllocSignal( -1 ) ) == -1 )
		return 0;
	lp->signal = 1L << lp->signum;
	lp->task = (long)FindTask( 0L );


	return 1;
}

void free_mouse( struct lp *lp )
{
	if( lp->InputIO )
		DeleteIORequest( lp->InputIO );
	if( lp->FakeEvent )
		FreeMem( lp->FakeEvent, sizeof( struct InputEvent ) );
	if( lp->InputMP )
		DeleteMsgPort( lp->InputMP );
	if( lp->dev )
		CloseDevice( ( struct IORequest *)lp->InputIO );
	if( lp->signum != -1 )
		FreeSignal( lp->signum );
}