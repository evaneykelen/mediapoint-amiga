#include "nb:pre.h"
//#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"
#include "mra:ECP/protos.h"
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

/******** DoTransaction() ********/

BOOL DoTransaction(struct CDF_Record *CDF_rec, struct ParseRecord *PR)
{
TEXT str[512];
TEXT sname[256],dtname[256],dname[256];

BOOL ret = TRUE;
int error, fileSize,stamp;


	sscanf(PR->argString[3],"\"%d\"",&fileSize);
	sscanf(PR->argString[4],"\"%d\"",&stamp);

	strcpy( sname, PR->argString[1]);
	strcpy( dtname, PR->argString[2]);

	RemoveQuotes( sname );
	RemoveQuotes( dtname );

	StripAlias( (int)CDF_rec->misc_ptr2, dtname, dname, CDF_rec->DestinationPath );

	if (PR->commandCode != 13 )		// SYSTEM
	{
		if( PR->commandCode == 15 )
		{
			stamp = 0;
			if( strstr(sname,".info" ) )
			{
				sprintf(str, "script.%d.info", CDF_rec->misc_ptr2 );
				MakeFullPath( CDF_rec->DestinationPath,str,dname );
				stamp = 0;
			}
			else
			{
				sprintf(str, "script.%d", CDF_rec->misc_ptr2 );
				MakeFullPath( CDF_rec->DestinationPath,str,dname );
				stamp = 0;
			}
		}
		else
			if( PR->commandCode == 12 )				// FONT 
			{
				sprintf( str, "%s",&dtname[6] );
				MakeFullPath( CDF_rec->DestinationPath, str,dname );
			}

		if( dname[0] != 0 )
		{

//			sprintf(str,"copy %s - %d bytes",dname,fileSize);
//			Report(str);

			if( ccopy( dname, stamp ) )
			{

				sprintf(str,"Copy %s - %d bytes",dname,fileSize);
				Report(str);

				strcat( str, "\n");

				LogAction(str);

				if( ( error = icopy( sname, dname, stamp ) ) != 0 )
				{
					sprintf(str,"Error in copy %d\n",error);
					LogAction(str);
					CDF_rec->SendError = 1;
					ret = FALSE;
				}
			}
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
		fwrite( mes, strlen( mes ),1,logfile );
}

/******** E O F ********/
