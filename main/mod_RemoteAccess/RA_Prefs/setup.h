struct StringRecord Edit_BR_SR = { 6, "       " };
struct StringRecord Edit_DE_SR = { 30, "                               " };
struct StringRecord Edit_BS_SR = { 6, "       " };
struct StringRecord Edit_PR_SR = { 4, "     " };
struct StringRecord Edit_DP_SR = { 60, "                                                                " };
struct StringRecord Edit_SP_SR = { 8, "         " };

UBYTE CR_Text_1[] = "0\0  1\0  2\0  3\0  4\0  5\0  6\0  7\0  8\0  9\0  10\0 11\0 12\0 13\0 14\0 15\0 ";
									 /*-----+++++-----+++++-----+++++-----+++++-----+++++-----+++++-----+++++-----+++++*/

UBYTE CR_Text_2[] = "Serial cable\0     Parallel cable\0   SCSI\0             Modem\0            Network\0          Data broadcast\0   Infrared\0         Email\0            ISDN\0             Leased line\0      ";
									 /*-------------------+++++++++++++++++++-------------------+++++++++++++++++++-------------------+++++++++++++++++++-------------------+++++++++++++++++++-------------------+++++++++++++++++++*/

UBYTE CR_Text_3[] = "None\0    RTS/CTS\0 ";
									 /*----------++++++++++*/

struct CycleRecord Edit_UN_CR = { 0, 16,  4, CR_Text_1, 0 };
struct CycleRecord Edit_CC_CR = { 0, 10, 18, CR_Text_2, 0 };
struct CycleRecord Edit_SW_CR = { 0,  2,  9, CR_Text_3, 0 };

struct GadgetRecord Edit_GR[] =
{
  0,   0, 600, 191, 0, NULL,	0,												DIMENSIONS, NULL,
  0,   0, 599, 190, 2, NULL,	0,												DBL_BORDER_REGION, NULL,
 12,   3, 588,  12, 0, "Remote Access Settings", 0,			TEXT_REGION, NULL,
 12,  15, 588,  15, 0, NULL, 0,													LO_LINE, NULL,
 12, 172,  93, 185, 2, "OK", 0,													BUTTON_GADGET, NULL,
506, 172, 588, 185, 2, "Cancel", 0,											BUTTON_GADGET, NULL,
 12, 167, 588, 167, 0, NULL, 0,													LO_LINE, NULL,
196,  18, 444,  31, 1, "Baudrate:", 0,									INTEGER_GADGET, (struct GadgetRecord *)&Edit_BR_SR, 
196,  34, 444,  47, 1, "Device:", 0,										STRING_GADGET, (struct GadgetRecord *)&Edit_DE_SR, 
196,  50, 444,  63, 1, "Unit:", 0,											CYCLE_GADGET, (struct GadgetRecord *)&Edit_UN_CR, 
196,  66, 444,  79, 1, "Handshaking:", 0,								CYCLE_GADGET, (struct GadgetRecord *)&Edit_SW_CR,
196,  82, 444,  95, 1, "Buffer size:", 0,								INTEGER_GADGET, (struct GadgetRecord *)&Edit_BS_SR,
196,  98, 444, 111, 1, "Priority:", 0,									INTEGER_GADGET, (struct GadgetRecord *)&Edit_PR_SR,
196, 114, 444, 127, 1, "Connection class:", 0,					CYCLE_GADGET, (struct GadgetRecord *)&Edit_CC_CR,
196, 130, 444, 143, 1, "Default path:", 0,							SPECIAL_STRING_GADGET, (struct GadgetRecord *)&Edit_DP_SR,
196, 146, 444, 159, 1, "Super password:", 0,						STRING_GADGET, (struct GadgetRecord *)&Edit_SP_SR,
-1
};

struct EasyStruct warningES =
{
	sizeof(struct EasyStruct),	// es_StructSize
	0,													// es_Flags
	"Warning",									// *es_Title
	"%s",												// *es_TextFormat
	"OK"												// *es_GadgetFormat
};	

/******** E O F ********/
