//	File		: protocolssend.h
//	Uses		:
//	Date		: 1 july 1994
//	Author	: Ing. C. Lieshout
//	Desc.		: proto's for protocolssend.c
//
int X_Transmit( SERDAT *ser, char *in );
int Y_Transmit( SERDAT *ser, char *in, char *out, int stamp );
