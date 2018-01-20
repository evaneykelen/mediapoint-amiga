#define CMD_FINISHED_DELAY 5L

struct standard_record
{
	/**** start standard part ****/
	struct Window *window;
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
};

struct Ion_record
{
	/**** start standard part ****/
	struct Window *window;
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
	/**** end standard part ****/
	TEXT unitStr[10];
	int action;
	int counterOn;
	int frame;
	int startFrame;
	int endFrame;
	int delay;
	int blank;
};

/******** E O F ********/
