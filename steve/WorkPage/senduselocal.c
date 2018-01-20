// File		: senduselocal.c
// Uses		: minc:
//	Date		: 19 oct 1994
// Author	: ing. C. Lieshout
// Desc.		: Send the globeproc a message that you are ready
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <workbench/startup.h>

#include <stdio.h>
#include <string.h>

//#include "nb:pre.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"

void SendReady( void );

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

void SendReady( void )
{

	struct MsgPort *replyport=0;
	GEDIALOGUE GD;
	geta4();

	replyport = CreatePort(0,0);

	if( replyport )
	{

		GD.gd_Cmd = EDC_USELOCALEVENTS;

		GD.gd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		GD.gd_Msg.mn_Length = sizeof( GEDIALOGUE );
		GD.gd_Msg.mn_ReplyPort = replyport;
		if( SafePutToPortQuit( &GD, "Port_GlobEProc") )
		{
//KPrintF("WP 1\n");
			WaitPort( replyport );
//KPrintF("WP 2\n");
			GetMsg( replyport );
//KPrintF("WP 3\n");
		}
		DeletePort( replyport );
	}
}


void SendReady2( void )
{
	struct MsgPort *replyport=0;
	GEDIALOGUE GD;

	replyport = CreatePort(0,0);

	if( replyport )
	{

		GD.gd_Cmd = EDC_USELOCALEVENTS;

		GD.gd_Msg.mn_Node.ln_Type = NT_MESSAGE;
		GD.gd_Msg.mn_Length = sizeof( GEDIALOGUE );
		GD.gd_Msg.mn_ReplyPort = replyport;
		if( SafePutToPortQuit( &GD, "Port_GlobEProc") )
		{
			WaitPort( replyport );
			GetMsg( replyport );
		}
		DeletePort( replyport );
	}
}
