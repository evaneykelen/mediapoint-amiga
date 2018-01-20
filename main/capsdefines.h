/******** CAPS_DEFINES.H ********/

#define RAW_SPACE				0x40
#define RAW_BACKSPACE		0x41
#define RAW_TAB					0x42
#define RAW_ENTER				0x43
#define RAW_RETURN			0x44
#define RAW_ESCAPE			0x45
#define RAW_DELETE			0x46
#define RAW_CURSORUP		0x4C
#define RAW_CURSORDOWN	0x4D
#define RAW_CURSORRIGHT	0x4E
#define RAW_CURSORLEFT	0x4F
#define RAW_HELP				0x5F
#define RAW_F1					0x50
#define RAW_F2					0x51
#define RAW_F3					0x52
#define RAW_F4					0x53
#define RAW_F5					0x54
#define RAW_F6					0x55
#define RAW_F7					0x56
#define RAW_F8					0x57
#define RAW_F9					0x58
#define RAW_F10					0x59
#define RAW_PAGEUP			0x3f
#define RAW_PAGEDOWN		0x1f
#define RAW_HOME				0x3d
#define RAW_END					0x1d
#define RAW_KEYPADUP		0x3e
#define RAW_KEYPADDOWN	0x1e
#define RAW_KEYPADLEFT	0x2d

#define PAL_MODE	1
#define NTSC_MODE	2

#define BUTTON_GADGET					1
#define STRING_GADGET 				2
#define CYCLE_GADGET 					3
#define RADIO_GADGET 					4
#define CHECK_GADGET 					5
#define SPECIAL_STRING_GADGET	6
#define DATE_GADGET						7
#define TIME_GADGET						8
#define INTEGER_GADGET				9

/* let op: CheckGadget(List)() kijkt of type < 10 is! */

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

#define HI_PEN		2L	/* pen color to draw top and left lines of buttons */
#define LO_PEN		1L	/* pen color to draw bottom and right lines of buttons */
#define TEXT_PEN	1L	/* pen color to draw text inside buttons */
#define AREA_PEN	3L	/* pen color to draw back of requesters, gadgets etc. */
#define BGND_PEN	0L

#define DRAW_LO	1
#define DRAW_HI	2

#define HI_LINE								30
#define HI_AREA								31
#define LO_AREA								32
#define HI_PATTERN_LINE				33
#define LO_PATTERN_LINE				34
#define POSPREFS							35

#define MAXEDITWINDOWS 50

#define BORDER_TOP				1
#define BORDER_BOTTOM			2
#define BORDER_LEFT				4
#define BORDER_RIGHT			8

#define DO_EW_OPENWDW			0
#define DO_EW_SIZE1				1	/* top left */
#define DO_EW_SIZE2				2	/* top middle */
#define DO_EW_SIZE3				3	/* top right */
#define DO_EW_SIZE4				4	/* right middle */
#define DO_EW_SIZE5				5	/* right bottom */
#define DO_EW_SIZE6				6	/* bottom middle */
#define DO_EW_SIZE7				7	/* left bottom */
#define DO_EW_SIZE8				8	/* left middle */
#define DO_EW_DRAG				9
#define DO_EW_SELECTED		10

#define DEFAULT_TM				0
#define DEFAULT_BM				0
#define DEFAULT_LM				0
#define DEFAULT_RM				0
#define DEFAULT_LINESP		0
#define DEFAULT_LETTERSP	0
#define DEFAULT_SHDEPTH		2
#define DEFAULT_SHTYPE		SHADOWTYPE_CAST
#define DEFAULT_LSOURCE		LIGHTSOURCE_NW
#define DEFAULT_BORDER		0
#define DEFAULT_BCOLOR		2
#define DEFAULT_BWIDTH		1
#define DEFAULT_BFTYPE		0
#define DEFAULT_BFCOLOR		1
#define DEFAULT_PATTERN		8

#define SET_OFF 0
#define SET_ON 1

#define SELECT_ONE	1	/* for color adjust */
#define SELECT_MANY	2	/* for color adjust */

#define SIGNALMASK (1L << capsPort->mp_SigBit)

#define SPECIAL_TEXT_TOP						1
#define SPECIAL_TEXT_BOTTOM					2
#define SPECIAL_TEXT_LEFT						3
#define SPECIAL_TEXT_RIGHT					4
#define SPECIAL_TEXT_CENTER					5
#define SPECIAL_TEXT_AFTER_STRING 	6
#define SPECIAL_TEXT_BEFORE_STRING	7

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

/**** XCMD defines ****/

#define MAXTOOLS		64
#define TOOLSWIDTH	Script_GR[2].x2-Script_GR[2].x1-3
#define TOOLSDEPTH	3L
#define ICONWIDTH		37
#define ICONHEIGHT	17

#define NUMRUNNING 8
#define NUMMENUS 6

#define MOVE_PHOTO			0x1
#define SIZE_PHOTO			0x2
#define CARD_PHOTO			0x4
#define MODIFIED_PHOTO	0x8
#define HASORI					0x10
#define REMAP_PHOTO			0x20
#define SCREEN_PHOTO		0x40
#define HAS_A_MASK			0x80

#define STARTSCREEN_PAGE		1
#define STARTSCREEN_SCRIPT	2

#define DBLCLICKED 0x00000002  /*  NEWSIZE message class misused */

#define AUSTRALIA				1
#define BELGIE					2
#define BELGIQUE				3
#define CANADA					4
#define	CANDADA_FR			5	
#define	DANMARK					6
#define	DEUTSCHLAND			7
#define	FRANCE					8
#define	GREAT_BRITTAIN	9
#define	ITALIA					10
#define	NEDERLAND				11
#define	NORGE						12
#define	OSTERREICH			13
#define	PORTUGAL				14
#define	SCHWEIZ					15
#define	SUISSE					16
#define	SVERIGE					17
#define	SVIZZERA				18
#define	UNITED_KINGDOM	19
#define	USA							20

#define DEFAULT_ANIM_SPEED	30

#define SMALL_THUMBNAILS	1
#define LARGE_THUMBNAILS	2

/**** used in scripttalk.c and others ****/

#define DOS_MEMSIZE								50L
#define AREXX_MEMSIZE							100L
#define USERAPPLIC_MEMSIZE				200L
#define LARGE_USERAPPLIC_MEMSIZE	1000L

/**** used to identify PAGETALK scripts ****/

#define PAGETALK_ID_1		0x50414745
#define PAGETALK_ID_2		0x54414c4b

#define SCRIPTTALK_ID_1	0x53435249
#define SCRIPTTALK_ID_2	0x50545441

/**** used in sprite.c ****/

#define SPRITE_NORMAL				0
#define SPRITE_BUSY					1
#define SPRITE_COLORPICKER	2
#define SPRITE_TOSPRITE			3
#define SPRITE_HAND					4

/**** script sizes ****/

#define SCRIPTSIZE_SMALL 	512		/* # of sub branches */
#define SCRIPTSIZE_MEDIUM	1024	/* # of sub branches */
#define SCRIPTSIZE_LARGE	8192	/* # of sub branches */

/**** defines ****/

#define ADD_TO_HEAD		1
#define ADD_TO_TAIL		2
#define ADD_TO_MIDDLE 3

#define DEFAULT_DELAY	10	/* 10 seconds */

/**** printer prefs ****/

#define PRINTER_DEST_PRINTER	1
#define PRINTER_DEST_PS_SER		2
#define PRINTER_DEST_PS_PAR		3
#define PRINTER_DEST_PS_FILE	4

#define PRINTER_ORI_LANDSCAPE			1
#define PRINTER_ORI_LANDSCAPE4X4	2
#define PRINTER_ORI_PORTRAIT			3
#define PRINTER_ORI_PORTRAIT4X4		4

#define PRINTER_QUALITY_DRAFT		1
#define PRINTER_QUALITY_LETTER	2

/**** page undo defines ****/

#define PAGE_UNDO_RESIZE	1
#define PAGE_UNDO_MOVE		2
#define PAGE_UNDO_CLEAR		3
#define PAGE_UNDO_DEFINE	4	// NOT USED
#define PAGE_UNDO_PICMOVE	5
#define PAGE_UNDO_IMPORT	6	// undo color change done by import

/**** screen refresh ****/

#define REFRESH_SINGLE	1	
#define REFRESH_DOUBLE	2

/**** typedefs ****/

typedef struct {
	UWORD version;
	UWORD nframes;
	ULONG flags;
} DPAnimChunk;

typedef struct ScriptNodeRecord *SNRPTR;

/******** E O F ********/
