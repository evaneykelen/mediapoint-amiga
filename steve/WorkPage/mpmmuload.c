//
// Try a resource progje
//
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "gen:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

void load_file( char *name , struct Library *MLMMULibBase )
{
	char err[170];
	FILE *fp;
	MEMTAG *memtag;
	int t;
	struct FileInfoBlock *Finfo;
	BPTR	lock;
	long size = 0;
	char *t_data;

// First check the name in the MLMMU lib

	memtag = MLMMU_FindMem( name );
	if( memtag == NULL )
	{

		size = 0;
		Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );

		if( Finfo )
		{
			lock = Lock( name, ACCESS_READ );
			if( lock != NULL )
			{
				Examine( lock, Finfo );
				size = Finfo->fib_Size;
				UnLock( lock );
			}
			FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
		}

		printf("Size file is %d\n",size );

		fp = fopen( name, "r" );
		if( fp != NULL )
		{
			t_data = (char *)MLMMU_AllocMem( size, MEMF_CLEAR|MEMF_PUBLIC|MEMF_STAY, name );

			if( t_data != NULL )
				if( (t = fread( t_data , 1, size , fp )) != size );
			MLMMU_SetMemStat( MTF_SETCLR | MTF_INIT, t_data );
			fclose( fp );
		}
		else
		{
			strcpy(err,"File not found : " );
			strcat(err, name );
			printf("%s\n",err );
//			MLMMU_AddMsgToQueue(err, 1 );
		}
	}
	else
		printf("Data already in MMU\n");
}

void main(int argc, char *argv[])
{
		struct Library *MLMMULibBase = NULL;
		if( argc == 2 )
		{
			MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0);
			if( MLMMULibBase != NULL )
			{
				load_file( argv[1] , MLMMULibBase );
			}
			else
				printf("Mpmmu lib not found\n");
		}
		else
			printf("Use %s filename\n",argv[0] );
}

