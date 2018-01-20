/**** CD record ****/

struct CD_record
{
	struct IOStdReq *IOReq1;
	struct IOStdReq *IOReq2;
	struct MsgPort  *IOPort;
	struct Device *CD_Device;
	union CDTOC Toc[100];

	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;

	int diskChange;

	int action, command;				// action is e.g. play track or play from to
															// command is e.g. fade out slowly.
	int song;										// play song
	int from, to;								// play from...to
	TEXT start[10], end[10];		// start and end index
	BOOL fadeIn;								// fade before issuing action or command
	int control;								// serial, cd-rom etc.		

	struct Window *window;

	TEXT devName[40];
	int portNr;
	int baudRate;
};

/******** E O F ********/
