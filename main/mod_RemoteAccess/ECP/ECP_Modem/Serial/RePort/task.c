// File		: test.c
// Uses		:
//	Date		:
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

#ifdef LATTICE
#include <proto/exec.h>
#include <stdio.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */
void main(void);
#endif

#include <stdio.h>
#include <string.h>


int CheckTask( char *name );

void main( void )
{

	ULONG Temp;
	ULONG WaitMask;
	ULONG SerMask;

	if( CheckTask( "task" ) )
	{
		printf("Task already running\n");
		return( 0 );
	}
	Temp = Wait( SIGBREAKF_CTRL_F );
}

int CheckTask( char *name )
{
	char buf[100];

	struct Task *t,*t2;
	struct List *list;
	struct Node *node;

	GetProgramName( buf,100 );
	printf("Program name is [%s]\n",buf);

SetProgramName("Cees");
	GetProgramName( buf,100 );
	printf("Program name is [%s]\n",buf);


	t = FindTask( 0 );
	t2 = FindTask( name );

	printf("My Task %lx,%lx\n",t,t2 );

	if( t )
	{
		node = &t->tc_Node;
		printf("name is [%s], type %d\n",node->ln_Name,node->ln_Type );

	}

	return( 0 );

}
