#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "gen:wait50hz.h"
#include "gen:general.h"

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
					CreateExtIO((struct MsgPort *)CDTV_rec->serialMP,
											sizeof(struct IOExtSer)))
		{
			/**** devicename, unit, iorequest, flags ****/

			CDTV_rec->serialIO->io_SerFlags = SERF_SHARED | SERF_XDISABLED;

			if (OpenDevice(	CDTV_rec->devName, CDTV_rec->portNr,
											(struct IORequest *)CDTV_rec->serialIO, 0L))
				; //UA_WarnUser(2000, NULL, NULL);
			else
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
		else
			; //UA_WarnUser(2002,NULL,NULL);
	}
	else
		; //UA_WarnUser(2003,NULL,NULL);

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
	*baudRate = 9600;
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
	else if ( MatchToolValue(s, "19200") )
		*baudRate = 19200;
	else if ( MatchToolValue(s, "31250") )
		*baudRate = 31250;

	FreeDiskObject(diskObj);

	CloseLibrary((struct Library *)IconBase);
}

/******** SendSerCmd() ********/

void SendSerCmd(struct CDTV_record *CDTV_rec, int cmd, int arg1, int arg2)
{
TEXT sendStr[256];
BOOL SerRetVal=TRUE;

	switch(cmd)
	{
		case DO_PLAYTRACK:
			sprintf(sendStr, "PLAY TRACK %d %s", CDTV_rec->song, CDTV_rec->fadeIn ? "FADE" : "NOFADE");
			SerRetVal = SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_PLAYFROMTO:
			sprintf(sendStr, "PLAY TRACKS %d TO %d %s", CDTV_rec->from, CDTV_rec->to, CDTV_rec->fadeIn ? "FADE" : "NOFADE");
			SerRetVal = SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_PLAYSTARTEND:
			sprintf(sendStr, "PLAY FROM %s TO %s %s", CDTV_rec->start, CDTV_rec->end, CDTV_rec->fadeIn ? "FADE" : "NOFADE");
			SerRetVal = SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_FADE:
			if (arg2==1)	// fade in
			{
				if (arg1==1)	// slow
					SerRetVal = SendStringViaSer(CDTV_rec, "FADE IN IN 10 SECONDS");
				else
					SerRetVal = SendStringViaSer(CDTV_rec, "FADE IN IN 5 SECONDS");
			}
			else
			{
				if (arg1==1)	// slow
					SerRetVal = SendStringViaSer(CDTV_rec, "FADE OUT IN 10 SECONDS");
				else
					SerRetVal = SendStringViaSer(CDTV_rec, "FADE OUT IN 5 SECONDS");
			}
			break;

		case DO_FRONTPANEL:
			if (arg1==1)
				SerRetVal = SendStringViaSer(CDTV_rec, "FRONTPANEL ON");
			else
				SerRetVal = SendStringViaSer(CDTV_rec, "FRONTPANEL OFF");
			break;

		case DO_MUTE:
			if (arg1==1)
				SerRetVal = SendStringViaSer(CDTV_rec, "MUTE ON");
			else
				SerRetVal = SendStringViaSer(CDTV_rec, "MUTE OFF");
			break;

		case DO_PAUSE:
			SerRetVal = SendStringViaSer(CDTV_rec, "PAUSE");
			break;

		case DO_RESET:
			SerRetVal = SendStringViaSer(CDTV_rec, "RESET");
			break;

		case DO_STOP:
			SerRetVal = SendStringViaSer(CDTV_rec, "STOP");
			break;

		case DO_ISPLAYING:
			SerRetVal = SendStringViaSer(CDTV_rec, "ISPLAYING");
			break;

		case DO_GETINFO:
			SerRetVal = SendStringViaSer(CDTV_rec, "GET");
			break;

		case DO_FFWD:
			sprintf(sendStr, "FAST FORWARD %d", arg1);
			SerRetVal = SendStringViaSer(CDTV_rec, sendStr);
			break;

		case DO_FREW:
			sprintf(sendStr, "FAST REWIND %d", arg2);
			SerRetVal = SendStringViaSer(CDTV_rec, sendStr);
			break;
	}

	if (!SerRetVal)
		; //UA_WarnUser(8000,NULL,NULL);
}

/******** SendStringViaSer() *******/

BOOL SendStringViaSer(struct CDTV_record *CDTV_rec, STRPTR str)
{
TEXT recBuff[256];
int i;
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	for(i=0; i<100; i++)
		recBuff[i] = '\0';

	strcpy(recBuff, str);

	if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);
	}

	CDTV_rec->serialIO->IOSer.io_Length 	= 64;
	CDTV_rec->serialIO->IOSer.io_Data			= (APTR)recBuff;
	CDTV_rec->serialIO->IOSer.io_Command	= CMD_WRITE;
	SendIO((struct IORequest *)CDTV_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500 );

	mask =	(1L << CDTV_rec->serialMP->mp_SigBit) | hz_signal;

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

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** GetTwoStringsFromSer() ********/

BOOL GetTwoStringsFromSer(struct CDTV_record *CDTV_rec, STRPTR strA, STRPTR strB)
{
TEXT recBuff[256];
int i;
ULONG mask, signal, class, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);
	}

	/**** clear receive buffer ****/

	for(i=0; i<256; i++)
		recBuff[i] = '\0';

	/**** receive bytes from sender ****/

	CDTV_rec->serialIO->IOSer.io_Length 	= 64;
	CDTV_rec->serialIO->IOSer.io_Data			= (APTR)recBuff;
	CDTV_rec->serialIO->IOSer.io_Command	= CMD_READ;
	SendIO((struct IORequest *)CDTV_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500 );

	mask =	(1L << CDTV_rec->serialMP->mp_SigBit) | hz_signal;

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

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	if (retval)
		sscanf(recBuff, "%s %s", strA, strB);
	else
	{
		strA[0] = '\0';
		strB[0] = '\0';
	}

	return(retval);
}

/******** E O F ********/
