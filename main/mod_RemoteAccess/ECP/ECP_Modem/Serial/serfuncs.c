//	File		: serfuncs.c
//	Uses		: serhost.h
//	Date		: 26 june 1994
//	Author	: ing. C. Lieshout
//	Desc.		: Do some serial reading and writing stuff
//

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>

#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"

void ChangePriSer( int Pri );


//===============================================
//	Name		: OpenSerial
//	Function	: Open all serial stuff
//	Inputs	: SERDAT
//	Result	: TRUE or FALSE
//	Updated	: 24 - 05 - 1994
//
int OpenSerial( SERDAT *ser )
{
	ULONG Temp;

	ser->secsig.on = 0;					// switch off second signaling

	ser->DOSBase         =  0;
	ser->SerialMP			=  0;
	ser->SerialWriteMP 	=  0;
	ser->SerialIO			=  0;
	ser->SerialWriteIO 	=  0;
	ser->DevOpen			=  1;
	ser->buffer = 0;

	ser->secsig.signum	= -1;

	ser->DOSBase = OpenLibrary( "dos.library", 0 );

	if( !install50hz( (struct wjif *)&ser->secsig.on,50L ) )
		return( 0 );

	ser->buffer = AllocMem( PROTO_BUFFER_SIZE, MEMF_ANY );

	if( ser->buffer == NULL )
		return( 0 );

	ser->secsig.signum = AllocSignal( -1L );
	if( ser->secsig.signum == -1 )
		return( 0 );

	ser->secsig.signal = 1L<< ser->secsig.signum;

//	printf("signal seconds %lx\n",ser->secsig.signal );
//	printf("signal seconds %lx\n",ser->secsig.signum );

	ser->SerialMP			= CreatePort(0,0);
	ser->SerialWriteMP 	= CreatePort(0,0);

/*
	printf("unit %d, controlbits %x, readbuffer %d Baudrate %d devname  [%s]\n",
						ser->pref.unit_number,
						ser->pref.controlbits,
						ser->pref.read_buffer_size,
						ser->pref.baudrate,
						ser->pref.devname );
*/
	if( ser->SerialMP && ser->SerialWriteMP )
	{
		ser->SerialIO=(struct IOExtSer *)CreateExtIO(ser->SerialMP,sizeof(struct IOExtSer));
		ser->SerialWriteIO = (struct IOExtSer *)CreateExtIO( ser->SerialWriteMP,sizeof(struct IOExtSer) );
		if( ser->SerialIO && ser->SerialWriteIO )
		{

			ser->SerialIO->io_SerFlags = ser->pref.controlbits;
			ser->SerialIO->io_SerFlags |= SERF_XDISABLED;
			ser->SerialIO->io_ExtFlags = 0;

			ser->DevOpen = OpenDevice(ser->pref.devname,ser->pref.unit_number,(struct IORequest *)ser->SerialIO,0 );

			if( ser->DevOpen )
				printf("Serial.device did not open\n");
			else
			{
//	printf("All seems well\n");

				ChangePriSer( 127 );

				ser->SerialIO->io_RBufLen 		= ser->pref.read_buffer_size;
				
				ser->SerialIO->IOSer.io_Command = SDCMD_SETPARAMS;
				ser->SerialIO->io_SerFlags     &= ~SERF_PARTY_ON;

				ser->SerialIO->io_SerFlags     = 0;
				ser->SerialIO->io_SerFlags     |= SERF_XDISABLED;
				ser->SerialIO->io_SerFlags     |= ser->pref.controlbits;

				ser->SerialIO->io_Baud          = ser->pref.baudrate;
				ser->SerialIO->io_ReadLen = 8;
				ser->SerialIO->io_WriteLen = 8;
				ser->SerialIO->io_StopBits = 1;
				ser->SerialIO->io_ExtFlags = 0;

				ser->SerialIO->IOSer.io_Flags = 0;

				if ( Temp = DoIO((struct IORequest *)ser->SerialIO) )
					printf("Error setting parameters - code %ld!\n",Temp);

				CopyMem( ser->SerialIO, ser->SerialWriteIO, sizeof(struct IOExtSer) );
				ser->SerialWriteIO->IOSer.io_Message.mn_ReplyPort = ser->SerialWriteMP;

				return( 1 );
			}
		}
	}
	return( 0 );
}

//===============================================
//	Name		: FreeSerial
//	Function	: Close all serial stuff
//	Inputs	: SERDAT
//	Result	: TRUE or FALSE
//	Updated	: 24 - 05 - 1994
//
void FreeSerial( SERDAT *ser )
{

	if( ser->buffer )
		FreeMem( ser->buffer, PROTO_BUFFER_SIZE );

	if( ser->DOSBase )
		CloseLibrary( ser->DOSBase );

	remove50hz( &ser->secsig );

	if( ser->secsig.signum != -1 )
		FreeSignal( ser->secsig.signum );
		
	if( !ser->DevOpen )
	{
		AbortIO((struct IORequest *)ser->SerialIO);
		WaitIO((struct IORequest *)ser->SerialIO);

		AbortIO((struct IORequest *)ser->SerialWriteIO);
		WaitIO((struct IORequest *)ser->SerialWriteIO);

		ChangePriSer( 0 );							// change Pri to original state
		CloseDevice((struct IORequest *)ser->SerialIO);
	}

	if ( ser->SerialMP)
		DeletePort( ser->SerialMP );

	if ( ser->SerialIO )
		DeleteExtIO( (struct IORequest *)ser->SerialIO );

	if ( ser->SerialWriteMP)
		DeletePort( ser->SerialWriteMP );

	if ( ser->SerialWriteIO )
		DeleteExtIO( (struct IORequest *)ser->SerialWriteIO );
}

void ChangePriSer( int Pri )
{
	struct ExecBase *b;
	struct Node *node;

	b = (struct ExecBase*) *(long *)4;
	node = FindName( &b->DeviceList, "serial.device" );
	if( node )
		node->ln_Pri = Pri;
}

//===============================================
//	Name		: Send and recieve serial bytes
//	Function	:
//	Inputs	:
//	Result	: NONE
//	Updated	: 24 - 05 - 1994
//
void RequestSerByte( SERDAT *ser, char *buf )
{
	ser->SerialIO->IOSer.io_Command	= CMD_READ;
	ser->SerialIO->IOSer.io_Length	= 1;
	ser->SerialIO->IOSer.io_Data	= (APTR)buf;
	SendIO((struct IORequest *)ser->SerialIO);
}

void RequestSer( SERDAT *ser, char *buf, int size )
{
	ser->SerialIO->IOSer.io_Command	= CMD_READ;
	ser->SerialIO->IOSer.io_Length	= size;
	ser->SerialIO->IOSer.io_Data	= (APTR)buf;
	SendIO((struct IORequest *)ser->SerialIO);
}

int StripSerIn( SERDAT *ser, char *buf, int size )
{
	int r;

	size -= 2;					// safety ????

	ser->SerialIO->IOSer.io_Command	= SDCMD_QUERY;
	DoIO((struct IORequest *)ser->SerialIO);

	r = ser->SerialIO->IOSer.io_Actual;

//	printf("Actual %d\n",r );

	if( r > 0 )
	{
		if( r > size )
			r = size;

		ser->SerialIO->IOSer.io_Command	= CMD_READ;
		ser->SerialIO->IOSer.io_Length	= r;
		ser->SerialIO->IOSer.io_Data	= (APTR)buf;
		DoIO((struct IORequest *)ser->SerialIO);
		buf[r] = 0;
		return( r );
	}
	else
		return( 0 );
}

//===============================================
//	Name		: WaitForReply
//	Function	: Wait until Send bytes are send
//	Inputs	: serdat, pointer to fill buffer, buffersize, bytes needed
//	Result	: Positive when bytes received, negative with timeout
//	Updated	: 24 - 08 - 1994
//
int WaitForReply( SERDAT *ser, char *tt, int size, int Send, int lost, int secs )
{
	int t,to,r,stop=0;
	long m,SerMask,WaitMask;

	size -= 2;						// safety ????

	r = Send;
	if( r > size )
		r = size;

	RequestSer( ser, tt, r );

	ser->secsig.on = 1;					// start 1 second timeout

	to = 0;									// timeout counter is 0

	SerMask = 1L << ser->SerialMP->mp_SigBit;

	WaitMask = SIGBREAKF_CTRL_F | SerMask | ser->secsig.signal;

	while( !stop )
	{
		m = Wait( WaitMask );
		if( m & SerMask )
		{
			if( CheckIO( (struct IORequest *)ser->SerialIO ) )
			{
				WaitIO((struct IORequest *)ser->SerialIO);
				tt[r]=0;
				stop = 1;
			}

		}

		if( m & SIGBREAKF_CTRL_F )
			stop = 3;

		if( m & ser->secsig.signal )
		{
			to++;

			if( IsTheButtonHit() )
				stop = 3;

			if( lost )
				if( CheckConnection( ser ) )
					stop = 3;

			if( to > secs )
			{
				if( CheckIO( (struct IORequest *)ser->SerialIO ) )
				{
					WaitIO((struct IORequest *)ser->SerialIO);		// last attempt
					tt[r]=0;
					stop = 1;
				}
				else
				{
					AbortIO((struct IORequest *)ser->SerialIO);			// abort current mission
					WaitIO((struct IORequest *)ser->SerialIO);
					stop = 2;
//					tt[r]=0;
//					printf("Timeout reply [%s]\n",tt);
				}
			}
		}
	}

	if( stop == 1)
		return( r );
	else
		return( -1 );
}

void WriteSerByte( SERDAT *ser, char mes )
{
	ser->SerialWriteIO->IOSer.io_Command	= CMD_WRITE;
	ser->SerialWriteIO->IOSer.io_Length	= 1;
	ser->SerialWriteIO->IOSer.io_Data	= (APTR)&mes;
	DoIO((struct IORequest *)ser->SerialWriteIO);
}

void WriteSer( SERDAT *ser, char *mes )
{
	ser->SerialWriteIO->IOSer.io_Command	= CMD_WRITE;
	ser->SerialWriteIO->IOSer.io_Length	= strlen( mes );
	ser->SerialWriteIO->IOSer.io_Data	= (APTR)mes;
	DoIO((struct IORequest *)ser->SerialWriteIO);
}

void WriteSerSize( SERDAT *ser, char *mes, long size )
{
	ser->SerialWriteIO->IOSer.io_Command	= CMD_WRITE;
	ser->SerialWriteIO->IOSer.io_Length	= size;
	ser->SerialWriteIO->IOSer.io_Data	= (APTR)mes;
	DoIO((struct IORequest *)ser->SerialWriteIO);
}

void SendSerSize( SERDAT *ser, char *mes, long size )
{
	ser->SerialWriteIO->IOSer.io_Command	= CMD_WRITE;
	ser->SerialWriteIO->IOSer.io_Length	= size;
	ser->SerialWriteIO->IOSer.io_Data	= (APTR)mes;
	SendIO((struct IORequest *)ser->SerialWriteIO);
}

void FlushSer( SERDAT *ser )
{

	AbortIO((struct IORequest *)ser->SerialIO);
	WaitIO((struct IORequest *)ser->SerialIO);

//	ser->SerialIO->IOSer.io_Command	= CMD_FLUSH;
//	DoIO((struct IORequest *)ser->SerialIO);

	ser->SerialIO->IOSer.io_Command	= CMD_CLEAR;
	DoIO((struct IORequest *)ser->SerialIO);

	SetSignal( 0L, 1 << ser->SerialMP->mp_SigBit );		// clear signal
}

//===============================================
//	Name		: CheckConnection
//	Function	: Check the connection
//	Inputs	: serdat
//	Result	: 1 when connection is lost
//	Updated	: 15 - 09 - 1994
//
int CheckConnection( SERDAT *ser )
{
	if( ser->pref.connectionclass != 1 )
	{
		ser->SerialWriteIO->IOSer.io_Command	= SDCMD_QUERY;
		DoIO((struct IORequest *)ser->SerialWriteIO);
		if( ser->SerialWriteIO->io_Status & 0x428 )
		{
//			printf("Lost contact\n");
			return( 1 );
		}
		else
		{
//			printf("contact oke\n");
			return( 0 );
		}
	}
	return( 0 );
}
