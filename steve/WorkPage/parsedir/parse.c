// File		: parse.c
// Uses		:
//	Date		: 17 january 1993
// Autor		: C. Lieshout
// Desc.		: Parse a document file
//

#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>

#define BUF_SIZE 1024


P_INFO *parse_head = NULL;

char *keywords[] = { 	"PAGETALK",
							 	"OBJECTSTART",
								"SCREEN",
								"PALETTE",
								"WINDOW",
								"CLIP",
								"TEXT",
								"FILE",
								"OBJECTEND", 0		};

P_INFO *get_info_struct( P_INFO *info )
{
	P_INFO *n;
	
	n = ( P_INFO *) AllocMem( sizeof( P_INFO ), MEMF_PUBLIC );
	if( n != NULL )
		n->next = NULL;
	/if( info != NULL )
		info->next = n;
	return n;
}

void release_info( P_INFO *info )
{
	if( info->next != NULL )
		release_info( info->next );
	else
	{
		if( info->data )
			FreeMem( info->data, info->datasize );
		FreeMem( info, sizeof( P_INFO ) );
	}
}

void copy_to_info( P_INFO *info, char *data )
{
	int len;

	len = strlen( data );
	info->datasize = len;
	info->data = AllocMem( len, MEMF_PUBLIC );
	if( info->data != NULL )
		strcpy( info->data, data );
}

void print_info( P_INFO *info )
{
	while( info != NULL )
	{
		switch( info->type )
		{
			case 0 : printf("pagetalk\n");
						break;
			case 1 :	printf("start\n");
						break;
			case 2 :	printf("screen %d,%d,%d,%d\n",
							info->parameters[0],
							info->parameters[1],
							info->parameters[2],
							info->parameters[3]);
						break;
			case 3 :	printf("Pallete %d\n",info->parameters[0]);
						break;
			case 4 :	printf("Window %d,%d,%d,%d, l - %d\n",
							info->parameters[0],
							info->parameters[1],
							info->parameters[2],
							info->parameters[3],
							info->parameters[10]);
						break;
			case 5 :	printf("Clip %s,%d,%d,%d,%d\n",
							info->data,
							info->parameters[0],
							info->parameters[1],
							info->parameters[2],
							info->parameters[3] );
						break;
			case 6 :	printf("text %s\n", info->data);
						break;
			case 7 :	printf("File %s\n", info->data );
						break;

			case 8 :	printf("Stop\n");
						break;
			Default	: printf("Error\n");
		}
		info = info->next;
	}
}

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
long read_int_args( char **dat, long size, int *array, int n )
{
	int end_ints = 1;
	long t_s = 0;
	char c;
	int tc = 0;
	int sign = 1;

	c = 0;

	for( tc = 0; tc < n; tc++ )
	{
		t_s += skip( dat, size - t_s );
		if( c == 10 )
		{
			printf("Error to few arguments\n");
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
		printf("%ld, ",array[ tc ] );

		if( c != 10 )
		{
			t_s++;
			*dat = *dat + 1;
		}
	}
	printf("\n");
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


	t_b = b;
	while( t_s < size && **dat != '"' && **dat != 10 )
	{
		*dat = *dat+1;
		t_s++;
	}

	if( **dat == 10 )
	{
		printf("Error string not found\n");
		return t_s;
	}

	*dat = *dat+1;					// skip '"'
	t_s++;

	end_str = 1;
	while( t_s < size && end_str )
	{
		c = **dat;
		if( c == '"' || c == 10 )
			end_str = 0;
		else
		{
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
	printf("str = %s\n",t_b );
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

//
// Parse throught the file in memory
//
void parse( char *dat, long size , P_INFO *info )
{
	int line_num = 1;
	int which_com;

	long t_size;
	char buffer[BUF_SIZE];
	int array[20];

	t_size = size;
	while( t_size > 0 )
	{
		t_size -= skip( &dat, t_size );
		t_size -= read_word( &dat, t_size, buffer );
		which_com = get_command( buffer );
//		printf("l(%d): Cnr. %d  -%s-\n",line_num,which_com,buffer );
		printf("l(%d): ",line_num );

		if( which_com != -1 )
		{
			info = get_info_struct( info );
			info->type = which_com ;
		}

		switch( which_com )
		{
			case -1	: 	printf("Empty line\n");
							break;
			case 0	:	printf("Page talk found\n");
							t_size -= read_int_args( &dat, t_size, info->parameters , 2 );
							break;
			case 1	:	printf("start found\n");
							break;
			case 2	:	printf("screen found\n");
							t_size -= read_int_args( &dat, t_size, info->parameters , 4 );
							break;
			case 3	:	printf("palette found\n");
							t_size -= read_int_args( &dat, t_size, info->parameters , 1 );
							t_size -= read_string_arg( &dat, t_size, buffer );
							copy_to_info( info, buffer );
							break;
			case 4	:	printf("window found\n");
							t_size -= read_int_args( &dat, t_size, info->parameters , 11 );
							break;
			case 5	:	printf("clip found\n");
							t_size -= read_string_arg( &dat, t_size, buffer );
							copy_to_info( info, buffer );
							t_size -= read_int_args( &dat, t_size, info->parameters , 4 );
							t_size -= read_word( &dat, t_size, buffer );
							printf("clip on off %s\n",buffer );
							break;
			case 6 :	printf("Text found\n");
							t_size -= read_string_arg( &dat, t_size, buffer );
							copy_to_info( info, buffer );
							break;

			case 7 :	printf("File found\n");
							t_size -= read_string_arg( &dat, t_size, buffer );
							copy_to_info( info, buffer );
							break;

			case 8	:	printf("stop found\n");
							break;
			default	:	printf("Error at line %d\n",line_num );
							
		}

		t_size -= skip_eoln( &dat, t_size );
		line_num++;
	}
}

char *load_file( FILE *fp, LONG *file_size )
{
	char *t_data = 0;

	fseek( fp, 0L ,SEEK_END );
	*file_size = ftell( fp);
	fseek( fp, 0L ,SEEK_SET );

	printf("lengte is %ld\n",*file_size );

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

void main( int argc, char *argv[] )
{
	FILE *fp;
	char *data = NULL;
	LONG file_size = 0;

	if( argc >= 1 )
	{
		fp = fopen(argv[1],"r");
		if( fp != NULL )
		{
			data = load_file( fp, &file_size );
			
			parse_head = get_info_struct( NULL );
			if( parse_head != NULL )
				parse_head->type = -1;							// dummy

			parse( data , file_size, parse_head );

			print_info( parse_head );

			if( parse_head != NULL )
				release_info( parse_head );

			if( data != NULL )
				FreeMem( data, file_size );
			fclose(fp);
		}
		else
			printf("Can't open file %s\n",argv[1]);
	}
	else
		printf("Usage %s: filename\n",argv[0]);
}
