/**** CDTV record ****/

struct CDTV_record
{
	struct IOStdReq *IOReq1;
	struct IOStdReq *IOReq2;
	struct MsgPort  *IOPort;
	struct CDTOC Toc[100];
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	int action, command;				// action is e.g. play track or play from to
															// command is e.g. fade out slowly.
	int song;										// play song
	int from, to;								// play from...to
	TEXT start[10], end[10];
	BOOL fadeIn;
	int control;								// serial, cd-rom etc.		

	struct Window *window;

	TEXT devName[40];
	int portNr;
	int baudRate;
};

#define CONTROL_VIA_SERIAL	0
#define CONTROL_VIA_CDROM		1

#define DO_PLAYTRACK				0
#define DO_PLAYFROMTO				1
#define DO_PLAYSTARTEND			2
#define DO_MUTE_ON					3
#define DO_MUTE_OFF					4
#define DO_PAUSE						5
#define DO_STOP							6
#define DO_FADE_IN_SLOW			7
#define DO_FADE_IN_FAST			8
#define DO_FADE_OUT_SLOW		9
#define DO_FADE_OUT_FAST		10
#define DO_GET_INFO					11
#define DO_GET_TRACK				12

#define DO_SINGLESTEP				20
#define DO_HALFSPEED				21
#define DO_QUARTERSPEED			22
#define DO_VIDEO_NO_AUDIO		23
#define DO_SLOMO_OFF				24

// Length of strings returned over serial port

#define GI_LENGTH 31
#define GT_LENGTH 21

/******** E O F ********/
