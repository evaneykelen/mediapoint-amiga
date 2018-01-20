#include <exec/exec.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <devices/serial.h>
// CLIB
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/alib_protos.h>
// PRAGMAS
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
// ROOT
#include <string.h>
#include <stdio.h>
// USER
#include "demo:Gen/mp.h"
#include "demo:Gen/wait50hz.h"

extern struct Library *IconBase;

struct SerRecord
{
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	int unit;											// read from .info file
	TEXT devName[SIZE_FILENAME];	// read from .info file
	int baudrate;									// read from .info file
	int handshaking;
	int parity;
	int bits_char;
	int stop_bits;
};

/**** functions ****/

/******** Open_SerialPort() ********/

BOOL Open_SerialPort(struct SerRecord *ser_rec)
{
	if ( ser_rec->serialMP=(struct MsgPort *)CreatePort(0,0) )
	{
		if (	ser_rec->serialIO = (struct IOExtSer *)
					CreateExtIO((struct MsgPort *)ser_rec->serialMP,sizeof(struct IOExtSer)))
		{
			/**** devicename, unit, iorequest, flags ****/

			ser_rec->serialIO->io_SerFlags = SERF_SHARED;

			if ( ser_rec->parity==1 )	// PARITY EVEN
			{
				ser_rec->serialIO->io_SerFlags |= SERF_PARTY_ON;
			}
			else if ( ser_rec->parity==2 )	// PARITY ODD
			{
				ser_rec->serialIO->io_SerFlags |= SERF_PARTY_ON | SERF_PARTY_ODD;
			}
			else if ( ser_rec->parity==3 )	// PARITY MARK
			{
				ser_rec->serialIO->io_SerFlags |= SERF_PARTY_ON;
			}
			else if ( ser_rec->parity==4 )	// PARITY SPACE
			{
				ser_rec->serialIO->io_SerFlags |= SERF_PARTY_ON;
			}

			if ( ser_rec->handshaking==1 )	// RTS/CTS ON
				ser_rec->serialIO->io_SerFlags |= SERF_7WIRE;

			ser_rec->serialIO->io_SerFlags |= SERF_XDISABLED;

			if (!OpenDevice(ser_rec->devName, ser_rec->unit,
											(struct IORequest *)ser_rec->serialIO, 0L))
			{
				/**** set up serial device ****/

    		ser_rec->serialIO->io_ReadLen				= ser_rec->bits_char+7;	// 7 or 8
    		ser_rec->serialIO->io_WriteLen 			= ser_rec->bits_char+7;	// 7 or 8
 				ser_rec->serialIO->io_StopBits 			= ser_rec->stop_bits+1;	// 1 or 2
				ser_rec->serialIO->io_Baud					= ser_rec->baudrate;
    		ser_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;

				if ( ser_rec->parity==3 )	// PARITY MARK
				{
					ser_rec->serialIO->io_ExtFlags |= SEXTF_MSPON | SEXTF_MARK;
				}
				else if ( ser_rec->parity==4 )	// PARITY SPACE
				{
					ser_rec->serialIO->io_ExtFlags |= SEXTF_MSPON;
				}

				DoIO((struct IORequest *)ser_rec->serialIO);

				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** Close_SerialPort() ********/

void Close_SerialPort(struct SerRecord *ser_rec)
{
	if (ser_rec->serialMP!=NULL && ser_rec->serialIO!=NULL)
	{
		AbortIO((struct IORequest *)ser_rec->serialIO);
		WaitIO((struct IORequest *)ser_rec->serialIO);

		CloseDevice((struct IORequest *)ser_rec->serialIO);
		DeleteExtIO((struct IORequest *)ser_rec->serialIO);
		DeletePort((struct MsgPort *)ser_rec->serialMP);

		ser_rec->serialMP=NULL;
		ser_rec->serialIO=NULL;
	}
}

/******** SendStringViaSer() *******/

BOOL SendStringViaSer(struct SerRecord *ser_rec, UBYTE *str)
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)ser_rec->serialIO) )
	{
		AbortIO((struct IORequest *)ser_rec->serialIO);
		WaitIO((struct IORequest *)ser_rec->serialIO);
	}

	ser_rec->serialIO->IOSer.io_Length 	= strlen(str);
	ser_rec->serialIO->IOSer.io_Data		= (APTR)str;
	ser_rec->serialIO->IOSer.io_Command	= CMD_WRITE;
	SendIO((struct IORequest *)ser_rec->serialIO);

	WJIF.signum = 0;
	hz_signal = set50hz(&WJIF, 500);
	mask = (1L << ser_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}
		if( CheckIO((struct IORequest *)ser_rec->serialIO) )
		{
			WaitIO((struct IORequest *)ser_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)ser_rec->serialIO);
	WaitIO((struct IORequest *)ser_rec->serialIO);
	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** GetInfoFile() ********/

void GetInfoFile(	STRPTR xappPath, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate)
{
struct DiskObject *diskObj;
char *s;
TEXT str[5], path[150];
int i;

	*portNr = 1;									// default unit
	*baudRate = 9600;
	strcpy(devName, SERIALNAME);	// serial.device

	IconBase = (struct Library *)OpenLibrary("icon.library", 0L);
	if ( IconBase==NULL )
		return;

	MakeFullPath(xappPath, appName, path);

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
