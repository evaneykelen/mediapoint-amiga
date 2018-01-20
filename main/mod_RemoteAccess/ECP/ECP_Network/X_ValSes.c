#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include "netfiles.h"
/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;

/**** functions ****/

/******** ValidateSession() ********/

BOOL ValidateSession(struct CDF_Record *CDF_rec)
{
//	Report("ValidateSession() called.");

	if ( strcmpi(CDF_rec->ConnectionClass,"5") )
	{
//		Report("Wrong Connection Class.");
		ReportError( 1 );
		return(FALSE);
	}

	return(TRUE);
}

/******** E O F ********/
