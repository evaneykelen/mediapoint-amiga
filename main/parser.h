/******** PARSER.H ********/

/**** D E F I N E S ****/

#define MAX_PARSER_ARGS					25		/* max number of arguments */
#define MAX_PARSER_CHARS				256		/* max length of one line of source */
#define MAX_OBJECTNAME_CHARS		32
#define PARSE_NEXT							1			/* fetch next line */
#define PARSE_INTERPRET					2			/* parse this line */
#define PARSE_STOP							3			/* stop parsing */
#define MAX_PARSER_NODES				32768	/* max number of objects in a script */

//#define MAX_GLOBALLOCAL_EVENTS	1000	//256		/* max no. of global and local events */
#define MAX_GLOBAL_EVENTS				100	// see keys.h NUMKEYS (96)
#define MAX_LOCAL_EVENTS				60	// see capsdefines.h MAXEDITWINDOWS (50)

#define NO_FUNCTION							NULL	/* used for function table */
#define PRINTERROR_CODE					-2		/* node type in PR when PrintError was called */

#define TIMESOURCE_INTERNAL			1
#define TIMESOURCE_EXTERNAL			2

#define TIMEFORMAT_HHMMSS				1
#define TIMEFORMAT_MIDI					2
#define TIMEFORMAT_SMPTE				3
#define TIMEFORMAT_MLTC					4		// MediaLink Time Code
#define TIMEFORMAT_CUSTOM				5

#define TIMERATE_24FPS					1		// obsolete
#define TIMERATE_25FPS					2
#define TIMERATE_30FPS_DF				3		// obsolete
#define TIMERATE_30FPS					4
#define TIMERATE_MIDICLOCK			5		// obsolete

#define ARGUMENT_OFF						0
#define ARGUMENT_ON							1

#define ARGUMENT_DEFER					1
#define ARGUMENT_CONTINUE				2

#define ARGUMENT_COMMAND				1
#define ARGUMENT_SCRIPT					2

#define ARGUMENT_CYCLICAL				1
#define ARGUMENT_CONTINUOUS			2

#define OBJ_SELECTED						0x01	/* miscFlags */
#define OBJ_NEEDS_REFRESH				0x02	/* miscFlags */
#define OBJ_TO_CLIP							0x04	/* miscFlags */
#define OBJ_OUTDATED						0x08	/* miscFlags, used by date programming */
#define OBJ_BEINGDRAGGED				0x10	/* miscFlags */

/**** raw key codes ****/

#define TALK_HELP_KC				0x5f
#define TALK_HELP_KT				"HELP_KEY"

#define TALK_ESC_KC					0x45	
#define TALK_ESC_KT					"ESC_KEY"

#define TALK_F1_KC					0x50
#define TALK_F1_KT					"F1_KEY"

#define TALK_F2_KC					0x51
#define TALK_F2_KT					"F2_KEY"

#define TALK_F3_KC					0x52
#define TALK_F3_KT					"F3_KEY"

#define TALK_F4_KC					0x53
#define TALK_F4_KT					"F4_KEY"

#define TALK_F5_KC					0x54
#define TALK_F5_KT					"F5_KEY"

#define TALK_F6_KC					0x55
#define TALK_F6_KT					"F6_KEY"

#define TALK_F7_KC					0x56
#define TALK_F7_KT					"F7_KEY"

#define TALK_F8_KC					0x57
#define TALK_F8_KT					"F8_KEY"

#define TALK_F9_KC					0x58
#define TALK_F9_KT					"F9_KEY"

#define TALK_F10_KC					0x59
#define TALK_F10_KT					"F10_KEY"

#define TALK_CURSORUP_KC		0x4c
#define TALK_CURSORUP_KT		"CURSORUP_KEY"

#define TALK_CURSORDOWN_KC	0x4d
#define TALK_CURSORDOWN_KT	"CURSORDOWN_KEY"

#define TALK_CURSORLEFT_KC	0x4f
#define TALK_CURSORLEFT_KT	"CURSORLEFT_KEY"

#define TALK_CURSORRIGHT_KC	0x4e
#define TALK_CURSORRIGHT_KT	"CURSORRIGHT_KEY"

#define TALK_TAB_KC					0x42
#define TALK_TAB_KT					"TAB_KEY"

#define TALK_DEL_KC					0x46
#define TALK_DEL_KT					"DEL_KEY"

#define TALK_BACKSPACE_KC		0x41
#define TALK_BACKSPACE_KT		"BACKSPACE_KEY"

/* only RETURN can be specified but the event handler should interpret */
/* RETURN the same as ENTER on the numeric key pad. */

#define TALK_RETURN_KC			0x44
#define TALK_RETURN_KT			"RETURN_KEY"

#define TALK_SPACE_KC				0x40
#define TALK_SPACE_KT				"SPACE_KEY"

#define TALK_OPEN_BRACKET_KC	0x5a
#define TALK_OPEN_BRACKET_KT	"(_KEY"

#define TALK_CLOSE_BRACKET_KC	0x5b
#define TALK_CLOSE_BRACKET_KT	")_KEY"

#define TALK_STAR_KC				0x5d
#define TALK_STAR_KT				"*_KEY"

#define TALK_PLUS_KC				0x5e
#define TALK_PLUS_KT				"+_KEY"

/**** S T R U C T U R E S ****/

struct ParseRecord
{
	/*************************/
	char **commandList;								/* P R I V A T E */
	char *argString[MAX_PARSER_ARGS];	/* P R I V A T E */
	FILE *filePointer;								/* P R I V A T E */
	int sourceLine;										/* P R I V A T E */
	/*************************/
	int commandCode;									/* P U B L I C */
	int numArgs;											/* P U B L I C */
	/*************************/
};

struct ScriptFuncs
{
	void (*func)(struct ParseRecord *, struct ScriptInfoRecord *);
};

struct PageFuncs
{
	void (*func)(struct ParseRecord *, struct PageInfoRecord *);
};

// One event can represent both key and mouse event
struct ScriptEventRecord
{
	int keyCode;							// if either one is not equal to -1 then this event
	int rawkeyCode;						// is a key event
	TEXT labelName[MAX_PARSER_CHARS];
	int x, y, width, height;	// if width != 0 then this event is a mouse event
	char renderType;
	int audioCue;
	struct ScriptNodeRecord *labelSNR;	// the label node belonging to this global/local event
  UWORD typeBits;											// see minc:ge.h for a description of typebits
	TEXT buttonName[50];
  TEXT assignment[75];
};

#define RENDERTYPE_NONE				1	
#define RENDERTYPE_INVERT			2
#define RENDERTYPE_BOX				4
#define RENDERTYPE_FATBOX			8
#define RENDERTYPE_AUTO				16
#define RENDERTYPE_IMMEDIATE	32
#define RENDERTYPE_STAY				64

/******************************************************************
 * Desc : Information on a certain punch, start/stop/offset info.
 */

typedef	union
{
	struct 
	{ 
		LONG Days, 
				 Minutes, 
				 Ticks; 
	} HHMMSS;

	struct 
	{ 
		BYTE HH, 
			   MM, 
				 SS, 
				 FF; 
	} TimeCode;

	ULONG ParHMSTOffset;			// Start/Stop  punch for objects within parallel events

} PUNCHDATA;

struct ScriptInfoRecord
{
	/*************************/
	struct List *currentList;								/* P R I V A T E */
	char listType;													/* P R I V A T E */
	struct List **lists;										/* P R I V A T E */
	UWORD *scrollPos;												/* P R I V A T E */
	struct ScriptNodeRecord *currentNode;		/* P R I V A T E */
	/*************************/
	BYTE version, revision;
	struct List **allLists;
	struct ScriptEventRecord *globallocalEvents[MAX_GLOBAL_EVENTS];	/* P U B L I C */
	char timeCodeSource;
	char timeCodeFormat;
	char timeCodeRate;
	PUNCHDATA Offset;
	BOOL timeCodeOut;
	struct List VIList;											// List of VarInfoRecords					
	/*************************/
};

struct ScriptNodeRecord
{
	struct Node 	node;						// Linked list of SNRs 
	struct List 	*list;					// Ptr to sub-list. Used by SERIAL and 
																// PARALLEL objects, else NULL.
																// PAGE object use this ptr to hold a list
																// to the local event list.
																// GOTO objects hold a list to VarInfoRecords
	BYTE 					nodeType;				// Object type
	WORD 					numericalArgs[MAX_PARSER_ARGS];
	TEXT 					objectPath[MAX_PARSER_CHARS];
	TEXT 					objectName[MAX_OBJECTNAME_CHARS];
	UBYTE 				*extraData;			// e.g. palette data 
	ULONG 				extraDataSize;	// sizeof(*ExtraData)
	LONG 					duration;				// Duration of event, HHMMSS mode only
	BYTE 					dayBits;				// Days on which the object must be shown, HHMMSS only
	PUNCHDATA 		Start;					// Start punch data
	PUNCHDATA 		End;						// Stop punch data
	BYTE 					startendMode;
	BYTE 					miscFlags;
	int						PageNr;
	ULONG 				*ProcInfo;			// Ptr to processinfo structure or NULL if 
																// not in PIList. More information on this
																// field can be found in pc_process.h, 
																// structure PROCESSINFO, field pi_SNR; 
	struct ScriptNodeRecord 
								*ParentSNR;			// ptr to the SNR of the owner of this object
																// This ptr is the same for all objects in a list
};

// TALK_SER objects use the extraDataSize field to keep track of the last performed
// object from its list.
// Used for SPNEXTGOSUB and SPPREVGOSUB goto objects
#define PlayTrackSNR extraDataSize		// for object player 
#define LoadTrackSNR duration					// for object loader

/**** put all objects without any time code behind TALK_GOTO ****/

enum {
TALK_ANIM, TALK_AREXX, TALK_USERAPPLIC, TALK_DOS,
TALK_STARTSER, TALK_ENDSER, TALK_STARTPAR, TALK_ENDPAR,
TALK_SOUND, TALK_START, TALK_END, TALK_DURATION, TALK_PROGRAM,
TALK_SCRIPTTALK, TALK_PAGE, TALK_COUNTER, TALK_GOTO, TALK_BINARY, TALK_MAIL,
TALK_GLOBALEVENT, TALK_TIMECODE, TALK_LABEL, TALK_NOP, TALK_LOCALEVENT,
TALK_VARS, TALK_INPUTSETTINGS,
};

enum {
TG_GOTO, TG_GOSUB, TG_SPNEXTGOSUB, TG_SPPREVGOSUB, TG_CONDITIONJUMP, TG_NOJUMP, TG_MAKE,
};

/**** P A G E T A L K ****/

enum {
TALK_SCREEN, TALK_PALETTE, TALK_WINDOW, TALK_CLIP, TALK_TEXT, TALK_PAGETALK,
TALK_EFFECT, TALK_FORMAT, TALK_STYLE, TALK_CRAWL, TALK_BUTTON, TALK_OBJECTSTART,
TALK_OBJECTEND, TALK_BACKGROUND, TALK_CLIPANIM
};

/**********************************************************
*Desc : Variable area
*/

typedef struct
{
  int		vc_Type;					// Type of variable
	
	union
 	{
		char 	vv_String[75];	// string value of the var
		int 	vv_Integer;			// integer value

	} vc_Value;

} VARCONTENTS;

#define VCT_STRING 		0		// Variable holds a string
#define VCT_INTEGER 	1		// Variable holds an integer value
#define VCT_EMPTY		2			// Used for VAR=VAR declarations and for
													// if's without expressions

/****************************************************
*Desc : The system holds a list of all variables
*				used by the script. These are linked together
*				through the VarInfoRecord.
*				vir_Name holds the name of the variable
*				vir_Value keeps track of the value assigned to
*				the variable.
*/
struct VarInfoRecord
{
  struct Node 	vir_Node;				// linked list of variables
  char 					vir_Name[20];		// name of the variable
  
  VARCONTENTS		vir_Contents;		// actual contents of the variable
};

#define vir_Type	  vir_Contents.vc_Type
#define vir_String  vir_Contents.vc_Value.vv_String
#define vir_Integer vir_Contents.vc_Value.vv_Integer

typedef struct VarInfoRecord VIR;

/******************************************************
*Desc : Each TALK_GOTO snr has a linked list of 
*				variable assignments. The list is stored
*				into SNR.list.
*				All assignments for the object are stored
*				in this list.
*				Also, for each assignment a new list of
*				conditions is kept in var_CondList.
*				the conditions in that list are checked
*				before or after the assignment has been
*				made, depending on the state of 
*				var_CheckBeforeAssign.
*				IF b0 or b1 of var_SourceBits is set
*				then the source1/source2 value is
*				a variable in vs_VIR. If either one is cleared
*				then the source for that bit will be 
*				the integer value in vs_Direct.
*/		
struct VarAssignRecord
{
	struct Node		var_Node;								// Linked list of assignments
	struct List		var_CondList;						// Linked list of conditions belonging to this assignment		
  BOOL					var_CheckBeforeAssign;	// if TRUE, conditions will be checked BEFORE
																				// assignments are done.

  UWORD					var_Operator;						// see OPER_ defines below
  UWORD					var_SourceBits;					// what kind of sources are used 
	VIR						*var_ResultVIR;					// where to put the result A = source1 (oper) source2
																				// vs_Direct or vs_VIR
		union
		{
			VIR					*vs_VIR;							// First source a = B + c
			VARCONTENTS	vs_Direct;						// a = [VALUE] + b
		} var_Source1;

		union
		{
			VIR					*vs_VIR;							// second source a = b + C	or NULL for a = b
			VARCONTENTS	vs_Direct;						// a = b + [VALUE]
		} var_Source2;

  UWORD					var_Operator_v2;				// see OPER_ defines below
  UWORD					var_SourceBits_v2;			// what kind of sources are used 
	VIR						*var_ResultVIR_v2;			// where to put the result A = source1 (oper) source2
																				// vs_Direct or vs_VIR
		union
		{
			VIR					*vs_VIR;							// First source a = B + c
			VARCONTENTS	vs_Direct;						// a = [VALUE] + b
		} var_Source3;

		union
		{
			VIR					*vs_VIR;							// second source a = b + C	or NULL for a = b
			VARCONTENTS	vs_Direct;						// a = b + [VALUE]
		} var_Source4;
};

#define var_Type1		 var_Source1.vs_Direct.vc_Type
#define var_String1  var_Source1.vs_Direct.vc_Value.vv_String
#define var_Integer1 var_Source1.vs_Direct.vc_Value.vv_Integer
#define var_Type2		 var_Source2.vs_Direct.vc_Type
#define var_String2  var_Source2.vs_Direct.vc_Value.vv_String
#define var_Integer2 var_Source2.vs_Direct.vc_Value.vv_Integer

#define var_Type3		 var_Source3.vs_Direct.vc_Type
#define var_String3  var_Source3.vs_Direct.vc_Value.vv_String
#define var_Integer3 var_Source3.vs_Direct.vc_Value.vv_Integer
#define var_Type4		 var_Source4.vs_Direct.vc_Type
#define var_String4  var_Source4.vs_Direct.vc_Value.vv_String
#define var_Integer4 var_Source4.vs_Direct.vc_Value.vv_Integer

#define SB_SOURCE1VIR (1<<0)						// if set in var_SourceBits then source1 will
																				// be taken from vs_VIR.
#define SB_SOURCE2VIR (1<<1)						// if set in var_SourceBits then source2 will
																				// be taken from vs_VIR.
#define SB_NOSOURCES	(1<<2)						// if set in var_SourceBits then this
																				// is an if statement
#define SB_SOURCE3VIR (1<<3)						// if set in var_SourceBits then source3 will
																				// be taken from vs_VIR.
#define SB_SOURCE4VIR (1<<4)						// if set in var_SourceBits then source4 will
																				// be taken from vs_VIR.

enum
{
  OPER_ADD,
  OPER_SUBTRACT,
  OPER_MULTIPLY,
  OPER_DIVIDE
};

typedef struct VarAssignRecord VAR;

/**********************************************
*Desc : Each variable assignment has a list
*				of conditions. The type of condition
*				and where to jump to is stored in this
*				record. Multiply conditions may be linked
*				if more checks are needed.
*/

struct VarCondRecord
{
  struct Node 	vcr_Node;								// Linked list of conditions 
																				// attached to var_CondList
  UWORD					vcr_Condition;					// Which condition check should be performed
																				// see COND_ defs 
  UWORD					vcr_CheckBits;					// Same as var_SourceBits 
  UWORD					vcr_JumpType;						// defined as TG_GOTO, TG_GOSUB etc.
		union
		{
			VIR					*vc_VIR;							// Check variable a (cond) B 
			VARCONTENTS	vc_Direct;						// Check value a (cond) [VALUE]
		} vcr_Check;

  struct ScriptNodeRecord
								*vcr_LabelSNR;					// if Condition is TRUE, the system will jump
	TEXT vcr_LabelName[MAX_PARSER_CHARS];	// used while loading script to resolve ptr's to labels that aren't there yet.
};

#define vcr_Type		vcr_Check.vc_Direct.vc_Type
#define vcr_String  vcr_Check.vc_Direct.vc_Value.vv_String
#define vcr_Integer vcr_Check.vc_Direct.vc_Value.vv_Integer

#define VC_CHECKVIR 1<<0								// if set, checking is performed on a variable
																				// else checking is done on a integer value

typedef struct VarCondRecord VCR;

enum
{ 
  COND_LESS,
  COND_LESSEQUAL,
  COND_EQUAL,
  COND_GREATEREQUAL,
  COND_GREATER,
  COND_NOTEQUAL
};

/**********************************************************
*Desc : Common header for all PageTalks records
*/
typedef struct
{
	struct Node 			ph_Node;							// Linked list of PageTalk records
	BYTE 							ph_NodeType;					// Object type, see TALK_ defs 
} PTHEADER;

/*********************************************
*Desc : Local events list
*Type : TALK_LOCALEVENT 
*/ 
struct PageEventRecord
{
  PTHEADER	 				er_Header;						// linked list of PageTalk records
	struct ScriptEventRecord 
										*er_LocalEvents[MAX_LOCAL_EVENTS];
};

typedef struct PageEventRecord PER;

struct PageInfoRecord
{
  PTHEADER 						ir_Header;								// linked list of Page records
	/*********************************************************/
	struct EditWindow 	*ew;											/* P R I V A T E */
	struct EditSupport	*es;											/* P R I V A T E */
	/*********************************************************/
	BYTE 								version, 
											revision;
	int 								LoadedScreenWidth,				/* P U B L I C */
											LoadedScreenHeight,				/* P U B L I C */
											LoadedScreenDepth;				/* P U B L I C */
	ULONG 							LoadedScreenModes;				/* P U B L I C */
	struct ColorMap 		*LoadedCM;								/* P U B L I C */
	ULONG								LoadedMonitorID;					/* P U B L I C */
	int									LoadedOverScan;						/* P U B L I C */

	//ULONG 							LoadedextraScreenModes;		/* P U B L I C */
	//UWORD 						LoadedScreenColors[64];		/* P U B L I C */
		/*********************************************************/
};

typedef struct PageInfoRecord PIR;

#define JUMPTYPE_GOTO 		1
#define JUMPTYPE_GOSUB		2
#define JUMPTYPE_PREV			3
#define JUMPTYPE_NEXT			4
#define JUMPTYPE_PREVPAGE	5
#define JUMPTYPE_NEXTPAGE	6

#define JUMPTYPE_GOTO_TEXT 			"GOTO"
#define JUMPTYPE_GOSUB_TEXT			"GOSUB"
#define JUMPTYPE_PREV_TEXT			"PREVIOUS"
#define JUMPTYPE_NEXT_TEXT			"NEXT"
#define JUMPTYPE_PREVPAGE_TEXT	"PREVPAGE"
#define JUMPTYPE_NEXTPAGE_TEXT	"NEXTPAGE"

/******** E O F ********/
