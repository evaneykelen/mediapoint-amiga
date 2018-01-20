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

struct VuPort_record
{
	/**** start standard part ****/
	struct Window *window;
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
	/**** end standard part ****/
	int command;
	int unit;
	int arg1, arg2;
};

/******** E O F ********/
