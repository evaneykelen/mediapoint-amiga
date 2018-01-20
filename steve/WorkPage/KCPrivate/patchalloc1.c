
#include	<stdio.h>

#include	<exec/memory.h>
#include	<exec/types.h>
#include <proto/exec.h>
#include	<pragmas/exec_pragmas.h>

#define MEMSIZE 1000000

void __asm SetFunctions( register __a0 long *mem1,
								register __a1 long *mem2 );
long  __asm *ResetFunctions( void );

void main( void )
{
	register int i,j;
	register long t;

	LONG *mem1,*mem2;
	LONG *count;

	mem1 = (LONG *)AllocMem( MEMSIZE, MEMF_PUBLIC|MEMF_CLEAR );
	mem2 = (LONG *)AllocMem( MEMSIZE, MEMF_PUBLIC|MEMF_CLEAR );
	if( mem1 && mem2 )
	{
		printf("Set functions\n");
		SetFunctions( mem1, mem2 );
		printf("Waiting for return\n");
		getch();
		count = ResetFunctions();
		printf("Alloc's done %ld, Free's done %ld, delta mem %ld\n",count[0],count[1],count[2] );

// find matching frees and allocs
		i=0;

		printf("Searching for missing freemem's\n");
/*
		i=0;
		while( mem1[i] != 0 )
		{
			printf("A %ld,%ld\n",mem1[i],mem1[i+1] );
			i+=2;
		}

		i=0;
		while( mem2[i] != 0 )
		{
			printf("F %ld,%ld\n",mem2[i],mem2[i+1] );
			i+=2;
		}
*/

		while( mem1[i] != 0 )
		{
			j=0;
			t = mem1[i];
			while( mem2[j] != t && mem2[j] != 0 )j+=2;
			if( mem2[j] == t )
			{
				mem2[j] = -1;
				mem2[j+1] = -1;
			}
			else
				printf("Alloc not freed %x, %ld\n",mem1[i],mem1[i+1]);
			i+=2;
		}
		i=0;
		while( mem2[i] != 0 )
		{
			if( mem2[i]!=-1 )
			printf("Unfreed %x,%ld\n",mem2[i],mem2[i+1] );
			i+=2;
		}
	}

	if( mem1 )
		FreeMem( (UBYTE *)mem1, MEMSIZE );

	if( mem2 )
		FreeMem( (UBYTE *)mem2, MEMSIZE );

}