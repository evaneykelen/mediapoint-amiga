// File		: parse.c
// Uses		:
//	Date		: 17 january 1993
// Updated	: 20 july 1993 ( @command option ), 28 sept 1993 ( @var option )
// Author	: ing. C. Lieshout
// Desc.		: Functions to parse a document file
//

#include <stdio.h>
#include <string.h>
#include	<ctype.h>
#include <time.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include "localedate.h"
//#include "pascal:include/textedit.h"
#include "pascal:include/txed.h"
#include <pragmas/exec_pragmas.h>

char *keywords[] = { 	"PAGETALK",
							 	"OBJECTSTART",
								"SCREEN",
								"PALETTE",
								"WINDOW",
								"CLIP",
								"TEXT",
								"FILE",
								"EFFECT",
								"FORMAT",
								"CRAWL",
								"OBJECTEND",
								"BACKGROUND",
								"CLIPANIM", 0		};

char *comwords[] = {
	"DATE",
	"TIME",
	"SECS",
	"DYNR",
	"LD",
	"SD",
	"LM",
	"SM",
	"LY",
	"SY",
	"FILE", 0 };

int non_asci( char c )
{
	if( c >= 'A' && c <= 'Z' ) return 0;
	if( c >= 'a' && c <= 'z' ) return 0;
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
			*b++ = c;
			t_s++;
			*dat = *dat + 1;
		}
	}
	*b = 0;
	return t_s;
}

//
// Read n nunber of ints from the input
//
long read_int_args( char **dat, long size, long *array, int n, int fill )
{
	int end_ints = 1;
	long t_s = 0;
	char c;
	int tc = 0;
	int sign = 1;

	for( tc=0; tc < n; tc++ )			// fill array with predefined integer
		array[ tc ] = fill;

	tc = 0;
	c = 0;

	for( tc = 0; tc < n; tc++ )
	{
		t_s += skip( dat, size - t_s );
		if( c == 10 )
		{
//			printf("Error to few arguments\n");
			break;
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
//		printf("%ld, ",array[ tc ] );

		if( c != 10 )
		{
			t_s++;
			*dat = *dat + 1;
		}
	}
//	printf("\n");
	return t_s;
}

int check_string_arg( char *dat, long size )
{
	int t_s = 0;

	while( t_s < size && *dat != 10 && *dat != '"' )
	{
		t_s++;
		dat++;
	}

	if( *dat == '"' )
		return 1;
	else
		return 0;
}

int get_aap_command( char *buf )
{
	int com = 0;
	char t[5];
	t[0] = toupper( buf[0] );
	t[1] = toupper( buf[1] );
	t[2] = toupper( buf[2] );
	t[3] = toupper( buf[3] );
	t[4] = 0;

	while( comwords[ com ] != NULL )
	{
		if( strncmp( t, comwords[ com ], strlen( comwords[ com ] ) ) == 0 )
			return com;
		com++;
	}
	return -1;
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

//===============================================
//	Name		:	paste_filename
//	Function	:	reads a file and adds it to the b string
//	Inputs	:	pointer to filename ( ended by \") and dest string
//	Result	:	a longer string
//	Updated	:	03-08-1993
//
int paste_filename( char *name, char *d )
{
	char c;
	int l = 0;
	FILE *fp;
	int file_size;	

	while( name[ l ] != '"' && name[ l ] != '\0' )
		l++;

	if( l != 0 )
	{
		c = name[ l - 1 ];
		name[ l - 1 ] = 0;
		fp = fopen( name, "r" );
		if( fp != NULL )
		{
			fseek( fp, 0L ,SEEK_END );
			file_size = ftell( fp);
			fseek( fp, 0L ,SEEK_SET );

			if( strlen( d ) + file_size > TEXTEDITSIZE )
				file_size = TEXTEDITSIZE - strlen( d ) - 1;

			d += strlen( d );
			fread( d , 1, file_size , fp );
			fclose( fp );
			d[ file_size ] = 0;
		}
		name[ l - 1 ] = c;			// restore character
	}
	return ++l;
}

//===============================================
//	Name		:	read_add_string_arg
//	Function	:	reads a string from dat and adds it to the b string
//	Inputs	:
//	Result	:	a longer string
//	Updated	:	15-05-1993
//
long read_add_string_arg( char **dat, long size, char *b, struct List *VIList )
{
	int skipit;
	char *t_b;
	int end_str = 1;
	long t_s = 0;
	char c,wd;
	char prev = 0;
	int com, date_form;
	char var_contents[51];
	char *t_dat;
	int t;

	t_dat = *dat;
	t_b = b;
	b += strlen( b );			// set on end string

	while( t_s < size && **dat != '"' && **dat != 10 )
	{
		*dat = *dat+1;
		t_s++;
	}

	if( **dat == 10 )
		return t_s;

	*dat = *dat+1;					// skip '"'
	t_s++;

	end_str = 1;
//	sprintf(tt,"%d",size );
	while( t_s < size && end_str )
	{
		c = **dat;
		if( ( c == '"' && prev != '\\' ) || c == 10 )
			end_str = 0;
		else
		{
			if( c =='@' )	// command string found
			{
				com = get_aap_command( *dat+1 );
				date_form = 0;
				switch( com )
				{
					case 0 : // date
						*dat = *dat + 5;
						wd = **dat;
						*dat = *dat + 1;
						t_s += 6;
						switch( wd )
						{
							case '1' :
								date_form = LD_DATE_1;
								break;
							case '2' :
								date_form = LD_DATE_2;
								break;
							case '3' :
								date_form = LD_DATE_3;
								break;
							case '4' :
								date_form = LD_DATE_4;
								break;
						}
						break;
					case 1 : // time
						*dat = *dat + 5;
						wd = **dat;
						*dat = *dat + 1;
						t_s += 6;
						switch( wd )
						{
							case '1' :
								date_form = LD_TIME_1;
								break;
							case '2' :
								date_form = LD_TIME_2;
								break;
							case '3' :
								date_form = LD_TIME_3;
								break;
						}
						break;
					case 2 : // secs
						*dat = *dat + 5;
						t_s += 5;
						date_form = LD_SECS;
						break;
					case 3 : // daynr
						*dat = *dat + 5;
						t_s += 5;
						date_form = LD_DATE;
						break;
					case 4 : // ld
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_LONG_DAY;
						break;
					case 5 : // sd
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_SHORT_DAY;
						break;
					case 6 : // lm
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_LONG_MONTH;
						break;
					case 7 : // sm
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_SHORT_MONTH;
						break;
					case 8 : //  ly
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_LONG_YEAR;
						break;
					case 9 : // lm
						*dat = *dat + 3;
						t_s += 3;
						date_form = LD_SHORT_YEAR;
						break;

					case 10: // fi-le=\"s:startup-sequence\"
						t_dat = *dat;
						while( **dat != '\\' && **dat != '\0' )
						{
							*dat = *dat + 1;
							t_s++;
						}
						if( *dat == 0 )		// if filename not found print everything
							*dat = t_dat;
						else
						{
							*dat += 2;			// should point to filename
							t_s += 2;
							*b = 0;
							t = paste_filename( *dat, t_b );
							b = t_b;
							b += strlen( b );
							*dat = *dat + t;
							t_s += t;
							date_form = 0;
						}
						prev = c;
						break;
				case -1: // no predefined command check to see if it is a Variable
					skipit = FindVarContents( *dat, VIList, var_contents );
					if( skipit != 0 )
					{
						*dat = *dat + skipit;		// skip the @var
						t_s += skipit;
						strcpy( b, var_contents );		// don't check the size YET ?????????
						b += strlen( b );							// update the destination pointer
						break;
					}																// else just copy the @var in the string
					default:
						*b++ = c;
						t_s++;
						*dat = *dat + 1;
				}
				if( date_form != 0 )
				{
					CreateLocaleTime( b, date_form );
					b += strlen( b );
				}
				prev = c;
			}
			else
			{
				if( c == '"' && prev == '\\' )		// you got a quot
				{
					*(b-1)='"';											// replace \ by "
					prev = ' ';
				}
				else
				{
					prev = c;
					*b++ = c;
				}
				t_s++;
				*dat = *dat + 1;
			}
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

int get_command( char *buf )
{
	int com = 0;

	if( buf[0] == 0 )							// empty line
		return -1;

	while( keywords[ com ] != NULL )
	{
		if( strcmp( buf, keywords[ com ] ) == 0 )
			return com;
		com++;
	}
	return -2;
}
