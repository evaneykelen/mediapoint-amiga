/**** globals ****/

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

struct Pdev_record
{
	/**** start standard part ****/

	struct Window *window;
	//struct MsgPort *serialMP;
	//struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;

	/**** end standard part ****/

	struct Library *PlayerBase;

	struct IOPlayer *IOblock;
	struct MsgPort *port;

	BOOL videoOn, audio1On, audio2On, indexOn;
	int action;
	int param1, param2;
	BOOL blank;
	BOOL init;
};

/******** E O F ********/