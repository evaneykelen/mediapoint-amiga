#include "nb:pre.h"
#include "all.h"
#include "protos.h"
#include "demo:gen/wait50hz.h"

/**** externals ****/

extern struct Library *IconBase;
extern void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest );

/**** functions ****/

/******** Open_SerialPort() ********/
/*
 * Creates a message port ( all_rec->serialMP ) and
 * opens the serial device ( all_rec->serialIO )
 *
 */

BOOL Open_SerialPort(struct all_record *all_rec)
{
	if ( all_rec->serialMP=(struct MsgPort *)CreatePort(0,0) )
	{
		if (	all_rec->serialIO = (struct IOExtSer *)
					CreateExtIO((struct MsgPort *)all_rec->serialMP,sizeof(struct IOExtSer)) )
		{
			/**** devicename, unit, iorequest, flags ****/

			all_rec->serialIO->io_SerFlags = SERF_SHARED | SERF_XDISABLED;

			if ( !OpenDevice(	all_rec->devName, all_rec->portNr,
												(struct IORequest *)all_rec->serialIO, 0L) )
			{
				/**** set up serial device ****/

    		all_rec->serialIO->io_ReadLen				= 8;
    		all_rec->serialIO->io_WriteLen 			= 8;
 				all_rec->serialIO->io_StopBits 			= 1;
				all_rec->serialIO->io_Baud					= all_rec->baudRate;
    		all_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;

				DoIO((struct IORequest *)all_rec->serialIO);

				all_rec->serialIO->io_SerFlags				= 0;
    		all_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)all_rec->serialIO);

				all_rec->serialIO->io_SerFlags				= SERF_SHARED;
    		all_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)all_rec->serialIO);

				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** Close_SerialPort() ********/

void Close_SerialPort(struct all_record *all_rec)
{
	if (all_rec->serialMP!=NULL && all_rec->serialIO!=NULL)
	{
		AbortIO((struct IORequest *)all_rec->serialIO);
		WaitIO((struct IORequest *)all_rec->serialIO);

		CloseDevice((struct IORequest *)all_rec->serialIO);
		DeleteExtIO((struct IORequest *)all_rec->serialIO);
		DeletePort((struct MsgPort *)all_rec->serialMP);

		all_rec->serialMP=NULL;
		all_rec->serialIO=NULL;
	}
}

/******** DoIOR() ********/

BOOL DoIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data)
{
  req->io_Command	= cmd;
  req->io_Offset	= off;
  req->io_Length	= len;
  req->io_Data		= data;

  if(!DoIO((struct IORequest *)req))
		WaitIO((struct IORequest *)req);

	if ( req->io_Error )
	{
		//printf("ERROR Code DoIOR = %ld\n", req->io_Error);
		return( FALSE );
	}

	return( TRUE );
}

/******** SendIOR() ********/

BOOL SendIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data)
{
	req->io_Command	= cmd;
	req->io_Offset	= off;
	req->io_Length	= len;
	req->io_Data		= data;

	SendIO((struct IORequest *)req);

	if ( req->io_Error )
	{
		//printf("ERROR Code SendIOR = %ld\n", req->io_Error);
		return( FALSE );
	}

	return( TRUE );
}

/******** GetInfoFile() ********/
/*
 * appName is without .info !
 *
 */

void GetInfoFile(STRPTR appName, STRPTR appPath, STRPTR devName, int *portNr, int *baudRate)
{
struct DiskObject *diskObj;
char *s;
TEXT str[5], path[SIZE_FULLPATH];
int i;

	*portNr = 1;									// default unit
	*baudRate = 14400;
	strcpy(devName, SERIALNAME);	// serial.device

	IconBase = (struct Library *)OpenLibrary("icon.library", 0L);
	if ( IconBase==NULL )
		return;

	if ( appPath!=NULL )
		MakeFullPath(appPath, appName, path);
	else
		strcpy(path, appName);

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
	else if ( MatchToolValue(s, "14400") )
		*baudRate = 14400;
	else if ( MatchToolValue(s, "19200") )
		*baudRate = 19200;
	else if ( MatchToolValue(s, "31250") )
		*baudRate = 31250;

	FreeDiskObject(diskObj);

	CloseLibrary((struct Library *)IconBase);
}

/******** DoSerIO() *******/

BOOL DoSerIO(struct all_record *all_rec, STRPTR str, int command)
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;
TEXT buf[70];
int i;

	for(i=0; i<70; i++)
		buf[i]='\0';
	if ( command==CMD_WRITE )
		strcpy(buf,str);

	if( CheckIO((struct IORequest *)all_rec->serialIO) )
	{
		AbortIO((struct IORequest *)all_rec->serialIO);
		WaitIO((struct IORequest *)all_rec->serialIO);
	}

	if ( command==CMD_WRITE )
		all_rec->serialIO->IOSer.io_Length 	= strlen(buf);
	else if ( command==CMD_READ )
		all_rec->serialIO->IOSer.io_Length 	= 64;
	all_rec->serialIO->IOSer.io_Data			= (APTR)buf;
	all_rec->serialIO->IOSer.io_Command		= command;
	SendIO((struct IORequest *)all_rec->serialIO);

	WJIF.signum = 0;
	hz_signal = set50hz(&WJIF, 500 );
	mask = (1L << all_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}
		if( CheckIO((struct IORequest *)all_rec->serialIO) )
		{
			WaitIO((struct IORequest *)all_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)all_rec->serialIO);
	WaitIO((struct IORequest *)all_rec->serialIO);

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** PutStringToSer() ********/

BOOL PutStringToSer(struct all_record *all_rec, STRPTR str)
{
	return( DoSerIO(all_rec, str, CMD_WRITE) );
}

/******** GetStringFromSer() ********/
/*
 * Make sure that str is at least 64 bytes large!!!!!!!!!
 *
 */

BOOL GetStringFromSer(struct all_record *all_rec, STRPTR str)
{
	return( DoSerIO(all_rec, str, CMD_READ) );
}

/******** E O F ********/
