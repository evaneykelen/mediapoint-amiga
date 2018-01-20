// File		: parse.h
// Uses		:
//	Date		: 1 March 1993
// Author	: ing. C. Lieshout
// Desc.		: Protos for the document parser
//
int get_command( char *buf );
long read_word( char **dat, long size, char *b );
long read_string_arg( char **dat, long size, char *b );
long read_add_string_arg( char **dat, long size, char *b, struct List *VIList );
long read_int_args( char **dat, long size, long *array, int n, int fill );
int skip_eoln( char **dat, long size );
long skip( char **dat, long size );
int check_string_arg( char *dat, long size );
