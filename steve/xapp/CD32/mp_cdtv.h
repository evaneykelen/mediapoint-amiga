/**** CDTV host gadgets ****/

struct GadgetRecord Host_GR[] =
{
  0,   0, 411, 148, 0, NULL,	0,												DIMENSIONS, NULL,
 10,  20, 400, 122, 2, NULL,	0,												LOBOX_REGION, NULL,
320, 130, 401, 143, 2, "Exit", 0,												BUTTON_GADGET, NULL,
-1
};

struct Host_record
{
	struct MsgPort *serialMP;
	struct IOExtSer *serialIO;
	TEXT devName[40];
	int portNr;
	int baudRate;
};

/******** E O F ********/
