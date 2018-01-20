#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "nb:pre.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"
#include "minc:external.h"

extern UBYTE **msgs;
extern struct Library *MLMMULibBase;

#define NUM_ERROR_CODES 8

/*
char ErrString[][30] = 
{
  "Timer has no tempo editor!",
  "Out of memory!",
  "Couldn't load XaPP!",
  "Stack overflow!",
  "Graphics memory error!",
  "Couldn't load 'INPUT'!",
  "Script is empty!",
  "Invalid Timecode!"
};
*/

/*********************************************************
*Func : Process errors from the processcontroller and
*		display a string with the error information
*in   : Err -> Error code
*out  : -
*/

void ProcessError(int Err)
{
	if(Err != NO_ERROR)
	{
		// Error codes range from 400 - 407
		// Make it 0...7

		Err -= 400;		

		if(Err < 0)
			MLMMU_AddMsgToQueue( msgs[Msg_UnableToPlayScript -1], 0 );
		else
		{
			if ( Err>=0 && Err<NUM_ERROR_CODES )
				MLMMU_AddMsgToQueue( msgs[Msg_NoTempoEditor+Err -1], 0 );
			else
				MLMMU_AddMsgToQueue( msgs[Msg_UnableToPlayScript -1], 0 );
		}
	}
}

/******** E O F ********/
