#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"

#include "serial/serhost.h"
#include "serial/serwork.h"

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

	if( strcmpi(CDF_rec->ConnectionClass,"4") && strcmpi(CDF_rec->ConnectionClass,"1")&& strcmpi(CDF_rec->ConnectionClass,"10") )
	{
//	Report("Wrong Connection Class.");
		ReportError( 1 );
		return(FALSE);
	}

	return(TRUE);
}

/******** E O F ********/
