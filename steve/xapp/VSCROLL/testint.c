#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <exec/interrupts.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <libraries/dosextens.h>

#include "inthandler.h"

void main( void )
{
	LONG sig;
	OBJECT_INPUT_STRUCT oi;

	printf("Test key interrupt handler\n");
	
	if( !SetupInputHandler( &oi ) )
	{
		printf("Signum is %x\n",oi.SigNum_NO );
		AddInputHandler( &oi );
		printf("int started\n");
		sig = Wait( 1L << oi.SigNum_NO | SIGBREAKF_CTRL_F );
		printf("sig is %lx\n",sig );
		if( sig &  1L << oi.SigNum_NO )
			printf("And yes it was the key\n");

		RemoveInputHandler( &oi );
		FreeInputHandler( &oi );
	}
}