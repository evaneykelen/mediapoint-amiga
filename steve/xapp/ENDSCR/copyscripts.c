// File		: copyscripts.c
// Uses		:
//	Date		: 21 sept 1994
// Autor		: C. Lieshout
// Desc.		: Copy script
//				: Procedures from serial host

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/datetime.h>
#include <exec/types.h>
#include <exec/memory.h>

#define BUFFER_REMAIN 100000
#define COPY_BUF_SIZE 10240

typedef struct
{
	long size;
	struct DateStamp ds;
	long stamp;
	long prot;
	UBYTE archive;
	UBYTE temp;

}FileInfo;

//===============================================
//	Name		: GetDateSize
//	Function	: Get the size and dat from a file
//	Inputs	:
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int GetDateSize( char *sname, FileInfo *fi, struct Library *DOSBase )
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

LONG GetVolumeSize( STRPTR path )
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
//	Name		: icopy
//	Function	: copy file from sname naar dname
//				: keep date the same ( copy CLONE )
//	Inputs	: Serdat, pointer to name
//	Result	: 0 by succes other error
//	Updated	: 23 - 08 - 1994
//
int icopy( char *sname, char *dname,struct Library *DOSBase )
{
	long volsize;
	int r,ret = 0;
	BPTR	ifile,ofile;
	unsigned char *mem;
	FileInfo fi;

//	KPrintF("icopy [%s],[%s]\n",sname,dname );

	GetDateSize( sname, &fi, DOSBase );

	volsize = GetVolumeSize( "MP_RA:" );

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
//	Name		:	check_arch
// Function	:	Clear the archive bit if it's on
//	Inputs	:	Pointer to a ExAllDate struct, long indicating current day
//	Result	:	None
// UpDated	:	29-08-94
//	
void check_arch( struct ExAllData *ead, long cday, char *path, FILE *f, struct Library *DOSBase )
{
	char fpath[256];

	if( ead->ed_Type == -3 )
	{
		if( ead->ed_Prot & FIBF_ARCHIVE )
		{
			MakeFullPath( path, ead->ed_Name, fpath );
//			printf("Clear archive bit from [%s]\n",fpath );

//			KPrintF("Clear archive\n");

			SetProtection( fpath, ead->ed_Prot & ~FIBF_ARCHIVE );
		}
	}
}

//===============================================
//	Name		:	copy_new_script
// Function	:	Copy file from script1 to script2
//				:	Also delete file from script1 that is't archived
//	Inputs	:	Pointer to a ExAllDate struct, long indicating current day
//	Result	:	None
// UpDated	:	29-08-94
//	
void copy_new_script( struct ExAllData *ead, long cday, char *path, FILE *f, struct Library *DOSBase )
{
	char *tt,spath[256],dpath[256];
	FileInfo fi;
	int cp = 0;
	long stamp;

	if( ead->ed_Type == -3 )
	{
		MakeFullPath( path, ead->ed_Name, spath );

		if( ead->ed_Prot & FIBF_ARCHIVE )
		{
			strcpy(dpath,spath );
			tt = strstr(dpath, "script" );
			if( tt )
			{
				if( tt[ 6 ] == '1' )
					tt[6] = '2';
				else
					tt[6] = '1';
				if( GetDateSize( dpath, &fi, DOSBase ) )
					cp = 1;											// file not there
				else
				{
					stamp = ead->ed_Days*(3600*24)+ead->ed_Mins*60 + ead->ed_Ticks/ TICKS_PER_SECOND;
					if( stamp != fi.stamp )
						cp = 1;
					else
					{
//						printf("File oke [%s]\n",spath );
						if( fi.archive )
							SetProtection( dpath, fi.prot & ~FIBF_ARCHIVE );	// clear the archive bit
					}
				}

				if( cp )
				{
//					printf("Copy file from [%s] to [%s]\n",spath, dpath );
					icopy( spath, dpath, DOSBase );
				}
				
			}
			else
				printf("Error in path no script found(1/2)\n");
		}
		else
		{
//			printf("Delete file [%s]\n",spath );
			DeleteFile( spath );
		}
	}
}

//===============================================
//	Name		:	get_file_names
//	Function	:	Retrieve filenames from a given path
//	Inputs	:	pointer to path name, pointer to output file
//	Result	:
//	Updated	:	19-09-94
//
int get_file_names( char *path, FILE *f, void (*action)( struct ExAllData *ead, long cday, char *path, FILE *f, struct Library *DOSBase ),struct Library *DOSBase )
{
	struct ExAllControl *eac;
	struct ExAllData *ead;
	struct DateStamp cdate;				// current date
	long cday;
	char localpath[200];					// space for total path name

	long *EAData;
	BPTR	lock;
	int more;

	lock = Lock( path, ACCESS_READ );
	if( lock != NULL )
	{
		cday = 0;
		DateStamp( &cdate );				// get current date

		if( cday == 0 )
			cday = 1<< (sizeof(long) -1 );

		EAData = ( long *)AllocMem( sizeof(struct ExAllData )*100,MEMF_PUBLIC );
		if( EAData != NULL )
		{
			eac = AllocDosObject(DOS_EXALLCONTROL,NULL);
			if (!eac)
			{
				printf("Alloc dos failed\n");
				exit(0);
			}
			eac->eac_LastKey = 0;
			eac->eac_MatchString = NULL;
			eac->eac_MatchFunc = NULL;
			do
			{
				more = ExAll(lock, ( struct ExAllData *)EAData, sizeof(struct ExAllData )*100, ED_DATE, eac);
				if ((!more) && (IoErr() != ERROR_NO_MORE_ENTRIES))
				{
					printf("Exall failed\n");
					break;
				}
				if (eac->eac_Entries == 0)
				{
//					printf("ExAll failed normally with no entries\n");
					break;
				}
				ead = (struct ExAllData *) EAData;
				do
				{

					(*action)( ead, cday,path,f, DOSBase );

					strcpy( localpath, path );
					if( localpath[ strlen( localpath )-1] != ':' )
						strcat(localpath,"/" );
					strcat(localpath,ead->ed_Name );
					if( ead->ed_Type == 2)				// deep scan
					{
						strcpy( localpath, path );				// copy global to local
						if( localpath[ strlen( localpath )-1] != ':' )
							strcat(localpath,"/" );
						strcat(localpath,ead->ed_Name );

						get_file_names( localpath, f,action, DOSBase );
					}
					ead = ead->ed_Next;
				} while( ead );
  	  		} while( more );
		   FreeDosObject(DOS_EXALLCONTROL,eac);
			if( EAData )
				FreeMem( EAData, sizeof(struct ExAllData ) *100 ); 
		}
		UnLock( lock );
	}
	else
	{
		printf("Cannot Lock dir %s\n",path );
		return( 1 );
	}

	return( 0 );
}

//===============================================
//	Name		:	ClearArchive
//	Function	:	Clear the archive bit of path path
//	Inputs	:	pointer to path name
//	Result	:	0 succes other error
//	Updated	:	29-08-94
//
int ClearArchiveEND( char *path, struct Library *DOSBase )
{
	int ret = 0;

	if( get_file_names( path, NULL, check_arch, DOSBase ) )
		ret = 1;

	return( ret );
}

//===============================================
//	Name		:	CopyNewScriptEnd
//	Function	:	Copy the archived file delete the others
//	Inputs	:	pointer to path name
//	Result	:	0 succes other error
//	Updated	:	29-08-94
//
int CopyNewScriptEND( char *path, struct Library *DOSBase )
{
	int ret = 0;

	if( get_file_names( path, NULL, copy_new_script, DOSBase ) )
		ret = 1;

	return( ret );
}
