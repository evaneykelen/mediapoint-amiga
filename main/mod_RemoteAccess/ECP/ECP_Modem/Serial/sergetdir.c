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
#include <dos/dos.h>
#include <dos/exall.h>
#include <dos/datetime.h>
#include <exec/types.h>
#include <exec/memory.h>

#include "serhost.h"
#include "serfiles.h"
#include "sergetdir.h"

#define MAX_TICKS ( TICKS_PER_SECOND * 60 )
#define MAX_MINS ( 60*24 )

/*
void main( int argc, char *argv[])
{
	if( argc >=3 )
	{
//		ListFiles( argv[1],argv[2] );
//		ClearArchive( argv[1],1 );
		CopyNewScript( "ram:script1",1 );
	}
	else
		printf("Usage %s pathname filename\n",argv[0]);
}
*/

//===============================================
//	Name		:	get_day
//	Function	:	Get the day as internal format, optional print the date
//	Inputs	:	If print = 1 then print the date
//	Result	:	long day
//	Updated	:	19-09-94
//
long get_day( void )
{
	struct DateTime ctime;

	DateStamp( &ctime.dat_Stamp );		// get current date
	return ctime.dat_Stamp.ds_Days;
}

//===============================================
//	Name		:	check_arch
// Function	:	Clear the archive bit if it's on
//	Inputs	:	Pointer to a ExAllDate struct, long indicating current day
//	Result	:	None
// UpDated	:	29-08-94
//	
void check_arch( struct ExAllData *ead, long cday, char *path, FILE *f, SERDAT *ser )
{
	char fpath[256];

	if( ead->ed_Type == -3 )
	{
		if( ead->ed_Prot & FIBF_ARCHIVE )
		{
			MakeFullPath( path, ead->ed_Name, fpath );
//			printf("Clear archive bit from [%s]\n",fpath );
			SetProtection( fpath, ead->ed_Prot & ~FIBF_ARCHIVE );
		}
	}
}

//===============================================
//	Name		:	copy_new_script
// Function	:	Copy file from script1 to script2
//				:	Also delete file from script1 that don't archived
//	Inputs	:	Pointer to a ExAllDate struct, long indicating current day
//	Result	:	None
// UpDated	:	29-08-94
//	
void copy_new_script( struct ExAllData *ead, long cday, char *path, FILE *f, SERDAT *ser )
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
				if( GetDateSize( ser, dpath, &fi ) )
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
					icopy( ser, spath, dpath );
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
//	Name		:	print_date
// Function	:	Print the date of a ExAllDate struct is it is valid (stdout )
//	Inputs	:	Pointer to a ExAllDate struct, long indicating current day
//	Result	:	None
// UpDated	:	19-09-94
//	
void print_date( struct ExAllData *ead, long cday, char *path, FILE *f, SERDAT *ser )
{
	int i;

	i=0;

	while(path[i] != ':' )i++;

	if( ead->ed_Type == -3 )
	{
		if( path[i+1] == 0 )
			fprintf(f, "\"%s\"",ead->ed_Name );
		else
			fprintf(f, "\"%s/%s\"",&path[i+1],ead->ed_Name );

		fprintf(f," \"%d\"",ead->ed_Size);

		if ( 	ead->ed_Days > cday ||
				ead->ed_Mins > MAX_MINS ||
				ead->ed_Ticks > MAX_TICKS )
			fprintf(f,",0\n" );
		else
			fprintf(f," \"%ld\"\n",ead->ed_Days*(3600*24)+(ead->ed_Mins*60)+ead->ed_Ticks/TICKS_PER_SECOND );
	}
}

//===============================================
//	Name		:	get_file_names
//	Function	:	Retrieve filenames from a given path
//	Inputs	:	pointer to path name, pointer to output file
//	Result	:
//	Updated	:	19-09-94
//
int get_file_names( char *path, FILE *f, SERDAT *ser, void (*action)( struct ExAllData *ead, long cday, char *path, FILE *f, SERDAT *ser ) )
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
		cday = get_day();
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

					(*action)( ead, cday,path,f, ser );

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

						get_file_names( localpath, f,ser, action );
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
//	Name		:	ListFiles
//	Function	:	Retrieve filenames from a given path
//				:	and copy to file
//	Inputs	:	pointer to path name
//	Result	:
//	Updated	:	19-08-94
//
int ListFiles( SERDAT *ser, char *path, char *filename )
{
	FILE *f;
	int ret = 0;

	f = fopen(filename,"w" );
	if( f )
	{
		if( get_file_names( path, f, ser , print_date ) )
			ret = 1;

		fclose( f );
	}
	else
		ret = 2;

	return( ret );
}

//===============================================
//	Name		:	ClearArchive
//	Function	:	Clear the archive bit of path path
//	Inputs	:	pointer to path name
//	Result	:	0 succes other error
//	Updated	:	29-08-94
//
int ClearArchive( SERDAT *ser, char *path )
{
	int ret = 0;

	if( get_file_names( path, NULL, ser , check_arch ) )
		ret = 1;

	return( ret );
}

//===============================================
//	Name		:	CopyNewScript
//	Function	:	Copy the archived file delete the others
//	Inputs	:	pointer to path name
//	Result	:	0 succes other error
//	Updated	:	29-08-94
//
int CopyNewScript( SERDAT *ser, char *path )
{
	int ret = 0;

	if( get_file_names( path, NULL, ser , copy_new_script ) )
		ret = 1;

	return( ret );
}
