/******** GENINIT.C ********/

#include <workbench/startup.h>
#include "minc:types.h"
#include "minc:process.h"

/**************************************************
*Func : Find out the environment in which this process 
*		has been started. If it is not from MediaPoint
*		tell the user about it and return NULL.
*		If a programmer wants to test its software from
*		the CLI then he should setup a PROCESSINFO
*		structure using AllocMem and put his parameters
*		into the sugestive PI->pi_Arguments.ar_Worker fields.
*		Notice that when runnig from the CLI, it is not possible
*		to communicate with other processes since all other fields
*		are not valid. 
*in   : argc -> if != 0 then CLI
*               if == 0 then workbench or ML ( if size msg < procinfo ) ML
*out  : NULL -> CLI or WORKBENCH process
*               else ptr to PROCESSINFO structure 
*/

PROCESSINFO *ml_FindBaseAddr( int argc, char **argv)
{
	if( argc == 0 )
		if(((struct WBStartup *)argv)->sm_Message.mn_Length >=
			( sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD)))
			return((PROCESSINFO *)((ULONG)argv - (ULONG)sizeof(struct Node) - sizeof(UWORD)) );
	return( NULL );
}

/******** E O F ********/
