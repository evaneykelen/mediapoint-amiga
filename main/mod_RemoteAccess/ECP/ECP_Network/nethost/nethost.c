// File		: Nethost.c
// Uses		:
//	Date		: 05 Oct 1994
// Author	: ing. C. Lieshout
// Desc.		: Execute commands from command file
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <stdio.h>
#include <dos.h>
#include <ctype.h>

int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */

#endif

#include <stdio.h>
#include <string.h>
#include "demo:gen/wait50hz.h"

#define TIME_INTERVAL 250L

void CheckCommand( void );
int CopyNewScriptEND( char *path, struct Library *DOSBase );
int ClearArchiveEND( char *path, struct Library *DOSBase );
int SendArexx( char *port, char *com );

void QuitOtherNet( void );
int OtherNet( void );
int run;

int main( int argc, char *argv[] )
{
	struct MsgPort *netport;
	struct Message	*Msg;
	ULONG PortMask;

	struct wjif wj;
	long insig,wsig;
	int stop = 0;

	wj.signum	= 0;

	if( OtherNet() )
	{
		if( argc >= 2 )
		{
			if( argv[1][0]=='q' || argv[1][0]=='Q' )
			{
				printf("Removing other host\n");
				QuitOtherNet();
				return( 2 );
			}
			if( argv[1][0]=='o' || argv[1][0]=='O' )
			{
				printf("Removing other host\nRestarting\n");
				QuitOtherNet();
			}
			else
			{
				printf("Nethost already running\n");
				return( 2 );
			}
		}
		else
		{
			printf("Nethost already running\n");
			return( 2 );
		}
	}

	netport = CreatePort("NetPort", 0);

	if( netport == NULL )
	{
		printf("Can't open NetPort\n");
		return( 3 );
	}

	PortMask = 1L << netport->mp_SigBit;


	if( set50hz( &wj, TIME_INTERVAL ) )
	{
		wsig = 1L << wj.signum | SIGBREAKF_CTRL_F | PortMask;

		while( !stop )
		{
			insig = Wait( wsig );
			if( insig & PortMask )
			{
				while ( Msg = (struct Message *)GetMsg((struct MsgPort *)netport))
					ReplyMsg( (struct Message *)Msg );
				break;
			}

			if( insig & SIGBREAKF_CTRL_F )
				stop = 1;
			if( insig & 1L << wj.signum )
			{
				wj.val = 0;
//				printf("second\n");
				CheckCommand();
			}
		}

		remove50hz( &wj );
	}
	if( netport )
		DeletePort( (struct MsgPort *)netport );

}

int OtherNet( void )
{
	int ret = 0;

	Forbid();
	if( FindPort("NetPort") )
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

void QuitOtherNet( void )
{
	struct MsgPort *replyport=0;
	struct Message msg;

	replyport = CreatePort(0,0);

	if( replyport )
	{
		msg.mn_Node.ln_Type = NT_MESSAGE;
		msg.mn_Length = sizeof( struct Message );
		msg.mn_ReplyPort = replyport;
		if( SafePutToPortQuit( &msg, "NetPort") )
		{
			WaitPort( replyport );
			GetMsg( replyport );
		}
		DeletePort( replyport );
	}

}

//===============================================
//	Name		: SayRun
//	Function	: Check which script runs
//	Inputs	: 
//	Result	: 1 or 2 or 0 when file not found
//	Updated	: 19 - 09 - 1994
//
int SayRun ( void )
{
	FILE *f;	
	int check=0;

	f = fopen("MP_RA:RUNS_script.1","r");

	if( f )
	{
		check = 1;
		fclose( f );
	}

	if( check ==  0 )
	{
		f = fopen("MP_RA:RUNS_script.2","r");
		if( f )
		{
			check = 2;
			fclose( f );
		}
	}
	return( check );
}

void DoCommand( char *str )
{
	char c,tt[256];

	int i=0;

	while( str[ i ] != 0 )
	{
		c = str[ i ];
		str[i] = toupper( c );
		i++;
	}

	//printf("Command is [%s]\n",str );

	if( !strcmp( str,"CHSCRIPT" ) )
	{
		run = SayRun();
//		printf("Say run says %d\n",run );
		SendArexx( "MEDIAPOINT","quit" );

#if 0
		if( run == 1 )									// set alias to other script
			run = 2;
		else
			run = 1;
		sprintf(tt,"assign ALIAS: mp_ra:script%d\n",run );
		Execute( tt,0,0 );							// do  alias
		printf("Changed script\n");
		printf("Alias changed\n");
#endif
	}
	else	
		if( !strcmp( str,"COPYSCRIPT" ) )
		{
//			printf("Copy script\n");
//			run = SayRun();

//			printf("Say run second says %d\n",run );
			if( run == 1 )
				strcpy(tt,"MP_RA:script1");
			else
				strcpy(tt,"MP_RA:script2");

//			printf("Clear archive bit %s\n",tt );

			ClearArchiveEND( tt, DOSBase );					// clear all the archive bits old dir

			if( run == 1 )
				strcpy(tt,"MP_RA:script2");
			else
				strcpy(tt,"MP_RA:script1");

			CopyNewScriptEND( tt, DOSBase );						// copy all the files
//			printf("Script copied\n");

		}

}

//===============================================
//	Name		: CheckCommand
//	Function	: Execute a command if present
//	Inputs	: None
//	Result	: None
//	Updated	: 05 - 10 - 1994
//
void CheckCommand( void )
{
	char str[256];
	FILE *f;	
	int i;
	char c;

	f = fopen("MP_RA:Command","r");

	i=0;

	if( f )
	{
		while( !feof( f ) )
		{
			c = fgetc( f );
			if( c != 0 && c != '\n' && !feof( f ) )
				str[ i++ ] = c;
			else
			{
				str[ i ] = 0;
				DoCommand( str );
				i = 0;
			}
		}

		if( i != 0 )
		{
			str[ i ] = 0;
			DoCommand( str );
		}

		fclose( f );
		DeleteFile( "MP_RA:Command" );
	}
}