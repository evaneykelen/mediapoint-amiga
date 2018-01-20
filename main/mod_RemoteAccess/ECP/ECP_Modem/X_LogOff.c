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

/******** LogOff() ********/

BOOL LogOff(struct CDF_Record *CDF_rec)
{
//	Report("LogOff() called.");


	if( CDF_rec->SendError == 0 )						// only swap script when
	{																				// there is no error
//		Report("Changing script's");
		if( CDF_rec->ChangeScriptType == 1 )
		{
			if( SwapScript( SERP ) );
//				Report("Swap script failed\n");
		}
		else
		{
			if( ChangeScript( SERP ) )
			{
//				Report("Change script failed\n");

			}
			else
			{
//				Report("Copy remote files");

				if( CopySerScript( SERP ) );
//					Report("Copy remote files failed\n");
			}
		}
	}

//	Report("Logoff");

	if( Logoff( SERP ) )
	{
//		Report("Logoff failed\n");
			HangUp( SERP );
		return( FALSE );
	}
	else
	{
		Report( msgs[ Msg_ECP_4 - 1 ] );
			HangUp( SERP );
		return(TRUE);
	}
}

/******** E O F ********/
