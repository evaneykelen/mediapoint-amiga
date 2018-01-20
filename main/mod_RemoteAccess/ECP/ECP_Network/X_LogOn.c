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

/******** LogOn() ********/

BOOL LogOn(struct CDF_Record *CDF_rec)
{
	char deb[256];

//	Report("LogOn() called.");
	CDF_rec->misc_ptr1 = (void *)NetSayRun( CDF_rec->DestinationPath );

	if( (int)CDF_rec->misc_ptr1 == 1 )
		CDF_rec->misc_ptr2 = (void *)2;
	else
		if( (int)CDF_rec->misc_ptr1 == 2 )
			CDF_rec->misc_ptr2 = (void *)1;
		else
			return( FALSE );

	//sprintf(deb,"Remote script %d runs",CDF_rec->misc_ptr1 );
	//Report( deb );

	return(TRUE);
}

/******** E O F ********/
