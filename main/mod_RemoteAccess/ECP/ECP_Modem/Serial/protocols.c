//	File		: protocols.c
//	Uses		: serhost.h
//	Date		: 26 june 1994
//	Author	: ing. C. Lieshout
//	Desc.		: Different modem protocols for receiving files
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
#include <time.h>

#include "serhost.h"
#include "serfiles.h"
#include "serfuncs.h"
#if _SERPRINT
#include "serprint.h"
#endif

BOOL Report(STRPTR str);
BOOL IsTheButtonHit(void);

//===============================================
//	Name		: Yb_Receive
//	Function	: Import a file with Y-b modem protocol
//				: With this you can do batch processing
//				: so continue until a null file
//	Inputs	: SERDAT output file
//	Result	: TRUE or FALSE
//	Updated	: 24 - 05 - 1994
//
int Yb_Receive( SERDAT *ser, char *path )
{

	char tt[255],name[256];
	BPTR outfile = NULL;

	unsigned char check,buf[4],*blockbuf;
	ULONG SerMask,WaitMask,Temp,Temp2;
	int i,tb,wbytes,filesize,date,t,block,stop = 0;
	int nextfile = 1;
	int to = 0;
	int rem = 1;
	int started = 0;
	struct DateStamp ds;

	blockbuf = ser->buffer;

	date = 0;

//	getch();
	ser->secsig.on = 1;					// start 1 second timeout

	block = 1;

	WriteSerByte( ser, NAK );			// send NAK to sender to start Xmodem

	SerMask = 1L << ser->SerialMP->mp_SigBit;
	WaitMask = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | SerMask | ser->secsig.signal;

#if _PRINT
	printf("Ymodem started\n\n\n");
#endif
#if _SERPRINT
	SerPrintProt( "Ymodem-b Receive  " );
#endif

	while( !stop )
	{

		if( rem )
		{
			RequestSerByte( ser, &buf[0] );
			rem = 0;
		}

		Temp = Wait( WaitMask );			// wait for the one byte

		if( Temp & SIGBREAKF_CTRL_F )
		{
			stop = 4;
			break;
		}

		if( Temp & SerMask )
		{

			WaitIO((struct IORequest *)ser->SerialIO);
			to = 0;
			rem = 1;

			if( buf[0] == 0x01 )				// Yes this is the Ymodem start
			{
				RequestSer( ser, blockbuf, 131 );

				do			// quit only when a sersignal or a break
				{
					Temp2 = Wait( WaitMask );			// wait for the block the complete
					if( Temp2 & ser->secsig.signal )
					{
						to++;
//						printf("Timeout %d\n",to );
						if( to > 5 )
							if( CheckConnection( ser ) )
							{
								stop = 3;
								break;
							}

						if( to > 30 )
						{
							Temp2 = 0;
							stop = 3;
							break;
						}
					}
				}while( !(Temp2 & SIGBREAKF_CTRL_F) && !( Temp2 & SerMask ) );

				if( Temp2 & SIGBREAKF_CTRL_F )
				{
					stop = 4;
					break;
				}

				if( Temp2 & SerMask )					// the block is in
				{
					WaitIO((struct IORequest *)ser->SerialIO);

		started = 1;

//				printf("block %d,%d, actual %lx\n",buf[0],blockbuf[0],ser->SerialIO->IOSer.io_Actual );

					for( check = 0, i = 2; i < 130; i++ )
						check += blockbuf[i];
					if( check == blockbuf[130] )
					{

//printf("block %x,%d\n",blockbuf[0], nextfile );

						if( blockbuf[0] == 0 && nextfile )
						{
//for(i=2; i<30;i++)
//	printf("[%x,%c]",blockbuf[i],blockbuf[i]);

							block = 1;
							WriteSerByte( ser, ACK );			// send ACK to sender

							nextfile = 0;							// prevent block count 0 next time
							t = strlen( &blockbuf[2] );
							if( t == 0 )
							{
//			printf("name = NULL\n");
								stop = 1;
								break;
							}
							else
							{
								for(i=2; i<131;i++)
									if( blockbuf[i] == ':' )
										break;

								if( blockbuf[i] == ':' )
									name[0] = 0;
								else
									strcpy(name, path );
								strcat(name,&blockbuf[2] );
//for(i=2; i<30;i++)
//	printf("[%x,%c]",blockbuf[i],blockbuf[i]);

#if _PRINT
								printf("filename [%s]\n",name );
#endif
								sprintf(tt,"filename [%s]",name );
#if _SERPRINT
								SerPrint( tt );
#endif
								outfile = Open( name, MODE_NEWFILE );
								if( outfile == NULL )
								{
									outfile = TryToCreate( name );		// try again
									if( outfile == NULL )
									{
										stop = 5;
										break;
									}
								}
								wbytes = 0;

								if( blockbuf[2+t+1] != 0 )
								{
									sscanf(&blockbuf[2+t+1],"%ld",&filesize );
#if _PRINT
									printf("Filesize is %d\n",filesize );
#endif
									t += strlen( &blockbuf[2+t+1] );
									if( blockbuf[2+t+2] != 0 )
									{	
										sscanf(&blockbuf[2+t+2],"%ld",&date );
#if _PRINT
										printf("date is %ld\n",date );
#endif
									}
#if _PRINT
									else
										printf("Nodate\n");
#endif
								}
#if _PRINT
								else
									printf("no file size\n");
#endif
							}
							if( stop == 0 )
								WriteSerByte( ser, NAK );			// send NAK to sender
//							getch();
						}
						else
							if( (UBYTE)blockbuf[0] == block )
							{
								WriteSerByte( ser, ACK );			// send ACK to sender
								if( outfile )
								{
									tb = ( wbytes + 128 > filesize ? filesize - wbytes : 128 );
									wbytes += tb;
									Write( outfile, &blockbuf[2], tb );
#if _SERPRINT
						SerPrintUpdate( filesize, wbytes );
#endif
								}
								block++;									// next block to load
								block &= 0xff;
							}
							else
							if(  blockbuf[0] == block - 1 )		// resend block ?
								WriteSerByte( ser, ACK );			// continue
							else
								WriteSerByte( ser, NAK );			// error out off sequence
					}
					else
					{
						WriteSerByte( ser, NAK );			// error out off sequence
#if _PRINT
						printf("Error in 128 checksum %x,%x,%x,%x\n",block,blockbuf[0],blockbuf[130],check );
#endif
					}
				}

			}
			else
			if( buf[0] == 0x02 )
			{
				RequestSer( ser, blockbuf, 1027 );

				do			// quit only when a sersignal or a break
				{
					Temp2 = Wait( WaitMask );			// wait for the block to complete
					if( Temp2 & ser->secsig.signal )
					{
						to++;
//						printf("Timeout %d\n",to );
						if( to > 5 )
							if( CheckConnection( ser ) )
							{
								stop = 3;
								break;
							}
						if( to > 30 )
						{
#if _PRINT
							printf("Quiting Ymodem %d\n",to );
#endif
							Temp2 = 0;
							stop = 3;
							break;
						}
					}
				}while( !(Temp2 & SIGBREAKF_CTRL_F) && !( Temp2 & SerMask ) );

				if( Temp2 & SIGBREAKF_CTRL_F )
				{
					stop = 2;
					break;
				}
				if( Temp2 & SerMask )					// the block is in
				{
					WaitIO((struct IORequest *)ser->SerialIO);

		started = 1;

//		printf("block %d,%d,%x,%lx\n",buf[0],blockbuf[0],blockbuf[1],ser->SerialIO->IOSer.io_Actual );

					for( check = 0,i = 2; i < 1026; i++ )
						check += blockbuf[i];

					if( check == blockbuf[1026] )
					{
						if( blockbuf[0] == block )
						{
							WriteSerByte( ser, ACK );			// send ACK to sender
							if( outfile )
							{
								tb = ( wbytes + 1024 > filesize ? filesize - wbytes : 1024 );
								wbytes += tb;
								Write( outfile, &blockbuf[2], tb );
#if _SERPRINT
						SerPrintUpdate( filesize, wbytes );
#endif
							}
							block++;									// next block to load
							block&= 0xff;
						}
						else
							if(  blockbuf[0] < block  )		// resend block ?
							{
#if _PRINT
					printf("A resend ?? %x,%x\n",blockbuf[0],block );
#endif
								WriteSerByte( ser, ACK );			// continue
							}
							else
							{
								WriteSerByte( ser, NAK );			// error out off sequence
#if _PRINT
								printf("out of sequence %x,%x\n",blockbuf[0],block );
#endif
							}
					}
					else
					{
						WriteSerByte( ser, NAK );			// error checksum
#if _PRINT
						printf("Error in K checksum %x,%x,%x,%x\n",block,blockbuf[0],blockbuf[1026],check );
#endif
					}
				}
			}
			else
			{
				if( buf[0] == CAN )
				{
					stop = 8;
#if _PRINT
					printf("A CAN received\n\n");
#endif
				}

				if( buf[0] == EOT )
				{
					if( outfile != NULL )
					{
						Close( outfile );
						if( date != 0 )
						{
							ConvertIntToStamp( date, &ds );
#if _PRINT
							printf("date is %d,%d,%d\n",ds.ds_Days,ds.ds_Minute,ds.ds_Tick );
#endif
							SetFileDate( name, &ds );
							SetProtection( name, FIBF_ARCHIVE );
						}
						outfile = NULL;
					}
					nextfile = 1;
					WriteSerByte( ser, ACK );			// send ACK to sender
					WriteSerByte( ser, NAK );			// send NAK for next file
				}

			}

		}

		if( Temp & ser->secsig.signal )			// second timeout
		{
			if( to > 5 )
				if( CheckConnection( ser ) )
				{
					stop = 3;
					break;
				}

			if( to < 30 )
			{
				if( block == 1 && !started )		// The receive is started
				{											// a second nothing received
#if _PRINT
					printf("Send NAK again %d\n",to);
#endif
					WriteSerByte( ser, NAK );		// Start send again
					to++;
				}
				else
					to++;
			}
			else
			{
#if _PRINT
				printf("Timeout %d\n",to);
#endif
				stop = 3;
				break;
			}
		}

	}
#if _PRINT
	printf("Exit command %d \n",stop);
#endif

	if( outfile != NULL )
	{
		Close( outfile );
		outfile = NULL;
	}

	FlushSer( ser );


	if( stop == 1 || stop == 0 )
	{
//		Delay(50L);										// WAIT a sec for sender to get ready
		return( 0 );
	}
	else
	{
#if _PRINT
		printf("Send CANCAN abort\n");
#endif
		sprintf(name,"%c%c",CAN,CAN);
		WriteSer( ser, name );						// send CANCAN abort
//		WriteSer( ser, "\0x18\0x18");				// send CANCAN abort

		FlushSer( ser );
		return( stop );
	}
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

	clock_t dt,starttijd;

	FileInfo fi;

	BPTR infile;
	char tt[256];

	unsigned char *buf;

	ULONG SerWMask,SerMask,w_bytes,WaitMask,Temp;
	int first_NAK,ctrl_f,first,re,to,i,block,stop = 0;

	char check;
	UBYTE ret;
	UBYTE buf128=0;
	UBYTE nameblocksend = 0;

	long filesize;
	buf = ser->buffer;

	filesize = first_NAK=ctrl_f=first=re=to=i=block=stop = 0;
	SerWMask=SerMask=w_bytes=WaitMask=Temp=0;

	StripSerIn( ser, tt, 255 );

	memset( buf, 0, 2048 );
	memset( tt, 0, 256 );

	starttijd = clock();

	ser->secsig.on = 1;					// start 1 second timeout

	block = 0;
	to = 0;									// timeout counter is 0
	first = 1;								// set start check to first ON
	first_NAK = 0;

// first open file and check size

	if( GetDateSize( ser, in, &fi ) )
	{
#if _PRINT
		printf("YTRANS ret FALSE on [%s]\n",in );
#endif
		{
#if _PRINT
			printf("Abort date not found Send CANCAN\n");
#endif
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
#if _PRINT
			printf("Abort File not open Send CANCAN\n");
#endif
			sprintf(buf,"%c%c",CAN,CAN);
			WriteSer( ser, buf );						// send CANCAN abort
		}

		return( FALSE );
	}

	filesize = fi.size;

#if _PRINT
	printf("filesize is %ld\n",filesize );
#endif

// file open

	SerMask = 1L << ser->SerialMP->mp_SigBit;
	SerWMask = 1L << ser->SerialWriteMP->mp_SigBit;

	WaitMask =	SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | 
					SerMask | SerWMask | ser->secsig.signal;

	to = 0;
	w_bytes = 0;
	ctrl_f = 0;

#if _PRINT
	printf("Ymodem Transmit started\n\n\n");
#endif

#if _SERPRINT
	SerPrintProt( "Ymodem-b Transmit" );
	sprintf(tt,"filename [%s]",out );
	SerPrint( tt );
#endif

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
#if _PRINT
					printf("Nak received\n");
#endif
					break;
				}
				else
					if( ret == ACK )
					{
						if( !first_NAK )
						{
#if _PRINT
							printf("Got ACK without NAK\n");
#endif
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
#if _PRINT
						printf("Waiting for NAK or ACK got (%x)\n",ret );
#endif
						RequestSerByte( ser, &ret );	// Resend wait for NAK
					}
			}

			if( Temp & ser->secsig.signal )
			{
				to++;

				if( IsTheButtonHit() )							// stop with a neat CANCAN when it CAN
				{
					ctrl_f = 1;
					Report("IS button 1");
				}

				if( to > 5 )
					if( CheckConnection( ser ) || IsTheButtonHit() )
					{
						stop = 3;
						break;
					}

				if( to > 30 )
				{
					stop = 3;
#if _PRINT
					printf("Timed out Y-modem\n");
#endif
					break;
				}
			}
		}

		buf128 = 0;

// The first block send the file info 
//	printf("b %d, ret %d, first %d\n",block,ret,first );

		if( block == 0 &&  ret == NAK && first )
		{
			for( check = 0,i = 3; i < 131; i++ )
				buf[i] = 0;

#if _PRINT
			printf("Send first block with filename\n");
#endif

			sprintf( &buf[3],"%s%c%d%c%d", out,0,filesize,0,fi.stamp );

#if _PRINT
		printf("in file [%s], out [%s]\n",in,out );
		printf("date %d,%d,%d,%d\n",fi.ds.ds_Days,fi.ds.ds_Minute,fi.ds.ds_Tick,fi.stamp );
#endif
			buf128 = 1;
			nameblocksend = 1;
		}

// the NAK is in send the first block

//	printf("block %d\n",block );

//		printf("%x,%x,%x,%x\n",block,buf[0],buf[1026],check );

#if _PRINT
		if( ret == NAK )
		{
			printf("Error in K checksum %x,%x,%x,%x\n",block,buf[0],buf[1026],check );
		}
#endif

		if( ret == ACK || ( ret == NAK && block == 1 && first) )
		{
			if( block == 1 && first )
			{
#if _PRINT
			printf("Seeking back to begin\n");
#endif

				Seek( infile, 0, OFFSET_BEGINNING );
				w_bytes = 0;
			}

			re = Read(infile, &buf[3], 1024 );

//			printf("file read %d,%d\n",w_bytes,re );

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
#if _PRINT
			printf("Abort Send CANCAN\n");
#endif
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
#if _SERPRINT
			SerPrintUpdate( fi.size, w_bytes );
#endif
			sprintf(tt,"~Bytes %d, %d",fi.size,fi.size - w_bytes );
			Report( tt );
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
				ctrl_f = 1;

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
				if( IsTheButtonHit() )
				{
					Report("IS button 2");
					ctrl_f = 1;
				}

				if( to > 5 )
					if( CheckConnection( ser ) || IsTheButtonHit() )
					{
						stop = 3;
						break;
					}

				if( to > 30 )
				{
					stop = 3;
#if _PRINT
					printf("Timed out Send Y-modem\n");
#endif
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
				if( IsTheButtonHit() )
				{
					Report("IS button 3");
					stop = 3;
					break;
				}
			if( to > 5 )
				if( CheckConnection( ser ) || IsTheButtonHit() )
				{
					stop = 3;
					break;
				}
			if( to > 10 )
			{
				stop = 3;
#if _PRINT
				printf("Timed out wait for ACK X-modem\n");
#endif
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
#if _PRINT
		printf("Last ACK binnen\n");
#endif
			stop = 1;
			break;
		}
		if( Temp & ser->secsig.signal )
		{
			to++;
				if( IsTheButtonHit() )
				{
					Report("IS button 4");
					stop = 3;
					break;
				}
			if( to > 5 )
				if( CheckConnection( ser ) || IsTheButtonHit() )
				{
					stop = 3;
					break;
				}
			if( to > 10 )
			{
				stop = 3;
#if _PRINT
				printf("222Timed out wait for ACK Y-modem\n");
#endif
				break;
			}
		}
	}
#if _PRINT
	printf("Exit command %d\n",stop);
#endif
	FlushSer( ser );

	dt = (float)(clock()-starttijd);

	ser->totalsend += fi.size;
	ser->totaltime += dt;

	ser->bytessend = fi.size;
	ser->time = dt;

	if( stop == 1 || stop == 0 )
	{
//		Delay(50L);										// WAIT a sec for sender to get ready
		Report("Returned from Y-send");
		return( 0 );
	}

	Report("Returned from Y-send with error");
	return( stop );
}
