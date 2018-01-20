#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include "serial/serhost.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;

/**** functions ****/

/******** DeInitConnection() ********/

BOOL DeInitConnection(struct CDF_Record *CDF_rec)
{
//	Report("DeInitConnection() called.");
	if( CDF_rec->misc_ptr1 )
		FreeMem( (unsigned char *)CDF_rec->misc_ptr1, sizeof( SERDAT ) );
	return(TRUE);
}

/******** E O F ********/
