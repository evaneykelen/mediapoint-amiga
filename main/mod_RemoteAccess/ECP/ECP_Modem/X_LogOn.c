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

void LogAction( char *mes );

/******** LogOn() ********/

BOOL LogOn(struct CDF_Record *CDF_rec)
{
	char tt[256];

//	Report("LogOn() called.");

	if( !Logon( SERP, SERP->pref.login ) )
	{
//	Report("Logon Oke\n");
		Report( msgs[ Msg_ECP_3 - 1 ] );
		CreateDateString(tt,"\n++++++++++++++++++++++++++++++\n\nLogon");
		strcat( tt, "\n\n" );
		LogAction( tt );
		if( WhichRuns( SERP ) )
		{
//			Report("Which runs gives error\n");
			//sprintf( tt, msgs[ Msg_ECP_12 - 1 ], 2 );
			//Report( tt );
			CDF_rec->SendError = 1;
			return( FALSE );
		}
		return(TRUE);
	}
	else
	{
//		Report("Logon Failed\n");
		Report( msgs[ Msg_ECP_5 - 1 ] );
		CDF_rec->SendError = 1;
		return( FALSE );
	}
}

/******** E O F ********/
