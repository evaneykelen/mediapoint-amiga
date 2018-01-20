#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "gen:general.h"

/**** externals ****/

extern struct Library *IconBase;

/**** functions ****/

/******** Open_SerialPort() ********/
/*
 * Creates a message port ( _rec->serialMP ) and
 * opens the serial device ( _rec->serialIO )
 *
 */

BOOL Open_SerialPort(	struct standard_record *std_rec, int readBits,
											int writeBits, int stopBits)
{
	if ( std_rec->serialMP=(struct MsgPort *)CreatePort(0,0) )
	{
		if (	std_rec->serialIO = (struct IOExtSer *)
					CreateExtIO((struct MsgPort *)std_rec->serialMP,
											sizeof(struct IOExtSer)))
		{
			/**** devicename, unit, iorequest, flags ****/

			std_rec->serialIO->io_SerFlags = SERF_SHARED | SERF_XDISABLED;

			if (OpenDevice(	std_rec->devName, std_rec->portNr,
											(struct IORequest *)std_rec->serialIO, 0L))
				; //UA_WarnUser(2000, NULL, NULL);
			else
			{
				/**** set up serial device ****/

//printf("%d %d %d %d\n",readBits,writeBits,stopBits,std_rec->baudRate);

    		std_rec->serialIO->io_ReadLen				= readBits;
    		std_rec->serialIO->io_WriteLen 			= writeBits;
 				std_rec->serialIO->io_StopBits 			= stopBits;
				std_rec->serialIO->io_Baud					= std_rec->baudRate;
    		std_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)std_rec->serialIO);

				std_rec->serialIO->io_SerFlags			= 0;
    		std_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)std_rec->serialIO);

				std_rec->serialIO->io_BrkTime				= 50000;	// microseconds
    		std_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)std_rec->serialIO);

				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** Close_SerialPort() ********/

void Close_SerialPort(struct standard_record *std_rec)
{
	if (std_rec->serialMP!=NULL && std_rec->serialIO!=NULL)
	{
		AbortIO((struct IORequest *)std_rec->serialIO);
		WaitIO((struct IORequest *)std_rec->serialIO);

		CloseDevice((struct IORequest *)std_rec->serialIO);
		DeleteExtIO((struct IORequest *)std_rec->serialIO);
		DeletePort((struct MsgPort *)std_rec->serialMP);

		std_rec->serialMP=NULL;
		std_rec->serialIO=NULL;
	}
}

/******** GetInfoFile() ********/
/*
 * appName is without .info !
 *
 */

void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate)
{
struct DiskObject *diskObj;
char *s;
TEXT str[5], path[SIZE_FULLPATH];
int i;

	*portNr = 1;									// default unit
	*baudRate = 9600;
	strcpy(devName, SERIALNAME);	// serial.device

	IconBase = (struct Library *)OpenLibrary("icon.library", 0L);
	if ( IconBase==NULL )
		return;

	MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath, appName, path);

	diskObj = GetDiskObject(path);
	if ( diskObj==NULL )
	{
		CloseLibrary((struct Library *)IconBase);
		return;
	}

	s = (char *)FindToolType(diskObj->do_ToolTypes, "DEVICE");
	if ( s==NULL )
	{
		FreeDiskObject(diskObj);
		CloseLibrary((struct Library *)IconBase);
	}
	else
		strcpy(devName, s);

	s = (char *)FindToolType(diskObj->do_ToolTypes, "PORTNUMBER");
	if ( s==NULL )
	{
		FreeDiskObject(diskObj);
		CloseLibrary((struct Library *)IconBase);
	}

	/**** look for PORTNUMBER=0 ... PORTNUMBER=16 ****/

	for(i=0; i<17; i++)
	{
		sprintf(str, "%d", i);
		if ( MatchToolValue(s, str) )
		{
			*portNr = i;
			break;
		}
	}

	s = (char *)FindToolType(diskObj->do_ToolTypes, "BAUDRATE");
	if ( s==NULL )
	{
		FreeDiskObject(diskObj);
		CloseLibrary((struct Library *)IconBase);
	}

	if ( MatchToolValue(s, "1200") )
		*baudRate = 1200;
	else if ( MatchToolValue(s, "2400") )
		*baudRate = 2400;
	else if ( MatchToolValue(s, "4800") )
		*baudRate = 4800;
	else if ( MatchToolValue(s, "9600") )
		*baudRate = 9600;
	else if ( MatchToolValue(s, "19200") )
		*baudRate = 19200;
	else if ( MatchToolValue(s, "31250") )
		*baudRate = 31250;

	FreeDiskObject(diskObj);

	CloseLibrary((struct Library *)IconBase);
}

/******** E O F ********/
