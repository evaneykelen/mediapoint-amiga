//	File		: serfuncs.h
//	Uses		:
//	Date		: 26 june 1994
//	Author	: Ing C. Lieshout
//	Desc.		: proto's for serfuncs
//

void WriteSer( SERDAT *ser, char *mes );
void WriteSerByte( SERDAT *ser, char mes );
void WriteSerSize( SERDAT *ser, char *mes, long size );
void SendSerSize( SERDAT *ser, char *mes, long size );
void RequestSerByte( SERDAT *ser, char *buf );
void RequestSer( SERDAT *ser, char *buf, int size );
int StripSerIn( SERDAT *ser, char *buf, int size );
void FlushSer( SERDAT *ser );
int OpenSerial( SERDAT *ser );
void FreeSerial( SERDAT *ser );
int WaitForReply( SERDAT *ser, char *tt, int size, int Send, int lost, int secs );
int CheckConnection( SERDAT *ser );
