#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "demo:gen/wait50hz.h"
#include "demo:gen/general_protos.h"
#include "protos.h"

/**** externals ****/

extern struct Library *IconBase;

/**** functions ****/

/******** Open_SerialPort() ********/
/*
 * Creates a message port ( CDTV_rec->serialMP ) and
 * opens the serial device ( CDTV_rec->serialIO )
 *
 */

BOOL Open_SerialPort(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec->serialMP=(struct MsgPort *)CreatePort(0,0) )
	{
		if (	CDTV_rec->serialIO = (struct IOExtSer *)
					CreateExtIO((struct MsgPort *)CDTV_rec->serialMP,sizeof(struct IOExtSer)))
		{
			/**** devicename, unit, iorequest, flags ****/

			CDTV_rec->serialIO->io_SerFlags = SERF_SHARED | SERF_XDISABLED;

//{ char str[100]; sprintf(str,"[%s] %d %d\n", CDTV_rec->devName, CDTV_rec->portNr, CDTV_rec->baudRate); KPrintF(str); }

			if (!OpenDevice(CDTV_rec->devName, CDTV_rec->portNr,
											(struct IORequest *)CDTV_rec->serialIO, 0L))
			{
				/**** set up serial device ****/

    		CDTV_rec->serialIO->io_ReadLen				= 8;
    		CDTV_rec->serialIO->io_WriteLen 			= 8;
 				CDTV_rec->serialIO->io_StopBits 			= 1;
				CDTV_rec->serialIO->io_Baud						= CDTV_rec->baudRate;
    		CDTV_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;

				DoIO((struct IORequest *)CDTV_rec->serialIO);

				CDTV_rec->serialIO->io_SerFlags				= 0;
    		CDTV_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)CDTV_rec->serialIO);

				CDTV_rec->serialIO->io_SerFlags				= SERF_SHARED;
    		CDTV_rec->serialIO->IOSer.io_Command	= SDCMD_SETPARAMS;
				DoIO((struct IORequest *)CDTV_rec->serialIO);

				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** Close_SerialPort() ********/

void Close_SerialPort(struct CDTV_record *CDTV_rec)
{
	if (CDTV_rec->serialMP!=NULL && CDTV_rec->serialIO!=NULL)
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);

		CloseDevice((struct IORequest *)CDTV_rec->serialIO);
		DeleteExtIO((struct IORequest *)CDTV_rec->serialIO);
		DeletePort((struct MsgPort *)CDTV_rec->serialMP);

		CDTV_rec->serialMP=NULL;
		CDTV_rec->serialIO=NULL;
	}
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

/******** SendSerCmd() ********/

BOOL SendSerCmd(struct CDTV_record *CDTV_rec, int cmd)
{
TEXT sendStr[256];
BOOL retval=TRUE;

	switch(cmd)
	{
		case DO_PLAYTRACK:
			sprintf(sendStr, "!P1 %d %d\n", CDTV_rec->song, CDTV_rec->fadeIn);
			retval=SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_PLAYFROMTO:
			sprintf(sendStr, "!P2 %d %d %d\n", CDTV_rec->from, CDTV_rec->to, CDTV_rec->fadeIn);
			retval=SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_PLAYSTARTEND:
			sprintf(sendStr, "!P3 %s %s %d\n", CDTV_rec->start, CDTV_rec->end, CDTV_rec->fadeIn);
			retval=SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_FADE_IN_SLOW:
			retval=SendStringViaSer(CDTV_rec, "!F1\n");
			break;

		case DO_FADE_IN_FAST:
			retval=SendStringViaSer(CDTV_rec, "!F2\n");
			break;

		case DO_FADE_OUT_SLOW:
			retval=SendStringViaSer(CDTV_rec, "!F3\n");
			break;

		case DO_FADE_OUT_FAST:
			retval=SendStringViaSer(CDTV_rec, "!F4\n");
			break;

		case DO_MUTE_ON:
			retval=SendStringViaSer(CDTV_rec, "!M1\n");
			break;

		case DO_MUTE_OFF:
			retval=SendStringViaSer(CDTV_rec, "!M2\n");
			break;

		case DO_PAUSE:
			retval=SendStringViaSer(CDTV_rec, "!PA\n");
			break;

		case DO_STOP:
			retval=SendStringViaSer(CDTV_rec, "!ST\n");
			break;

		case DO_SINGLESTEP:
			retval=SendStringViaSer(CDTV_rec, "!SS\n");
			break;

		case DO_HALFSPEED:
			retval=SendStringViaSer(CDTV_rec, "!S2\n");
			break;

		case DO_QUARTERSPEED:
			retval=SendStringViaSer(CDTV_rec, "!S3\n");
			break;

		case DO_VIDEO_NO_AUDIO:
			retval=SendStringViaSer(CDTV_rec, "!S1\n");
			break;

		case DO_SLOMO_OFF:
			retval=SendStringViaSer(CDTV_rec, "!S0\n");
			break;
	}
	return(retval);
}

/******** SendStringViaSer() *******/

BOOL SendStringViaSer(struct CDTV_record *CDTV_rec, STRPTR str)
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);
	}

	CDTV_rec->serialIO->IOSer.io_Length 	= strlen(str);
	CDTV_rec->serialIO->IOSer.io_Data			= (APTR)str;
	CDTV_rec->serialIO->IOSer.io_Command	= CMD_WRITE;
	SendIO((struct IORequest *)CDTV_rec->serialIO);

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);
	mask = (1L << CDTV_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}
		if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
		{
			WaitIO((struct IORequest *)CDTV_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)CDTV_rec->serialIO);
	WaitIO((struct IORequest *)CDTV_rec->serialIO);
	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** GetStringFromSer() ********/

BOOL GetStringFromSer(struct CDTV_record *CDTV_rec, STRPTR str, int length)
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;
int i;

	for(i=0; i<length; i++)
		str[i] = '\0';

	if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);
	}

	CDTV_rec->serialIO->IOSer.io_Length 	= length;
	CDTV_rec->serialIO->IOSer.io_Data			= (APTR)str;
	CDTV_rec->serialIO->IOSer.io_Command	= CMD_READ;
	SendIO((struct IORequest *)CDTV_rec->serialIO);

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);
	mask = (1L << CDTV_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}
		if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
		{
			WaitIO((struct IORequest *)CDTV_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)CDTV_rec->serialIO);
	WaitIO((struct IORequest *)CDTV_rec->serialIO);
	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	str[ length ] = '\0';

	return(retval);
}

/******** E O F ********/
