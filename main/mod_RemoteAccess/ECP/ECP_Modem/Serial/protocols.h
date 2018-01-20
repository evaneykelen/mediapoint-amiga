//	File		: protocols.h
//	Uses		:
//	Date		: 26 june 1994
//	Author	: Ing. C. Lieshout
//	Desc.		: proto's for protocols.c
//
int Yb_Receive( SERDAT *ser, char *out );
int Y_Transmit( SERDAT *ser, char *in, char *out, long stamp );
