#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include <time.h>

#include "serial/serhost.h"
#include "serial/serfuncs.h"
#include "serial/serprint.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;

/**** functions ****/

void LogAction( char *mes );

/******** CloseConnection() ********/

BOOL CloseConnection(struct CDF_Record *CDF_rec)
{
	char tt[256];
	float t;

//	Report("CloseConnection() called.");
	if( SERP->totaltime > 0 )
	{
		long mins,secs;

		t = (float)SERP->totalsend*CLOCKS_PER_SEC;

		secs = SERP->totaltime/CLOCKS_PER_SEC;
		mins = secs/60;
		secs = secs-mins*60;

		sprintf( tt, msgs[ Msg_ECP_13 - 1 ],
						SERP->totalsend,
						mins, secs,
					(long) (t/(float)SERP->totaltime ) );

/*		sprintf(	tt, "Total %d bytes send in %d seconds. %d bytes/second",
					SERP->totalsend,
					SERP->totaltime/CLOCKS_PER_SEC,
					(long) (t/(float)SERP->totaltime ) );
*/
		Report( tt );
		strcat( tt,"\n" );
		LogAction( "\n" );
		LogAction( tt );
	}
	ExitPrint();
	FreeSerial( SERP );

	return(TRUE);
}

/******** E O F ********/
