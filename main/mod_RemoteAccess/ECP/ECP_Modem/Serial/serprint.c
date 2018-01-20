// File		: sergetdir.c
// Uses		:
//	Date		: 19 august 1994
// Autor		: C. Lieshout
// Desc.		: Read the directory structure from a give path
//				; and store result in a file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/datetime.h>
#include <exec/types.h>
#include <exec/memory.h>

#include "serprint.h"
#include "RePort/report.h"

struct PPMessage msg;
struct MsgPort *replyport=0;

/*void main( void )
{
	SerPrint( "hallo" );
	getch();
	SerPrintUpdate( 123,345 );
	getch();
	SerPrintUpdate( 567,10 );
	getch();
}
*/

int InitPrint( void )
{
	replyport = CreatePort(0,0);
	if( replyport == NULL )
		return( 0 );
	else
		return( 1 );
}

void ExitPrint( void )
{
	if( replyport )
		DeletePort( replyport );
}

int SafePutToPort( struct Message *message, STRPTR portname)
{
	struct MsgPort *port;

	Forbid();
	port = FindPort(portname);
	if (port)
		PutMsg(port,message);
	Permit();
	return((BOOL)port); /* If zero, the port has gone away */
}

void SerPrint( char *mes )
{
	msg.pp_max = -1;
	msg.pp_count = -1;
	msg.pp_Msg.mn_Node.ln_Type = NT_MESSAGE;
	msg.pp_Msg.mn_Length = sizeof( struct PPMessage );
	msg.pp_Msg.mn_ReplyPort = replyport;
	msg.pp_mes = mes;
	if( SafePutToPort( &msg.pp_Msg, "ProtPort") )
	{
		WaitPort( replyport );
		GetMsg( replyport );
	}
}

void SerPrintProt( char *mes )
{

	msg.pp_max = -1;
	msg.pp_count = -1;
	msg.pp_Msg.mn_Node.ln_Type = NT_MESSAGE;
	msg.pp_Msg.mn_Length = sizeof( struct PPMessage );
	msg.pp_Msg.mn_ReplyPort = replyport;
	msg.pp_mes = NULL;
	msg.pp_protocol = mes;
	if( SafePutToPort( &msg.pp_Msg, "ProtPort") )
	{
		WaitPort( replyport );
		GetMsg( replyport );
	}
}

void SerPrintUpdate( int max,int count )
{
	msg.pp_max = max;
	msg.pp_count = count;
	msg.pp_Msg.mn_Node.ln_Type = NT_MESSAGE;
	msg.pp_Msg.mn_Length = sizeof( struct PPMessage );
	msg.pp_Msg.mn_ReplyPort = replyport;
	msg.pp_mes = NULL;
	msg.pp_protocol = NULL;
	if( SafePutToPort( &msg.pp_Msg, "ProtPort") )
	{
		WaitPort( replyport );
		GetMsg( replyport );
	}
}
