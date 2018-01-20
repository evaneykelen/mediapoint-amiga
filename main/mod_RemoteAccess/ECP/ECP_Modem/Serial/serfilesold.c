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
#include <dos/dos.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <stdio.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */
void main(void);
#endif

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serprefs.h"
#include "sergetdir.h"
#include "protocols.h"


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
			return(0);
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
	struct Library *DOSBase;

	struct FileInfoBlock *Finfo;
	BPTR	lock;

	DOSBase  = ser->DOSBase;

	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );
	fi->ds.ds_Days = 0;
	fi->ds.ds_Minute = 0;
	fi->ds.ds_Tick = 0;

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
			UnLock( lock );
		}
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );
	}
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
	long volsize,filesize;
	int r,ret = 0;
	BPTR	ifile,ofile;
	unsigned char *mem;
	struct DateStamp ds;
	struct Library *DOSBase;
	struct FileInfoBlock *Finfo;
	BPTR	lock;

	DOSBase  = ser->DOSBase;
	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );
	ds.ds_Days = 0;
	ds.ds_Minute = 0;
	ds.ds_Tick = 0;

	if( Finfo )
	{
		lock = Lock( sname, ACCESS_READ );
		if( lock != NULL )
		{
			Examine( lock, Finfo );
			filesize = Finfo->fib_Size;

			ds.ds_Days = Finfo->fib_Date.ds_Days;
			ds.ds_Minute = Finfo->fib_Date.ds_Minute;
			ds.ds_Tick = Finfo->fib_Date.ds_Tick;
			UnLock( lock );
		}
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );
	}

	volsize = GetVolumeSize( ser->pref.dirname );

	printf("Volume size is %ld, %ld\n",volsize,filesize );

	if( (filesize + BUFFER_REMAIN ) < volsize )
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
			SetFileDate( dname, &ds );
	}
	else
		ret = 3;

	return( ret );

}
