// File		: readdir.c
// Uses		:
// Date		: 24 april 1993
// Autor	: C. Lieshout
// Desc.	: Read the directory structure from a give path and store it
//

#include "nb:pre.h"
#include "protos.h"
#include "deepscan.h"
#include "structs.h"

extern UBYTE **msgs;
extern FILE *logfile;

/**** LOCAL FUNCTIONS ****/

STATIC void clear_list( DISK_LIST *head );
STATIC void place_in_list( DISK_LIST *head, char *name );
STATIC int search_disks( DISK_LIST *head, char *string, char *dest, mempool *pool, int type );
STATIC long get_file_names( char *path, int print , mempool *pool );
STATIC void store_entry( directory *dir, mempool *pool );
STATIC int find_in_list( mempool *pool, long p, char *dirName, char *findStr, char *destStr, int type );
STATIC BOOL RightType(STRPTR str, int type);

/**** LOCAL GLOBALS ****/

STATIC char dat[500];										// space for var structs dir/file
STATIC DISK_LIST headdisk;
STATIC mempool mainpool;

/**** FUNCTIONS ****/

/*********************** ERIK'S BIJDRAGE AAN DE FEESTVREUGDE ****************/

/******** OpenDiskList() ********/

void OpenDiskList(void)
{
	headdisk.next = NULL;
}

/******** CloseDiskList() ********/

void CloseDiskList(void)
{
	clear_list( headdisk.next );
}

/******** OpenScanMem() ********/

BOOL OpenScanMem(void)
{
ULONG safe_largest=0L;

	safe_largest = AvailMem(MEMF_PUBLIC|MEMF_LARGEST);

	/**** check if after alloc at least 50k remains and if buffer is at least 150k ****/

	if ( ((safe_largest-100000) < 50000) || (safe_largest < 150000) )
		return(FALSE);

	if ( safe_largest > POOL_SIZE )
		safe_largest = POOL_SIZE;

	mainpool.pool			= (char *)AllocMem( safe_largest, MEMF_PUBLIC );
	mainpool.size			= POOL_SIZE;
	mainpool.pointer	= 0;
	if ( mainpool.pool==NULL )
		return(FALSE);
	return(TRUE);
}

/******** CloseScanMem() ********/

void CloseScanMem(void)
{
	if (mainpool.pool!=NULL)
		FreeMem(mainpool.pool,mainpool.size);
}

/******** PlaceInList() ********/

void PlaceInList(STRPTR volume)
{
	place_in_list( &headdisk, volume );
}

/******** FindNameInList() ********/

BOOL FindNameInList(STRPTR oriPath, STRPTR name, int type)
{
struct FileLock *FL;
TEXT report[256];

	FL = (struct FileLock *)Lock((STRPTR)oriPath, (LONG)ACCESS_READ);
	if (FL)
	{
		UnLock((BPTR)FL);
		return(TRUE);	// why bother?
	}	

	if( search_disks( headdisk.next, name, oriPath, &mainpool, type ) )
		return(TRUE);

	if ( logfile )
	{
		// **WARNING** Unable to find file ...
		// To screen:
		sprintf(report, msgs[Msg_SM_32-1], name);
		Report(report);
		// To file:
		//fprintf(logfile,"DS ");
		fprintf(logfile,report);
		fprintf(logfile,"\n");
	}

	return(FALSE);
}

/*********************** CEES Z'N BIJDRAGE AAN DE FEESTVREUGDE ****************/

//===============================================
//	Name		:	clear_list
//	Function	:	free the list recursively
//	Inputs	:	pointer to headlist
//	Result	:	the list is freed
//	Updated	:	27-04-39
//
STATIC void clear_list( DISK_LIST *head )
{
DISK_LIST *keep_head;

	keep_head = head;
	while( head = keep_head->next )
	{
		FreeMem(keep_head , sizeof( DISK_LIST ));
		keep_head = head;
	}
	FreeMem(keep_head, sizeof( DISK_LIST ));
}

//===============================================
//	Name		:	place_in_list
//	Function	:	put a name at the bottom of the list
//	Inputs	:	pointer to a the head of a disklist and a name
//	Result	:	entry is placed at bottom of the list
//	Updated	:	27-04-93
//
STATIC void place_in_list( DISK_LIST *head, char *name )
{
	DISK_LIST *Tp;

	if( head == NULL )
		;	//KPrintF("place_in_list failure 1\n");
	else
	{
		Tp = head;
		while( Tp->next != NULL)				// find last entry
			Tp = Tp->next;

		Tp->next = ( DISK_LIST *)AllocMem( sizeof( DISK_LIST ) , MEMF_PUBLIC );
		if( Tp->next == NULL )
			;	//KPrintF("place_in_list failure 2\n");
		else
		{
			strcpy( Tp->next->name, name );
			Tp->next->pointer = -1;
			Tp->next->next = NULL;
		}
	}
}

//===============================================
//	Name		:	search_disks
//	Function	:	search for the string at all names pointed at by head
//	Inputs	:	head struct , search string, destination string
//	Result	:	TRUE if found, and string set FALSE otherwise
//	Updated	:	26-04-93
//
STATIC int search_disks( DISK_LIST *head, char *string, char *dest, mempool *pool, int type )
{
	int found=0;
	DISK_LIST *Tpointer;
	TEXT report[256];

	Tpointer = head;

	// Searching for file ...
	sprintf(report,msgs[Msg_SM_20-1],string);
	Report(report);

	while( !found && Tpointer != NULL )
	{
		if( Tpointer->pointer == -1 )				// the disk needs scanning
		{
			// Scanning volume ...
			sprintf(report,msgs[Msg_SM_21-1],Tpointer->name);
			Report(report);

			Tpointer->pointer = get_file_names( Tpointer->name, 0, pool );
		}

		if ( find_in_list(pool,Tpointer->pointer,Tpointer->name,string,dest,type) )
		{
			found = 1;
		}
		else
		{
#if 0
			if ( logfile )
			{
				// **ERROR** Unable to find file ...
				sprintf(report, msgs[Msg_SM_32-1], string);
				Report(report);
				fprintf(logfile,report);
				fprintf(logfile,"\n");
				//numErrors++;
			}
#endif
		}
		Tpointer = Tpointer->next;
	}
	return( found );
}

//===============================================
//	Name		:	get_file_names
//	Function	:	Retrieve a filenames from a given path
//	Inputs	:	pointer to path name
//	Result	:	store the data in the mempool
//	Updated	:	26-04-93 ( 1.3 version )
//
STATIC long get_file_names( char *path, int print , mempool *pool )
{
	char localpath[200];					// space for total path name
	long oldpointer;
	long returnpointer=-1;
	long preffilepointer = -1;
	directory	*dir;
	files			*file;
	int more_entries = 1;
	SHORT first = 0;

	BPTR	lock;
	struct FileInfoBlock *Finfo;

	returnpointer = pool->pointer;

	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );

	if( Finfo != NULL )
	{
		lock = Lock( path, ACCESS_READ );
		if( lock != NULL )
		{
			Examine( lock, Finfo );
			while( more_entries )
			{
				if( Finfo->fib_DirEntryType > 0 )				// deep scan
				{
					if( first > 0  )								// don't deep the first entry 
					{
						if( preffilepointer != -1 )			// store in list
						{
							dir = ( directory *)(pool->pool+preffilepointer );
							dir->next = pool->pointer;
						}
						preffilepointer = pool->pointer;

						oldpointer = pool->pointer;
						dir = ( directory *)dat;
						dir->type = TYPE_DIR;
						dir->next=0;
						dir->sub=-1;
						strcpy(dir->name,Finfo->fib_FileName );
						store_entry( dir, pool );

						dir = ( directory *)(pool->pool + oldpointer );

						strcpy( localpath, path );				// copy global to local
						if( localpath[ strlen( localpath )-1] != ':' )
							strcat(localpath,"/" );
						strcat( localpath,Finfo->fib_FileName );
						dir->sub = get_file_names( localpath, print, pool );
						first++;
					}
					else
						first++;
				}
				else
				{
					if( preffilepointer != -1 )		// store in list
					{
						file = ( files *)(pool->pool+preffilepointer );
						file->next = pool->pointer;
					}
					preffilepointer = pool->pointer;
					file = ( files *)dat;
					file->type = TYPE_FILE;
					file->next=0;
					strcpy(file->name,Finfo->fib_FileName );
					store_entry( ( directory *)file, pool );
					first++;
				}
				more_entries = ExNext( lock, Finfo );
			}
			UnLock( lock );
		}
/*
		else
			printf("Cannot Lock %s\n",path );
*/
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
	}
/*
	else
		printf("Memory error : can't allocate fileinfoblock\n");
*/

	if( preffilepointer != -1 )		// store in list
	{
		file = ( files *)(pool->pool+preffilepointer );
		file->next = -1;
	}
	if( first <= 1 )
		returnpointer=-1;

	return returnpointer;
}

//===============================================
//	Name		:	store_entry
//	Function	:	store a directory structure in the memory pool
//	Inputs	:	pointer to directory struct, to mempool struct
//	Result	:	none
//	Updated	:	24-04-93
//
STATIC void store_entry( directory *dir, mempool *pool )
{
	int len;
	directory *tdir;
	files *tfile;
	files *filestruct;

	if( dir->type == TYPE_DIR )
	{
		len = strlen( dir->name );
		if( ( len + pool->pointer + sizeof( directory ) ) < pool->size )
		{
			tdir = ( directory *)(pool->pool+pool->pointer );
			len += sizeof( directory );
			len = ((len+1)>>1)<<1;
			pool->pointer += len;
			tdir->type = dir->type;
			tdir->sub = dir->sub;
			tdir->next = dir->next;
			strcpy( tdir->name, dir->name );
		}
/*
		else
			printf("DIR Pool to small\n");
*/
	}
	else
	{
		//namecount++;
		len = strlen( ((files *)dir)->name );
		if( ( len + pool->pointer + sizeof( directory ) ) < pool->size )
		{
			filestruct = ( files *)dir;
			tfile = ( files *)(pool->pool+pool->pointer );
			len += sizeof( files );
			len = ((len+1)>>1)<<1;
			pool->pointer += len;
			tfile->type = dir->type;
			tfile->next = dir->next;
			strcpy( tfile->name, filestruct->name );

		//fprintf(logfile,tfile->name);
		//fprintf(logfile,"\n");

		}
/*
		else
			printf("FILE Pool to small\n");
*/
	}
}

int my_strcmp(char *str1, char *str2)
{
	if ( strlen(str1) != strlen(str2) )
		return(-1);
	return( strcmpi(str1,str2) );
}

//===============================================
//	Name		:	find_in_list
//	Function	:	search a string in the mempool directory struct
//	Inputs	:	pool, startpointer, root name, zoekstring, destination string
//	Result	:	destStr is set with the path and the filename,
//				:	if so a 1 is returned
//	Updated	:	26-04-93
//
STATIC int find_in_list( mempool *pool, long p, char *dirName, char *findStr, char *destStr, int type )
{
	char *tpool;
	char dir[200];

	strcpy(dir, dirName);
	tpool = pool->pool + p;
	while( p != -1 && p < POOL_SIZE )
	{
		if( ((directory *)tpool)->type == TYPE_FILE )
		{
			if ( !my_strcmp(((files *)tpool)->name,findStr) )
			{
				strcpy( destStr, dir );
				if( dir[ strlen( dir )-1] != ':' )
					strcat(destStr,"/" );
				strcat( destStr, ((files *)tpool)->name);
//printser("--> [%s] %d <--\n",destStr,type);
				if ( RightType(destStr,type) )
					return( 1 );
			}
			p = ((files *)tpool)->next;
		}
		else
		{
			strcpy( dir, dirName );
			if( dir[ strlen( dir )-1] != ':' )
				strcat(dir, "/" );
			strcat(dir,((directory *)tpool)->name);
			if( find_in_list( pool, ((directory *)tpool)->sub, dir, findStr, destStr, type ))
				return( 1 );
			p = ((directory *)tpool)->next;
			strcpy( dir, dirName );
		}
		tpool = pool->pool + p;
	}
	return( 0 );
}

/******** RightType() ********/

STATIC BOOL RightType(STRPTR str, int type)
{
struct FileHandle *FH;
ULONG buf[10], filetype=0L;

	FH = (struct FileHandle *)Open((STRPTR)str, (LONG)MODE_OLDFILE);
	if (FH!=NULL)
	{
		Read((BPTR)FH, buf, 12);
		Close((BPTR)FH);
		if (      buf[0]==FORM            && buf[2]==ILBM )
			filetype = ILBM;
		else if ( buf[0]==FORM            && buf[2]==ANIM )
			filetype = ANIM;
		else if ( buf[0]==PAGETALK_ID_1   && buf[1]==PAGETALK_ID_2 )
			filetype = PAGETALK_ID_1;
		else if ( buf[0]==SCRIPTTALK_ID_1 && buf[1]==SCRIPTTALK_ID_2 )
			filetype = SCRIPTTALK_ID_1;
	}
	//else
	//	printser("file %s couldn't be opened\n");

//printser("filetype=%x wanted type=%d\n",filetype,type);

	switch(type)
	{
		case SM_CLIPANIM:
		case SM_ANIM:		if ( filetype==ANIM )
											return(TRUE);
										break;
		case SM_AREXX:
		case SM_DOS:
		case SM_SOUND:
		case SM_CRAWL:
		case SM_TEXT:
										if ( 	filetype!=ILBM && filetype!=ANIM &&
													filetype!=PAGETALK_ID_1 && filetype!=SCRIPTTALK_ID_1 )
											return(TRUE);										
										break;
		case SM_PAGE:		if ( filetype==PAGETALK_ID_1 ) 
											return(TRUE);
										break;
		case SM_CLIP:
		case SM_IFF:		if ( filetype==ILBM ) 
											return(TRUE);
										break;
		case SM_XAPP:		return(TRUE);
										break;
	}

	return(FALSE);
}

/******** E O F ********/
