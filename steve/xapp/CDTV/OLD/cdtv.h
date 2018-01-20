/**** CDTV record ****/

struct CDTV_record {
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

/*
	int control;
	int portNr;

	TEXT devName[40];
	int portNr;
	int baudRate;
ç*/

#define CONTROL_VIA_SERIAL	0
#define CONTROL_VIA_CDROM		1

#define DO_PLAYTRACK			1
#define DO_PLAYFROMTO			2
#define DO_PLAYSTARTEND		3
#define DO_RESET					4
#define DO_MUTE						5
#define DO_FRONTPANEL			6
#define DO_PAUSE					7
#define DO_STOP						8
#define DO_ISPLAYING			9
#define DO_GETINFO				10
#define DO_FADE						11
#define DO_FFWD						12
#define DO_FREW						13

/******** E O F ********/
