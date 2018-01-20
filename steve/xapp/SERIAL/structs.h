struct SerRecord
{
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;

	UBYTE data[256];
	int baudrate;
	int handshaking;
	int parity;
	int bits_char;
	int stop_bits;
	int unit;
	int pacing;
	TEXT devName[SIZE_FILENAME];
};
