//	File		: protocolssend.c
//	Uses		: serhost.h
//	Date		: 1 july 1994
//	Author	: ing. C. Lieshout
//	Desc.		: Different modem protocols for transmiting files
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serfiles.h"
#include "serprint.h"

//===============================================
//	Name		: X_Receive
//	Function	: Transmit a file with X modem
//	Inputs	: SERDAT input file
//	Result	: TRUE or FALSE
//	Updated	: 24 - 05 - 1994
//
int X_Transmit( SERDAT *ser, char *in )
{

	BPTR infile;

	unsigned char buf[2048];
	ULONG SerWMask,SerMask,WaitMask,Temp;
	int first,re,to,i,block,stop = 0;
	char check;
	UBYTE ret;


	ser->secsig.on = 1;					// start 1 second timeout

	block = 1;
	to = 0;									// timeout counter is 0
	first = 1;								// set start check to first ON

// first open file and check size

	infile = Open( in, MODE_OLDFILE );

	if( !infile )
		return( FALSE );

// file open

	RequestSerByte( ser, &ret );	// Wait for NAK from receiver to start Xmodem

	SerMask = 1L << ser->SerialMP->mp_SigBit;
	SerWMask = 1L << ser->SerialWriteMP->mp_SigBit;

//	printf("%lx,%lx\n",SerMask,SerWMask );

	WaitMask =	SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | 
					SerMask | SerWMask | ser->secsig.signal;

	to = 0;

	printf("Xmodem started\n\n\n");

	while( !stop )
	{
		while( !stop )
		{
			Temp = Wait( WaitMask );			// wait for the one byte

			if( Temp & SIGBREAKF_CTRL_F )
			{
				stop = 1;
				break;
			}
			if( Temp & SerMask )
			{
				WaitIO((struct IORequest *)ser->SerialIO);	// remove request
				if( ret == NAK )
				{
					printf("Nak received\n");
					break;
				}
				else
					if( ret = ACK )
					{
//						printf("Ack received\n");
						block++;
						break;
					}
					else
					{
						printf("Waiting for NAK or ACK got (%x)\n",ret );
						RequestSerByte( ser, &ret );	// Resend wait for NAK
					}
			}

			if( Temp & ser->secsig.signal )
			{
				to++;
				if( to > 30 )
				{
					stop = 1;
					printf("Timed out X-modem\n");
					break;
				}
			}
		}

// the NAK is in send the first block
//	printf("block %d\n",block );

		if( ret == ACK || ( ret == NAK && block == 1 && first) )
		{
			if( block == 1)
				Seek( infile, 0, OFFSET_BEGINNING );

			re = Read(infile, &buf[3], 1024 );

//			printf("file read %d\n",re );

			if( re == 0 )
			{
				WriteSerByte(ser,EOT);
				stop = 0;
				break;
			}
		}

		if( block > 250 )			// once loop switch off first
			first = 0;

		block &= 0xff;

		buf[0] = 0x02;				// 1024 bytes blocks
		buf[1] = block;
		buf[2] = 255 - block;

		for( check = 0,i = 3; i < 1027; i++ )
			check += buf[i];

		buf[ 1027 ] = check;

//printf("%x,%x,%x,check %x\n",(UBYTE)buf[0],(UBYTE)buf[1],(UBYTE)buf[2],(UBYTE)buf[1027]);

		SendSerSize( ser, buf, 1028 );
//		printf("Block send %d\n",(UBYTE)buf[1]);

		to = 0;										// set timeout to zero

		while( !stop )
		{
			Temp = Wait( WaitMask );			// wait for the block to finish

			if( Temp & SIGBREAKF_CTRL_F )
			{
				stop = 1;
				break;
			}

			if( Temp & SerWMask )
			{
				WaitIO((struct IORequest *)ser->SerialWriteIO);	// remove request
//				printf("Block asend %d\n",(UBYTE)buf[1]);
//			Delay(25L);										// WAIT a sec for sender to get ready
				RequestSerByte( ser, &ret );	// Resend wait for NAK or ACK
				break;
			}
			if( Temp & ser->secsig.signal )
			{
				to++;
				if( to > 30 )
				{
					stop = 1;
					printf("Timed out Send X-modem\n");
					break;
				}
			}
		}
	}

	if( infile )
		Close( infile );

// place this in the wile for safety

	WaitIO((struct IORequest *)ser->SerialWriteIO);

	RequestSerByte( ser, &ret );	// Resend wait for ACK


// wait for the last ACK
	to = 0;

	while( !stop )
	{
		Temp = Wait( WaitMask );			// wait for the one byte

		if( Temp & SIGBREAKF_CTRL_F )
		{
			stop = 1;
			break;
		}
		if( Temp & SerMask )
		{
			printf("Last Ack received\n");
			WaitIO((struct IORequest *)ser->SerialIO);	// remove request
			stop = 1;
			break;
		}
		if( Temp & ser->secsig.signal )
		{
			to++;
			if( to > 10 )
			{
				stop = 1;
				printf("Timed out wait for ACK X-modem\n");
				break;
			}
		}
	}

	FlushSer( ser );
	Delay(50L);										// WAIT a sec for sender to get ready
	printf("Exit command\n");
	return( 1 );
}

//===============================================
//	Name		: Y_Transmit
//	Function	: Transmit a file with Y-b modem protocol
//				: With this you can do batch processing
//				: so continue until a null file
//	Inputs	: SERDAT and pointer to filenames to transmit
//	Result	: TRUE or FALSE
//	Updated	: 01 - 06 - 1994
//
int Y_Transmit( SERDAT *ser, char *in, char *out, long stamp )
{

	FileInfo fi;

	BPTR infile;
	char tt[256];

	unsigned char buf[2048];
	ULONG SerWMask,SerMask,w_bytes,WaitMask,Temp;
	int first_NAK,ctrl_f,first,re,to,i,block,stop = 0;
	char check;
	UBYTE ret;
	UBYTE buf128=0;
	UBYTE nameblocksend = 0;

	long t,y,z,filesize;

	ser->secsig.on = 1;					// start 1 second timeout

	block = 0;
	to = 0;									// timeout counter is 0
	first = 1;								// set start check to first ON
	first_NAK = 0;

// first open file and check size

//	printf("wacht\n");
//	getch();

	if( GetDateSize( ser, in, &fi ) )
	{
		printf("YTRANS ret FALSE on [%s]\n",in );
		{
			printf("Abort date not found Send CANCAN\n");
			sprintf(buf,"%c%c",CAN,CAN);
			WriteSer( ser, buf );						// send CANCAN abort
			stop = 2;
		}

		return( FALSE );
	}

	if( stamp > 0 )
		fi.stamp = stamp;									// use stamp from original file

	infile = Open( in, MODE_OLDFILE );

	if( !infile )
	{
		{
			printf("Abort File not open Send CANCAN\n");
			sprintf(buf,"%c%c",CAN,CAN);
			WriteSer( ser, buf );						// send CANCAN abort
		}

		return( FALSE );
	}

	filesize = fi.size;

	printf("filesize is %ld\n",filesize );

// file open

	SerMask = 1L << ser->SerialMP->mp_SigBit;
	SerWMask = 1L << ser->SerialWriteMP->mp_SigBit;

	WaitMask =	SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | 
					SerMask | SerWMask | ser->secsig.signal;

	to = 0;
	w_bytes = 0;
	ctrl_f = 0;

	printf("Ymodem Transmit started\n\n\n");

	SerPrintProt( "Ymodem-b Transmit" );
	sprintf(tt,"filename [%s]",out );
	SerPrint( tt );

	while( StripSerIn( ser, buf, 2040 ) );		// strip all chars previous

	RequestSerByte( ser, &ret );	// Wait for NAK from receiver to start Ymodem

// Clear buffer but one byte

	ret = 0;

	while( !stop )
	{
		while( !stop )
		{
			Temp = Wait( WaitMask );			// wait for the one byte

			if( Temp & SIGBREAKF_CTRL_F )
				ctrl_f = 1;

			if( Temp & SIGBREAKF_CTRL_C )
			{
				stop = 2;
				break;
			}

			if( Temp & SerMask )
			{
				WaitIO((struct IORequest *)ser->SerialIO);	// remove request
				if( ret == NAK )
				{
					first_NAK = 1;					// there is a starting NAK received
					printf("Nak received\n");
					break;
				}
				else
					if( ret = ACK )
					{
						if( !first_NAK )
						{
							printf("Got ACK without NAK\n");
							RequestSerByte( ser, &ret );	// Resend wait for NAK
						}
						else
						{
							if( !nameblocksend )
							{
//								printf("Ack received\n");
								block++;
								break;
							}
							else
							{
//								printf("Got ACK for nameblock\n");
								nameblocksend = 0;
								block++;
								RequestSerByte( ser, &ret );	// Resend wait for NAK
							}
						}
					}
					else
					{
						printf("Waiting for NAK or ACK got (%x)\n",ret );
						RequestSerByte( ser, &ret );	// Resend wait for NAK
					}
			}

			if( Temp & ser->secsig.signal )
			{
				to++;
				if( to > 30 )
				{
					stop = 3;
					printf("Timed out Y-modem\n");
					break;
				}
			}
		}

		buf128 = 0;

// The first block send the file info 
	printf("b %d, ret %d, first %d\n",block,ret,first );

		if( block == 0 &&  ret == NAK && first )
		{
			for( check = 0,i = 3; i < 131; i++ )
				buf[i] = 0;

			printf("Send first block with filename\n");

			sprintf( &buf[3],"%s%c%d%c%d", out,0,filesize,0,fi.stamp );

		printf("in file [%s], out [%s]\n",in,out );
		printf("date %d,%d,%d,%d\n",fi.ds.ds_Days,fi.ds.ds_Minute,fi.ds.ds_Tick,fi.stamp );
			buf128 = 1;
			nameblocksend = 1;
		}

// the NAK is in send the first block

//	printf("block %d\n",block );

		if( ret == ACK || ( ret == NAK && block == 1 && first) )
		{
			if( block == 1 )
			{
				Seek( infile, 0, OFFSET_BEGINNING );
				w_bytes = 0;
			}

			re = Read(infile, &buf[3], 1024 );

//			printf("file read %d\n",re );

			w_bytes += re;
			if( re == 0 )
			{
				WriteSerByte(ser,EOT);
				stop = 0;
				break;
			}
		}

		if( block > 250 )			// once loop switch off first
			first = 0;

		block &= 0xff;

		if( ctrl_f == 1 )
		{
			printf("Abort Send CANCAN\n");
			sprintf(buf,"%c%c",CAN,CAN);
			WriteSer( ser, buf );						// send CANCAN abort
			stop = 2;
			break;
		}

		if( buf128 == 0 )
		{
			buf[0] = 0x02;				// 1024 bytes blocks
			buf[1] = block;
			buf[2] = 255 - block;

			for( check = 0,i = 3; i < 1027; i++ )
				check += buf[i];

			buf[ 1027 ] = check;

			SendSerSize( ser, buf, 1028 );
			SerPrintUpdate( fi.size, w_bytes );
		}
		else
		{
			buf[0] = 0x01;				// 128 bytes blocks
			buf[1] = block;
			buf[2] = 255 - block;

			for( check = 0,i = 3; i < 131; i++ )
				check += buf[i];

			buf[ 131 ] = check;
			SendSerSize( ser, buf, 132 );

/*for(i=0; i<131;i++)
	{
		printf("[%x,%c]",buf[i],buf[i]);
		if( !(i%8))printf("\n");
	}
*/

//			Delay(25L);										// WAIT a sec for sender to get ready

		}

//		printf("Block send %d\n",(UBYTE)buf[1]);

		to = 0;										// set timeout to zero

		while( !stop )
		{
			Temp = Wait( WaitMask );			// wait for the block to finish

			if( Temp & SIGBREAKF_CTRL_F )
			{
				ctrl_f = 1;
			}
			if( Temp & SIGBREAKF_CTRL_C )
			{
				stop = 2;
				break;
			}

			if( Temp & SerWMask )
			{
				WaitIO((struct IORequest *)ser->SerialWriteIO);	// remove request
//				printf("Block asend %d\n",(UBYTE)buf[1]);
//			Delay(25L);										// WAIT a sec for sender to get ready
				RequestSerByte( ser, &ret );	// Resend wait for NAK or ACK
				break;
			}
			if( Temp & ser->secsig.signal )
			{
				to++;
				if( to > 30 )
				{
					stop = 3;
					printf("Timed out Send Y-modem\n");
					break;
				}
			}
		}
	}

	if( infile )
		Close( infile );


// place this in the while for safety ????

	WaitIO((struct IORequest *)ser->SerialWriteIO);

	RequestSerByte( ser, &ret );	// Resend wait for ACK

// wait for the last ACK
	to = 0;

	while( !stop )
	{
		Temp = Wait( WaitMask );			// wait for the one byte

		if( Temp & SIGBREAKF_CTRL_F )
		{
			stop = 2;
			break;
		}
		if( Temp & SerMask )
		{
			WaitIO((struct IORequest *)ser->SerialIO);	// remove request
			break;
		}
		if( Temp & ser->secsig.signal )
		{
			to++;
			if( to > 10 )
			{
				stop = 3;
				printf("Timed out wait for ACK X-modem\n");
				break;
			}
		}
	}

	if( !stop )
	{
		for( check = 0,i = 3; i < 131; i++ )
			buf[i] = 0;

		buf[0] = 0x01;				// 128 bytes blocks
		buf[1] = 0;
		buf[2] = 255 - 0;
		for( check = 0,i = 3; i < 131; i++ )
			check += buf[i];
		buf[ 131 ] = check;
		SendSerSize( ser, buf, 132 );
		WaitIO((struct IORequest *)ser->SerialWriteIO);

		RequestSerByte( ser, &ret );	// Resend wait for ACK
	}

// wait for the last ACK

	to = 0;

	while( !stop )
	{
		Temp = Wait( WaitMask );			// wait for the one byte

		if( Temp & SIGBREAKF_CTRL_F )
		{
			stop = 2;
			break;
		}
		if( Temp & SerMask )
		{
			WaitIO((struct IORequest *)ser->SerialIO);	// remove request
		printf("Last ACK binnen\n");
			stop = 1;
			break;
		}
		if( Temp & ser->secsig.signal )
		{
			to++;
			if( to > 10 )
			{
				stop = 3;
				printf("222Timed out wait for ACK X-modem\n");
				break;
			}
		}
	}
	printf("Exit command %d\n",stop);
	FlushSer( ser );

	if( stop == 1 || stop == 0 )
	{
		Delay(50L);										// WAIT a sec for sender to get ready
		return( 0 );
	}

	return( stop );
}

