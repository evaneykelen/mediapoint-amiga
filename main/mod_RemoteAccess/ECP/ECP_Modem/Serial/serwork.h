// File		: Serwork.h
// Uses		:
//	Date		: 30 august 1994
// Author	: ing. C. Lieshout
// Desc.		: Proto's for the workstation
//

void ReportError( int i );
int Logon( SERDAT *ser, char *name );
int Logoff( SERDAT *ser );
int WhichRuns( SERDAT *ser );
int UpLoadY( SERDAT *ser, char *name, char *outname, int stamp );
int CCopy( SERDAT *ser, char *sname, char *dname, int stamp, int size );
int ChangeScript( SERDAT *ser );
int SwapScript( SERDAT *ser );
int StandaardCommand( SERDAT *ser, char *name );
int ChangeAlias( SERDAT *ser );
int CopySerScript( SERDAT *ser );
int MakeConnection( SERDAT *ser );
void SoundOff( SERDAT *ser );
void SetLeased( SERDAT *ser );
void CreateDateString( char *d , char *p);
