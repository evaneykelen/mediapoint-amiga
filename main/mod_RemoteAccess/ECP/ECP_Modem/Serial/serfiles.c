// File		: Serfiles.c
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
#include <dos.h>
#include <dos/dos.h>

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serprefs.h"
#include "sergetdir.h"
#include "serfiles.h"
#include "protocols.h"

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
//	Inputs	: Serdat, pointer to name, pointer to size, pointer to datestamp
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int GetDateSize( SERDAT *ser, char *sname, FileInfo *fi )
{
	int ret = 0;

	struct Library *DOSBase;

	struct FileInfoBlock *Finfo;
	BPTR	lock;

	DOSBase  = ser->DOSBase;

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
//	Inputs	: Serdat, pointer to name
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int icopy( SERDAT *ser, char *sname, char *dname )
{
	long volsize;
	int r,ret = 0;
	BPTR	ifile,ofile;
	unsigned char *mem;
	FileInfo fi;

	struct Library *DOSBase;

	GetDateSize( ser, sname, &fi );
	DOSBase  = ser->DOSBase;

	volsize = GetVolumeSize( "MP_RA:" );

//	printf("Volume size is [%s] %ld, %ld\n",ser->dirname,volsize,fi.size );

	if( (fi.size + BUFFER_REMAIN ) < volsize )
	{
		mem = AllocMem( COPY_BUF_SIZE, MEMF_ANY );

		if( mem )
		{

			ifile = Open( sname, MODE_OLDFILE );
			ofile = Open( dname, MODE_NEWFILE );

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
			SetFileDate( dname, &fi.ds );
			SetProtection( dname, fi.prot & ~FIBF_ARCHIVE );	// copy the protbits

		}
	}
	else
		ret = 3;

	return( ret );

}

//===============================================
//	Name		: ccopy
//	Function	: Check if the file has strstamp datestamp
//	Inputs	: Serdat, pointer to name
//	Result	: 0 by succes other error
//	Updated	: 29 - 08 - 1994
//
int ccopy( SERDAT *ser, char *sname, char *strstamp )
{
	FileInfo fi;
	long i,stamp;

	struct Library *DOSBase;

	if( GetDateSize( ser, sname, &fi ) )
	{
#if _PRINT
		printf("file does not excist\n");
#endif
		return( 1 );
	}

	DOSBase  = ser->DOSBase;
	sscanf( strstamp, "%d",&stamp );

#if _PRINT
	printf("Stamp in %d, file %d\n",stamp, fi.stamp );
#endif

	if( stamp != fi.stamp )
		return( 2 );

	if( fi.archive )
	{
#if _PRINT
		printf("Archive already on\n");
#endif
		return( 0 );
	}

	i = SetProtection( sname, fi.prot | FIBF_ARCHIVE );

	if( i == 0 )
	{
		printf("protection failed\n");
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
		printf("Create dir failed\n");
		return( NULL );
	}

	UnLock( lock );

	f = Open( name, MODE_NEWFILE );
	if( f )
		return( f );
	else
		return( NULL );
	
}

int ChangeDirectory( char *name, char *path )
{
	BPTR lock;

	lock = Lock( name ,ACCESS_READ );
	if( lock )
	{
		CurrentDir( lock );
		getpath( lock, path );
		return( TRUE );
	}
	return( FALSE );
}
