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

/******** OpenConnection() ********/

BOOL OpenConnection(struct CDF_Record *CDF_rec)
{
	char tt[256];

//	Report("OpenConnection() called.");
	CreateDateString(tt,"\n++++++++++++++++++++++++++++++\n\nLogon");
	strcat( tt, "\n\n" );
	LogAction( tt );

	return(TRUE);
}

/******** E O F ********/
