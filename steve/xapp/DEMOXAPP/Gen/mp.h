#include "demo:gen/mllib/MediaLinkLib_proto.h"
#include "demo:gen/mllib/MediaLinkLib_pragma.h"

void MakeFullPath(STRPTR, STRPTR, STRPTR);
void GiveMessage(struct Window *, char *, ...);
void StrToScript(char *, char *);
void ScriptToStr(char *, char *);
void RemoveQuotes(STRPTR);

#define RAW_RETURN			0x44
#define RAW_ESCAPE			0x45
#define ML_RENDEZ_VOUS	"MP rendez-vous"
#define MEDIALINKPORT		"MediaPointPort"
#define MAXEDITWINDOWS	50
#define SIZE_FULLPATH		150
#define SIZE_PATH				100
#define SIZE_FILENAME		50

#define USERAPPLIC_MEMSIZE 200L
#define LARGE_USERAPPLIC_MEMSIZE 1000L

#define BUTTON_GADGET					1
#define STRING_GADGET 				2
#define CYCLE_GADGET 					3
#define RADIO_GADGET 					4
#define CHECK_GADGET 					5
#define SPECIAL_STRING_GADGET	6
#define DATE_GADGET						7
#define TIME_GADGET						8
#define INTEGER_GADGET				9
#define HIBOX_REGION					10
#define LOBOX_REGION 					11
#define BORDER_REGION 				12
#define DBL_BORDER_REGION 		13
#define TEXT_REGION 					14
#define DIMENSIONS						15
#define LO_LINE								16
#define TEXT_LEFT							17
#define INVISIBLE_GADGET			18
#define TEXT_RIGHT						19
#define PREV_GR								20
#define DOTTED_LINE						21
#define COMBOBOX_REGION				22
#define HI_LINE								30
#define HI_AREA								31
#define LO_AREA								32
#define HI_PATTERN_LINE				33
#define LO_PATTERN_LINE				34
#define POSPREFS							35

#define HI_PEN		2L	/* pen color to draw top and left lines of buttons */
#define LO_PEN		1L	/* pen color to draw bottom and right lines of buttons */
#define TEXT_PEN	1L	/* pen color to draw text inside buttons */
#define AREA_PEN	3L	/* pen color to draw back of requesters, gadgets etc. */
#define BGND_PEN	0L

#define SPECIAL_TEXT_TOP						1
#define SPECIAL_TEXT_BOTTOM					2
#define SPECIAL_TEXT_LEFT						3
#define SPECIAL_TEXT_RIGHT					4
#define SPECIAL_TEXT_CENTER					5
#define SPECIAL_TEXT_AFTER_STRING 	6
#define SPECIAL_TEXT_BEFORE_STRING	7

#define NO_ICON 					0
#define QUESTION_ICON 		1
#define EXCLAMATION_ICON	2

struct StringRecord
{
	int maxLen;
	UBYTE *buffer;
};

struct GadgetRecord
{
	WORD x1,y1,x2,y2;
	UBYTE color;
	STRPTR txt;
	WORD lanCode;
	WORD type;
	struct GadgetRecord *ptr;
};

struct CycleRecord
{
	int active;
	int number;
	int width;
	STRPTR ptr; 
	WORD lanCode;
};

struct RadioRecord
{
	int active;
	int lowest;
	int highest;
};

struct GenericRecord
{
	int value;
};

struct VectorRecord
{
	WORD x1,y1,x2,y2;
	char type;
};

struct PopUpRecord
{
	struct Window *window;
	struct GadgetRecord *GR;
	UBYTE *ptr;
	int active;
	int number;
	int width;
	int fit;
	int top;
};

struct EventData
{
	int menuNum, itemNum, itemFlags;
	ULONG Class, extraClass;
	USHORT Code;
	USHORT Qualifier;
	APTR IAddress;
	ULONG Seconds, Micros;
	SHORT MouseY, MouseX;
};

struct RendezVousRecord
{
	struct Library *intuition;
	struct Library *graphics;
	struct Library *medialink;
	struct CapsPrefs *capsprefs;
	struct eventHandlerInfo *ehi;
	struct TextFont *smallfont;
	struct TextFont *largefont;
	struct TextFont *systemfont;
	struct Screen *pagescreen;
	struct Screen *scriptscreen;
	struct MsgPort *capsport;
	UWORD *paletteList;
	BOOL returnCode;
	struct ObjectInfo *ObjRec;
	struct BitMap *gfxBM;
	UBYTE *aPtr;	// general ptr, may be used to pass stuff 
	ULONG aLong;	// long, may be used to pass stuff 
	struct Library *console;
	struct EditWindow		*EW[MAXEDITWINDOWS];
	struct EditSupport	*ES[MAXEDITWINDOWS];
	struct Window				*EWL[MAXEDITWINDOWS];
	UBYTE **msgs;
	BYTE *homeDirs, *homePaths;
	BOOL (*openfunc)(APTR,APTR,APTR,APTR,int,BOOL);
	BOOL (*savefunc)(APTR,APTR,APTR,APTR,int);
	UBYTE *aPtrTwo;	// second general ptr, may be used to pass stuff 
	struct TextFont *tiny_smallfont;
	struct TextFont *tiny_largefont;
	struct FER *FontEntryRecord;
	UBYTE *aPtrThree;	// third general ptr, may be used to pass stuff 
	BOOL (*miscfunc)(APTR);
  BOOL (*miscfunc2)(APTR);

	// future additions

	ULONG future_long[31];
};

struct UserApplicInfo
{
	struct Screen *userScreen;
	struct Screen *medialinkScreen;
	struct Window *userWindow;
	int screenWidth, screenHeight, screenDepth, screenModes;
	struct TextAttr small_TA;
	struct TextFont *small_TF;
	struct TextAttr large_TA;
	struct TextFont *large_TF;
	int windowX, windowY, windowWidth, windowHeight, windowModes;
	struct IntuitionBase *IB;
	ULONG wflg;
};

struct FileReqRecord
{
	STRPTR path;
	STRPTR fileName;
	STRPTR title;
	int opts;
	BOOL multiple;
};

#define DIR_OPT_ALL				0x0001
#define DIR_OPT_ILBM			0x0002
#define DIR_OPT_NOINFO		0x0004
#define DIR_OPT_ANIM			0x0008
#define DIR_OPT_THUMBS		0x0010
#define DIR_OPT_ONLYINFO	0x0020
#define DIR_OPT_SCRIPTS		0x0040
#define DIR_OPT_ONLYDIRS	0x0080
#define DIR_OPT_NODIRS		0x0100
#define DIR_OPT_MIDI			0x0200
#define DIR_OPT_CDXL			0x0400
#define DIR_OPT_SAMPLE		0x0800
#define DIR_OPT_MAUD			0x1000
#define DIR_OPT_MUSIC			0x2000
