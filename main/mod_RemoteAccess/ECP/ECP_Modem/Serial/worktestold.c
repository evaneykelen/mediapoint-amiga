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
void Report( char *t )
{
	printf("Report [%s]\n",t);

}
void main( int argc, char *argv[] )
{

	int i,a;

	SERDAT ser;

	long oldpri;
	struct Task *task;

	char comm[1000];

	task = FindTask( 0 );
	oldpri = SetTaskPri(task, 127 );

	read_prefs( &ser,"prefs.serial" );

	strcpy( ser.pref.dialpref, "ATDT");

	strcpy( ser.pref.phonenr, "47587" );

	if( OpenSerial( &ser ) )
	{
		if( InitPrint() )
		{
//		WriteSer( &ser, "hallo\xd" );
//		WaitForReply(&ser,comm,100,8 );

//	if( MakeConnection( &ser ) )
			if( !Logon(&ser,"cees****") )
			{
	printf("logon oke\n");

				if( argc >= 2 )
				{
					printf("Argv1 = [%s]\n",argv[1]);
					StandaardCommand( &ser, argv[1] );
		
				}

//				if( WhichRuns( &ser ) )
//					printf("Whichruns error\n");

//				if( CCopy( &ser, "MP_RA:pages/aim.titling","ALIAS:pages/aim.titling", 524768983 ) )
//					printf("Error in ccopy\n");
//				if( ChangeScript( &ser ) )
//					printf("Error couldn't change script\n");

				if( UpLoadY( &ser, "mp_RA:graphics/pictures/fotoNatuur24sept2","ram:t1",0 ) )
					printf("Error in upload\n");

				if( ser.time )
				{
					sprintf(comm,"Bytes send %d, %d sec. %d b/s",ser.bytessend,ser.time,ser.bytessend/ser.time );
					Report( comm );
				}

//				if( UpLoadY( &ser, "work:kladwerk/mag","ram:t2" ) )
//					printf("Error in upload\n");

/*				if( UpLoadY( &ser, "pics:8bit/EnVogue.256","work:temppic/c2" ) )
					printf("Error in upload\n");
				if( UpLoadY( &ser, "pics:8bit/ray1","work:temppic/c3" ) )
					printf("Error in upload\n");
				if( UpLoadY( &ser, "pics:8bit/ray2","work:temppic/c4" ) )
					printf("Error in upload\n");
*/
/*

				if( GetBigFromRemote( &ser, "ram:bigfile" ) )
					printf("Get bigfile error\n");
*/

				if( Logoff(&ser) )
					printf("logoff failed\n");
			}
			else
				printf("logon Failed\n");
			ExitPrint();
		}
		else
			printf("print functions error\n");
	}
	FreeSerial( &ser );
	SetTaskPri( task,oldpri );

}
void LogAction( char *tt )
{
	printf("log [%s]\n",tt );

}
