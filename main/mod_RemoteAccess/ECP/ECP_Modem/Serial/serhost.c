// File		: Serhost.c
// Uses		:
//	Date		: 22 june 1994
// Author	: ing. C. Lieshout
// Desc.		: PLay with the serial device
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#ifdef LATTICE
#include <proto/exec.h>
#include <stdio.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL-C handling */
#endif

#include <stdio.h>
#include <string.h>

#include "serhost.h"
#include "serfuncs.h"
#include "serprefs.h"
#include "serfiles.h"
#include "serrexx.h"
#include "serprint.h"
#include "sergetdir.h"
#include "protocols.h"

#define WAITTIME 12

char *Asci[] = {	"NUL","SOH","STX","ETX","EOT","ENQ","ACK",
						"BEL","BS ","TAB","LF ","VT ","FF ","CR ",
						"SO ","SI ","DLE","DC1","DC2","DC3","DC4",
						"NAK","SYN","ETB","CAN","EM ","SUB","ESC",
						"FS ","GS ","RS ","US ",0 };

int ExDos( char *in );
void PrintFile( char *filename, SERDAT *ser );
int Command( char *in, SERDAT *ser );
int SayRun ( SERDAT *ser );
int doswap( SERDAT *ser );
int CheckStack( void );
int OtherSer( void );
void DoInitString( SERDAT *ser );
void QuitOtherSer( void );

#define Dprintf printf

BOOL IsTheButtonHit(void){return( 0 ); }

int main( int argc, char *argv[] )
{

	struct MsgPort *serport;
	struct Message	*Msg;

	SERDAT ser;

	char SerialReadBuffer[READ_BUFFER_SIZE];

	short int echo = 0;
	ULONG Temp;
	ULONG WaitMask;
	ULONG SerMask,PortMask;

	int i=0;
	int stop = 0;
	long oldpri;
	struct Task *task;

	if( !CheckStack() )
	{
		printf("Stacksize should be 20000\n");
		return( 1 );
	}
	
	if( OtherSer() )
	{
		if( argc >= 2 )
		{
			if( argv[1][0]=='q' || argv[1][0]=='Q' )
			{
				printf("Removing other host\n");
				QuitOtherSer();
				return( 2 );
			}
			if( argv[1][0]=='o' || argv[1][0]=='O' )
			{
				printf("Removing other host\nRestarting\n");
				QuitOtherSer();
			}
			else
			{
				printf("Serhost already running\n");
				return( 2 );
			}

		}
		else
		{
			printf("Serhost already running\n");
			return( 2 );
		}
	}

	serport = CreatePort("SerPort", 0);

	if( serport == NULL )
	{
		printf("Can't open SerPort\n");
		return( 3 );
	}

	read_prefs( &ser,"prefs.serial" );
	SayRun( &ser );

	task = FindTask( 0 );
	oldpri = SetTaskPri(task, ser.pref.priority );

#if _SERPRINT
	if( InitPrint() )
#endif
	{

	if( OpenSerial( &ser ) )
	{

		DoInitString( &ser );

		if( ser.pref.connectionclass == 1 )
			echo = 1;

//		printf("Waiting CheckIO %lx\n",CheckIO((struct IORequest *)ser.SerialWriteIO));
		WaitIO((struct IORequest *)ser.SerialWriteIO);

		PortMask = 1L << serport->mp_SigBit;

		SerMask = 1L << ser.SerialMP->mp_SigBit;

		WaitMask = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | SerMask | PortMask;

		RequestSerByte( &ser, &SerialReadBuffer[0] );

		printf("Sleeping until CTRL-C, CTRL-F, or serial input %d\n",echo);

		while( !stop )
		{
			Temp = Wait(WaitMask);

			if( SIGBREAKF_CTRL_F & Temp)
			{
#if _PRINT
				printf("BREAKME\n");
#endif
				break;
			}

			if( Temp & PortMask )
			{

				while ( Msg = (struct Message *)GetMsg((struct MsgPort *)serport))
					ReplyMsg( (struct Message *)Msg );
				break;
			}

			if( Temp & SerMask )
			{
				WaitIO((struct IORequest *)ser.SerialIO);
//				printf("%x,",SerialReadBuffer[i]);

				if( SerialReadBuffer[i] == 8 )
				{
					if( i > 0 )
					{
						if( echo )
							WriteSer( &ser,"\x08 \x8");
						i--;
					}
				}
				else
				if( SerialReadBuffer[i] != 0xd && i < READ_BUFFER_SIZE-2 )
				{
					if( (UBYTE)SerialReadBuffer[i] < (UBYTE)' ' || SerialReadBuffer[i] == '·' )
					{
						if( SerialReadBuffer[i] == '·' )
						{
#if _PRINT
							printf("No reply for '·'\n");
#endif
						}
//						else
//							printf("Non asci %x received\n",(UBYTE)SerialReadBuffer[i] );
					}
					else
					{
						if( echo )
							WriteSerByte( &ser, SerialReadBuffer[i] );
						i++;
						SerialReadBuffer[i] = 0;
					}
				}
				else
				{
					// end of line received check the string
					if( echo )
						WriteSer( &ser, "\n\r");
					SerialReadBuffer[i] = 0;
#if _PRINT
					printf("i = %d,string is [%s]\n",i, SerialReadBuffer );
#endif

					if( !strncmp(SerialReadBuffer, "CONNECT", 7 ) )
					{	
						echo = 1;
#if _PRINT
						printf("Echo on\n");
#endif
					}

					if( !strncmp(SerialReadBuffer, "NO CARR", 7 ) )
					{	
						echo = 0;
						if( ser.ID == -1 )
						{
#if _PRINT
							printf("All seems oke\n");
#endif
						}
						else
						{
							ser.ID = -1;
#if _PRINT
							printf("Echo off\nAuto logoff\n");
#endif
						}
					}


					if( !Command( SerialReadBuffer, &ser ) && ser.ID != -1 )
					{
						ExDos( SerialReadBuffer );
						PrintFile( "ram:dout", &ser );
					}
					if( echo )
						WriteSer( &ser, "\n\r·");
					i = 0;
				}

				if(  ser.ID != -1 && CheckConnection( &ser ) )
				{	
					echo = 0;
					ser.ID = -1;
//#if _PRINT
					printf("Lost Contact\n");
					printf("Echo off\nAuto logoff\n");
//#endif
				}

				// Got byte start for another one
				RequestSerByte( &ser, &SerialReadBuffer[i] );
			}
		}

	}
	FreeSerial( &ser );
	if( serport )
		DeletePort( (struct MsgPort *)serport );

#if _SERPRINT
	ExitPrint();
#endif
	}
	SetTaskPri( task,oldpri );
	return( 0 );
}

int CheckStack(void)
{
	struct Process *process;
	struct CommandLineInterface *CLI;

	process = (struct Process *)FindTask( 0 );

	process->pr_WindowPtr = (void *)-1;													// switch off requesters

	CLI = (struct CommandLineInterface *)BADDR(process->pr_CLI);

	if ( (CLI->cli_DefaultStack*4) < 20000 )
		return( 0 );
	else
		return( 1 );
}


void DoInitString( SERDAT *ser )
{
	char com[256];
	if( ser->pref.connectionclass == 4 )
	{
		WriteSer( ser, "ATZ\xd");
		Delay(150L);

		while( StripSerIn( ser, com, 255 ) );

		WriteSer( ser, "ATS0=3\xd");
		Delay(50L);
		while( StripSerIn( ser, com, 255 ) );
		WriteSer( ser, "ATm0\xd");
		Delay(50L);
		while( StripSerIn( ser, com, 255 ) );
	}
	else
		if( ser->pref.connectionclass == 10 )
		{
			WriteSer( ser, "ATZ\xd");
			Delay(150L);

			while( StripSerIn( ser, com, 255 ) );
			WriteSer( ser, "ATm0\xd");
			Delay(50L);
			while( StripSerIn( ser, com, 255 ) );
			WriteSer( ser, "AT&l1\xd");
			Delay(50L);
			while( StripSerIn( ser, com, 255 ) );
			WriteSer( ser, "ATa\xd");
		}
}

int OtherSer( void )
{
	int ret = 0;

	Forbid();
	if( FindPort("SerPort") )
		ret = 1;
	Permit();

	return( ret );
}

int SafePutToPortQuit( struct Message *message, STRPTR portname)
{
	struct MsgPort *port;

	Forbid();
	port = FindPort(portname);
	if (port)
		PutMsg(port,message);
	Permit();
	return( (BOOL)port ); /* If zero, the port has gone away */
}

void QuitOtherSer( void )
{
	struct MsgPort *replyport=0;
	struct Message msg;

	replyport = CreatePort(0,0);

	if( replyport )
	{
		msg.mn_Node.ln_Type = NT_MESSAGE;
		msg.mn_Length = sizeof( struct Message );
		msg.mn_ReplyPort = replyport;
		if( SafePutToPortQuit( &msg, "SerPort") )
		{
			WaitPort( replyport );
			GetMsg( replyport );
		}
		DeletePort( replyport );
	}

}

//===============================================
//	Name		: GetFiles
//	Function	: Get files from directory name
//	Inputs	: pointer to SERDAT and pointer to dirname
//	Result	: None
//	Updated	: 19 - 09 - 1994
//
void GetFiles2 ( SERDAT *ser, char *name )
{
#if _PRINT
	printf("get files from [%s]\n",name );
#endif
	ListFiles(ser, name,"ram:dout");
	PrintFile( "ram:dout", ser );

}

int GetFiles ( SERDAT *ser, char *name, char *outname )
{
#if _PRINT
	printf("get files from [%s] to [%s]\n",name, outname );
#endif

	if( !ListFiles(ser, name, outname ) )
	{
		if( Y_Transmit( ser, outname,outname,0 ) )
			return( 1 );
	}
	else 
		return( 1 );

	return( 0 );
}

//===============================================
//	Name		: SayRun
//	Function	: Check which script runs
//	Inputs	: pointer to SERDAT
//	Result	: 1 or 2 or 0 when file not found
//	Updated	: 19 - 09 - 1994
//
int SayRun ( SERDAT *ser )
{
	char str[256];
	FILE *f;	
	int check=0;
	int stop;

	stop = WAITTIME;

	while( stop > 0 )
	{
		Forbid();
		f = fopen("MP_RA:swapping","r" );
		if( f )
		{
			fclose( f );
			Permit();
			Delay( 100 );
			stop--;
		}
		else
		{
			Permit();
			if( stop != WAITTIME )
				Delay(500);
			stop = -10;
		}
	}

//	sprintf(str,"%sRUNS_script.1",ser->dirname );
	f = fopen( "MP_RA:RUNS_script.1","r" );

	if( f )
	{
		check =1;
		fclose( f );
	}

	if( check ==  0 )
	{
//		sprintf(str,"%sRUNS_script.2",ser->dirname );
		f = fopen( "MP_RA:RUNS_script.2", "r");
		if( f )
		{
			check =2;
			fclose( f );
		}
	}

	ser->pref.run = check;
	ser->pref.notrun = check;

	if( check == 1 )
		ser->pref.notrun = 2;

	if( check == 2 )
		ser->pref.notrun = 1;

	if( stop == -10 )
		return( check );
	else
		return( 0 );

}

//===============================================
//	Name		: GetSecParameter
//	Function	: Copy the second parameter in in to uit
//	Inputs	: pointer to the command
//	Result	: TRUE or FALSE
//	Updated	: 30 - 05 - 1994
//
int GetSecParameter( char *in, char *uit )
{
	int j,i,e;

	e = strlen( in );

	for( i =0; i < e; i++ )		// find space
		if( in[i] == ' ' )
			break;
	i++;

	if( i >= e )
		return( FALSE );
	

	if( in[i] == '"' )
	{
		i++;
		j = 0;

		while( in[i] != '"' && in[i] != 0 && i < e)
			uit[j++] = in[i++];
		uit[j] = 0;
	}
	else
	{
		j = 0;
		while( in[i] != ' ' && in[i] != '"' && in[i] != 0 && i < e)
			uit[j++] = in[i++];
		uit[j] = 0;
	}
	return( TRUE );
}

//===============================================
//	Name		: GetThirdParameter
//	Function	: Copy the third parameter in in to uit
//	Inputs	: pointer to the command
//	Result	: TRUE or FALSE
//	Updated	: 23 - 08 - 1994
//
int GetThirdParameter( char *in, char *uit )
{
	int j,i,e;

	e = strlen( in );

	for( i =0; i < e; i++ )		// find space
		if( in[i] == ' ' )
			break;
	i++;

	if( i >= e )
		return( FALSE );

	if( in[i] != '"' )
	{
		for(; i < e; i++ )		// find space 2
			if( in[i] == ' ' )
				break;
	}
	else
	{
		i++;
		for(; i < e; i++ )		// find space 2
			if( in[i] == '"' )
				break;
		i++;
	}

	i++;

	if( i >= e )
		return( FALSE );

	
	if( in[i] == '"' )
	{
		i++;
		j = 0;

		while( in[i] != '"' && in[i] != 0 && i < e)
			uit[j++] = in[i++];
		uit[j] = 0;
	}
	else
	{
		j = 0;
		while( in[i] != ' ' && in[i] != '"' && in[i] != 0 && i < e)
			uit[j++] = in[i++];
		uit[j] = 0;
	}
	return( TRUE );
}

void sayprefs( SERDAT *ser )
{
	char tt[512];
	char end[3];

	if( ser->pref.lftocr )			// need to convert CR or LF's
		strcpy(end,"\n\r");
	else
		strcpy(end,"\n");

	SayRun( ser );						// set the run var

	sprintf(tt,"Script %d runs%s",ser->pref.run, end );
	WriteSer(ser, tt );

	sprintf(tt,"Baud          : %d%s",ser->pref.baudrate,end );
	WriteSer(ser, tt );
	sprintf(tt,"Unit          : %d%s",ser->pref.unit_number,end );
	WriteSer(ser, tt );
	sprintf(tt,"Buffer size   : %d%s",ser->pref.read_buffer_size,end );
	WriteSer(ser, tt );
	sprintf(tt,"Priority      : %d%s",ser->pref.priority,end );
	WriteSer(ser, tt );
	sprintf(tt,"Device name   : %s%s",ser->pref.devname,end );
	WriteSer(ser, tt );
//	sprintf(tt,"MP path       : %s%s",ser->dirname,end );
//	WriteSer(ser, tt );
	sprintf(tt,"Upload path   : %s%s",ser->pref.currentname,end );
	WriteSer(ser, tt );
//	sprintf(tt,"Download path : %s%s",ser->pref.Upname,end );
//	WriteSer(ser, tt );
	if( ser->pref.lftocr )			// need to convert CR or LF's
		sprintf(tt,"Convert CR to LF is on\n");
	else
		sprintf(tt,"Convert CR to LF is off\n");
	WriteSer(ser, tt );
}

char *keywords[] = { 	"LOGON",					// 0
							 	"DOSWAP",				// 1
								"UPLOADY",				// 2
								"CD",						// 3
								"-SETUPATH",			// 4
								"-DOWNLOADX",			// 5
								"DOWNLOADY",			// 6
								"WHICHRUNS",			// 7
								"CHALIAS",				// 8
								"GETPREFS",				// 9
								"REBOOT",				// 10
								"CHANGEPW",				// 11
								"LOGOFF",				// 12
								"CCOPY",					// 13
								"CHSCRIPT",				// 14
								"COPYSCRIPT",			// 15
								 0	};




//===============================================
//	Name		: Command
//	Function	: check if the string is a command and execute
//	Inputs	: pointer to the command
//	Result	: TRUE or FALSE
//	Updated	: 25 - 05 - 1994
//
int Command( char *in, SERDAT *ser )
{
	int i,error,t,com = 0;
	char name[250];
	char name2[250];
	char tt[250];

	while( keywords[ com ] != NULL )
	{

//printf("in[%s],[%s]\n",in,keywords[ com ]);
		if( strnicmp( in, keywords[ com ] , strlen( keywords[com])) == 0 )
			break;
		com++;
	}

	if( keywords[ com ] == NULL )
		return( FALSE );

//	printf("Ser id is %d\n",ser->ID );
	if( ser->ID != -1 && com != 0 )
	switch( com )
	{
		case 0 :
			WriteSer(ser, "You first need to logoff\n" );
			break;
		case 1 :				// doswap
			if( ( error = doswap( ser ) ) )
				WriteSer( ser, "\rError\n" );
			else			
				WriteSer( ser, "\rOk\n" );
			break;
		case 2 :

			printf("uploady wait\n");getch();

			if( ( error = Yb_Receive( ser, "" ) ) )
				WriteSer( ser, "\rError\n" );
			else			
				WriteSer( ser, "\rOk\n" );
			break;
		case 3 :
			if( GetSecParameter( in, name ) && ChangeDirectory( name,name2 ) )
				strncpy( ser->pref.currentname, name2, 250 );
			sprintf(tt,"%s\n",ser->pref.currentname);
			WriteSer(ser, tt );
			WriteSer( ser, "\rOk\n" );
			break;
		case 4 :
/*			if( GetSecParameter( in, name ) )
			{
//				strncpy( ser->dirname, name, 250 );
				strncpy( ser->pref.Downname, name, 250 );
			}
			sprintf(tt,"Upload path set to [%s]\n\r",ser->pref.Downname);
			WriteSer(ser, tt );
			WriteSer( ser, "\rOk\n" );
			break;
*/
		case 5 :				// - downloadx
/*			if( GetSecParameter( in, name ) )
			{
				sprintf(tt,"Download x [%s]\n\r",name );
				WriteSer(ser, tt );
				X_Transmit( ser, name );
				WriteSer( ser, "\rOk\n" );
			}
*/
			break;
		case 6 :
			if( GetSecParameter( in, name ) )
			{
				sprintf(tt,"Download Y [%s]\n\r",name );
				WriteSer(ser, tt );
				Y_Transmit( ser, name, name,0 );
				WriteSer( ser, "\rOk\n" );
			}
			break;

		case 7 : // which runs ?
			t = SayRun( ser );
			sprintf( tt, "<%d%d>\n",t ,t );
			WriteSer( ser, tt );
			WriteSer( ser, "\rOk\n" );
			break;

		case 8 :		// chalias
			sprintf(tt,"assign ALIAS: mp_RA:script%d\n",ser->pref.run );
			ExDos( tt );
//				if( )
//					WriteSer( ser, "\rError\n" );
//				else
			WriteSer( ser, "\rOk\n" );
			break;
		case 9 :		// getprefs
			sayprefs( ser );
			WriteSer( ser, "\rOk\n" );
			break;
		case 10 :			// Reboot
				t = -1;
				if( GetSecParameter( in, name ) )
				if( strnicmp( ser->superpwd, name , MAX_PSWD_SIZE ) == 0 )
					t = ID_SUPER;
				else
					for( i = 0; i < MAX_PSWDS; i++ )
					{
						if( strnicmp( name, ser->passwords[i] , MAX_PSWD_SIZE ) == 0 && ser->passwords[i][0]!=' ' )
						{
							t = i;
							break;
						}
					}
				if( t == -1 )
					WriteSer( ser, "\rError not a correct password\n" );
				else
				{
					WriteSer( ser, "\rTrying to reboot machine\n" );
					ColdReboot();
				}

/*			if( GetSecParameter( in, name ) )
				if( ( error = fcopy( ser, name ) ) )
					WriteSer( ser, "\rError\n" );
				else
					WriteSer( ser, "\rOk\n" );
*/
			break;
		case 11 :			// change password
			if( ser->ID != ID_SUPER )
			{
				if( GetSecParameter( in, name ) )
				{
					if( strlen( name ) != MAX_PSWD_SIZE )
					{
						WriteSer( ser, "Password should be 8 characters long\n");
					}
					else
					{
						for( i = 0; i < MAX_PSWD_SIZE; i++)
							ser->passwords[ ser->ID][i] = name[i];
						WritePassWords( ser );
						sprintf(tt,"New password set to [%.8s]\n",ser->passwords[ser->ID]);
						WriteSer( ser, tt );
					}
				}
			}
			else
			{
				if( GetSecParameter( in, name2 ) )
				{
					if( GetThirdParameter( in, name ) )
					{
						sscanf(name2,"%d",&t);

						if( t < MAX_PSWDS && t >= 0 )
						{
							if( strlen( name ) != MAX_PSWD_SIZE )
							{
								WriteSer( ser, "Password should be 8 characters long\n");
							}
							else
							{
								for( i = 0; i < MAX_PSWD_SIZE; i++)
									ser->passwords[ t ][i] = name[i];
								WritePassWords( ser );
								sprintf(tt,"New password ID %d set to [%.8s]\n",t,ser->passwords[t]);
								WriteSer( ser, tt );
							}
						}
						else
							WriteSer( ser, "ID not known\n");
					}
					else
					{
						sscanf(name2,"%d",&t);
						if( t < MAX_PSWDS && t >= 0 )
						{
							sprintf(tt,"ID %d, name [%.8s]\n",t,ser->passwords[t] );
							WriteSer( ser, tt );
						}
					}
				}
				else
					WriteSer( ser, "Use changepw idnumber newname\n");
			}
			break;
		case 12 :
			if( ser->ID == ID_SUPER )
				sprintf(tt,"Logoff [%.8s]\n",ser->superpwd );
			else
				sprintf(tt,"Logoff [%.8s]\n",ser->passwords[ser->ID] );
			WriteSer( ser, tt );
			WriteSer( ser, "\rOk\n" );
			ser->ID = -1;
			break;

		case 13 : 			// ccopy checks filename with stamp and sets archive bit
			if( GetSecParameter( in, name ) )
			{
				if( GetThirdParameter( in, name2 ) )
				{
					if( ( error = ccopy( ser, name, name2 ) ) )
						WriteSer( ser, "\rError\n" );					// the file needs uploading
					else
						WriteSer( ser, "\rOk\n" );
				}
				else
					WriteSer( ser, "\rError\n" );
			}
			else
				WriteSer( ser, "\rError\n" );
			break;
		case 14 :
			if( SendArexx( "MEDIAPOINT","quit" ) )
				WriteSer( ser, "\rError\n" );
			else
			{
				i = ser->pref.run;
				ser->pref.run = ser->pref.notrun;
				ser->pref.notrun = i;
				WriteSer( ser, "\rOk\n" );
			}
//			ExDos( "sendrexx MEDIAPOINT quit" );
			break;
		case 15 :			// copy script1: to script2:
				if( ser->pref.run == 1 )
					strcpy(name,"MP_RA:script2" );
				else
					strcpy(name,"MP_RA:script1" );

				if( ( error = ClearArchive( ser, name ) ) )
					printf("Error with clearing archive bits");

				if( ser->pref.run == 1 )
					strcpy(name,"MP_RA:script1" );
				else
					strcpy(name,"MP_RA:script2" );
				if( ( error = CopyNewScript( ser, name ) ) )
					WriteSer( ser, "\rError\n" );
				else
					WriteSer( ser, "\rOk\n" );
			break;

		default:
			WriteSer(ser,"Command not yet implemented\n");
	}
	else		// not yet ID'd the user
	{
		if( com == 0 )		// the logon command
		{
			if( GetSecParameter( in, name ) )
			{
#if _PRINT
				printf("check logon [%.8s]\n",name );
#endif
				if( strnicmp( ser->superpwd, name , MAX_PSWD_SIZE ) == 0 )
				{
					ser->ID = ID_SUPER;
//					WriteSer(ser, "Logon SUPER\n");
				}
				else
					for( i = 0; i < MAX_PSWDS; i++ )
					{
						if( strnicmp( name, ser->passwords[i] , MAX_PSWD_SIZE ) == 0 && ser->passwords[i][0]!=' ' )
						{
							ser->ID = i;
							break;
						}
					}
				if( ser->ID == -1 )
				{
					printf("logon failed\n");
					WriteSer(ser, "Logon failed\n");
					WriteSer(ser, "\rError\n");
				}
				else
				{
					printf("logon oke\n");
					WriteSer(ser, "\rOk\n");
					DeleteFile("MP_RA:swap");
				}

			}
		}

	}
	
	return( TRUE );
}

//===============================================
//	Name		: PrintFile
//	Function	: print a file to the stream
//	Inputs	: pointer to the filename
//	Result	: NONE
//	Updated	: 25 - 05 - 1994
//
void PrintFile( char *filename, SERDAT *ser )
{
	BPTR file;
	struct FileInfoBlock *Finfo;
	BPTR	lock;
	long size = 0;
	char *mem = NULL;
	int i;

	Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );
	if( Finfo )
	{
		lock = Lock( filename, ACCESS_READ );
		if( lock != NULL )
		{
			Examine( lock, Finfo );
			size = Finfo->fib_Size;
			UnLock( lock );
		}
		FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
	}

	if( size != 0 )
	{
		file = Open((UBYTE *)filename,MODE_OLDFILE );
		if( file != 0 )
		{
			mem = (char *)AllocMem( size, MEMF_ANY );
			if( mem )
			{
				Read( file, mem, size );

				if( ser->pref.lftocr )			// need to convert CR or LF's
				{
					for( i=0; i < size; i++ )
					{
						WriteSerByte( ser, mem[i] );
						if( mem[i] == 0xa )
							WriteSerByte( ser, 0x0d );
					}
				}
				else
					if( ser->pref.crtolf )		// need to convert CR or LF's
					{
						for( i=0; i < size; i++ )
						{
							if( mem[i] != 0xd )
								WriteSerByte( ser, mem[i] );
						}
					}
					else
						WriteSerSize( ser, mem, size );

				FreeMem( mem, size );
			}
			Close( file );
		}
	}
}

//===============================================
//	Name		: ExDos
//	Function	: Execute a dos command string 
//	Inputs	: Pointer to input string
//	Result	: Error if commanf failed
//				  Output written to ram:dout 
//	Updated	: 25 - 05 - 1994
//
int ExDos( char *in )
{
	BPTR fo;
	int ret = 0;
	
	fo = Open("ram:dout",MODE_NEWFILE );
	if( fo )
	{
		Execute( in, 0, fo );
		Close( fo );
	}
	else
		return 0;
	return ret;
}

//===============================================
//	Name		: doswap
//	Function	: Create a swap file
//	Inputs	:
//	Result	: Error if command failed
//	Updated	: 21 - 09 - 1994
//
int doswap( SERDAT *ser )
{
	BPTR fo;
	int ret = 0;

	fo = Open("MP_RA:swap",MODE_NEWFILE );
	if( fo )
	{
		Write( fo,"SWAP",4);
		Close( fo );
	}
	else
		return 0;
	return ret;
}
