/**** events ****/

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

/**** player device structure ****/

struct PlayerDeviceRecord
{
	TEXT playerName[41];
	int baudRate;
	TEXT deviceName[41];
	int unit;
};

/**** prefs ****/

struct CapsPrefs
{
	int PageScreenWidth,
			PageScreenHeight,
			PageScreenDepth;
	ULONG PageScreenModes;
	ULONG extraPageScreenModes;
	UBYTE PagePalNtsc;
	struct ColorMap *PageCM;

	int ScriptScreenWidth,
			ScriptScreenHeight,
			ScriptScreenDepth;
	ULONG ScriptScreenModes;
	ULONG extraScriptScreenModes;
	UBYTE ScriptPalNtsc;

	int ThumbnailScreenWidth,
			ThumbnailScreenHeight,
			ThumbnailScreenDepth;
	ULONG ThumbnailScreenModes;
	ULONG extraThumbnailScreenModes;

	ULONG scriptMonitorID;
	ULONG pageMonitorID;

	ULONG playerMonitorID;
	UBYTE PlayerPalNtsc;

	BOOL SystemTwo;
	BOOL ECS_available;
	BOOL AA_available;

	UBYTE overScan;

	UBYTE colorSet;

	BOOL WorkBenchOn;

	BOOL locale;
	int countryCode;			// see capsdefines.h

	UBYTE userLevel;

	BOOL showDays;

	TEXT textEditor[130];

	UBYTE thumbnailSize;	// see capsdefines.h

	int MaxNumLists;

	struct FileLock *appdirLock;

	UBYTE printerDest;
	BOOL printerTextOnly;
	int printerCopies;
	UBYTE printerScaleAndOri;
	UBYTE printerQuality;
	int paperLength;

	BOOL fromWB;

	TEXT F1_TIMECODE_STR[20];
	TEXT F2_TIMECODE_STR[20];
	TEXT F3_TIMECODE_STR[20];
	TEXT F4_TIMECODE_STR[20];
	TEXT F5_TIMECODE_STR[20];
	TEXT F6_TIMECODE_STR[20];

	TEXT import_picture_Path[SIZE_FULLPATH];
	TEXT import_text_Path[SIZE_FULLPATH];
	TEXT document_Path[SIZE_FULLPATH];
	TEXT script_Path[SIZE_FULLPATH];
	TEXT anim_Path[SIZE_FULLPATH];
	TEXT music_Path[SIZE_FULLPATH];
	TEXT sample_Path[SIZE_FULLPATH];

	struct PlayerDeviceRecord PDevice;

	UBYTE objectPreLoading;
	UBYTE playOptions;
	UBYTE bufferOptions;
	UBYTE scriptTiming;	// 0 = normal, 1 = precise 

	int maxMemSize;
	int	gosubStackSize;

	BOOL noGenLock;

	UBYTE lanCode;
	TEXT lanExtension[30];

	TEXT customTimeCode[100];

	UBYTE mousePointer;
	UBYTE standBy;
	UBYTE playFrom;

	// gameport stuff
	UBYTE gameport[15];
	BOOL gameport_used;
	ULONG gameport_delay;

	// fast menu
	BOOL fastMenus;

	// monitors

	TEXT scriptMonName[50];
	TEXT pageMonName[50];
	TEXT playerMonName[50];

	ULONG input_delay;

	BOOL fastIcons;
	WORD padding;

	// future additions

	ULONG future[9];
};

/**** gui structs ****/

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

/**** window structs ****/

struct EditSupport
{
	BOOL Active;
	struct IFF_FRAME *iff;
	UBYTE photoOpts;
	TEXT picPath[SIZE_FULLPATH+10];
	struct ColorMap *cm;

	struct BitMap24 ori_bm;
	WORD ori_w, ori_h;

	struct BitMap scaled_bm;
	WORD scaled_w, scaled_h;

	struct BitMap remapped_bm;
	WORD remapped_w, remapped_h;
	WORD remapped_x, remapped_y;	// offsets to ori_bm

	struct BitMap restore_bm;
	WORD restore_w, restore_h;

	struct BitMap mask_bm;
	WORD mask_w, mask_h;

	struct BitMap ori_mask_bm;
	WORD ori_mask_w, ori_mask_h;

	BYTE ditherMode;	// eg DITHER_FLOYD
};

/**** WINDOW INITS ARE DONE IN CONFIG.C, PTREAD.C, BACKWIN.C, WINDOWS.C ****/

struct EditWindow
{
	/**** text editor stuff ****/

	struct MinNode frameNode;			// linked list of frames
	UWORD	FirstChar;							// used internally
	UWORD	LastChar;								// used internally
	UBYTE	antiAliasLevel;					// anti-aliasing level setting
	UBYTE justification;
	struct RastPort *rastPort;
	struct TEInfo *TEI;
	struct BitMap *undoBM;				// points to valid BM or NULL 

	WORD	xSpacing;								// waarde binnen redelijke grenzen, +/- 3 ofzo.
	WORD	ySpacing;								// waarde binnen redelijke grenzen, +/- 3 ofzo.
	UWORD	slantAmount;						// per hoeveel pixels gaan we slantValue verschuiven
																// 0=uit, 1-??
	WORD	slantValue;							// 0=uit, -?? tot ?? pixels verschuiving
	UWORD	underLineHeight;				// 1-?? denk aan afstand baseline tot onderzijde char
	WORD	underLineOffset;				// 0=default, lijn onder baseline. pos = lager

	UBYTE shadowDepth;						// 1-16 pixels, waarde 15 = 16 pixels
	UBYTE	shadow_Pen;							// pen nummer voor shadow
	UBYTE	shadowType;							// pascal:textstyles.h
	UBYTE shadowDirection;				// pascal:textstyles.h

	UBYTE wdw_shadowDepth;				// 1-30
	UBYTE wdw_shadowDirection;		// none, 1,2,3,4
	UBYTE wdw_shadowPen;

	/**** misc stuff ****/

	WORD X,Y,Width,Height;
	WORD TopMargin,BottomMargin,LeftMargin,RightMargin;

	UBYTE Border;
	UWORD BorderColor[4];
	WORD BorderWidth;
	UWORD BackFillType;
	UWORD BackFillColor;
 	WORD PhotoOffsetX, PhotoOffsetY;

	WORD patternNum;

	UWORD DrawSeqNum;

	struct TextFont *charFont;

	//UWORD	charStyle;
	UBYTE underlineColor;
	UBYTE charStyle;

	UBYTE	charColor;

	UBYTE flags;

	/**** transitions stuff ****/

	WORD in1[3], in2[3], in3[3];			// in effects:	-1 is default value
	WORD out1[3], out2[3], out3[3];		// out effects:	-1 is default value
	int inDelay[3], outDelay[3];

	/**** crawl stuff ****/

	TEXT crawl_fontName[50];
	WORD crawl_fontSize;
	UBYTE crawl_speed;
	UBYTE crawl_flags;
	UBYTE *crawl_text;
	UWORD crawl_length;
	UBYTE crawl_color;

	/**** button stuff ****/

	WORD	bx, by, bwidth, bheight;
	UBYTE jumpType;
	UBYTE	renderType;
	UBYTE	audioCue;
	WORD 	keyCode;
	WORD	rawkeyCode;
	TEXT  buttonName[50];
	TEXT  assignment[75];

	/**** animation stuff ****/

	BOOL animIsAnim;
	BYTE animLoops;
	BYTE animSpeed;
	BOOL animFromDisk;
	BOOL animAddFrames;
};

/**** menus ****/

struct MenuRecord
{
	struct BitMap menuBM;
	struct RastPort menuRP;
	int titleX1, titleX2;
	int x,y,width,height;
	TEXT commandKey[16];	/* a maximum of 16 menu items is allowed */
	BOOL shifted[16];
	BOOL disabled[16];
	STRPTR title[16];
};

/**** document struct ****/

struct Document
{
	TEXT title[SIZE_FILENAME+10];
	BOOL modified;
	BOOL untitled;
	BOOL opened;
	TEXT path[SIZE_FULLPATH+10];
};

/**** object info ****/

struct ObjectInfo
{
	struct ScriptInfoRecord scriptSIR;
	struct List *objList;
	struct ScriptNodeRecord *firstObject;
	int maxObjects;	/* e.g. 12 or 24 */
	int numObjects;
};

/**** User application structure for use with our library ****/

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

/**** EHI ****/

struct eventHandlerInfo
{
	int activeScreen;
	BOOL paletteVisible;
	BOOL thumbsVisible;
};

/**** RendezVousRecord for IAC ****/

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

/**** FLI ****/

struct FileListInfo
{
	UBYTE *fileList;
	int numFiles;

	UBYTE *selectionList;

	UBYTE *assignList;
	int numAssigns;

	UBYTE *deviceList;
	int numDevices;

	UBYTE *homeList;
	int numHomes;
};

struct Funcs { void (*func)(void); };

/**** FONT STRUCTS ****/

struct FontEntry
{
	TEXT fontName[51];
	short fontSize[30]; /* was 24 <!> YES, a short; sqsort() only eats shorts. */
};

struct FER
{
	struct FontEntry **FEList;
	int numEntries1, numEntries2;
	int top1, top2;
	int selected1, selected2;
	short fontSize;
};

struct FontListRecord
{
	struct Node	node;
	TEXT fontName[51];		// all lowercase
	UWORD fontSize;
	struct TextFont *ptr;
};

/**** SR ****/

struct ScrollRecord
{
struct GadgetRecord *GR;
struct Window *window;
UBYTE *list;
UBYTE *selectionList;
int entryWidth;
int numDisplay;
int numEntries;
UBYTE **sublist;
};

/**** FRR ****/

struct FileReqRecord
{
	STRPTR path;
	STRPTR fileName;
	STRPTR title;
	int opts;
	BOOL multiple;
};

/**** clipboard ****/

struct cbbuf {
	ULONG size;
	ULONG count;
	UBYTE *mem;
};

/**** used in opticols.c ****/

typedef struct rgb
{
	UBYTE	red;
	UBYTE	green;
	UBYTE	blue;
	int dist;
} RGB;

/******** E O F ********/
