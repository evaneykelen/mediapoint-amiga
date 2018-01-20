/******** SCRIPT_GADGETS.H ********/

/*****************************************************
 * date programming
 *****************************************************/

struct StringRecord Prog_StartDay_SR 		= {  8, "         " };
struct StringRecord Prog_EndDay_SR 			= {  8, "         " };
struct StringRecord Prog_StartTime_SR 	= { 10, "           " };
struct StringRecord Prog_EndTime_SR 		= { 10, "           " };
struct StringRecord Prog_Duration_SR		= { 10, "           " };

TEXT DaysList[] =	{ TL_DayTypes };

TEXT MonthList[] = { TL_MonthTypes };

TEXT YearList[] =	{ 
 "1990\0 1991\0 1992\0 1993\0 1994\0 1995\0 1996\0 1997\0 1998\0 " };
/*-------+++++++-------+++++++-------+++++++-------+++++++-------*/

struct CycleRecord Prog_Day1_CR 		= { 0,  7, 15, (STRPTR)&DaysList };
struct CycleRecord Prog_Date1_CR 		= { 0, 31,  4, (STRPTR)&WDef_BWidthList };
struct CycleRecord Prog_Month1_CR 	= { 0, 12, 15, (STRPTR)&MonthList };
struct CycleRecord Prog_Year1_CR		= {	0,  9,  6, (STRPTR)&YearList };

struct CycleRecord Prog_Day2_CR 		= { 0,  7, 15, (STRPTR)&DaysList };
struct CycleRecord Prog_Date2_CR 		= { 0, 31,  4, (STRPTR)&WDef_BWidthList };
struct CycleRecord Prog_Month2_CR		= { 0, 12, 15, (STRPTR)&MonthList };
struct CycleRecord Prog_Year2_CR		= {	0,  9,  6, (STRPTR)&YearList };

struct GadgetRecord ProgBackup_GR[] =
{
100,  23, 185,  36, NULL, 									DATE_GADGET, NULL,
100,  40, 194,  53, NULL, 									TIME_GADGET, NULL,
  2,  21, 196,  55, NULL,										LOBOX_REGION, NULL,
-1
};

struct GadgetRecord Prog_GR[] =
{
  0,   0, 640, 200, NULL,										DIMENSIONS, NULL,	
  0,	 0,	639, 199, NULL,										DBL_BORDER_REGION, NULL,
100,   6, 185,  19, TL_StartDay, 						DATE_GADGET, (struct GadgetRecord *)&Prog_StartDay_SR,
100,  23, 185,  36, TL_EndDay, 							DATE_GADGET, (struct GadgetRecord *)&Prog_EndDay_SR,
100,  40, 194,  53, TL_StartTime, 					TIME_GADGET, (struct GadgetRecord *)&Prog_StartTime_SR, 
100,  57, 194,  70, TL_EndTime, 						TIME_GADGET, (struct GadgetRecord *)&Prog_EndTime_SR, 
219,  41, 238,  51, TL_DateTimeRelation,		CHECK_GADGET, NULL,
219,   6, 361,  19, NULL, 									CYCLE_GADGET, (struct GadgetRecord *)&Prog_Day1_CR,
368,   6, 415,  19, NULL,										CYCLE_GADGET,	(struct GadgetRecord *)&Prog_Date1_CR,
422,   6, 554,  19, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Prog_Month1_CR,
561,   6, 629,  19, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Prog_Year1_CR,
219,  23, 361,  36, NULL, 									CYCLE_GADGET, (struct GadgetRecord *)&Prog_Day2_CR,
368,  23, 415,  36, NULL,										CYCLE_GADGET,	(struct GadgetRecord *)&Prog_Date2_CR,
422,  23, 554,  36, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Prog_Month2_CR, 
561,  23, 629,  36, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Prog_Year2_CR,
415,  40, 443,  52, TL_Su,									BUTTON_GADGET, NULL,
446,  40, 474,  52, TL_Mo,									BUTTON_GADGET, NULL,
477,  40, 505,  52, TL_Tu,									BUTTON_GADGET, NULL, 
508,  40, 536,  52, TL_We,									BUTTON_GADGET, NULL, 
539,  40, 567,  52, TL_Th,									BUTTON_GADGET, NULL, 
570,  40, 598,  52, TL_Fr,									BUTTON_GADGET, NULL, 
601,  40, 629,  52, TL_Sa,									BUTTON_GADGET, NULL, 
/*219,  57, 357,  70, TL_MoreChoices,					BUTTON_GADGET, NULL,*/
219,  57, 357,  70, NULL,										TEXT_REGION, NULL,
446,  57, 528,  70, TL_OK,									BUTTON_GADGET, NULL,
547,  57, 629,  70, TL_Cancel,							BUTTON_GADGET, NULL,
299,  57, 393,  70, TL_DURATION,						TIME_GADGET, (struct GadgetRecord *)&Prog_Duration_SR, 
-1
};

/*****************************************************
 * script window
 *****************************************************/

struct GadgetRecord Script_GR[] =
{
  4,  15, 420, 255, NULL,										BUTTON_GADGET, NULL,	/* large scroll area */
427,  15, 445, 255, NULL,										HIBOX_REGION, NULL,		/* large scroll area prop slider */
454,  15, 614, 166, NULL,										BUTTON_GADGET, NULL, /* icon area */
621,  15, 639, 166, NULL,										HIBOX_REGION, NULL,	/* icon area prop slider */
552, 170, 639, 188, TL_Play,								BUTTON_GADGET, NULL,
454, 170, 541, 188, TL_Parent, 							BUTTON_GADGET, NULL,
458, 194, 494, 210, NULL,										TEXT_REGION, NULL,
458, 214, 635, 236, NULL,										TEXT_REGION, NULL,
454, 192, 639, 238, NULL,										LOBOX_REGION, NULL,
-1
};

/*****************************************************
 * small buttons
 *****************************************************/

struct GadgetRecord SmallButtons_GR[] =
{
468, 220, 556, 230, NULL,										BUTTON_GADGET, NULL, /* 0 00:00:00:0 NO DAYS */
558, 220, 576, 230, NULL,										BUTTON_GADGET, NULL, /* 1 INC */
578, 220, 596, 230, NULL,										BUTTON_GADGET, NULL, /* 2 DEC */
599, 217, 635, 233, NULL,										BUTTON_GADGET, NULL, /* 3 effect */
469, 220, 490, 230, NULL,										BUTTON_GADGET, NULL, /* 4 hh */
494, 220, 515, 230, NULL,										BUTTON_GADGET, NULL, /* 5 mm */
519, 220, 540, 230, NULL,										BUTTON_GADGET, NULL, /* 6 ss */
544, 220, 555, 230, NULL,										BUTTON_GADGET, NULL, /* 7 ff */
458, 214, 476, 224, NULL,										BUTTON_GADGET, NULL, /* 8 su */
478, 214, 496, 224, NULL,										BUTTON_GADGET, NULL, /* 9 mo */
498, 214, 516, 224, NULL,										BUTTON_GADGET, NULL, /* 10 tu */
518, 214, 536, 224, NULL,										BUTTON_GADGET, NULL, /* 11 we */
538, 214, 556, 224, NULL,										BUTTON_GADGET, NULL, /* 12 th */
558, 214, 576, 224, NULL,										BUTTON_GADGET, NULL, /* 13 fr */
578, 214, 596, 224, NULL,										BUTTON_GADGET, NULL, /* 14 sa */
458, 226, 546, 236, NULL,										BUTTON_GADGET, NULL, /* 15 00:00:00:0 DAYS */
548, 226, 571, 236, NULL,										BUTTON_GADGET, NULL, /* 16 INC */
573, 226, 596, 236, NULL,										BUTTON_GADGET, NULL, /* 17 DEC */
459, 226, 480, 236, NULL,										BUTTON_GADGET, NULL, /* 18 hh */
484, 226, 505, 236, NULL,										BUTTON_GADGET, NULL, /* 19 mm */
509, 226, 530, 236, NULL,										BUTTON_GADGET, NULL, /* 20 ss */
534, 226, 545, 236, NULL,										BUTTON_GADGET, NULL, /* 21 ff */
458, 214, 556, 224, NULL,										BUTTON_GADGET, NULL, /* 22 00:00:00:00 TOP */
459, 214, 480, 224, NULL,										BUTTON_GADGET, NULL, /* 23 hh */
484, 214, 505, 224, NULL,										BUTTON_GADGET, NULL, /* 24 mm */
509, 214, 530, 224, NULL,										BUTTON_GADGET, NULL, /* 25 ss */
534, 214, 555, 224, NULL,										BUTTON_GADGET, NULL, /* 26 ff */
468, 217, 635, 233, NULL,										BUTTON_GADGET, NULL, /* 27 bitmap pos */
458, 214, 635, 233, NULL,										BUTTON_GADGET, NULL, /* 28 bitmap pos */
458, 214, 635, 233, NULL,										BUTTON_GADGET, NULL, /* 29 bitmap pos */
558, 214, 576, 224, NULL,										BUTTON_GADGET, NULL, /* 30 INC */
578, 214, 596, 224, NULL,										BUTTON_GADGET, NULL, /* 31 DEC */
558, 226, 576, 236, NULL,										BUTTON_GADGET, NULL, /* 32 INC */
578, 226, 596, 236, NULL,										BUTTON_GADGET, NULL, /* 33 DEC */
459, 226, 480, 236, NULL,										BUTTON_GADGET, NULL, /* 34 hh */
484, 226, 505, 236, NULL,										BUTTON_GADGET, NULL, /* 35 mm */
509, 226, 530, 236, NULL,										BUTTON_GADGET, NULL, /* 36 ss */
534, 226, 555, 236, NULL,										BUTTON_GADGET, NULL, /* 37 ff */
458, 226, 556, 236, NULL,										BUTTON_GADGET, NULL, /* 38 00:00:00:00 BOTTOM */

469, 214, 490, 224, NULL,										BUTTON_GADGET, NULL, /* 39 hh */
494, 214, 515, 224, NULL,										BUTTON_GADGET, NULL, /* 40 mm */
519, 214, 540, 224, NULL,										BUTTON_GADGET, NULL, /* 41 ss */
544, 214, 555, 224, NULL,										BUTTON_GADGET, NULL, /* 42 ff */

469, 226, 490, 236, NULL,										BUTTON_GADGET, NULL, /* 43 hh */
494, 226, 515, 236, NULL,										BUTTON_GADGET, NULL, /* 44 mm */
519, 226, 540, 236, NULL,										BUTTON_GADGET, NULL, /* 45 ss */
544, 226, 555, 236, NULL,										BUTTON_GADGET, NULL, /* 46 ff */

469, 214, 556, 224, NULL,										BUTTON_GADGET, NULL, /* 47 00:00:00:00 TOP */
469, 226, 556, 236, NULL,										BUTTON_GADGET, NULL, /* 48 00:00:00:00 BOTTOM */

468, 214, 635, 233, NULL,										BUTTON_GADGET, NULL, /* 49 bitmap pos */

-1
};

/*****************************************************
 * small windows for script objects
 *****************************************************/

struct StringRecord SharedWdw_String_SR = { 74,
"                                                                           " };

struct GadgetRecord SharedWdw_GR[] =
{
  0,   0, 640,  57, NULL,											DIMENSIONS, NULL,	
  0,	 0,	639,  56, NULL,											DBL_BORDER_REGION, NULL,
 60,   3, 633,  16, NULL,											STRING_GADGET, (struct GadgetRecord *)&SharedWdw_String_SR,
450,  40, 532,  53, TL_OK,										BUTTON_GADGET, NULL,
551,  40, 633,  53, TL_Cancel,								BUTTON_GADGET, NULL,
-1
};

/* remember to set extraData size to 34 in ScriptTalk.c */
struct StringRecord Arexx_Port_SR = { 34,
																		"                                   " };

/*****************************************************
 * arexx
 *****************************************************/

struct GadgetRecord ArexxWdw_GR[] =
{
178,  19, 198,  29, TL_Wait,									CHECK_GADGET, NULL,
 60,  19,  76,  27, TL_Command,								RADIO_GADGET, NULL,
 60,  29,  76,  37, TL_Script,  							RADIO_GADGET, NULL,
 60,  40, 171,  53, TL_View,									BUTTON_GADGET, NULL,
178,  40, 289,  53, TL_Edit,									BUTTON_GADGET, NULL,
362,  19, 633,  32, TL_Port,									STRING_GADGET, (struct GadgetRecord *)&Arexx_Port_SR,
-1
};

/*****************************************************
 * dos
 *****************************************************/

struct GadgetRecord DosWdw_GR[] =
{
178,  19, 198,  29, TL_Wait,									CHECK_GADGET, NULL,
 60,  19,  76,  27, TL_Command,								RADIO_GADGET, NULL,
 60,  29,  76,  37, TL_Script,  							RADIO_GADGET, NULL,
 60,  40, 171,  53, TL_View,									BUTTON_GADGET, NULL,
178,  40, 289,  53, TL_Edit,									BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * anim
 *****************************************************/

TEXT FPSList[] = { "\
Auto\0 1\0    2\0    3\0    4\0    5\0    6\0    7\0    8\0    9\0    \
10\0   11\0   12\0   13\0   14\0   15\0   16\0   17\0   18\0   19\0   20\0   \
21\0   22\0   23\0   24\0   25\0   26\0   27\0   28\0   29\0   30\0   31\0   \
32\0   33\0   34\0   35\0   36\0   37\0   38\0   39\0   40\0   41\0   42\0   \
43\0   44\0   45\0   46\0   47\0   48\0   49\0   50\0" };

struct CycleRecord AnimWdw_FPS_CR	= { 30, 51, 6, (STRPTR)&FPSList };
struct CycleRecord AnimWdw_Loops_CR	= { 0, 30, 4, (STRPTR)&WDef_BWidthList };

struct GadgetRecord AnimWdw_GR[] =
{
 60,  40, 171,  53, TL_Show,									BUTTON_GADGET, NULL,
243,  19, 321,  32, TL_FramesPerSecond,				CYCLE_GADGET, (struct GadgetRecord *)&AnimWdw_FPS_CR,
394,  19, 451,  32, TL_Loops,									CYCLE_GADGET, (struct GadgetRecord *)&AnimWdw_Loops_CR,
478,  19, 493,  27, TL_ColorCycle,						CHECK_GADGET, NULL,
478,  29, 493,  37, TL_PlayFromDisk,					CHECK_GADGET, NULL,
271,  42, 286,  50, TL_AddLoopFrames,					CHECK_GADGET, NULL,
-1
};

/*****************************************************
 * page
 *****************************************************/

struct GadgetRecord PageWdw_GR[] =
{
 60,  40, 171,  53, TL_Show,									BUTTON_GADGET, NULL,
178,  40, 289,  53, TL_Edit,									BUTTON_GADGET, NULL,
505,  19, 524,  29, TL_ColorCycle,						CHECK_GADGET, NULL,
-1
};

/*****************************************************
 * sound
 *****************************************************/

struct CycleRecord SoundWdw_Loops_CR = { 0, 30, 4, (STRPTR)&WDef_BWidthList };

//TEXT SoundTypeList[] = { TL_SoundTypes };

//struct CycleRecord Sound_TypeList_CR = { 5, 8, 16, (STRPTR)&SoundTypeList };

struct StringRecord Sound_Freq_SR = { 5, "      " };

struct StringRecord Sound_Volume_SR = { 2, "   " };

struct GadgetRecord SoundWdw_GR[] =
{
  6,  40, 117,  53, TL_Play,								BUTTON_GADGET, NULL,
120,  21, 179,  34, TL_Frequency,					 	INTEGER_GADGET, (struct GadgetRecord *)&Sound_Freq_SR,
271,  21, 328,  34, TL_Loops,								CYCLE_GADGET, (struct GadgetRecord *)&SoundWdw_Loops_CR,
130,  40, 350,  53, NULL,										LOBOX_REGION, NULL,	/* type */
439,  21, 468,  34, TL_Volume,							INTEGER_GADGET, (struct GadgetRecord *)&Sound_Volume_SR,
551,  21, 571,  31, TL_STOP,								CHECK_GADGET, NULL,
-1
};

/*****************************************************
 * binary
 *****************************************************/

struct GadgetRecord BinaryWdw_GR[] =
{
 60,  40, 171,  53, TL_View,								BUTTON_GADGET, NULL,
178,  40, 289,  53, TL_Edit,								BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * mail
 *****************************************************/

struct GadgetRecord MailWdw_GR[] =
{
-1
};

/*****************************************************
 * label
 *****************************************************/

struct GadgetRecord LabelWdw_GR[] =
{
  0,   0, 191, 129, NULL,										DIMENSIONS, NULL,	
  0,   0, 190, 128, NULL,										DBL_BORDER_REGION, NULL,	
 11,   3, 182,  11, TEXT_LABEL_1,						TEXT_REGION, NULL,
  7,  13, 161,  86, NULL,										BUTTON_GADGET, NULL,
165,  13, 183,  86, NULL,										HIBOX_REGION, NULL,
  7,  88, 161, 100, NULL,										LOBOX_REGION, NULL,
  7, 111,  89, 124, TL_OK,									BUTTON_GADGET, NULL,
101, 111, 183, 124, TL_Cancel,							BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * global events
 *****************************************************/

struct GadgetRecord GlobalEventsWdw_GR[] =
{
  0,   0, 336, 141, NULL,										DIMENSIONS, NULL,	
  0,   0, 335, 140, NULL,										DBL_BORDER_REGION, NULL,	
 11,   3, 152,  11, NULL,										TEXT_REGION, NULL,
155,   3, 329,  11, NULL,										TEXT_REGION, NULL,
  7,  13, 306,  98, NULL,										BUTTON_GADGET, NULL,
310,  13, 328,  98, NULL,										HIBOX_REGION, NULL,
  7, 100, 154, 112, NULL,										LOBOX_REGION, NULL,
159, 100, 306, 112, NULL,										BUTTON_GADGET, NULL,
  7, 123,  89, 136, TL_OK,									BUTTON_GADGET, NULL,
246, 123, 328, 136, TL_Cancel,							BUTTON_GADGET, NULL,
310, 100, 329, 112, "š",										BUTTON_GADGET, NULL,	/* 9a */
-1
};

/*****************************************************
 * timecode
 *****************************************************/

struct StringRecord TimeCode_SR = { 11, "            " };
struct StringRecord TimeCode2_SR = { 36, "                                     " };

struct CycleRecord TimeCode_PL_CR = { 0,  5, 28, TL_PreLoad };
struct CycleRecord TimeCode_PO_CR = { 0,  3, 15, TL_PlayOptions };
struct CycleRecord TimeCode_ST_CR = { 0,  2,  9, TL_Timing };

struct GadgetRecord TimeCodeWdw_GR[] =
{
  0,   0, 398, 199, NULL,										DIMENSIONS, NULL,	
  0,   0, 397, 198, NULL,										DBL_BORDER_REGION, NULL,	
 11,   3, 177,  12, NULL,										TEXT_REGION, NULL,
189,   3, 387,  12, NULL,										TEXT_REGION, NULL,
 11,  46, 178,  55, NULL,										TEXT_REGION, NULL,
 11,  18,  27,  26, TEXT_TIMECODE_5,				RADIO_GADGET, NULL,	/* internal */
 11,  29,  27,  37, TEXT_TIMECODE_6,				RADIO_GADGET, NULL,	/* external */
 11,  61,  27,  69, TEXT_TIMECODE_7,				RADIO_GADGET, NULL,	/* hh:mm:ss:t */
 11,  72,  27,  80, TEXT_TIMECODE_8,				RADIO_GADGET, NULL,	/* MIDI time code */
 11,  83,  27,  91, TEXT_TIMECODE_9,				RADIO_GADGET, NULL,	/* SMPTE */
 11,  94,  27, 102, TEXT_TIMECODE_10,				RADIO_GADGET, NULL,	/* ML time code */
 11, 105,  27, 113, NULL,										RADIO_GADGET, NULL,	/* user definable */
 33, 105, 311, 118, NULL,										STRING_GADGET, (struct GadgetRecord *)&TimeCode2_SR,
189,  18, 205,  26, TEXT_TIMECODE_11,				RADIO_GADGET, NULL,	/* 25 FPS */
189,  29, 205,  37, TEXT_TIMECODE_12,				RADIO_GADGET, NULL,	/* 30 FPS */
285,  45, 389,  58, TEXT_TIMECODE_13,				TIME_GADGET, (struct GadgetRecord *)&TimeCode_SR,
222,  61, 238,  69, TEXT_TIMECODE_14,				CHECK_GADGET, NULL,
140, 130, 389, 143, TL_PREFS11,							CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_ST_CR,
140, 146, 389, 159, TEXT_TIMECODE_15,				CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_PL_CR,
140, 162, 389, 175, TL_PREFS39,							CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_PO_CR,
  8, 181,  90, 194, TL_OK,									BUTTON_GADGET, NULL,
307, 181, 389, 194, TL_Cancel,							BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * object name
 *****************************************************/

struct StringRecord ObjectName_SR = { 29, "                              " };

struct GadgetRecord ObjectNameWdw_GR[] =
{
  0,   0, 300,  65, NULL,										DIMENSIONS, NULL,	
  0,   0, 299,  64, NULL,										DBL_BORDER_REGION, NULL,	
  4,   5, 294,  14, NULL,										TEXT_REGION, NULL,
  5,  18, 294,  31, NULL,										STRING_GADGET, (struct GadgetRecord *)&ObjectName_SR,
  7,  47,  89,  60, TL_OK,									BUTTON_GADGET, NULL,
210,  47, 292,  60, TL_Cancel,							BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * effect
 *****************************************************/

struct GadgetRecord ChooseEffect_GR[] =
{
  0,   0, 357, 164, NULL,										DIMENSIONS, NULL,	
  0,   0, 356, 163, NULL,										DBL_BORDER_REGION, NULL,	
  7,  14, 327, 116, NULL,										BUTTON_GADGET, NULL,	// scroll area
331,  14, 349, 116, NULL,										HIBOX_REGION, NULL,	// slider
  7, 146,  89, 159, TL_OK,									BUTTON_GADGET, NULL,
267, 146, 349, 159, TL_Cancel,							BUTTON_GADGET, NULL,
  6,   3, 350,  12, TL_SETTRANSITION,				TEXT_REGION, NULL,
116, 119, 299, 129, NULL,										BUTTON_GADGET, NULL,
116, 132, 299, 142, NULL,										BUTTON_GADGET, NULL,
302, 119, 327, 129, NULL,										LOBOX_REGION, NULL,
302, 132, 327, 142, NULL,										LOBOX_REGION, NULL,
  7, 120, 113, 129, TL_EFFECTSPEED,					TEXT_RIGHT, NULL,
  7, 133, 113, 142, TL_CHUNCKSIZE,					TEXT_RIGHT, NULL,
-1
};

/*****************************************************
 * debug
 *****************************************************/

struct StringRecord Debug_na1_SR = { 5, "      " };
struct StringRecord Debug_na2_SR = { 5, "      " };
struct StringRecord Debug_na3_SR = { 5, "      " };
struct StringRecord Debug_na4_SR = { 5, "      " };
struct StringRecord Debug_na5_SR = { 5, "      " };
struct StringRecord Debug_na6_SR = { 5, "      " };
struct StringRecord Debug_na7_SR = { 5, "      " };
struct StringRecord Debug_na8_SR = { 5, "      " };
struct StringRecord Debug_na9_SR = { 5, "      " };
struct StringRecord Debug_na10_SR = { 5, "      " };
struct StringRecord Debug_na11_SR = { 5, "      " };
struct StringRecord Debug_na12_SR = { 5, "      " };
struct StringRecord Debug_na13_SR = { 5, "      " };
struct StringRecord Debug_na14_SR = { 5, "      " };
struct StringRecord Debug_na15_SR = { 5, "      " };
struct StringRecord Debug_na16_SR = { 5, "      " };

struct StringRecord Debug_nodetype_SR = { 2, "   " };

struct StringRecord Debug_startendmode_SR = { 2, "   " };

struct StringRecord Debug_effect_SR = { 4, "     " };

struct StringRecord Debug_miscflags_SR = { 4, "     " };

struct StringRecord Debug_daybits_SR = { 3, "    " };

struct StringRecord Debug_duration_SR = { 10, "           " };

struct StringRecord Debug_extradata1_SR = { 60, "                                                             " };
struct StringRecord Debug_extradata2_SR = { 60, "                                                             " };
struct StringRecord Debug_path_SR = { 60, "                                                             " };
struct StringRecord Debug_name_SR = { 60, "                                                             " };

struct StringRecord Debug_start_days_SR = { 10, "           " };
struct StringRecord Debug_start_minutes_SR = { 10, "           " };
struct StringRecord Debug_start_ticks_SR = { 10, "           " };
struct StringRecord Debug_start_frames_SR = { 10, "           " };

struct StringRecord Debug_end_days_SR = { 10, "           " };
struct StringRecord Debug_end_minutes_SR = { 10, "           " };
struct StringRecord Debug_end_ticks_SR = { 10, "           " };
struct StringRecord Debug_end_frames_SR = { 10, "           " };

struct GadgetRecord Debug_GR[] =
{
  0,   0, 640, 200, NULL,										DIMENSIONS, NULL,	
  0,   0, 639, 199, NULL,										DBL_BORDER_REGION, NULL,	

560, 182, 630, 195, TL_OK,									BUTTON_GADGET, NULL,

 27,   2, 100,  14, "0",										STRING_GADGET, (struct GadgetRecord *)&Debug_na1_SR,
 27,  16, 100,  28, "1",										STRING_GADGET, (struct GadgetRecord *)&Debug_na2_SR,
 27,  30, 100,  42, "2",										STRING_GADGET, (struct GadgetRecord *)&Debug_na3_SR,
 27,  44, 100,  56, "3",										STRING_GADGET, (struct GadgetRecord *)&Debug_na4_SR,
 27,  58, 100,  70, "4",										STRING_GADGET, (struct GadgetRecord *)&Debug_na5_SR,

133,   2, 206,  14, "5",										STRING_GADGET, (struct GadgetRecord *)&Debug_na6_SR,
133,  16, 206,  28, "6",										STRING_GADGET, (struct GadgetRecord *)&Debug_na7_SR,
133,  30, 206,  42, "7",										STRING_GADGET, (struct GadgetRecord *)&Debug_na8_SR,
133,  44, 206,  56, "8",										STRING_GADGET, (struct GadgetRecord *)&Debug_na9_SR,
133,  58, 206,  70, "9",										STRING_GADGET, (struct GadgetRecord *)&Debug_na10_SR,

239,   2, 312,  14, "10",										STRING_GADGET, (struct GadgetRecord *)&Debug_na11_SR,
239,  16, 312,  28, "11",										STRING_GADGET, (struct GadgetRecord *)&Debug_na12_SR,
239,  30, 312,  42, "12",										STRING_GADGET, (struct GadgetRecord *)&Debug_na13_SR,
239,  44, 312,  56, "13",										STRING_GADGET, (struct GadgetRecord *)&Debug_na14_SR,
239,  58, 312,  70, "14",										STRING_GADGET, (struct GadgetRecord *)&Debug_na15_SR,

345,   2, 418,  14, "15",										STRING_GADGET, (struct GadgetRecord *)&Debug_na16_SR,

133,  75, 206,  87, "Node type:",						STRING_GADGET, (struct GadgetRecord *)&Debug_nodetype_SR,
133,  89, 206, 101, "StartEnd:",						STRING_GADGET, (struct GadgetRecord *)&Debug_startendmode_SR,
133, 103, 206, 115, "Effect nr.",						STRING_GADGET, (struct GadgetRecord *)&Debug_effect_SR,

345,  75, 418,  87, "MiscFlags:",						STRING_GADGET, (struct GadgetRecord *)&Debug_miscflags_SR,
345,  89, 418, 101, "DayBits:",							STRING_GADGET, (struct GadgetRecord *)&Debug_daybits_SR,
345, 103, 418, 115, "Duration:",						STRING_GADGET, (struct GadgetRecord *)&Debug_duration_SR,

 65, 122, 630, 134, "ED:",									STRING_GADGET, (struct GadgetRecord *)&Debug_extradata1_SR,
 65, 136, 630, 148, "ED+50:",								STRING_GADGET, (struct GadgetRecord *)&Debug_extradata2_SR,
 65, 150, 630, 161, "Path:",								STRING_GADGET, (struct GadgetRecord *)&Debug_path_SR,
 65, 164, 630, 176, "Name:",								STRING_GADGET, (struct GadgetRecord *)&Debug_name_SR,

478,   2, 551,  14, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_start_days_SR,
478,  16, 551,  28, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_start_minutes_SR,
478,  30, 551,  42, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_start_ticks_SR,
478,  44, 551,  56, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_start_frames_SR,

557,   2, 630,  14, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_end_days_SR,
557,  16, 630,  28, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_end_minutes_SR,
557,  30, 630,  42, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_end_ticks_SR,
557,  44, 630,  56, NULL,										STRING_GADGET, (struct GadgetRecord *)&Debug_end_frames_SR,

-1
};

/******** E O F ********/
