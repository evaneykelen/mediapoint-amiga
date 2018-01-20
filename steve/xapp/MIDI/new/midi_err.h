//	File		:	midi_err.h
//	Uses		:
//	Date		:	25-04-93
//	Author	:	ing. C. Lieshout
//	Desc.		:	Error codes for the camdxapp
//

#define FILE_OKE 0
#define CANNOT_OPEN_ERR 1
#define BAD_FILE_ERR 2
#define UNKNOWN_HEADER_ERR 3
#define CANT_PARSE_ERR 4
#define TOO_MANY_TRAX_ERR 5
#define WRONG_TIME_BASE_ERR 6
#define NO_MEM_ERR 7
#define MISSING_TRACKS_ERR 8
#define NO_MEM_LINK_ERR 9
#define NO_MEM_NODE_ERR 10
#define NO_MEM_WORK_ERR 11
#define NO_SIGNAL_ERR 12
#define NO_PLAYER_ERR 13
#define NO_STOP_COND_ERR 14
#define NO_START_COND_ERR 15
#define NO_PLAY_ATTR1_ERR 16
#define NO_PLAY_ATTR2_ERR 17
#define NO_CAMD_LIB_ERR 18
#define NO_REAL_LIB_ERR 19
#define NO_FILENAME_ERR 20
#define OBAD_FILE_ERR 21
#define FORMAT_ERR 22
#define USER_ABORT 23
#define NO_PORTS_ERR 24

char *errors[25]=
{
	"Oke",
	"Cannot open file",
	"Bad file",
	"Unknown header",
	"Can't parse",
	"Too many trax",
	"Wrong time base",
	"Memory error",
	"Missing tracks",
	"No memory for links",
	"No memory for node",
	"No memory for work area",
	"Couldn't allocate signal",
	"Couldn't allocate player",
	"Couldn't stop conductor",
	"Couldn't start conductor",
	"Couldn't allocate timer1",
	"Couldn't allocate timer2",
	"Camd library not found",
	"Realtime library found",
	"File not found",
	"Bad file",
	"Format error",
	"User abort",
	"Couldn't open ports"
};