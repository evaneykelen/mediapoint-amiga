//	File		: Netfiles.h
//	Uses		:
//	Date		: 23 august 1994
//	Author	: Ing. C. Lieshout
//	Desc.		: proto's for NetFiles.c
//

#define COPY_BUF_SIZE 64000
#define BUFFER_REMAIN 100000

typedef struct
{
	long size;
	struct DateStamp ds;
	long stamp;
	long prot;
	UBYTE archive;
	UBYTE temp;

}FileInfo;

void ReportError( int i );
void RemoveQuotes(STRPTR str);
int CreateSwap( char *path );
int CreateCommand( char *path );
char *StripAlias( int script, char *name, char *d, char *path );
int NetSayRun ( char *path );
int icopy( char *sname, char *dname, int stamp );
LONG GetVolumeSize( STRPTR path );
void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest );
int GetDateSize( char *sname, FileInfo *fi );
void ConvertIntToStamp( int i, struct DateStamp *ds );
int ccopy( char *sname, int stamp );
BPTR TryToCreate( char *name );
void CreateDateString( char *dest , char *p);


void LogAction( char *mes );

