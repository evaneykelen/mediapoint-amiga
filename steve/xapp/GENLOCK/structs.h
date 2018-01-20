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

struct Genlock_record
{
	/**** start standard part ****/
	struct Window *window;
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
	/**** end standard part ****/
	int mode;
	int alwaysLaced;
};

/******** E O F ********/
