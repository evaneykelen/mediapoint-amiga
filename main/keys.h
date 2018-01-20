/************************/
/******** KEYS.H ********/
/************************/

/**** This file contains the list of keys which appears in the ****/
/**** selection list of global events and labels ****/

#define NUMKEYS 96	// see parser.h MAX_GLOBAL_EVENTS

static TEXT *keyListTexts[NUMKEYS] =
{
TALK_HELP_KT,
TALK_ESC_KT,
TALK_F1_KT, TALK_F2_KT, TALK_F3_KT, TALK_F4_KT, TALK_F5_KT,
TALK_F6_KT, TALK_F7_KT, TALK_F8_KT, TALK_F9_KT, TALK_F10_KT,
TALK_CURSORUP_KT, TALK_CURSORDOWN_KT, TALK_CURSORLEFT_KT, TALK_CURSORRIGHT_KT,
TALK_TAB_KT,
TALK_DEL_KT,
TALK_BACKSPACE_KT,
TALK_RETURN_KT,
TALK_SPACE_KT,
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
TALK_OPEN_BRACKET_KT, TALK_CLOSE_BRACKET_KT,
"/",
TALK_STAR_KT,
"-",
TALK_PLUS_KT,
".",
"À","Á","Â","Ã","Ä","Å","Æ","Ç","È","É","Ê","Ë",
"Ì","Í","Î","Ï","Ð","Ñ","Ò","Ó","Ô","Õ","Ö","×",
"Ø","Ù","Ú","Û","Ü","Ý","Þ","ß",
};

static int keyListCodes[NUMKEYS] =
{
-1,
-1,
-1, -1, -1, -1, -1,
-1, -1, -1, -1, -1,
-1, -1, -1, -1,
-1,
-1,
-1,
-1,
-1,
0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
-1, -1,
0x2f,
-1,
0x2d,
-1,
0x2e,
0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
};

static int keyListRawCodes[NUMKEYS] =
{
TALK_HELP_KC,
TALK_ESC_KC,
TALK_F1_KC, TALK_F2_KC, TALK_F3_KC, TALK_F4_KC, TALK_F5_KC,
TALK_F6_KC, TALK_F7_KC, TALK_F8_KC, TALK_F9_KC, TALK_F10_KC,
TALK_CURSORUP_KC, TALK_CURSORDOWN_KC, TALK_CURSORLEFT_KC, TALK_CURSORRIGHT_KC,
TALK_TAB_KC,
TALK_DEL_KC,
TALK_BACKSPACE_KC,
TALK_RETURN_KC,
TALK_SPACE_KC,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
TALK_OPEN_BRACKET_KC, TALK_CLOSE_BRACKET_KC,
-1,
TALK_STAR_KC,
-1,
TALK_PLUS_KC,
-1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,
};

struct ScriptEventRecord *keySERs[NUMKEYS];

/******** E O F ********/
