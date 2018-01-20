// File		: Worktest.c
// Uses		:
//	Date		: 30 aug 1994
// Author	: ing. C. Lieshout
// Desc.		: Try out function from the workstations point
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
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */
#endif

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serprefs.h"
#include "serprint.h"
#include "sergetdir.h"
#include "protocols.h"
#include "serwork.h"
UBYTE **msgs = NULL;

void Report( char *t )
{
	printf("Report [%s]\n",t);

}

int IsTheButtonHit( void )
{
	return( FALSE );
}

void main( int argc, char *argv[] )
{
	SERDAT *ser;

	int i,a;

//	SERDAT ser;

	long oldpri;
	struct Task *task;

	char comm[1000];

	ser = ( SERDAT *)AllocMem( sizeof( SERDAT ), MEMF_ANY|MEMF_CLEAR );

	task = FindTask( 0 );
	oldpri = SetTaskPri(task, 127 );

	read_prefs( ser,"prefs.serial" );

	strcpy( ser->pref.dialpref, "ATDT");

	strcpy( ser->pref.phonenr, "47587" );

	if( OpenSerial( ser ) )
	{
		if( InitPrint() )
		{
//		WriteSer( &ser, "hallo\xd" );
//		WaitForReply(&ser,comm,100,8 );

//	if( MakeConnection( ser ) )
			if( !Logon(ser,"cees****") )
			{
	printf("logon oke\n");

				if( argc >= 2 )
				{
					printf("Argv1 = [%s]\n",argv[1]);
					StandaardCommand( ser, argv[1] );
		
				}

//				if( WhichRuns( ser ) )
//					printf("Whichruns error\n");

//				if( CCopy( ser, "mp_RA:graphics/pictures/fotoNatuur24sept2","ram:t1",498224786 ,196692 ) )
//					printf("Error in ccopy\n");

//				if( ChangeScript( &ser ) )
//					printf("Error couldn't change script\n");

				if( UpLoadY( ser, "pics:sexy","ram:t1",0 ) )
					printf("Error in upload\n");

				if( Logoff(ser) )
					printf("logoff failed\n");
			}
			else
				printf("logon Failed\n");
			ExitPrint();
		}
		else
			printf("print functions error\n");
//	HangUp( ser );

	}
	FreeSerial( ser );
	SetTaskPri( task,oldpri );
	
	FreeMem( (unsigned char *)ser, sizeof( SERDAT ) );

}
void LogAction( char *tt )
{
	printf("log [%s]\n",tt );

}
