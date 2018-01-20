// File		: Serwork.c
// Uses		:
//	Date		: 22 june 1994
// Author	: ing. C. Lieshout
// Desc.		: PLay with the serial device
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serprefs.h"
#include "serprint.h"
#include "sergetdir.h"
#include "protocols.h"
#include "nb:gui_texts.h"
#include "mra:ecp/protos.h"

void LogAction( char *mes );

//#define _PRINT 1

extern UBYTE **msgs;

void ReportError( int i )
{
	char tt[100];
	sprintf(tt,msgs[ Msg_ECP_12 - 1 ],i );
	Report( tt );
}

/******** SystemDate() ********/
/* - input: pointers to day, month, year 
 *
 */

void SystemDate(int *day, int *month, int *year)
{
struct DateStamp ds;
LONG n;
int m,d,y;

	DateStamp(&ds);
	n = ds.ds_Days - 2251;
	y = (4 * n + 3) / 1461;
	n -= 1461 * y / 4;
	y += 1984;
	m = (5 * n + 2) / 153;
	d = n - (153 * m + 2) / 5 + 1;
	m += 3;
	if (m > 12)
	{
		y++;
		m -= 12;
	}
	*month = m;
	*day = d;
	*year = y % 100;
}

/******** SystemTime() ********/
/* - input: pointers to hours, mins, secs 
 *
 */

void SystemTime(int *hours, int *mins, int *secs)
{
struct DateStamp ds;

	DateStamp(&ds);
	*hours = (int)(ds.ds_Minute/60L);
	*mins = (int)(ds.ds_Minute%60L);
	*secs = (int)(ds.ds_Tick/50L);
}

//===============================================
//	Name		: CreateDateString
//	Function	: Return string with current date and time
//	Inputs	: pointer to destination string, pointer to start string
//	Result	: destination is filled with date and time
//	Updated	: 03 - 10 - 1994
//
void CreateDateString( char *dest , char *p)
{
	struct DateStamp ds;
	LONG n;
	int hours,mins,secs,m,d,y;

	DateStamp(&ds);
	n = ds.ds_Days - 2251;
	y = (4 * n + 3) / 1461;
	n -= 1461 * y / 4;
	y += 1984;
	m = (5 * n + 2) / 153;
	d = n - (153 * m + 2) / 5 + 1;
	m += 3;
	if (m > 12)
	{
		y++;
		m -= 12;
	}
	y = y % 100;

	hours = (int)(ds.ds_Minute/60L);
	mins = (int)(ds.ds_Minute%60L);
	secs = (int)(ds.ds_Tick/50L);
	sprintf(dest,"%s %d-%d-%d %02d:%02d:%02d",p,d,m,y,hours,mins,secs );
}

//===============================================
//	Name		: StripAlias
//	Function	: Strip Alias: from filename and insert mp_ra:script1
//	Inputs	: whichscriptrun, pointer naar name
//	Result	: pointer to path plus name
//	Updated	: 29 - 08 - 1994
//
char *StripAlias( int script, char *name, char *d )
{
	char loc[256];
	char al[]="ALIA";

	if( *(LONG*)name == *(LONG*)al )
	{
		strcpy( loc, &name[6] );
		sprintf(d,"MP_RA:script%d/",script );
		strcat(d,loc );
	}
	else
	{
#if _PRINT
		printf("ALIAS just copy\n");
#endif
		strcpy( d, name );
	}
	return( d );
}

char *convertlf( char *d )
{
	int i=0;
	while( d[i] != 0 )
	{
		if( d[i] == 0xd )
			d[i] = '_';
		if( d[i] == 0xa )
			d[i] = '.';

		i++;
	}
	return d;
}

void Dprintf(char *d )
{
#if _PRINT
	convertlf( d );
	printf("%s",d );
#endif
}

//===============================================
//	Name		: CheckReply
//	Function	: Return if command was succesful or not
//	Inputs	: pointer to command & return value
//	Result	: TRUE when result is ok, FALSE otherwise
//	Updated	: 24 - 08 - 1994
//
int CheckReply( char *s, char *d )
{
	int i,r;
	char *rep;

	r = strlen( s ) - 1;			// -1 for the linefeed
	
//	printf("in [%s], uit [%s]\n",convertlf(s),convertlf(d));

	for( i=0; i< r; i++ )
		if( s[i] != d[i] )
			return( 0 );

	rep = strstr(&d[r],"Ok" );
	if( rep == NULL )
		return( 0 );
	else
		return( 1 );
}

int CheckStandaardReturn( SERDAT *ser, char *com, int extra )
{
	char tt[256];

	int ret,Bt,BSend,Breceiv=0;

	BSend = strlen( com ) + extra;

	while( BSend > Breceiv )
	{
		Bt = WaitForReply( ser, tt, 255, BSend,1, 30 );
		if( Bt > 0 )
			Breceiv += Bt;
		else
			break;
	}

	if( Breceiv == BSend )
	{
		if( CheckReply( com, tt ) )
			ret = 0;
		else
			ret = 1;
//	sprintf(tt2,"reply [%s]\n",convertlf(tt));
//	Report( tt2 );

	}

	while( StripSerIn( ser, tt, 255 ) );					// Strip all data stil there
	return( ret );
}

int CheckStandaardReturntime( SERDAT *ser, char *com, int extra, int secs )
{
	char tt[256];

	int ret,Bt,BSend,Breceiv=0;

	BSend = strlen( com ) + extra;

	while( BSend > Breceiv )
	{
		Bt = WaitForReply( ser, tt, 255, BSend,1, secs );
		if( Bt > 0 )
			Breceiv += Bt;
		else
			break;
	}

	if( Breceiv == BSend )
	{
		if( CheckReply( com, tt ) )
			ret = 0;
		else
			ret = 1;
//	sprintf(tt2,"reply [%s]\n",convertlf(tt));
//	Report( tt2 );

	}

	while( StripSerIn( ser, tt, 255 ) );					// Strip all data stil there
	return( ret );
}

int Logon( SERDAT *ser, char *name )
{
	char com[256];
	int oke,i;

	oke = 1;

#if _PRINT
	printf("{Logon}\n");
#endif

	i = 0;

	while( oke && i < 4 )
	{
		i++;

//		sprintf(com,"Logon try %d\n",i );
//		Report( com );

		Delay( 50L );
		while( StripSerIn( ser, com, 255 ) )
		{
#if _PRINT
			Dprintf("Pre Buffer is [");
			Dprintf(com);
			Dprintf("]\n");
#endif
		}
		sprintf(com,"logon %s\xd",name );
		WriteSer( ser, com );
		oke =  CheckStandaardReturn( ser, com, 8 );
	}
	return( oke );
}

int Logoff( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{Logof}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}

	sprintf(com,"logoff\xd");
	WriteSer( ser, com );
	return( CheckStandaardReturn( ser, com, 8 + 14 ) );
}

int WhichRuns( SERDAT *ser )
{
	char *rep,com[256],tt[256];
	int ret,Bt,BSend,Breceiv=0;

	ret = 0;

#if _PRINT
	printf("{WhichRuns}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}

	sprintf(com,"whichruns\xd");
	WriteSer( ser, com );
	BSend = strlen( com ) + 12;

	while( BSend > Breceiv )
	{
		Bt = WaitForReply( ser, tt, 255, BSend, 1, 30 );
		if( Bt > 0 )
			Breceiv += Bt;
		else
			break;
	}

//	printf("Reply YTrans [%s]\n",convertlf(tt) );

	if( strstr(tt, "1" ) )
		ser->pref.run = 1;
	else
		if( strstr(tt, "2" ) )
			ser->pref.run = 2;
		else
			ser->pref.run = 0;

	rep = strstr(tt,"Ok" );

	if( rep == NULL )
		ret = 1;
	else
		ret = 0;

	if( ser->pref.run == 0 )
	{
		Report( msgs[ Msg_ECP_14 - 1 ] );
		ret = 1;
	}

	while( StripSerIn( ser, tt, 255 ) );

#if _PRINT
	printf("Script %d runs\n",ser->pref.run );
#endif

	if( ser->pref.run == 1)
		ser->pref.notrun = 2;
	else
		ser->pref.notrun = 1;

	return( ret );
}

int UpLoadY( SERDAT *ser, char *name, char *outname, long stamp )
{

	char *rep,com[256],tt[256];
	int ret,Bt,BSend,Breceiv=0;

	ret = 0;

#if _PRINT
	printf("{UpLoadY}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}

#if _PRINT
	printf("in [%s],out [%s]\n",name,outname );
#endif

	sprintf(com,"uploady %s\xd",name );

	WriteSer( ser, com );

	if( Y_Transmit( ser, name, outname, stamp ) )				// did receive go oke
		ret = 1;

	BSend = 7;

	if( ret == 0 )
	{
		while( BSend > Breceiv )
		{
			Bt = WaitForReply( ser, tt, 255, BSend,1, 30 );
			if( Bt > 0 )
				Breceiv += Bt;
			else
				break;
		}

//	printf("Reply YTrans [%s]\n",convertlf(tt) );

		rep = strstr(tt,"Ok" );

		if( rep == NULL )
			ret = 1;
		else
			ret = 0;
	}

	while( StripSerIn( ser, tt, 255 ) );

//	printf("Wacht\n");
//	getch();
	return( ret );
}

void ReportName( char *name, int size )
{
	char tt[255];
	char al[]="ALIA";

	if( *(LONG*)name == *(LONG*)al )
		sprintf( tt, msgs[ Msg_ECP_8 - 1 ], &name[6] );
	else
		sprintf( tt, msgs[ Msg_ECP_8 - 1 ], name );

	Report(tt);
}

//===============================================
//	Name		: CCopy
//	Function	: Copy the file to remote
//				: if the file already excists just set the archive bit
//				: otherwise upload the file
//	Inputs	: SERDAT, pointer to sname, dname, datstamp
//	Result	: Zero when succesfull otherwise error
//	Updated	: 29 - 08 - 1994
//
int CCopy( SERDAT *ser, char *sname, char *dname, int stamp, int size )
{
	int ret = 0;
	char tt[256],com[256];

#if _PRINT
	printf("{CCopy} %s,%s,%d,%d\n",sname,dname,stamp,size);
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}

	StripAlias( ser->pref.notrun, dname, tt );

	sprintf(com,"ccopy %s %d\xd", tt, stamp );

	WriteSer( ser, com );

	if( CheckStandaardReturn( ser, com, 8 ) )		// error or oke ?
	{
#if _PRINT
		printf("Need to upload here[%s]\n",convertlf( com ));
#endif

		ReportName( dname, size );
		Report("^");

		ret = UpLoadY( ser, sname, tt, stamp );

		if( ret == 0 )
			if( ser->time )
			{
				long mins,secs;
				secs = ser->time/CLOCKS_PER_SEC;
				mins = secs/60;
				secs = secs-mins*60;

				tt[0] = '%';
				sprintf( &tt[1], msgs[ Msg_ECP_10 - 1 ],
								(ser->bytessend*CLOCKS_PER_SEC)/ser->time,
								ser->bytessend,
								mins, secs );
// sprintf(tt,"~Bytes send %d, %d sec. %d b/s",ser->bytessend,ser->time/CLOCKS_PER_SEC,(ser->bytessend*CLOCKS_PER_SEC)/ser->time );
			}
			else
				sprintf(tt,"~Bytes send %d, %d sec. %d b/s",ser->bytessend,ser->time,ser->bytessend );
			Report( tt );


			if( ser->time )
				sprintf( tt, "%s %d bytes %d b/s\n",sname,ser->bytessend,(ser->bytessend*CLOCKS_PER_SEC)/ser->time );
			else
				sprintf( tt, "%s %d bytes %d b/s\n",sname,ser->bytessend,ser->bytessend );
			LogAction( tt );


		return( ret );
	}
	else
	{
//		ReportName( dname, size );
		return( 0 );
	}
}


//===============================================
//	Name		: ChangeScript
//	Function	: Changes the remote script
//	Inputs	: SERDAT,
//	Result	: Zero when succesfull otherwise error
//	Updated	: 29 - 08 - 1994
//
int ChangeScript( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{ChangeScript}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}
	sprintf(com,"chscript\xd" );
	WriteSer( ser, com );
	return( CheckStandaardReturntime( ser, com, 8, 180 ) );
}

//===============================================
//	Name		: SwapScript
//	Function	: Changes the remote script when finished
//	Inputs	: SERDAT,
//	Result	: Zero when succesfull otherwise error
//	Updated	: 21 - 09 - 1994
//
int SwapScript( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{SwapScript}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}
	sprintf(com,"doswap\xd" );
	WriteSer( ser, com );
	return( CheckStandaardReturn( ser, com, 8 ) );
}

int ChangeAlias( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{ChangeAlias}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}
	sprintf(com,"chalias\xd" );
	WriteSer( ser, com );
	return( CheckStandaardReturn( ser, com, 8 ) );
}

int CopySerScript( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{CopyScript}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}
	sprintf(com,"copyscript\xd" );
	WriteSer( ser, com );
	return( CheckStandaardReturn( ser, com, 8 ) );
}


int StandaardCommand( SERDAT *ser, char *name )
{
	char com[256];

#if _PRINT
	printf("{StandaardCommand}\n");
#endif

	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com);
		Dprintf("]\n");
#endif
	}

	sprintf(com,"%s\xd",name );
	printf("Send stadaard command [%s]\n",com );
	WriteSer( ser, com );

	return( CheckStandaardReturn( ser, com, 8 ) );

}

int GiveConnect( SERDAT *ser )
{
	char tt2[256];
	char tt[256];
	char *p;
	int stop,i,b,c;
	int to = 0;

	c = i = stop = 0;

	while( !stop && i < 255 )
	{
		b = WaitForReply( ser, &tt[i], 1, 1, 0, 30 );
		if( b == -1 )																// Time out
		{
			to++;
			sprintf(tt2, "%s %d",msgs[ Msg_ECP_6 - 1 ], to );
			Report(tt2);

			if( to > 2 )
				stop = 2;
		}
		else
		{
			if( tt[i] == 'C' )
				c = 1;
			if( tt[i] == '\xd' && c )
				stop = 1;
			i++;
		}

		if( ( strstr( tt, "BUSY" ) ) )
			stop = 3;

		if( ( strstr( tt, "NO CARRIER" ) ) )
			stop = 4;
	}

	tt[ i ] = 0;

	sprintf(tt2,"[%s]\n",convertlf( tt ) );
	Report( tt2 );

	if( stop == 1 )
		if( ( p = strstr( tt, ser->pref.replystring ) ) )
			Report("Found reply string");
		else
			stop = 2;

	if( stop == 1 )
		return( 1 );
	else
	{
		if( stop == 2 )
			ReportError( 6 );
		if( stop == 3 )
			ReportError( 7 );
		if( stop == 4 )
			ReportError( 8 );
		return( 0 );
	}
}

int MakeConnection( SERDAT *ser )
{
	char com[256];

#if _PRINT
	printf("{MakeConnection}\n");
#endif
	while( StripSerIn( ser, com, 255 ) )
	{
#if _PRINT
		Dprintf("Pre Buffer is [");
		Dprintf(com  );
		Dprintf("]\n");
#endif
	}

	sprintf(com,"%s%s\xd",ser->pref.dialpref,ser->pref.phonenr );

//printf("send command connection[%s]\n",com );

	WriteSer( ser,com );

	return( GiveConnect( ser ) );
}

void SoundOff( SERDAT *ser )
{
	char com[26];

	WriteSer( ser, "ATm0\xd");
	Delay(25L);
	while( StripSerIn( ser, com, 25 ) )
	{
//		Dprintf("Pre Buffer is [");
//		Dprintf(com  );
//		Dprintf("]\n");
//		Report("sound off ret");
//		Report( com );
	}
//	Delay(150L);
}

void SetLeased( SERDAT *ser )
{
	char com[26];

	WriteSer( ser, "AT&l1\xd");
	Delay(25L);
	while( StripSerIn( ser, com, 25 ) )
	{
//		Dprintf("Pre Buffer is [");
//		Dprintf(com  );
//		Dprintf("]\n");
//		Report("setleased off ret");
//		Report( com );
	}
//	Delay(150L);
}

void HangUp( SERDAT *ser )
{
	char com[26];

	Report( "Send escape string\n");
	WriteSer( ser, "+");
	Delay( 10L);
	WriteSer( ser, "+");
	Delay( 10L);
	WriteSer( ser, "+");
//	Delay( 10L);

	Delay(50L);

	Report( "Send hangup string string\n");
	WriteSer( ser, "ATH0\xd");
	Delay(50L);
	while( StripSerIn( ser, com, 25 ) )
	{
//		Dprintf("Pre Buffer is [");
//		Dprintf(com  );
//		Dprintf("]\n");
//		Report("setleased off ret");
//		Report( com );
	}
//	Delay(150L);
}
