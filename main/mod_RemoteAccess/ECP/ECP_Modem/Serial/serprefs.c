// File		: Serprefs.c
// Uses		:
//	Date		: 19 august 1994
// Author	: ing. C. Lieshout
// Desc.		: Read serial pref data
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>

#include <devices/serial.h>

#include "serhost.h"
//#include "serfiles.h"
int ChangeDirectory( char *name, char *path );

char *prefwords[] = { 	"BAUDRATE",
							 	"SEVENWIRE",
								"DEVICE",
								"UNIT",
								"DEFAULT_PATH",
								"BUFFER_SIZE",
								"CONVERTLF",
								"CONVERTCR",
								"PRIORITY",
								"SUPERNAME",
								"CONNECTIONCLASS", 0 };


int non_asci( char c )
{
	if( c >= 'A' && c <= 'Z' ) return 0;
	if( c >= 'a' && c <= 'z' ) return 0;
	if( c == '_' ) return 0;
	return 1;
}

int non_num( char c )
{
	if( c == '-')return 0;
	if( c >= '0' && c <= '9' ) return 0;
	return 1;
}

//
// skip until you find a asci
//
long skip( char **dat, long size )
{
	long t_s = 0;

	while( t_s < size && (**dat == ' ' || **dat == 9 || **dat == ',' ) )
	{
		*dat = *dat+1;
		t_s++;
	}
	return t_s;
}

int skip_eoln( char **dat, long size )
{
	long t_s = 0;

	while( t_s < size && (**dat != 10 ) )
	{
		*dat = *dat+1;
		t_s++;
	}
	*dat = *dat+1;
	t_s++;
	return t_s;

}

long read_word( char **dat, long size, char *b )
{
	int end_word = 1;
	long t_s = 0;
	char c;

	while( t_s < size && end_word )
	{
		c = **dat;
		if( non_asci( c ) )
			end_word = 0;
		else
		{
			*b++ = toupper(c);
			t_s++;
			*dat = *dat + 1;
		}
	}
	*b = 0;
	return t_s;
}

//
// Read a string from the input
//
long read_string_arg( char **dat, long size, char *b )
{

	char *t_b;
	int end_str = 1;
	long t_s = 0;
	char c;
	char prev = 0;

	t_b = b;
	while( t_s < size && **dat != '"' && **dat != 10 )	// find first '"'
	{
		*dat = *dat+1;
		t_s++;
	}

	if( **dat == 10 )
		return t_s;

	*dat = *dat+1;					// skip '"'
	t_s++;

	end_str = 1;
	while( t_s < size && end_str )		// add string until '"' or EOLN
	{
		c = **dat;
		if( ( c == '"' && prev != '\\') || c == 10 )
			end_str = 0;
		else
		{
				prev = c;
				*b++ = c;
				t_s++;
				*dat = *dat + 1;
		}
	}
	*b = 0;

	if( c != 10 )
	{
		t_s++;
		*dat = *dat + 1;
		t_s += skip( dat, size - t_s );
		if( **dat == ',' )
		{
			t_s++;
			*dat = *dat + 1;
		}
	}
	return t_s;
}

//
// Read n nunber of ints from the input
//
long read_int_args( char **dat, long size, long *array )
{
	int end_ints = 1;
	long t_s = 0;
	char c;
	int tc = 0;
	int sign = 1;

	array[0] = 0;

	tc = 0;
	c = 0;

	t_s += skip( dat, size - t_s );
	if( c == 10 )
	{
		return t_s;
	}
	array[ tc ] = 0;
	end_ints = 1;
	while( t_s < size && end_ints )
	{
		c = **dat;

		if( non_num( c ) )
			end_ints = 0;
		else
		{
			if( c == '-' )
				sign = -sign;
			else
				array[ tc ] = array[ tc ] * 10 + (int)( c - '0' );
			t_s++;
			*dat = *dat + 1;
		}
	}
	array[ tc ] *= sign;
	sign = 1;

	if( c != 10 )
	{
		t_s++;
		*dat = *dat + 1;
	}
	return t_s;
}

int get_command( char *buf )
{
	int com = 0;

	if( buf[0] == 0 )							// empty line
		return -1;

	while( prefwords[ com ] != NULL )
	{
		if( strcmp( buf, prefwords[ com ] ) == 0 )
			return com;
		com++;
	}
	return -2;
}

void PrefParse( char *dat, long t_size, SERDAT *ser )
{
	int i,which_com;
	long t;
	char buffer[128];

	while( t_size > 0 )
	{
		t_size -= skip( &dat, t_size );
		t_size -= read_word( &dat, t_size, buffer );

		which_com = get_command( buffer );

		switch( which_com )
		{
			case -2	: 	printf("unkwown command\n");

			case -1	: 	
							break;

			case 0	:	// baudrate
							t_size -= read_int_args( &dat, t_size, &ser->pref.baudrate );
							break;

			case 1	:	// 7wire
							ser->pref.controlbits |= SERF_7WIRE;
							break;
			case 2	:	// device
							t_size -= read_string_arg( &dat, t_size, ser->pref.devname );
							break;
			case 3	:	// unit
							t_size -= read_int_args( &dat, t_size, &ser->pref.unit_number );
							break;
			case 4	:	// default path
							t_size -= read_string_arg( &dat, t_size, ser->dirname );
							break;
			case 5	:	// buffer size
							t_size -= read_int_args( &dat, t_size, &ser->pref.read_buffer_size );
							break;
			case 6 : ser->pref.lftocr = 1;
							break;
			case 7 : ser->pref.crtolf = 1;
							break;
			case 8	:	// unit
							t_size -= read_int_args( &dat, t_size, &ser->pref.priority );
							break;
			case 9	:	// supername
							t_size -= read_string_arg( &dat, t_size, buffer );
							{
								for( i = 0; i < MAX_PSWD_SIZE; i++ )
									ser->superpwd[i] = buffer[i];
							}
							break;
			case 10	:	// ConnectionClass
							t_size -= read_int_args( &dat, t_size, &t );
							ser->pref.connectionclass = t;
							break;

		}
		t_size -= skip_eoln( &dat, t_size );
	}
}

char *load_file( FILE *fp, long *file_size )
{
	char *t_data = 0;

	fseek( fp, 0L ,SEEK_END );
	*file_size = ftell( fp);
	fseek( fp, 0L ,SEEK_SET );

	t_data = (char *)AllocMem( *file_size, MEMF_PUBLIC );

	if( t_data != NULL )
	{
		if( fread( t_data , 1, *file_size , fp ) != *file_size )
			printf("Error couldn't read all bytes\n");
		return t_data;
	}
	else
		return NULL;
}

//===============================================
//	Name		: WritePassWords
//	Function	: Get the passwords from file
//	Inputs	: pointer to SERDAT
//	Result	: None
//	Updated	: 24 - 08 - 1994
//
void WritePassWords( SERDAT *ser )
{
	FILE *pf;
	pf = fopen( "passwords.serial","w" );
	if( pf )
	{
		fwrite( ser->passwords, MAX_PSWDS * MAX_PSWD_SIZE, 1, pf );
		fclose( pf );
	}
}

//===============================================
//	Name		: ReadPassWords
//	Function	: Get the passwords from file
//	Inputs	: pointer to SERDAT
//	Result	: None
//	Updated	: 24 - 08 - 1994
//
void ReadPassWords( SERDAT *ser )
{
	FILE *pf;

	int j,i;

	for( i = 0; i < MAX_PSWDS; i++ )
		for( j = 0; j < MAX_PSWD_SIZE; j++ )
			ser->passwords[i][j] = ' ';			// set all to spaces

	ser->passwords[i][0] = 'c';
	ser->passwords[i][1] = 'h';
	ser->passwords[i][2] = 'a';
	ser->passwords[i][3] = 'n';
	ser->passwords[i][4] = 'g';
	ser->passwords[i][5] = 'e';
	ser->passwords[i][6] = 'm';
	ser->passwords[i][7] = 'e';

	pf = fopen( "passwords.serial","r" );

	if( pf )
	{
		fread( ser->passwords, MAX_PSWDS * MAX_PSWD_SIZE, 1, pf );

//		for( i = 0; i < MAX_PSWDS; i++ )
//			printf("pwds %d [%.8s]\n",i,ser->passwords[i] );

		fclose( pf );
	}
	else					// there is no passwords file create one
		WritePassWords( ser );
}

//===============================================
//	Name		: read_prefs
//	Function	: Get the serprefs from file
//	Inputs	: pointer to SERDAT
//	Result	: None
//	Updated	: 19 - 08 - 1994
//
void read_prefs( SERDAT *ser, char *filename )
{

	FILE *pf;
	char *dat;
	long t_size;
	ser->pref.baudrate = DEFAULT_BAUDRATE;
	ser->pref.controlbits = 0;
	ser->pref.unit_number = UNIT_NUMBER;
	ser->pref.read_buffer_size = SERIAL_BUFFER_SIZE;
	ser->pref.priority = DEFAULT_PRIOR;
	ser->pref.run = 0;
	ser->ID = -1;

	strcpy( ser->pref.devname, DEVICE_NAME );
	strcpy( ser->dirname, DEFAULT_DIRNAME );

//	strcpy( ser->pref.currentname, DEFAULT_PATH );
	strcpy( ser->pref.currentname, "serra:" );

	strncpy( ser->superpwd, SUPER_NAME, MAX_PSWD_SIZE );

	ser->pref.lftocr = 0;
	ser->pref.crtolf = 0;

	pf = fopen( filename, "r" );
	if( pf )
	{
		dat = load_file( pf, &t_size );
		if( dat != NULL )
		{
			PrefParse( dat, t_size, ser );
		}
		else
			printf("Couldn't allocate memory for pref file\n");

		fclose( pf );
	}
	else
		printf("Couldn't open serial pref file '%s'\n",filename );
	ReadPassWords( ser );

	ChangeDirectory( ser->pref.currentname, ser->pref.currentname );
}
