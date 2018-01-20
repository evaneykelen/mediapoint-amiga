// File		: Netfiles.c
// Uses		:
//	Date		: 23 august 1994
// Author	: ing. C. Lieshout & E. van Eykelen
// Desc.		: Functions for copying and handling file
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include "nb:gui_texts.h"
#include "mra:ecp/protos.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "NetFiles.h"

extern UBYTE **msgs;

void ReportError( int i )
{
	char tt[100];
	sprintf(tt,msgs[ Msg_ECP_12 - 1 ],i );
	Report( tt );
}

//===============================================
//	Name		: CreateSwap
//	Function	: Create to swap file in dir path
//	Inputs	: pointer to path
//	Result	:
//	Updated	: 06 - 10 - 1994
//
int CreateSwap( char *path )
{
	char str[256];
	FILE *f;	

	MakeFullPath( path, "swap",str );

	f = fopen( str, "w");
	if( f )
	{
		fwrite( "SWAP", 4, 1, f );
		fclose( f );
		return( 0 );
	}
	return( 1 );
}

//===============================================
//	Name		: CreateCommand
//	Function	: Create the command file at path
//				  Set the functions chscript and copyscript
//	Inputs	: pointer to path
//	Result	:
//	Updated	: 06 - 10 - 1994
//
int CreateCommand( char *path )
{
	char str[256];
	FILE *f;	

	MakeFullPath( path, "command",str );

	f = fopen( str, "w");
	if( f )
	{
		fwrite( "chscript\n", strlen("chscript\n"), 1, f );
		fwrite( "copyscript\n", strlen("copyscript\n"), 1, f );
		fclose( f );
		return( 0 );
	}
	return( 1 );
}

//===============================================
//	Name		: StripAlias
//	Function	: Strip Alias: from filename and insert path/script 1/2
//	Inputs	: whichscriptrun, pointer naar name
//	Result	: pointer to path plus name
//	Updated	: 29 - 08 - 1994
//
char *StripAlias( int script, char *name, char *d, char *path )
{
	char loc[256];
	char tname[256];

	char al[]="ALIA";

	if( *(LONG*)name == *(LONG*)al )
	{
		strcpy( loc, &name[6] );
		sprintf(tname,"script%d/%s", script, loc );
		MakeFullPath( path, tname, d );
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

//===============================================
//	Name		: NetSayRun
//	Function	: Check which script runs
//	Inputs	: pointer to path
//	Result	: 1 or 2 or 0 when file not found
//	Updated	: 19 - 09 - 1994
//
int NetSayRun ( char *path )
{
	char str[256];
	FILE *f;	
	int check=0;

	MakeFullPath( path, "RUNS_script.1",str );
	f = fopen( str, "r");

	if( f )
	{
		check =1;
		fclose( f );
	}

	if( check ==  0 )
	{
		MakeFullPath( path, "RUNS_script.2",str );
		f = fopen( str, "r");
		if( f )
		{
			check =2;
			fclose( f );
		}
	}
	return( check );
}

void ConvertIntToStamp( int i, struct DateStamp *ds )
{
	int d;

	d = i / ( 3600*24 );
	i -= d * ( 3600*24);
	ds->ds_Days = d;
	d = i / 60;
	i -= d * 60;
	ds->ds_Minute = d;
	ds->ds_Tick = i * TICKS_PER_SECOND;
}

void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest ) 
{
  int PathLength;

	PathLength = strlen(Path);
	if(Path[PathLength-1] == ':' || Path[PathLength-1] == '/')
	{
		strcpy(Dest,Path);
		strcat(Dest,Name);
	}
	else 
	{
		if(Path != NULL && Path[0] != '\0')
		{
			strcpy(Dest,Path);
			strcat(Dest,"/");
			strcat(Dest,Name);
		}
		else
			strcpy(Dest, Name);
	}	
}

LONG GetVolumeSize(STRPTR path)
{
	int error;
	struct InfoData __aligned info;

	if ( !strnicmp(path,"ram",3) )	// this is RAM: or RAM DISK:
		return( (LONG)AvailMem(MEMF_PUBLIC) );
	else
	{
		error = getdfs( path, &info );
		if ( error!=0 )
			return( 0 );
		else
			return( (info.id_NumBlocks - info.id_NumBlocksUsed ) * info.id_BytesPerBlock );
	}
}

//===============================================
//	Name		: GetDateSize
//	Function	: Get the size and dat from a file
//	Inputs	: pointer to name, pointer to size, pointer to datestamp
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int GetDateSize( char *sname, FileInfo *fi )
{
	int ret = 0;

	struct FileInfoBlock *Finfo;
	BPTR	lock;

	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );
	fi->size = 0;
	fi->ds.ds_Days = 0;
	fi->ds.ds_Minute = 0;
	fi->ds.ds_Tick = 0;
	fi->stamp = 0;

	if( Finfo )
	{
		lock = Lock( sname, ACCESS_READ );
		if( lock != NULL )
		{
			Examine( lock, Finfo );
			fi->size = Finfo->fib_Size;
			fi->ds.ds_Days = Finfo->fib_Date.ds_Days;
			fi->ds.ds_Minute = Finfo->fib_Date.ds_Minute;
			fi->ds.ds_Tick = Finfo->fib_Date.ds_Tick;
			fi->stamp = fi->ds.ds_Days*(3600*24)+fi->ds.ds_Minute*60 + fi->ds.ds_Tick/ TICKS_PER_SECOND;
			fi->prot = Finfo->fib_Protection;
			if( Finfo->fib_Protection & FIBF_ARCHIVE )
				fi->archive = 1;
			else
				fi->archive = 0;
			UnLock( lock );
		}
		else
			ret = 1;
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );
	}
	else
		ret = 2;

	return ret;
}

//===============================================
//	Name		: icopy
//	Function	: copy file from sname naar dname
//				: keep date the same ( copy CLONE )
//	Inputs	: pointer to name
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int icopy( char *sname, char *dname, int stamp )
{
	long volsize;
	int r,ret = 0,i;
	BPTR ifile,ofile;
	unsigned char *mem;
	FileInfo fi;
	struct DateStamp ds;
	TEXT volName[40];

	GetDateSize( sname, &fi );

	i=0;
	volName[0]='\0';
	while( i<40 && dname[i]!=':' )
	{	
		volName[i]=dname[i];
		i++;
	}
	volName[i  ]=':';
	volName[i+1]='\0';

	volsize = GetVolumeSize( volName );	// WAS "MP_RA:"

//{ char dbug[100]; sprintf(dbug, "Volume size is [%s] %ld, %ld\n",dname,volsize,fi.size ); KPrintF(dbug); }

	if( (fi.size + BUFFER_REMAIN ) < volsize )
	{
		mem = AllocMem( COPY_BUF_SIZE, MEMF_ANY );

		if( mem )
		{

			ifile = Open( sname, MODE_OLDFILE );
			ofile = TryToCreate( dname );

			if( ifile != NULL && ofile != NULL )
			{
				while( ( r = Read( ifile, mem, COPY_BUF_SIZE )) )
					Write( ofile,mem,r );
			}
			else
				ret = 1;
		
			if( ifile )
				Close( ifile );
			if( ofile )
				Close( ofile );

			FreeMem( mem, COPY_BUF_SIZE );
		}
		else
			ret = 2;

		if( ret == 0 )
		{
			ConvertIntToStamp(stamp, &ds);

// Don't filedat if you are using the AmigaTalk network

			if( stamp > 0 )
				SetFileDate( dname, &ds );
//

			SetProtection( dname, fi.prot | FIBF_ARCHIVE );	// copy the protbits
		}
	}
	else
		ret = 3;

	return( ret );

}

//===============================================
//	Name		: ccopy
//	Function	: Check if the file has strstamp datestamp
//	Inputs	: pointer to name
//	Result	: 0 by succes other error
//	Updated	: 29 - 08 - 1994
//
int ccopy( char *sname, int stamp )
{
	FileInfo fi;
	long i;

	if( GetDateSize( sname, &fi ) )
		return( 1 );

	if( stamp != fi.stamp )
		return( 2 );

// Check if you are using the AmigaTalk network
//	if( stamp >= fi.stamp )
//		return( 2 );
// 

	if( fi.archive )
		return( 0 );

	i = SetProtection( sname, fi.prot | FIBF_ARCHIVE );

	if( i == 0 )
	{
//		printf("protection failed\n");
		return( 1 );			// protection failed
	}

	return( 0 );
}

//===============================================
//	Name		: TryToCreate
//	Function	: Try to create a file even if with directory
//	Inputs	: pointer to name
//	Result	: Pointer to BPTR or NULL
//	Updated	: 04 - 09 - 1994
//
BPTR TryToCreate( char *name )
{
	int l,i;

	char tt[256];

	BPTR	lock;
	BPTR f;

	f = Open( name, MODE_NEWFILE );
	if( f )
		return( f );										// Is the directory present ?

#if _PRINT
	printf("Dir scan [%s]\n",name );
#endif

	l = i = strlen( name );

	while( i > 0 )
		if( name[i] == '/' )
			break;
		else
			i--;

	if( i == 0 )
		return( NULL );
	strncpy( tt, name, i );
	tt[ i ] = 0;
#if _PRINT
	printf("Create directory [%s]\n",tt );
#endif

	if( !( lock = CreateDir( tt ) ) )
	{
//		printf("Create dir failed\n");
		return( NULL );
	}

	UnLock( lock );

	f = Open( name, MODE_NEWFILE );
	if( f )
		return( f );
	else
		return( NULL );
	
}
//===============================================
//	Name		: CreateDateString
//	Function	: Return string with current date and time
//	Inputs	: pointer to destination string, pointer to start string
//	Result	: destination is filled with date and time
//	Updated	: 03 - 10 - 1994
//
void CreateDateString( char *dest , char *p )
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

