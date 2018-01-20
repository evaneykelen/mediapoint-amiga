//	File		:	midi_err.h
//	Uses		:
//	Date		:	25-04-93
//	Author	:	ing. C. Lieshout
//	Desc.		:	Error codes for the midixapp
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
#define NO_MEM_WORK_ERR 9
#define NO_SIGNAL_ERR 10
#define NO_PLAYER_ERR 11
#define NO_FILENAME_ERR 12
#define OBAD_FILE_ERR 13
#define FORMAT_ERR 14
#define USER_ABORT 15
#define NO_PORTS_ERR 16

#if _XAPP!=TRUE
char *errors[17]=
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
	"No memory for work area",
	"File not found",
	"Bad file",
	"Format error",
	"User abort",
	"Couldn't open ports"
};
#endif