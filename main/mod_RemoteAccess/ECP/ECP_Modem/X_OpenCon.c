#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "mra:ECP/structs.h"

#include "serial/serhost.h"
#include "serial/serfuncs.h"
#include "serial/serprint.h"
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
void GetSerPrefs( SERDAT *ser, struct CDF_Record *CDF_rec )
{
	long t;

	strcpy(ser->pref.replystring,"CONNECT" );

	ser->pref.baudrate = DEFAULT_BAUDRATE;
	ser->pref.controlbits = 0;						//SERF_7WIRE;
	ser->pref.unit_number = UNIT_NUMBER;
	ser->pref.read_buffer_size = READ_BUFFER_SIZE;
	ser->pref.priority = DEFAULT_PRIOR;
	ser->pref.run = 0;
	ser->pref.connectionclass = 1;
	ser->ID = -1;

	strcpy( ser->pref.devname, DEVICE_NAME );
	strcpy( ser->dirname, DEFAULT_DIRNAME );
	strcpy( ser->pref.currentname, DEFAULT_PATH );
	strncpy( ser->superpwd, SUPER_NAME, MAX_PSWD_SIZE );
	strncpy( ser->pref.login, "12345678",8 );

	ser->pref.lftocr = 0;
	ser->pref.crtolf = 0;

	if(!strncmp( CDF_rec->HandShaking,"RTS",3 ) )
		ser->pref.controlbits = SERF_7WIRE;

	sscanf( CDF_rec->UnitNumber,"%ld",&ser->pref.unit_number );
	sscanf( CDF_rec->BaudRate,"%ld",&ser->pref.baudrate );
	sscanf( CDF_rec->BufferSize,"%ld",&ser->pref.read_buffer_size );


	sscanf( CDF_rec->ConnectionClass,"%ld",&t );
	ser->pref.connectionclass = t;

	strcpy( ser->pref.devname, CDF_rec->SerialDevice );
	strcpy( ser->pref.dialpref, CDF_rec->DialPrefix );
	strcpy( ser->pref.phonenr, CDF_rec->PhoneNr );
	strcpy( ser->pref.replystring, CDF_rec->ReplyString );
	strncpy( ser->pref.login, CDF_rec->PassWord,8);

#if _PRINT
	printf("sername is    [%s]\n",ser->pref.devname );
	printf("dialprefix is [%s]\n",ser->pref.dialpref );
	printf("phonenr is 		[%s]\n",ser->pref.phonenr );
	printf("login   is 		[%.8s]\n",ser->pref.login );
	printf("replystr is   [%s]\n",ser->pref.replystring );
#endif


}

/******** OpenConnection() ********/

BOOL OpenConnection(struct CDF_Record *CDF_rec)
{
	short int ret;

	ret = FALSE;

//	Report("OpenConnection() called.");

	GetSerPrefs( SERP, CDF_rec );
	SERP->totaltime = 0;
	SERP->totalsend = 0;

	if( OpenSerial( SERP ) )
	{
		InitPrint();			// DEBUG
		if( SERP->pref.connectionclass == 4 || SERP->pref.connectionclass == 10 )		// is modem send prefix
		{
			SoundOff( SERP );

			if( SERP->pref.connectionclass == 10 )
				SetLeased( SERP );

			if( MakeConnection( SERP ) )
			{
//				Report("Connection Oke\n");
				Report( msgs[ Msg_ECP_2 - 1 ] );

				ret = TRUE;
			}
			else
			{
//			Report("Connection failed\n");
				ReportError( 3 );
				CDF_rec->SendError = 1;
			}
		}
		else
			ret = TRUE;												// Null modem cable already connected
	}

	if( ret == FALSE )										// if connection failed free serial here
	{
		FreeSerial( SERP );
//	Report("Failed to open serial port");

		ReportError( 4 );
	}

	return( ret );

}

/******** E O F ********/
