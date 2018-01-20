//	File		: sergetdir.h
//	Uses		:
//	Date		: 19 august 1994
//	Author	: Ing. C. Lieshout
//	Desc.		: proto's for sergetdir.c
//

int ListFiles( SERDAT *ser, char *path, char *filename );
int ClearArchive( SERDAT *ser, char *path );
int CopyNewScript( SERDAT *ser, char *path );
