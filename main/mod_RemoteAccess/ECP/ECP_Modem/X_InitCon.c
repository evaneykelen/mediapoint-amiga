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

/******** InitConnection() ********/

BOOL InitConnection(struct CDF_Record *CDF_rec)
{
	SERDAT *ser;
//	Report("InitConnection() called.");

	ser = ( SERDAT *)AllocMem( sizeof( SERDAT ), MEMF_ANY|MEMF_CLEAR );
	if( ser )
	{
//		Report("Alloc ser data Oke\n");
		CDF_rec->misc_ptr1 = ser;
		return( TRUE );
	}
	else
	{
//		Report("Alloc ser data failed\n");
		ReportError( 5 );
		return( FALSE );
	}
}

/******** E O F ********/
