#define SIZE_FULLPATH 150
#define SIZE_PATH			100
#define SIZE_FILENAME	50

#define HIRES_INTERWIDTHTEXT "œœœœœœœœ"
#define LORES_INTERWIDTHTEXT "œ"
#define MHEIGHT 12														// height of menu items

#define PAGESCREEN_FLAG			0x00000001L
#define PAGEWINDOW_FLAG			0x00000002L
#define SCRIPTSCREEN_FLAG		0x00000004L
#define SCRIPTWINDOW_FLAG		0x00000008L
#define PALETTESCREEN_FLAG	0x00000010L
#define PALETTEWINDOW_FLAG	0x00000020L
#define COPPERLIST_FLAG			0x00000040L
#define CLOCK_FLAG					0x00000080L
#define SCRIPTINMEM_FLAG		0x00000100L
#define SMALLNODES_FLAG			0x00000200L
#define SCRIPTINFOREC_FLAG	0x00000400L
#define EFFECTINFO_FLAG			0x00000800L
#define FONTS_SCANNED_FLAG	0x00001000L
#define XAPPSLOADED_FLAG		0x00002000L
#define SPECIALS_WDW_FLAG		0x00004000L
#define WINDEF_WDW_FLAG			0x00008000L
#define STYLE_WDW_FLAG			0x00010000L

#define QUIT_MEDIALINK	1
#define DO_LOOP 	1
#define DO_QUIT 	2
#define DO_OTHER	3

#define DONT_QUIT_MEDIALINK	2

#define IDCMP_TO_EXAMINE 		1
#define NOTHING_TO_EXAMINE	2
#define DO_SOMETHING				3

#define MAX_FILE_LIST_ENTRIES 1024
#define FILELISTSIZE MAX_FILE_LIST_ENTRIES*SIZE_FILENAME

#define PRINT_LEFTPART 	1
#define PRINT_RIGHTPART 2
#define PRINT_CENTERED	3

#define FILE_PRECODE	'Š'
#define DIR_PRECODE		'‰'

#define SELECT_ONE_FILE 			1
#define SELECT_MULTIPLE_FILES 2

#define MAKE_INACTIVE TRUE
#define LEAVE_ACTIVE 	FALSE

#define MINWINWIDTH 	10
#define MINWINHEIGHT	6
//#define HANDLESNIF		4

//#define BESTCOLORS 	1
#define STDCOLORS  	2
#define USECOLORS		3

#define NO_ICON 					0
#define QUESTION_ICON 		1
#define EXCLAMATION_ICON	2

#define MAXSCANDEPTH 999	// parser.c etc.

/**** special chars ****/

//#define SPECIAL_CHAR_84	"„\0"

/**** resources ****/

#define OPENED_SYSTEMLIBS	0x00000001L
#define OPENED_APPLIBS		0x00000002L

/***** ew flags ****/

#define EW_LOCKED			0x01
#define EW_PIC_SCALED	0x02	// only used during windef.c
#define EW_HAS_PIC		0x04	// only used during windef.c
#define EW_IS_BACKWIN	0x08	// only used during loading time
#define EW_IS_TILED		0x10	// only used during loading time
#define EW_IS_OPAQUE	0x20

/**** languages ****/

#define LAN_English			0x00000001L
#define LAN_Nederlands	0x00000002L
#define LAN_Deutsch			0x00000004L
#define LAN_Français		0x00000008L
#define LAN_Español			0x00000010L
#define LAN_Italiano		0x00000020L
#define LAN_Português		0x00000040L
#define LAN_Dansk				0x00000080L
#define LAN_Svenska			0x00000100L
#define LAN_Norsk				0x00000200L

/**** localedate.c defines ****/

#define LD_DATE_1 			1
#define LD_DATE_2 			2
#define LD_DATE_3 			3
#define LD_DATE_4 			4
#define LD_TIME_1 			5
#define LD_TIME_2 			6
#define LD_TIME_3 			7
#define LD_SECS					8
#define LD_DATE					9
#define LD_LONG_DAY			10
#define LD_SHORT_DAY		11
#define LD_LONG_MONTH		12
#define LD_SHORT_MONTH	13
#define LD_LONG_YEAR		14
#define LD_SHORT_YEAR		15
#define LD_FILE					16

#define GFX_W							256
#define GFX_H							108

#define GFX_LARGE_EXCLA_X	0 
#define GFX_LARGE_EXCLA_Y	0 
#define GFX_LARGE_EXCLA_W	43 
#define GFX_LARGE_EXCLA_H	44

#define GFX_LARGE_QUEST_X	43 
#define GFX_LARGE_QUEST_Y	0 
#define GFX_LARGE_QUEST_W	43 
#define GFX_LARGE_QUEST_H	44

#define GFX_SMALL_EXCLA_X	87 
#define GFX_SMALL_EXCLA_Y	0 
#define GFX_SMALL_EXCLA_W	43 
#define GFX_SMALL_EXCLA_H	22

#define GFX_SMALL_QUEST_X	130 
#define GFX_SMALL_QUEST_Y	0 
#define GFX_SMALL_QUEST_W	43 
#define GFX_SMALL_QUEST_H	22

#define GFX_SCIS_X				174
#define GFX_SCIS_Y				0
#define GFX_SCIS_W				37
#define GFX_SCIS_H				17

#define GFX_PROG_X				87
#define GFX_PROG_Y				23
#define GFX_PROG_W				111
#define GFX_PROG_H				19

#define GFX_PAST_X				211
#define GFX_PAST_Y				0
#define GFX_PAST_W				13
#define GFX_PAST_H				7

#define GFX_FUTU_X				211
#define GFX_FUTU_Y				8
#define GFX_FUTU_W				13
#define GFX_FUTU_H				7

#define GFX_DONT_X				211
#define GFX_DONT_Y				16
#define GFX_DONT_W				13
#define GFX_DONT_H				7

#define GFX_JUST_W				22
#define GFX_NL_JUST_H			10
#define GFX_NL_JUST_Y			45
#define GFX_L_JUST_H			20
#define GFX_L_JUST_Y			55

#define GFX_JUST_L_X			0
#define GFX_JUST_C_X			24
#define GFX_JUST_R_X			48

#define GFX_SHAD_W				18
#define GFX_NL_SHAD_H			10
#define GFX_NL_SHAD_Y			45
#define GFX_L_SHAD_H			20
#define GFX_L_SHAD_Y			55

#define GFX_SHAD_N_X			72
#define GFX_SHAD_C_X			91
#define GFX_SHAD_S_X			110
#define GFX_SHAD_O_X			129
#define GFX_SHAD_T_X			148

#define GFX_LOGO_X				199
#define GFX_LOGO_Y				24
#define GFX_LOGO_W				34
#define GFX_LOGO_H				19

#define GFX_SDW_1_X				167
#define GFX_SDW_2_X				186
#define GFX_SDW_3_X				205
#define GFX_SDW_4_X				224

#define GFX_EYE_X_NL			228
#define GFX_EYE_Y_NL			0
#define GFX_EYE_W_NL			26
#define GFX_EYE_H_NL			8

#define GFX_EYE_X_L				228
#define GFX_EYE_Y_L				9
#define GFX_EYE_W_L				26
#define GFX_EYE_H_L				16

#define GFX_SM_EXCL_X_NL	199
#define GFX_SM_EXCL_Y_NL	26
#define GFX_SM_EXCL_W_NL	23
#define GFX_SM_EXCL_H_NL	12

#define GFX_SM_EXCL_X_L		223
#define GFX_SM_EXCL_Y_L		26
#define GFX_SM_EXCL_W_L		23
#define GFX_SM_EXCL_H_L		22

/* Cassette recorder buttons */

#define GFX_CR_Y_NL				77
#define GFX_CR_Y_L				89

#define GFX_CR_W					23

#define GFX_CR_H_NL				9
#define GFX_CR_H_L				18

#define GFX_CR_REC_X			4
#define GFX_CR_STOP_X			29
#define GFX_CR_REW_X			54
#define GFX_CR_FWD_X			79
#define GFX_CR_STEPREW_X	104
#define GFX_CR_STEPFWD_X	129
#define GFX_CR_FASTREW_X	154
#define GFX_CR_FASTFWD_X	179
#define GFX_CR_PAUSE_X		204
#define GFX_CR_LOOP_X			229

/******** E O F ********/
