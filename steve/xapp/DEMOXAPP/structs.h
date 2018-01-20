/******** STRUCTS.H ********/

struct DemoRecord
{
	// START - EMBEDDED SERIAL RECORD
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	int unit;											// read from .info file
	TEXT devName[SIZE_FILENAME];	// read from .info file
	int baudrate;									// read from .info file
	int handshaking;							// 0=XON|XOFF  1=RTS|CTS  2=NONE
	int parity;										// 0=NONE  1=EVEN  2=ODD  3=MARK  4=SPACE
	int bits_char;								// 0=7 bits  1=8 bits
	int stop_bits;								// 0=1 bit  1=2 bits
	// END - EMBEDDED SERIAL RECORD

	TEXT arexxCommand[50];
	TEXT port[50];
	TEXT serialText[50];
	int am_fm;
	int stereo;
	int dolby;
};

/****
struct SerialRecord
{
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	int unit;											// read from .info file
	TEXT devName[SIZE_FILENAME];	// read from .info file
	int baudrate;									// read from .info file
	int handshaking;
	int parity;
	int bits_char;
	int stop_bits;
};
****/

/******** E O F ********/
