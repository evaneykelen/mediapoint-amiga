#include "nb:pre.h"
//#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include "mra:ECP/protos.h"

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

/******** DoTransaction() ********/

BOOL DoTransaction(struct CDF_Record *CDF_rec, struct ParseRecord *PR)
{
TEXT sname[256],dname[256];
BOOL ret = TRUE;
int fileSize,stamp;

	// The next line comes from a BigFile...

	// SYSTEM "Work:SC/proj2/main/System/Transitions" "ALIAS:System/Transitions" "134604" "525357675"

	// PR->argString[0] = "SYSTEM"
	// PR->argString[1] = "Work:SC/proj2/main/System/Transitions"
	// PR->argString[2] = "ALIAS:System/Transitions"
	// PR->argString[3] = "134604" --> file size of file which will be sent (e.g. the .TMP file and not the original page)
	// PR->argString[4] = "525357675" --> date stamp

//	sprintf(str, "DoTrans: [%s]", PR->argString[2]);
//	Report(str);

	sscanf(PR->argString[3],"\"%d\"",&fileSize);
	sscanf(PR->argString[4],"\"%d\"",&stamp);

	strcpy( sname, PR->argString[1]);
	strcpy( dname, PR->argString[2]);

	RemoveQuotes( sname );
	RemoveQuotes( dname );

	if( PR->commandCode == 15 )
	{
		stamp = 0;
		if( strstr(sname,".info" ) )
		{
			sprintf(dname, "MP_RA:script.%d.info",SERP->pref.notrun );
//			dname[0]=0;
		}
		else
		{
//		Report("Copy script");
			sprintf(dname, "MP_RA:script.%d",SERP->pref.notrun );
		}
	}
	else
		if( PR->commandCode == 12 )				// FONT 
		{
			sprintf(dname, "MP_RA:%s",&dname[6] );
		}
		else
			if( PR->commandCode == 13 )				// SYSTEM
			{
				sprintf(dname, "MP_RA:%s",&dname[6] );
			}

	if( dname[0] != 0 )
	{
//	sprintf(str,"%s - %d bytes",dname,fileSize);
//	Report(str);

		if( CCopy( SERP, sname, dname,stamp, fileSize ) )
		{
			
//			Report("Error in copy\n");
//			LogAction("Error in copy\n");

			Report( msgs[ Msg_ECP_11 - 1 ] );
			LogAction( msgs[ Msg_ECP_11 - 1 ] );

			CDF_rec->SendError = 1;
			ret = FALSE;
		}

	}

	if( IsTheButtonHit() )
	{
//		Report("Return from DOTrans with FALSE");
		CDF_rec->SendError = 1;
		return( FALSE );
	}
	return( ret );
}

void LogAction( char *mes )
{
	if( logfile )
	{
//		printf("logfile entry [%s]\n",mes );
		fwrite( mes, strlen( mes ),1,logfile );
	}
}

/******** E O F ********/
