#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include "NetFiles.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;

/**** functions ****/

/******** LogOff() ********/

BOOL LogOff(struct CDF_Record *CDF_rec)
{
//	Report("LogOff() called.");

	if( CDF_rec->SendError == 0 )						// only swap script when
	{																				// there is no error
		//Report("Changing script's");
		if( CDF_rec->ChangeScriptType == 1 )
		{
			if( CreateSwap(CDF_rec->DestinationPath ) )
				Report("Swap script failed\n");
		}
		else
		{
			if( CreateCommand( CDF_rec->DestinationPath ) )	
				Report("Change script failed\n");
		}
	}
	return(TRUE);
}

/******** E O F ********/
