#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "demo:gen/wait50hz.h"
#include "structs.h"

void WaitMilli(int milli);

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

BOOL SendStringViaSer(struct SerRecord *ser_rec, UBYTE *val)
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)ser_rec->serialIO) )
	{
		AbortIO((struct IORequest *)ser_rec->serialIO);
		WaitIO((struct IORequest *)ser_rec->serialIO);
	}

	ser_rec->serialIO->IOSer.io_Length 	= 1L;
	ser_rec->serialIO->IOSer.io_Data		= (APTR)val;
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

	if ( ser_rec->pacing )
		WaitMilli(ser_rec->pacing);

	return(retval);
}

/******** DoSerial() ********/

BOOL DoSerial(struct SerRecord *ser_rec)
{
BOOL retval=FALSE;
int i,j,val;
TEXT sub[10];
UBYTE str[2];

	str[1]='\0';

	if ( Open_SerialPort(ser_rec) )
	{
		for(i=0; i<strlen(ser_rec->data); i++)
		{
			// look for [xxx] or [xx] or [x]

			if ( ser_rec->data[i]=='[' )
			{
				j=0;
				while(j<3)
				{
					if ( ser_rec->data[i] )
					{
						if ( ser_rec->data[i]==']' )
							break;
						else if ( isdigit(ser_rec->data[i]) )
							sub[j++] = ser_rec->data[i];
						i++;
					}
					else
						break;
				}
				sub[j] = '\0';
				sscanf(sub,"%d",&val);
				str[0] = (UBYTE)val;
				retval=SendStringViaSer(ser_rec,str);
			}

			// look for {xx} or {x}

			if ( ser_rec->data[i]=='{' )
			{
				j=0;
				while(j<2)
				{
					if ( ser_rec->data[i] )
					{
						if ( ser_rec->data[i]=='}' )
							break;
						else if ( isxdigit(ser_rec->data[i]) )
							sub[j++] = ser_rec->data[i];
						i++;
					}
					else
						break;
				}
				sub[j] = '\0';
				sscanf(sub,"%x",&val);
				str[0] = (UBYTE)val;
				retval=SendStringViaSer(ser_rec,str);
			}

			// look for string between '

			if ( ser_rec->data[i]=='\'' )
			{
				while(1)
				{
					i++;
					if ( ser_rec->data[i] )
					{
						if ( ser_rec->data[i]=='\'' )
							break;
						else
						{
							str[0] = (UBYTE)ser_rec->data[i];
							retval = SendStringViaSer(ser_rec,str);
							if ( !retval )
								break;
						}
					}
					else
						break;
				}
			}

			// delay 1 sec.

			if ( ser_rec->data[i]=='^' )
				WaitMilli(1000);
		}

		Close_SerialPort(ser_rec);
	}

	return(retval);
}

/******** WaitMilli() ********/

void WaitMilli(int milli)
{
struct wjif WJIF;
ULONG hz_signal=0, signal;

	WJIF.signum = 0;
	hz_signal = set50hz(&WJIF,milli/25);
	while(1)
	{
		signal = Wait( hz_signal );
		if ( signal & hz_signal )
			break;
	}
	if ( hz_signal != 0 )
		remove50hz(&WJIF);
}

/******** E O F ********/
