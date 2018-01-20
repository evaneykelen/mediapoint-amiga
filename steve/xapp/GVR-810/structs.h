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

struct GVR_record
{
	/**** start standard part ****/
	struct Window *window;
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
	/**** end standard part ****/
	int video;
	int audio;
	int blank;
	int init;
     // Next line added 5/10/93 by Chris Palmer
     TEXT current[15];
	TEXT start[15];
	TEXT end[15];
	int cmd;
};

/******** E O F ********/
