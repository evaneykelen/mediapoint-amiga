//	File		: Serfiles.h
//	Uses		:
//	Date		: 23 august 1994
//	Author	: Ing. C. Lieshout
//	Desc.		: proto's for serfiles.c
//

typedef struct
{
	long size;
	struct DateStamp ds;
	long stamp;
	long prot;
	UBYTE archive;
	UBYTE temp;

}FileInfo;

int icopy( SERDAT *ser, char *sname, char *dname );
LONG GetVolumeSize( STRPTR path );
void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest );
int GetDateSize( SERDAT *ser, char *sname, FileInfo *fi );
void ConvertIntToStamp( int i, struct DateStamp *ds );
int ccopy( SERDAT *ser, char *sname, char *strstamp );
BPTR TryToCreate( char *name );
int ChangeDirectory( char *name,char *path );
