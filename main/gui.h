/******** GUI.H ********/

/*******************************************************
 * Favourite Applications (aka DAs or desk accessories) 
 *******************************************************/

struct StringRecord DANames_SR = { SIZE_FILENAME, "                                                   " };

struct GadgetRecord DANames_GR[] =
{
  0,   0, 317,  79, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	316,  78, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
139,   3, 258,  16, 1, NULL, Msg_FA_1,									STRING_GADGET, (struct GadgetRecord *)&DANames_SR,
  4,  19, 312,  33, 1, NULL, Msg_FA_2,									BUTTON_GADGET, NULL,
  4,  36, 312,  49, 2, NULL, 0,													LOBOX_REGION, NULL,
  6,  61,  89,  74, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
227,  61, 310,  74, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
130,  61, 223,  74, 0, NULL, Msg_FA_4,									BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/*****************************************************
 * Open file requester
 *****************************************************/

struct StringRecord FR_SR = { 150, "                                                                                                                                                        " };

struct GadgetRecord FileRequester_GR[] =
{
  0,   0, 320, 136, 0, NULL, 0,													DIMENSIONS, NULL,
  6,  13, 231,  26, 0, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&FR_SR,
  6,  28, 211, 131, 1, NULL, 0,													BUTTON_GADGET, NULL,
237,  13, 314,  26, 0, NULL, Msg_Parent,								BUTTON_GADGET, NULL,
237,  63, 314,  76, 1, NULL, Msg_Disks,									BUTTON_GADGET, NULL,
237,  79, 314,  92, 1, NULL, Msg_Assigns,								BUTTON_GADGET, NULL,
237, 100, 314, 113, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
237, 118, 314, 131, 0, NULL, Msg_Open,									BUTTON_GADGET, NULL,
214,  28, 231, 131, 0, NULL, 0,													HIBOX_REGION, NULL,
  0,	 0,	319, 135, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
237,  96, 314,  96, 0, NULL, 0,													LO_LINE, NULL,
251,  31, 273,  42, 0, NULL, Msg_Char_SelectAll,				BUTTON_GADGET, NULL,
279,  31, 301,  42, 0, NULL, Msg_Char_Thumbnails,				BUTTON_GADGET, NULL,
  5,   2, 312,  11, 0, NULL, 0,													TEXT_REGION, NULL,
237,  47, 314,  60, 1, NULL, Msg_Home,									BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)DANames_GR,
-1
};

/*****************************************************
 * "don't save, cancel, save" requester
 *****************************************************/

struct GadgetRecord DontSaveRequester_GR[] =
{
  0,   0, 300, 100, 0, NULL, 0,													DIMENSIONS, NULL,
 10,  82,  92,  95, 0, NULL, Msg_DontSave,							BUTTON_GADGET, NULL,
120,  82, 202,  95, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
207,  82, 289,  95, 1, NULL, Msg_Save,									BUTTON_GADGET, NULL,
  0,	 0,	299,  99, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)FileRequester_GR,
-1
};

/*****************************************************
 * Save file requester
 *****************************************************/

struct StringRecord SaveFile1_SR = { 150, "                                                                                                                                                        " };
struct StringRecord SaveFile2_SR = {  40, "                                         " };

struct GadgetRecord SaveFile_GR[] =
{
  0,   0, 320, 165, 0, NULL, 0,													DIMENSIONS, NULL,
  6,  13, 231,  26, 0, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&SaveFile1_SR,
  6,  28, 211, 131, 1, NULL, 0,													BUTTON_GADGET, NULL,
237,  13, 314,  26, 0, NULL, Msg_Parent,								BUTTON_GADGET, NULL,
237,  93, 314, 106, 1, NULL, Msg_Disks,									BUTTON_GADGET, NULL,
237, 109, 314, 122, 1, NULL, Msg_Assigns,								BUTTON_GADGET, NULL,
237, 130, 314, 143, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
237, 148, 314, 161, 0, NULL, Msg_Save,									BUTTON_GADGET, NULL,
214,  28, 231, 131, 0, NULL, 0,													HIBOX_REGION, NULL,
  0,	 0,	319, 164, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
237, 126, 314, 126, 0, NULL, 0,													LO_LINE, NULL,
  6, 148, 231, 161, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&SaveFile2_SR,
  5, 135, 235, 144, 0, NULL, 0,													TEXT_REGION, NULL,	/* 'Select a file' */
237,  77, 314,  90, 1, NULL, Msg_Home,									BUTTON_GADGET, NULL,
238,  47, 255,  55, 0, NULL, Msg_IFF,										INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)DontSaveRequester_GR,
-1
};

/*****************************************************
 * distributor
 *****************************************************/

struct CycleRecord Distri_1_CR = { 0, 2, 26, NULL, Msg_Distribute_List };
struct CycleRecord Distri_2_CR = { 0, 2, 16, NULL, Msg_Center_List };

struct GadgetRecord Distributor_GR[] =
{
  0,   0, 320, 126, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 125, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 312,  10, 0, NULL, Msg_Distributor,						TEXT_REGION, NULL,
 10,  16,  26,  24, 1, NULL, Msg_Distribute,						RADIO_GADGET, NULL,
 10,  47,  26,  55, 1, NULL, Msg_Center,								RADIO_GADGET, NULL,
 38,  27, 297,  40, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Distri_1_CR,
 38,  58, 297,  72, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Distri_2_CR,
 38,  81, 160, 121, 2, NULL, 0,													LOBOX_REGION, NULL,
228,  90, 310, 103, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
228, 108, 310, 121, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SaveFile_GR,
-1
};

/*****************************************************
 * duplicator
 *****************************************************/

struct StringRecord Dupli_times_SR	= { 2, "   " };
struct StringRecord Dupli_x_SR			= { 3, "    " };
struct StringRecord Dupli_y_SR			= { 3, "    " };

struct GadgetRecord Duplicator_GR[] =
{
  0,   0, 320,  74, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319,  73, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 312,  10, 0, NULL, Msg_Duplicator,						TEXT_REGION, NULL,
 91,  19, 121,  30, 1, NULL, Msg_Copy,									INTEGER_GADGET, (struct GadgetRecord *)&Dupli_times_SR,
 56,  37, 102,  48, 1, NULL, Msg_Add,										INTEGER_GADGET, (struct GadgetRecord *)&Dupli_x_SR,
 56,  52, 102,  63, 1, NULL, Msg_Add,										INTEGER_GADGET, (struct GadgetRecord *)&Dupli_y_SR,
228,  38, 310,  51, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
228,  56, 310,  69, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Distributor_GR,
-1
};

/*****************************************************
 * import requester
 *****************************************************/

struct GadgetRecord ImportType_GR[] =
{
  0,   0, 300,  99, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	299,  98, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 292,  10, 0, NULL, Msg_SelectImportType,			TEXT_REGION, NULL,
 11,  17,  27,  25, 1, NULL, Msg_Background,						RADIO_GADGET, NULL,
 11,  29,  27,  37, 1, NULL, Msg_PictureAndAnim,				RADIO_GADGET, NULL,
 11,  41,  27,  49, 1, NULL, Msg_Screen,								RADIO_GADGET, NULL,
 11,  53,  27,  61, 1, NULL, Msg_Text,									RADIO_GADGET, NULL,
 11,  65,  27,  73, 1, NULL, Msg_DataType,							RADIO_GADGET, NULL,
 11,  81,  93,  94, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
207,  81, 289,  94, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
157,  17, 173,  25, 1, NULL, Msg_ScaleIt,								CHECK_GADGET, NULL,
146,  29, 165,  39, 1, NULL, Msg_RemapIt,								INVISIBLE_GADGET, NULL,	/* CHECK_GADGET, NULL, */
  7,  12, 292,  12, 0, NULL, 0,													LO_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Duplicator_GR,
-1
};

/*****************************************************
 * windef window definition
 *****************************************************/

struct StringRecord WDef_X_SR 			= { 5, "     " };
struct StringRecord WDef_Y_SR 			= { 5, "     " };
struct StringRecord WDef_Width_SR		= { 5, "     " };
struct StringRecord WDef_Height_SR 	= { 5, "     " };

struct CycleRecord WDef_Fill_CR 		= { 0,  3, 10, NULL, Msg_FillType_List };
struct CycleRecord WDef_BWidth_CR		= { 0, 20,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord WDef_Pattern_CR	= { 0, 21,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord WDef_SdwWeight_CR= { 0, 20,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord WDef_SdwType_CR	= { 0,  5,  9, NULL, Msg_WDefShadowList };

struct GadgetRecord WDef_GR[] =
{
  0,   0, 640,  78, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	639,  77, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
548,  60, 630,  73, 0, NULL, Msg_Hide,									BUTTON_GADGET, NULL,

169,   2, 284,  15, 1, NULL, Msg_FillType,							CYCLE_GADGET, (struct GadgetRecord *)&WDef_Fill_CR,
169,  17, 284,  30, 1, NULL, Msg_PatternType, 					CYCLE_GADGET, (struct GadgetRecord *)&WDef_Pattern_CR,
169,  32, 284,  45, 1, NULL, Msg_BorderWidth,						CYCLE_GADGET, (struct GadgetRecord *)&WDef_BWidth_CR,
169,  47, 284,  60, 1, NULL, Msg_WDefShadow,						CYCLE_GADGET, (struct GadgetRecord *)&WDef_SdwType_CR,
169,  62, 284,  75, 1, NULL, Msg_Style_SLen_List,				CYCLE_GADGET, (struct GadgetRecord *)&WDef_SdwWeight_CR,
316,   2, 368,  15, 1, NULL, Msg_X,											INTEGER_GADGET, (struct GadgetRecord *)&WDef_X_SR, 
316,  17, 368,  30, 1, NULL, Msg_Y,											INTEGER_GADGET, (struct GadgetRecord *)&WDef_Y_SR, 
316,  32, 368,  45, 1, NULL, Msg_Width,									INTEGER_GADGET, (struct GadgetRecord *)&WDef_Width_SR, 
316,  47, 368,  60, 1, NULL, Msg_Height,								INTEGER_GADGET, (struct GadgetRecord *)&WDef_Height_SR, 
316,  62, 341,  75, 1, NULL, 0,													BUTTON_GADGET, NULL,

386,   3, 500,  13, 0, NULL, Msg_Borders,								TEXT_LEFT, NULL,

492,   3, 606,  13, 0, NULL, Msg_Colors,								TEXT_LEFT, NULL,

418,  17, 442,  25, 1, NULL, 0,													CHECK_GADGET, NULL,
450,  29, 474,  37, 1, NULL, 0,													CHECK_GADGET, NULL,
418,  41, 442,  49, 1, NULL, 0,													CHECK_GADGET, NULL,
386,  29, 410,  37, 1, NULL, 0,													CHECK_GADGET, NULL,

524,  17, 548,  25, 1, NULL, 0,													CHECK_GADGET, NULL,
556,  29, 580,  37, 1, NULL, 0,													CHECK_GADGET, NULL,
524,  41, 548,  49, 1, NULL, 0,													CHECK_GADGET, NULL,
492,  29, 516,  37, 1, NULL, 0,													CHECK_GADGET, NULL,
524,  29, 548,  37, 1, NULL, 0,													CHECK_GADGET, NULL,
556,  41, 580,  49, 1, NULL, 0,													CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ImportType_GR,
-1
};

/*****************************************************
 * palette (aka color adjust)
 *****************************************************/

struct GadgetRecord ColorAdjust_GR[] =
{
  0,   0, 640,  78, 0, NULL, 0,                  				DIMENSIONS, NULL,
  0,	 0,	639,  77, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
377,  62, 457,  74, 2, NULL, Msg_Hide,									BUTTON_GADGET, NULL,
 10,  32,  22,  45, 0, NULL, Msg_R,											TEXT_REGION, NULL,
 10,  47,  22,  60, 0, NULL, Msg_G,											TEXT_REGION, NULL,
 10,  62,  22,  75, 0, NULL, Msg_B,											TEXT_REGION, NULL,
 24,  32, 298,  45, 0, NULL, 0,													BUTTON_GADGET, NULL,
 24,  47, 298,  60, 0, NULL, 0,													BUTTON_GADGET, NULL,
 24,  62, 298,  75, 0, NULL, 0,													BUTTON_GADGET, NULL,
302,  32, 329,  75, 2, NULL, 0,													BUTTON_GADGET, NULL,
335,  32, 372,  75, 2, NULL, 0,													LOBOX_REGION, NULL,
337,  34, 370,  45, 2, NULL, 0,													TEXT_REGION, NULL,
337,  48, 370,  59, 2, NULL, 0,													TEXT_REGION, NULL,
337,  62, 370,  73, 2, NULL, 0,													TEXT_REGION, NULL,
377,  32, 457,  44, 0, NULL, Msg_Spread,								BUTTON_GADGET, NULL,
461,  32, 541,  44, 0, NULL, Msg_Copy,									BUTTON_GADGET, NULL,
377,  47, 457,  59, 0, NULL, Msg_Swap,									BUTTON_GADGET, NULL,
461,  47, 541,  59, 0, NULL, Msg_Undo,									BUTTON_GADGET, NULL,
545,  32, 625,  45, 2, NULL, Msg_LoadPalette,						BUTTON_GADGET, NULL,
545,  47, 625,  59, 2, NULL, Msg_SavePalette,						BUTTON_GADGET, NULL,
545,  62, 625,  74, 2, NULL, Msg_Limits,								BUTTON_GADGET, NULL,
461,  62, 541,  74, 2, NULL, Msg_BestColors,						BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)WDef_GR,
-1
};

/*****************************************************
 * harmonize (aka broadcast limits)
 *****************************************************/

struct GadgetRecord Harmonize_GR[] =
{
  0,   0, 388,  68, 0, NULL, 0,													DIMENSIONS, NULL,
  0,   0, 387,  67, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  4,   2, 383,  11, 0, NULL, Msg_BroadcastLimits,				TEXT_REGION, NULL,
  2,  11,  48,  23, 0, NULL, Msg_Min,										TEXT_RIGHT, NULL,
  2,  23,  48,  35, 0, NULL, Msg_Max,										TEXT_RIGHT, NULL,
 51,  12, 325,  22, 0, NULL, 0,													BUTTON_GADGET, NULL,
 51,  24, 325,  34, 0, NULL, 0,													BUTTON_GADGET, NULL,
330,  12, 382,  22, 2, NULL, 0,													LOBOX_REGION, NULL,
330,  24, 382,  34, 2, NULL, 0,													LOBOX_REGION, NULL,
  7,  50,  89,  63, 2, NULL, Msg_OK,										BUTTON_GADGET, NULL,
300,  50, 382,  63, 2, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 51,  36, 325,  46, 0, NULL, 0,													BUTTON_GADGET, NULL,
330,  36, 382,  46, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ColorAdjust_GR,
-1
};

/*****************************************************
 * color ramp 
 *****************************************************/

struct GadgetRecord ColorRamp_GR[] =
{
  0,   0, 388,  68, 0, NULL, 0,													DIMENSIONS, NULL,
  0,   0, 387,  67, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  4,   2, 383,  11, 0, NULL, Msg_BroadcastLimits2,			TEXT_REGION, NULL,
  2,  11,  48,  23, 0, NULL, Msg_R,											TEXT_RIGHT, NULL,
  2,  23,  48,  35, 0, NULL, Msg_G,											TEXT_RIGHT, NULL,
 51,  12, 325,  22, 0, NULL, 0,													BUTTON_GADGET, NULL,
 51,  24, 325,  34, 0, NULL, 0,													BUTTON_GADGET, NULL,
330,  12, 382,  22, 2, NULL, 0,													LOBOX_REGION, NULL,
330,  24, 382,  34, 2, NULL, 0,													LOBOX_REGION, NULL,
  7,  50,  89,  63, 2, NULL, Msg_OK,										BUTTON_GADGET, NULL,
300,  50, 382,  63, 2, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 51,  36, 325,  46, 0, NULL, 0,													BUTTON_GADGET, NULL,
330,  36, 382,  46, 2, NULL, 0,													LOBOX_REGION, NULL,
  2,  35,  48,  47, 0, NULL, Msg_B,											TEXT_RIGHT, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Harmonize_GR,
-1
};

/*****************************************************
 * screen size (aka format requester)
 *****************************************************/

struct CycleRecord Format_Overscan_CR	= { 0,  5,  6, NULL, Msg_Overscan_List };
struct CycleRecord Format_Color_CR		= { 0,  0,  0, NULL, NULL };

struct GadgetRecord FormatRequester_GR[] =
{
  0,   0, 320, 186, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 185, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 312,  10, 0, NULL, Msg_Format,								TEXT_REGION, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
  7, 163, 312, 163, 0, NULL, 0,													LO_LINE, NULL,
  7, 168,  89, 181, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
226, 168, 308, 181, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 11,  18, 286, 100, 1, NULL, 0,													BUTTON_GADGET, NULL,
290,  18, 308, 100, 0, NULL, 0,													HIBOX_REGION, NULL,
 11, 113, 128, 126, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Format_Color_CR,
140, 113, 308, 126, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Format_Overscan_CR,
 11, 131, 308, 144, 2, NULL, 0,													LOBOX_REGION, NULL,
 11, 149,  28, 157, 1, NULL, Msg_OptimizePalette,				CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ColorRamp_GR,
-1
};

/*****************************************************
 * extras (aka specials)
 *****************************************************/

struct CycleRecord Specials_CR = { 1, 2, 20, NULL, Msg_Ex_Cycle };

struct GadgetRecord Specials_GR[] =
{
  0,   0, 640,  78, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	639,  77, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
357,   4, 630,  17, 0, NULL, NULL,											CYCLE_GADGET, (struct GadgetRecord *)&Specials_CR,
548,  60, 630,  73, 0, NULL, Msg_Hide,									BUTTON_GADGET, NULL,
  4,   2, 326,  74, 2, NULL, 0,													BUTTON_GADGET, NULL,
331,   2, 348,  74, 0, NULL, 0,													HIBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)FormatRequester_GR,
-1
};

/*****************************************************
 * window transitions
 *****************************************************/

struct StringRecord WT_1_SR = { 2, "   " };
struct StringRecord WT_2_SR = { 2, "   " };
struct StringRecord WT_3_SR = { 2, "   " };
struct StringRecord WT_4_SR = { 2, "   " };
struct StringRecord WT_5_SR = { 2, "   " };
struct StringRecord WT_6_SR = { 2, "   " };

struct GadgetRecord WT_GR[] =
{
  0,   0, 320, 134, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 133, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  9,   2, 310,  11, 0, NULL, Msg_WindowTransitions,			TEXT_REGION, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
  7,  86, 312,  86, 0, NULL, 0,													LO_LINE, NULL,
  7, 109, 312, 109, 0, NULL, 0,													LO_LINE, NULL,
 91,  27, 121,  39, 1, NULL, Msg_Window2,								HIBOX_REGION, NULL,
 91,  45, 121,  57, 1, NULL, Msg_Picture,								HIBOX_REGION, NULL,
 91,  63, 121,  75, 1, NULL, Msg_Text,									HIBOX_REGION, NULL,
139,  27, 168,  39, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_1_SR,
139,  45, 168,  57, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_2_SR,
139,  63, 168,  75, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_3_SR,
206,  27, 236,  39, 1, NULL, 0,													BUTTON_GADGET, NULL,
206,  45, 236,  57, 1, NULL, 0,													BUTTON_GADGET, NULL,
206,  63, 236,  75, 1, NULL, 0,													BUTTON_GADGET, NULL,
254,  27, 283,  39, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_4_SR,
254,  45, 283,  57, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_5_SR,
254,  63, 283,  75, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&WT_6_SR,
 11,  91,  93, 104, 1, NULL, Msg_ApplyThisToAll,				BUTTON_GADGET, NULL,
265,  91, 290, 104, 2, NULL, Msg_Window,								LOBOX_REGION, NULL,
295,  89, 308,  96, 1, NULL, 0,													BUTTON_GADGET, NULL,
295,  99, 308, 106, 1, NULL, 0,													BUTTON_GADGET, NULL,
 11, 116,  93, 129, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
226, 116, 308, 129, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
290,   2, 310,  10, 0, NULL, 0,													INVISIBLE_GADGET, NULL,	// former ZIP
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Specials_GR,
-1
};

/*****************************************************
 * file thumbs
 *****************************************************/

struct GadgetRecord Thumbnail_GR[] =
{
  5, 186,  88, 199, 2, NULL, Msg_Previous,							BUTTON_GADGET, NULL,
 96, 186, 179, 199, 2, NULL, Msg_Next,									BUTTON_GADGET, NULL,
460, 186,	543, 199, 2, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
551, 186,	634, 199, 2, NULL, Msg_OK,										BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)WT_GR,
-1
};

/*****************************************************
 * screen thumbs
 *****************************************************/

struct GadgetRecord ScreenThumbs_GR[] =
{
  7,   0,  91,   0, 2, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 98,   0, 181,   0, 2, NULL, Msg_Grab,									BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Thumbnail_GR,
-1
};

/*******************************************************
 * select transition (Script Editor version)
 *******************************************************/

struct GadgetRecord ChooseEffect_GR[] =
{
  0,   0, 278, 194, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 277, 193, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  7,  14, 250, 146, 1, NULL, 0,													BUTTON_GADGET, NULL,	// scroll area
254,  14, 272, 146, 0, NULL, 0,													HIBOX_REGION, NULL,		// slider
  7, 176,  89, 189, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
190, 176, 272, 189, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  6,   3, 273,  12, 0, NULL, Msg_SelectTransition,			TEXT_REGION, NULL,
101, 149, 244, 159, 1, NULL, 0,													BUTTON_GADGET, NULL,
101, 162, 244, 172, 1, NULL, 0,													BUTTON_GADGET, NULL,
247, 149, 272, 159, 2, NULL, 0,													LOBOX_REGION, NULL,
247, 162, 272, 172, 2, NULL, 0,													LOBOX_REGION, NULL,
  7, 150,  98, 159, 1, NULL, Msg_Speed,									TEXT_RIGHT, NULL,
  7, 163,  98, 172, 1, NULL, Msg_ChunkSize,							TEXT_RIGHT, NULL,
  0,   0, 353,   0, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ScreenThumbs_GR,
-1
};

/*******************************************************
 * select transition (smaller one, PLS version)
 *******************************************************/

struct GadgetRecord PLS_ChooseEffect_GR[] =
{
  0,   0, 278, 164, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 277, 163, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  7,  14, 250, 116, 1, NULL, 0,													BUTTON_GADGET, NULL,	// scroll area
254,  14, 272, 116, 0, NULL, 0,													HIBOX_REGION, NULL,		// slider
  7, 146,  89, 159, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
190, 146, 272, 159, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  6,   3, 273,  12, 0, NULL, Msg_SelectTransition,			TEXT_REGION, NULL,
112, 119, 235, 129, 1, NULL, 0,													BUTTON_GADGET, NULL,
112, 132, 235, 142, 1, NULL, 0,													BUTTON_GADGET, NULL,
238, 119, 263, 129, 2, NULL, 0,													LOBOX_REGION, NULL,
238, 132, 263, 142, 2, NULL, 0,													LOBOX_REGION, NULL,
  7, 120, 109, 129, 0, NULL, Msg_Speed,									TEXT_RIGHT, NULL,
  7, 133, 109, 142, 0, NULL, Msg_ChunkSize,							TEXT_RIGHT, NULL,
 94, 146, 185, 159, 1, NULL, Msg_NoEffect,							BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ChooseEffect_GR,
-1
};

/**************************************************************************
 * select transition (smaller one, PLS version, with ApplyThisToAll etc.)
 **************************************************************************/

struct StringRecord PLS_SR = { 2, "   " };

struct GadgetRecord PLS_ChooseEffect_2_GR[] =
{
  0,   0, 320, 191, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 319, 190, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  7,  14, 290,  96, 1, NULL, 0,													BUTTON_GADGET, NULL,	// scroll area
294,  14, 312,  96, 0, NULL, 0,													HIBOX_REGION, NULL,		// slider
  7, 173,  89, 186, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
230, 173, 312, 186, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  6,   3, 312,  12, 0, NULL, Msg_SelectTransition,			TEXT_REGION, NULL,		// title
112,  99, 275, 109, 1, NULL, 0,													BUTTON_GADGET, NULL,	// speed slider
112, 112, 275, 122, 1, NULL, 0,													BUTTON_GADGET, NULL,	// vari slider
278,  99, 303, 109, 2, NULL, 0,													LOBOX_REGION, NULL,		// speed digits
278, 112, 303, 122, 2, NULL, 0,													LOBOX_REGION, NULL,		// vari digits
  7, 100, 109, 109, 0, NULL, Msg_Speed,									TEXT_RIGHT, NULL,
  7, 113, 109, 122, 0, NULL, Msg_ChunkSize,							TEXT_RIGHT, NULL,
  7, 125, 312, 125, 0, NULL, 0,													LO_LINE, NULL,
  7, 167, 312, 167, 0, NULL, 0,													LO_LINE, NULL,
293,   2, 313,  10, 0, NULL, 0,													INVISIBLE_GADGET, NULL,	// former ZIP
 97, 130, 122, 143, 2, NULL, Msg_Window,								LOBOX_REGION, NULL,		// window nr.
127, 128, 140, 135, 1, NULL, 0,													BUTTON_GADGET, NULL,	// up gadget
127, 138, 140, 145, 1, NULL, 0,													BUTTON_GADGET, NULL,	// down gadget
145, 149, 175, 162, 1, NULL, Msg_Delay2,								STRING_GADGET, (struct GadgetRecord *)&PLS_SR,
199, 129, 294, 143, 1, NULL, Msg_NoEffect,							BUTTON_GADGET, NULL,
199, 148, 294, 162, 1, NULL, Msg_ApplyThisToAll,				BUTTON_GADGET, NULL,
145, 129, 175, 143, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)PLS_ChooseEffect_GR,
-1
};

/*****************************************************
 * script window
 *****************************************************/

/**** THIS ONE IS STORED SO SEE GLOBALALLOCS FOR ALLOCMEMSIZE !!!!!!!!!!!!!!!! ****/

struct GadgetRecord Script_GR[] =
{
  4,  15, 420,   0, 2, NULL, 0,													BUTTON_GADGET, NULL,	// large scroll area
426,  15, 445,   0, 0, NULL, 0,													HIBOX_REGION, NULL,		// large scroll area prop slider
454,  15, 614,   0, 0, NULL, 0,													BUTTON_GADGET, NULL,	// icon area
620,  15, 639,   0, 0, NULL, 0,													HIBOX_REGION, NULL,		// icon area prop slider
552,  41, 639,   0, 1, NULL, Msg_Play,									BUTTON_GADGET, NULL,
454,  41, 541,   0, 1, NULL, Msg_Parent,								BUTTON_GADGET, NULL,
620,   0, 639,   9, 0, NULL, Msg_DepthArr,							BUTTON_GADGET, NULL,
454,  25, 541,   0, 1, NULL, Msg_Edit,									BUTTON_GADGET, NULL,
552,  25, 639,   0, 1, NULL, Msg_Show,									BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)PLS_ChooseEffect_2_GR,
-1
};

/*****************************************************
 * global events
 *****************************************************/

struct GadgetRecord GlobalEventsWdw_GR[] =
{
  0,   0, 406, 141, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 405, 140, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 11,   3, 152,  11, 0, NULL, 0,													TEXT_REGION, NULL,
194,   3, 405,  11, 0, NULL, 0,													TEXT_REGION, NULL,
  7,  13, 377,  98, 1, NULL, 0,													BUTTON_GADGET, NULL,
381,  13, 399,  98, 0, NULL, 0,													HIBOX_REGION, NULL,
  7, 100, 189, 112, 2, NULL, 0,													LOBOX_REGION, NULL,
194, 100, 376, 112, 1, NULL, 0,													BUTTON_GADGET, NULL,
  7, 123,  89, 136, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
316, 123, 398, 136, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
380, 100, 399, 112, 0, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Script_GR,
-1
};

/*****************************************************
 * input settings
 *****************************************************/

struct CycleRecord IS_PI_CR = { 0,  4, 16, NULL, Msg_PlayerInputList };
struct CycleRecord IS_AC_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
struct CycleRecord IS_MP_CR = { 0,  3,  6, NULL, Msg_SpriteOnOffAutoList };
struct CycleRecord IS_SP_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
struct CycleRecord IS_GP_CR = { 0,  2,  6, NULL, Msg_OnOff_List };
struct CycleRecord IS_SB_CR = { 0,  2,  5, NULL, Msg_YesNo_List };

struct GadgetRecord InputSettingsWdw_GR[] =
{
  0,   0, 398, 172, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 397, 171, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 10, 154,  92, 167, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
305, 154, 387, 167, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   1, 390,  10, 0, NULL, Msg_InputSettings,					TEXT_REGION, NULL,
  7,  12, 390,  12, 0, NULL, 0,													LO_LINE, NULL,
  7,  75, 390,  75, 0, NULL, 0,													DOTTED_LINE, NULL,
  7, 147, 390, 147, 0, NULL, 0,													LO_LINE, NULL,
187,  23, 335,  36, 1, NULL, Msg_PlayerInput,						CYCLE_GADGET, (struct GadgetRecord *)&IS_PI_CR,
187,  40, 335,  53, 1, NULL, Msg_MousePointer,					CYCLE_GADGET, (struct GadgetRecord *)&IS_MP_CR,
187,  57, 335,  70, 1, NULL, Msg_AsyncClicking,					CYCLE_GADGET, (struct GadgetRecord *)&IS_AC_CR,
187,  80, 335,  93, 1, NULL, Msg_ShowDateProg,					CYCLE_GADGET, (struct GadgetRecord *)&IS_SP_CR,
187,  97, 335, 110, 1, NULL, Msg_GamePort,							CYCLE_GADGET, (struct GadgetRecord *)&IS_GP_CR,
187, 114, 335, 127, 1, NULL, Msg_Standby,								CYCLE_GADGET, (struct GadgetRecord *)&IS_SB_CR,
347,  57, 373,  70, 0, "?",  0,													BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)GlobalEventsWdw_GR,
-1
};

/*****************************************************
 * label
 *****************************************************/

struct GadgetRecord LabelWdw_GR[] =
{
  0,   0, 191, 129, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 190, 128, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 11,   3, 182,  11, 0, NULL, Msg_SelectALabel,					TEXT_REGION, NULL,
  7,  13, 161,  86, 1, NULL, 0,													BUTTON_GADGET, NULL,
165,  13, 183,  86, 0, NULL, 0,													HIBOX_REGION, NULL,
  7,  88, 161, 100, 2, NULL, 0,													LOBOX_REGION, NULL,
  7, 111,  89, 124, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
101, 111, 183, 124, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)InputSettingsWdw_GR,
-1
};

/*****************************************************
 * object name
 *****************************************************/

struct StringRecord ObjectName_SR = { 29, "                              " };

struct GadgetRecord ObjectNameWdw_GR[] =
{
  0,   0, 300,  65, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 299,  64, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  4,   5, 294,  14, 0, NULL, 0,													TEXT_REGION, NULL,
  5,  18, 294,  31, 1, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&ObjectName_SR,
  7,  47,  89,  60, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
210,  47, 292,  60, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)LabelWdw_GR,
-1
};

/*****************************************************
 * timecode
 *****************************************************/

struct StringRecord TimeCode_SR		= { 11, "            " };
struct StringRecord TimeCode2_SR	= { 36, "                                     " };

struct CycleRecord TimeCode_ST_CR = { 0,  2, 11, NULL, Msg_ScriptTiming_List };
struct CycleRecord TimeCode_PL_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
struct CycleRecord TimeCode_PO_CR = { 0,  2, 15, NULL, Msg_PlayOptions_List };
struct CycleRecord TimeCode_PB_CR = { 0,  2, 20, NULL, Msg_PlayBuffer_List };

struct GadgetRecord TimeCodeWdw_GR[] =
{
  0,   0, 398, 199, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 397, 198, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 11,   3, 177,  12, 0, NULL, 0,													TEXT_REGION, NULL,
189,   3, 387,  12, 0, NULL, 0,													TEXT_REGION, NULL,
 11,  46, 178,  55, 0, NULL, 0,													TEXT_REGION, NULL,
 11,  18,  27,  26, 1, NULL, Msg_Internal,							RADIO_GADGET, NULL,
 11,  29,  27,  37, 1, NULL, Msg_External,							RADIO_GADGET, NULL,
 11,  61,  27,  69, 1, NULL, Msg_HHMMSST,								RADIO_GADGET, NULL,
 11,  72,  27,  80, 1, NULL, Msg_MIDI,									RADIO_GADGET, NULL,
 11,  83,  27,  91, 1, NULL, Msg_SMPTE,									RADIO_GADGET, NULL,
160, 162, 389, 175, 1, NULL, Msg_PlayBuffer,						CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_PB_CR, 
 11,  94,  27, 102, 1, NULL, 0,													RADIO_GADGET, NULL,	// custom 
 33,  94, 311, 107, 1, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&TimeCode2_SR,
189,  18, 205,  26, 1, NULL, Msg_25FPS,									RADIO_GADGET, NULL,
189,  29, 205,  37, 1, NULL, Msg_30FPS,									RADIO_GADGET, NULL,
285,  45, 389,  58, 1, NULL, Msg_Offset,								TIME_GADGET, (struct GadgetRecord *)&TimeCode_SR,
222,  61, 238,  69, 1, NULL, Msg_SendOut,								CHECK_GADGET, NULL,
160, 114, 389, 127, 1, NULL, Msg_ScriptTiming,					CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_ST_CR,
160, 130, 292, 143, 1, NULL, Msg_Preload,								CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_PL_CR,
160, 146, 389, 159, 1, NULL, Msg_PlayOptions,						CYCLE_GADGET, (struct GadgetRecord *)&TimeCode_PO_CR,
  8, 181,  90, 194, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
307, 181, 389, 194, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
304, 132, 320, 140, 0, NULL, Msg_Cache,									CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ObjectNameWdw_GR,
-1
};

/*****************************************************
 * Shared Window - small windows for script objects
 *****************************************************/

struct StringRecord SharedWdw_String_SR = { 74, "                                                                           " };

struct GadgetRecord SharedWdw_GR[] =
{
  0,   0, 640,  60, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	639,  59, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
 60,   3, 633,  16, 1, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&SharedWdw_String_SR,
450,  42, 532,  55, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
551,  42, 633,  55, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)TimeCodeWdw_GR,
-1
};

/*****************************************************
 * anim
 *****************************************************/

struct CycleRecord AnimWdw_FPS_CR		= { 30, 51, 6, NULL, Msg_FPS_Auto_60 };
struct CycleRecord AnimWdw_Loops_CR	= {  0, 31, 4, NULL, Msg_Infinite_30 };

struct GadgetRecord AnimWdw_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
 60,  42, 171,  55, 0, NULL, Msg_Show,									BUTTON_GADGET, NULL,
233,  19, 299,  32, 1, NULL, Msg_FramesPerSecond,				CYCLE_GADGET, (struct GadgetRecord *)&AnimWdw_FPS_CR,
394,  19, 451,  32, 1, NULL, Msg_Loops,									CYCLE_GADGET, (struct GadgetRecord *)&AnimWdw_Loops_CR,
478,  19, 493,  27, 1, NULL, Msg_AddLoopFrames,					CHECK_GADGET, NULL,
478,  29, 493,  37, 1, NULL, Msg_PlayFromDisk,					CHECK_GADGET, NULL,
195,  42, 306,  55, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SharedWdw_GR,
-1
};

/*****************************************************
 * arexx
 *****************************************************/

/* remember to set extraData size to 34 in ScriptTalk.c */
struct StringRecord Arexx_Port_SR = { 34, "                                   " };

struct GadgetRecord ArexxWdw_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
178,  19, 198,  29, 1, NULL, Msg_Wait,									CHECK_GADGET, NULL,
 60,  19,  76,  27, 1, NULL, Msg_Command,								RADIO_GADGET, NULL,
 60,  29,  76,  37, 1, NULL, Msg_Script2, 							RADIO_GADGET, NULL,
 60,  42, 171,  55, 0, NULL, Msg_View,									BUTTON_GADGET, NULL,
178,  42, 289,  55, 0, NULL, Msg_Edit,									BUTTON_GADGET, NULL,
362,  19, 633,  32, 1, NULL, Msg_Port,									STRING_GADGET, (struct GadgetRecord *)&Arexx_Port_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)AnimWdw_GR,
-1
};

/*****************************************************
 * dos
 *****************************************************/

struct StringRecord DosWdw_Stack_SR = { 6, "       " };

struct GadgetRecord DosWdw_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
178,  19, 198,  29, 1, NULL, Msg_Wait,									CHECK_GADGET, NULL,
 60,  19,  76,  27, 1, NULL, Msg_Command,								RADIO_GADGET, NULL,
 60,  29,  76,  37, 1, NULL, Msg_Script2, 							RADIO_GADGET, NULL,
 60,  42, 171,  55, 0, NULL, Msg_View,									BUTTON_GADGET, NULL,
178,  42, 289,  55, 0, NULL, Msg_Edit,									BUTTON_GADGET, NULL,
529,  19, 633,  32, 1, NULL, Msg_Stack,									INTEGER_GADGET, (struct GadgetRecord *)&DosWdw_Stack_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ArexxWdw_GR,
-1
};

/*****************************************************
 * page
 *****************************************************/

struct GadgetRecord PageWdw_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
 60,  42, 171,  55, 0, NULL, Msg_Show,									BUTTON_GADGET, NULL,
178,  42, 289,  55, 0, NULL, Msg_Edit,									BUTTON_GADGET, NULL,
478,  19, 493,  27, 1, NULL, Msg_ColorCycle,						CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)DosWdw_GR,
-1
};

/*****************************************************
 * sound
 *****************************************************/

struct CycleRecord SoundWdw_Action_CR = {  0, 3, 10, NULL, Msg_SoundObjList };

struct GadgetRecord SoundWdw_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
 60,  42, 171,  55, 0, NULL, Msg_Play,									BUTTON_GADGET, NULL,
 60,  19, 171,  32, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&SoundWdw_Action_CR,
192,  19, 412,  32, 2, NULL, 0,													LOBOX_REGION, NULL,	// type
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)PageWdw_GR,
-1
};

/*****************************************************
 * date programming
 *****************************************************/

struct StringRecord Prog_StartDay_SR 		= {  8, "         " };
struct StringRecord Prog_EndDay_SR 			= {  8, "         " };
struct StringRecord Prog_StartTime_SR 	= { 10, "           " };
struct StringRecord Prog_EndTime_SR 		= { 10, "           " };
struct StringRecord Prog_Duration_SR		= { 10, "           " };

struct CycleRecord Prog_Day1_CR 		= { 0,  7, 15, NULL, Msg_LongDayNames };
struct CycleRecord Prog_Date1_CR 		= { 0, 31,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Prog_Month1_CR 	= { 0, 12, 15, NULL, Msg_MonthNames };
struct CycleRecord Prog_Year1_CR		= {	0,  9,  6, NULL, Msg_Years_1990_1998 };

struct CycleRecord Prog_Day2_CR 		= { 0,  7, 15, NULL, Msg_LongDayNames };
struct CycleRecord Prog_Date2_CR 		= { 0, 31,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Prog_Month2_CR		= { 0, 12, 15, NULL, Msg_MonthNames };
struct CycleRecord Prog_Year2_CR		= {	0,  9,  6, NULL, Msg_Years_1990_1998 };

struct GadgetRecord ProgBackup_GR[] =
{
  0,   0,   0,   0, 0, NULL, 0,													DIMENSIONS, NULL,	
100,  23, 185,  36, 1, NULL, 0,													DATE_GADGET, NULL,
100,  40, 194,  53, 1, NULL, 0,													TIME_GADGET, NULL,
  2,  21, 196,  55, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SoundWdw_GR,
-1
};

struct GadgetRecord Prog_GR[] =
{
  0,   0, 640, 200, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	639, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
100,   6, 185,  19, 1, NULL, Msg_StartDay, 							DATE_GADGET, (struct GadgetRecord *)&Prog_StartDay_SR,
100,  23, 185,  36, 1, NULL, Msg_EndDay, 								DATE_GADGET, (struct GadgetRecord *)&Prog_EndDay_SR,
100,  40, 194,  53, 1, NULL, Msg_StartTime, 						TIME_GADGET, (struct GadgetRecord *)&Prog_StartTime_SR, 
100,  57, 194,  70, 1, NULL, Msg_EndTime, 							TIME_GADGET, (struct GadgetRecord *)&Prog_EndTime_SR, 
219,  41, 238,  51, 1, NULL, Msg_DateTimeRelation,			CHECK_GADGET, NULL,
219,   6, 361,  19, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Day1_CR,
368,   6, 415,  19, 1, NULL, 0,													CYCLE_GADGET,	(struct GadgetRecord *)&Prog_Date1_CR,
422,   6, 554,  19, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Month1_CR,
561,   6, 629,  19, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Year1_CR,
219,  23, 361,  36, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Day2_CR,
368,  23, 415,  36, 1, NULL, 0,													CYCLE_GADGET,	(struct GadgetRecord *)&Prog_Date2_CR,
422,  23, 554,  36, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Month2_CR, 
561,  23, 629,  36, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Prog_Year2_CR,
415,  40, 443,  52, 1, NULL, Msg_ShortSunday,						BUTTON_GADGET, NULL,
446,  40, 474,  52, 1, NULL, Msg_ShortMonday,						BUTTON_GADGET, NULL,
477,  40, 505,  52, 1, NULL, Msg_ShortTuesday,					BUTTON_GADGET, NULL, 
508,  40, 536,  52, 1, NULL, Msg_ShortWednesday,				BUTTON_GADGET, NULL, 
539,  40, 567,  52, 1, NULL, Msg_ShortThursday,					BUTTON_GADGET, NULL, 
570,  40, 598,  52, 1, NULL, Msg_ShortFriday,						BUTTON_GADGET, NULL, 
601,  40, 629,  52, 1, NULL, Msg_ShortSaturday,					BUTTON_GADGET, NULL, 
446,  57, 528,  70, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
547,  57, 629,  70, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
299,  57, 393,  70, 1, NULL, Msg_Duration,							TIME_GADGET, (struct GadgetRecord *)&Prog_Duration_SR, 
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ProgBackup_GR,
-1
};

struct GadgetRecord SmallWarning_GR[] =
{
  0,   0, 320,  68, 0, NULL, 0,													DIMENSIONS, NULL,
  0,   0, 319,  67, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,  50,  89,  63, 0, NULL, 0,													BUTTON_GADGET, NULL,	// LEFT
230,  50, 312,  63, 0, NULL, 0,													BUTTON_GADGET, NULL,	// RIGHT
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prog_GR,
-1
};

/*****************************************************
 * font selector
 *****************************************************/

struct StringRecord FontSelect_SR = { 3, "    " };

struct GadgetRecord FontSelect_GR[] =
{
  0,   0, 320, 172, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 171, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   3, 224,  60, 1, NULL, 0,													BUTTON_GADGET, NULL,	// name scroll area
227,   3, 245,  60, 0, NULL, 0,													HIBOX_REGION, NULL,		// prop gad
252,   3, 291,  45, 1, NULL, 0,													BUTTON_GADGET, NULL,	// size scroll area
294,   3, 312,  45, 0, NULL, 0,													HIBOX_REGION, NULL,		// prop gad
252,  47, 291,  60, 0, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&FontSelect_SR,
  7,  63, 312, 149, 0, NULL, 0,													BUTTON_GADGET, NULL,		// sample
  7, 154,  89, 167, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
230, 154, 312, 167, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SmallWarning_GR,
-1
};

/*****************************************************
 * style selector
 *****************************************************/

struct CycleRecord Style1_CR	= { 0,  5, 6, NULL, Msg_AntiAlias_List };
struct CycleRecord Style2_CR	= { 0,  3, 2, NULL, 0 };
struct CycleRecord Style3_CR	= { 0, 14, 4, NULL, Msg_Minus3_15 };
struct CycleRecord Style4_CR	= { 0, 14, 4, NULL, Msg_Minus3_15 };
struct CycleRecord Style5_CR	= { 0,  6, 9, NULL, Msg_Italicize_List };
struct CycleRecord Style6_CR	= { 0,  5, 2, NULL, 0 };
struct CycleRecord Style7_CR	= { 0, 16, 4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Style8_CR	= { 0,  8, 2, NULL, Msg_ShadowDirection_List };
struct CycleRecord Style9_CR	= { 0,  5, 4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Style10_CR	= { 0, 14, 4, NULL, Msg_Minus3_15 };

struct GadgetRecord Style_GR[] =
{
  0,   0, 640,  78, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	639,  77, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
163,   2, 238,  15, 1, NULL, Msg_Style_Anti_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style1_CR,
163,  17, 238,  30, 1, NULL, Msg_Style_Just_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style2_CR,
163,  32, 238,  45, 1, NULL, Msg_Style_CSpc_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style3_CR,
163,  47, 238,  60, 1, NULL, Msg_Style_LSpc_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style4_CR,
163,  62, 238,  75, 1, NULL, Msg_Style_Slan_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style5_CR,
440,   2, 515,  15, 1, NULL, Msg_Style_STyp_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style6_CR,
440,  17, 515,  30, 1, NULL, Msg_Style_SLen_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style7_CR,
440,  32, 515,  45, 1, NULL, Msg_Style_SDir_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style8_CR,
440,  47, 515,  60, 1, NULL, Msg_Style_UWei_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style9_CR,
440,  62, 515,  75, 1, NULL, Msg_Style_UOff_List,				CYCLE_GADGET, (struct GadgetRecord *)&Style10_CR,
525,   2, 558,  15, 1, NULL, 0,													BUTTON_GADGET, NULL,
548,  60, 630,  73, 0, NULL, Msg_Hide,									BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)FontSelect_GR,
-1
};

/*****************************************************
 * crawl
 *****************************************************/

struct CycleRecord Crawl_CR	= { 0, 30, 4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord Crawl_GR[] =
{
  0,   0, 320,  98, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319,  97, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 312,  10, 0, NULL, Msg_Crawl,									TEXT_REGION, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
 33,  17, 312,  30, 1, NULL, 0,													STRING_GADGET, NULL,
 87,  35, 312,  48, 0, NULL, Msg_CrawlFont,							HIBOX_REGION, NULL,
 87,  53, 143,  66, 0, NULL, Msg_Speed,									CYCLE_GADGET, (struct GadgetRecord *)&Crawl_CR,
  7,  80,  89,  93, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
230,  80, 312,  93, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
171,  53, 253,  66, 0, NULL, Msg_CrawlColor,						BUTTON_GADGET, NULL,
  7,  17,  30,  30, 0, NULL, 0,													BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Style_GR,
-1
};

#if 0
/*****************************************************
 * Help
 *****************************************************/

struct GadgetRecord Help_GR[] =
{
  0,   0, 320, 168, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 167, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   2, 312,  11, 0, NULL, Msg_HelpHeader,						TEXT_REGION, NULL,
  7,  13, 290, 146, 2, NULL, 0,                  				LOBOX_REGION, NULL,
242, 150, 312, 163, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
295,  13, 312, 146, 0, NULL, 0,													HIBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Crawl_GR,
-1
};
#endif

/*****************************************************
 * Literals
 *****************************************************/

struct GadgetRecord Literals_GR[] =
{
  0,   0, 314, 199, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	313, 198, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
222, 181, 304, 194, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  9,   5,  29,  16, 1, NULL, 0,													BUTTON_GADGET, NULL,	// single char box
  9, 142, 155, 193, 1, NULL, 0,													BUTTON_GADGET, NULL,	// scroll box
159, 142, 177, 193, 0, NULL, 0,													HIBOX_REGION, NULL,		// slider
222, 162, 304, 175, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  9, 125, 304, 138, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Crawl_GR,
-1
};

/*****************************************************
 * PLS Interactivity
 *****************************************************/

struct CycleRecord Inter1_CR	= { 0, 7, 18, NULL, Msg_Inter6 };
struct CycleRecord Inter2_CR	= { 0, 7, 18, NULL, Msg_Inter7 };
struct CycleRecord Inter3_CR	= { 0, 4, 11, NULL, Msg_Inter8 };
struct CycleRecord Inter4_CR	= { 0, 2,  5, NULL, Msg_YesNo_List };

struct StringRecord Inter1_SR = { 50, "                                                   " };
struct StringRecord Inter2_SR = { 50, "                                                   " };

struct GadgetRecord Interactive_GR[] =
{
  0,   0, 320, 175, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 174, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
 10, 157,  92, 170, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
227, 157, 309, 170, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   1, 312,  10, 0, NULL, Msg_Inter1,								TEXT_REGION, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
101,  18, 309,  31, 1, NULL, Msg_LocalEvents_9,					CYCLE_GADGET, (struct GadgetRecord *)&Inter1_CR,
101,  33, 309,  46, 1, NULL, Msg_Inter2,								STRING_GADGET, (struct GadgetRecord *)&Inter1_SR,
101,  48, 309,  61, 1, NULL, Msg_Inter3,								CYCLE_GADGET, (struct GadgetRecord *)&Inter2_CR,
101,  78, 309,  91, 1, NULL, Msg_Inter4,								CYCLE_GADGET, (struct GadgetRecord *)&Inter3_CR,
101,  93, 287, 106, 1, NULL, Msg_Inter5,								HIBOX_REGION, NULL,
291,  93, 309, 106, 1, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
  7, 127, 312, 127, 0, NULL, 0,													LO_LINE, NULL,
  7, 150, 312, 150, 0, NULL, 0,													LO_LINE, NULL,
 11, 132,  93, 145, 0, NULL, Msg_ApplyThisToAll,				BUTTON_GADGET, NULL,
257, 132, 290, 145, 2, NULL, Msg_Window,								LOBOX_REGION, NULL,		// wdw nr
295, 130, 308, 137, 0, NULL, 0,													BUTTON_GADGET, NULL,	// next wdw
295, 140, 308, 147, 0, NULL, 0,													BUTTON_GADGET, NULL,	// prev wdw
101,  63, 309,  76, 1, NULL, Msg_AutoDetect,						CYCLE_GADGET, (struct GadgetRecord *)&Inter4_CR,
  5,  65,  15,  73, 2, NULL, NULL,											INVISIBLE_GADGET, NULL,
101, 108, 309, 121, 1, NULL, Msg_Inter9,								STRING_GADGET, (struct GadgetRecord *)&Inter2_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Literals_GR,
-1
};

/*****************************************************
 * Script variable declaration
 *****************************************************/

struct StringRecord VarDec_SR = { 70, "                                                                                " };

struct GadgetRecord VarDec_GR[] =
{
  0,   0, 398, 173, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	397, 172, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
201, 154, 283, 167, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
305, 154, 387, 167, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   1, 390,  10, 0, NULL, Msg_VarDec1,								TEXT_REGION, NULL,
  7,  12, 390,  12, 0, NULL, 0,													LO_LINE, NULL,
 10,  26, 365, 109, 1, NULL, 0,													BUTTON_GADGET, NULL,	// decl list
370,  26, 387, 109, 0, NULL, 0,													BUTTON_GADGET, NULL,	// decl scroll bar
 10, 113, 365, 126, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&VarDec_SR,
 10, 129,  88, 142, 1, NULL, Msg_VarDec2,								BUTTON_GADGET, NULL,	// new
101, 129, 179, 142, 1, NULL, Msg_VarDec3,								BUTTON_GADGET, NULL,	// delete
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Interactive_GR,
-1
};

/*****************************************************
 * Script expression list
 *****************************************************/

struct StringRecord ExpDec_SR = { 70, "                                                                                " };

struct GadgetRecord ExpDec_GR[] =
{
  0,   0, 624, 173, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	623, 172, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
427, 154, 509, 167, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
531, 154, 613, 167, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   1, 616,  10, 0, NULL, Msg_ExpDec1,								TEXT_REGION, NULL,
  7,  12, 616,  12, 0, NULL, 0,													LO_LINE, NULL,
 10,  26, 365, 109, 1, NULL, 0,													BUTTON_GADGET, NULL,	// expr list
370,  26, 387, 109, 0, NULL, 0,													BUTTON_GADGET, NULL,	// expr scroll bar
 10, 113, 365, 126, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&VarDec_SR,
 10, 129,  88, 142, 1, NULL, Msg_VarDec2,								BUTTON_GADGET, NULL,	// new
101, 129, 179, 142, 1, NULL, Msg_VarDec3,								BUTTON_GADGET, NULL,	// delete
 10, 149, 366, 162, 2, NULL, NULL,											LOBOX_REGION, NULL,
403,  26, 591, 109, 2, NULL, 0,													LOBOX_REGION, NULL,	// decl list
596,  26, 613, 109, 0, NULL, 0,													BUTTON_GADGET, NULL,	// decl scroll bar
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)VarDec_GR,
-1
};

/*****************************************************
 * Key list
 *****************************************************/

struct GadgetRecord KeyList_GR[] =
{		
  0,   0, 165, 116, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	164, 115, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
 32,  98, 134, 111, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   6, 135,  91, 1, NULL, 0,													BUTTON_GADGET, NULL,	// key list
139,   6, 157,  91, 0, NULL, 0,													HIBOX_REGION, NULL,	// key scroll bar
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ExpDec_GR,
-1
};

/*****************************************************
 * Local Events
 *****************************************************/

struct GadgetRecord LocalEvents_GR[] =
{
  0,   0, 518, 158, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	517, 157, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 510,  10, 0, NULL, Msg_LocalEvents_1,					TEXT_REGION, NULL,	// title
  7,  12, 510,  12, 0, NULL, 0,													LO_LINE, NULL,
  7,  21, 135,  30, 0, NULL, Msg_LocalEvents_2,					TEXT_LEFT, NULL,
376,  21, 504,  30, 0, NULL, Msg_LocalEvents_3,					TEXT_LEFT, NULL,
  7,  32, 229, 115, 1, NULL, 0,													BUTTON_GADGET, NULL,
233,  32, 251, 115, 0, NULL, 0,													HIBOX_REGION, NULL,
266,  32, 488, 115, 1, NULL, 0,													BUTTON_GADGET, NULL,
492,  32, 510, 115, 0, NULL, 0,													HIBOX_REGION, NULL,
  7, 119,  89, 132, 1, NULL, Msg_Edit,									INVISIBLE_GADGET, NULL,
  7, 140,  89, 153, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
428, 140, 510, 153, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
266, 119, 348, 132, 0, NULL, Msg_LocalEvents_5,					BUTTON_GADGET, NULL,
266,  21, 376,  30, 0, NULL, Msg_LocalEvents_7,					TEXT_LEFT, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)KeyList_GR,
-1
};

/*****************************************************
 * Time Code Tweaker
 *****************************************************/

struct GadgetRecord TC_Tweaker_GR[] =
{
  0,   0, 220, 118, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	219, 117, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 212,  10, 0, NULL, Msg_TC_Title,							TEXT_REGION, NULL,	// title
  7,  12, 212,  12, 0, NULL, 0,													LO_LINE, NULL,
  8, 100,  90, 113, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
129, 100, 211, 113, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)LocalEvents_GR,
-1
};

struct StringRecord TC_Start_SR		= { 11, "            " };
struct StringRecord TC_End_SR			= { 11, "            " };
struct StringRecord TC_Delta_SR		= { 11, "            " };
struct StringRecord TC_Offset_SR	= { 11, "            " };

struct GadgetRecord TC_Tweaker1_GR[] =
{
 97,  24, 201,  37, 1, NULL, Msg_TC_Start,							TIME_GADGET, (struct GadgetRecord *)&TC_Start_SR,
 97,  41, 201,  54, 1, NULL, Msg_TC_End,								TIME_GADGET, (struct GadgetRecord *)&TC_End_SR,
 97,  58, 201,  71, 1, NULL, Msg_TC_Delta,							TIME_GADGET, (struct GadgetRecord *)&TC_Delta_SR,
 97,  75, 201,  88, 1, NULL, Msg_TC_Offset,							TIME_GADGET, (struct GadgetRecord *)&TC_Offset_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)TC_Tweaker_GR,
-1
};

struct StringRecord TCP_Start_SR	= { 10, "           " };
struct StringRecord TCP_End_SR		= { 10, "           " };
struct StringRecord TCP_Delta_SR	= { 10, "           " };
struct StringRecord TCP_Offset_SR	= { 10, "           " };

struct GadgetRecord TC_Tweaker2_GR[] =
{
 97,  24, 191,  37, 1, NULL, Msg_TC_Start,							TIME_GADGET, (struct GadgetRecord *)&TCP_Start_SR,
 97,  41, 191,  54, 1, NULL, Msg_TC_End,								TIME_GADGET, (struct GadgetRecord *)&TCP_End_SR,
 97,  58, 191,  71, 1, NULL, Msg_TC_Delta,							TIME_GADGET, (struct GadgetRecord *)&TCP_Delta_SR,
 97,  75, 191,  88, 1, NULL, Msg_TC_Offset,							TIME_GADGET, (struct GadgetRecord *)&TCP_Offset_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)TC_Tweaker1_GR,
-1
};

/*****************************************************
 * SetTime1
 *****************************************************/

struct StringRecord SetTime1_SR = { 10, "           " };

struct GadgetRecord SetTime1_GR[] =
{
  0,   0, 189,  62, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	188,  61, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
 52,   7,  71,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,   7,  96,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,   7, 121,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,   7, 138,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 52,  30,  71,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,  30,  96,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,  30, 121,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,  30, 138,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 47,  16, 141,  28, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime1_SR,
  5,  45,  87,  58, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
101,  45, 183,  58, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)TC_Tweaker2_GR,
-1
};

/*****************************************************
 * SetTime2
 *****************************************************/

struct StringRecord SetTime2_1_SR = { 10, "           " };
struct StringRecord SetTime2_2_SR = { 10, "           " };

struct GadgetRecord SetTime2_GR[] =
{
  0,   0, 189,  98, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	188,  97, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
 52,   7,  71,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,   7,  96,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,   7, 121,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,   7, 138,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 52,  30,  71,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,  30,  96,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,  30, 121,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,  30, 138,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 47,  16, 141,  28, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime2_1_SR,
 52,  43,  71,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,  43,  96,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,  43, 121,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,  43, 138,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
 52,  66,  71,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
 77,  66,  96,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
102,  66, 121,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
125,  66, 138,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
 47,  52, 141,  64, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime2_2_SR,
  5,  81,  87,  94, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
101,  81, 183,  94, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SetTime1_GR,
-1
};

/*****************************************************
 * SetTime3
 *****************************************************/

struct StringRecord SetTime3_SR = { 10, "           " };

struct GadgetRecord SetTime3_GR[] =
{
  0,   0, 232,  97, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	231,  96, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
 74,   7,  93,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 99,   7, 118,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
124,   7, 143,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
147,   7, 160,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 74,  30,  93,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 99,  30, 118,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
124,  30, 143,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
147,  30, 160,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 69,  16, 163,  28, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime3_SR,
  5,  43,  34,  54, 1, NULL, Msg_ShortSunday,						BUTTON_GADGET, NULL,
 37,  43,  66,  54, 1, NULL, Msg_ShortMonday,						BUTTON_GADGET, NULL,
 69,  43,  98,  54, 1, NULL, Msg_ShortTuesday,					BUTTON_GADGET, NULL,
101,  43, 130,  54, 1, NULL, Msg_ShortWednesday,				BUTTON_GADGET, NULL,
133,  43, 162,  54, 1, NULL, Msg_ShortThursday,					BUTTON_GADGET, NULL,
165,  43, 194,  54, 1, NULL, Msg_ShortFriday,						BUTTON_GADGET, NULL,
197,  43, 226,  54, 1, NULL, Msg_ShortSaturday,					BUTTON_GADGET, NULL,
 16,  60, 215,  73, 1, NULL, Msg_Sched2,								BUTTON_GADGET, NULL,
  5,  80,  87,  93, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
144,  80, 226,  93, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SetTime2_GR,
-1
};

/*****************************************************
 * SetTime4
 *****************************************************/

struct StringRecord SetTime4_1_SR = { 11, "            " };
struct StringRecord SetTime4_2_SR = { 11, "            " };

struct GadgetRecord SetTime4_GR[] =
{
  0,   0, 189,  98, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	188,  97, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  0,   0,   1,   1, 0, NULL, 0,													POSPREFS, NULL,
 47,   7,  66,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 72,   7,  91,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 97,   7, 116,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
122,   7, 141,  14, 1, NULL, 0,													BUTTON_GADGET, NULL,
 47,  30,  66,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 72,  30,  91,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 97,  30, 116,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
122,  30, 141,  37, 1, NULL, 0,													BUTTON_GADGET, NULL,
 42,  16, 146,  28, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime4_1_SR,
 47,  43,  66,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
 72,  43,  91,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
 97,  43, 116,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
122,  43, 141,  50, 1, NULL, 0,													BUTTON_GADGET, NULL,
 47,  66,  66,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
 72,  66,  91,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
 97,  66, 116,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
122,  66, 141,  73, 1, NULL, 0,													BUTTON_GADGET, NULL,
 42,  52, 146,  64, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&SetTime4_2_SR,
  5,  81,  87,  94, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
101,  81, 183,  94, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SetTime3_GR,
-1
};

/*****************************************************
 * DEMO
 *****************************************************/

struct GadgetRecord Demo_GR[] =
{
  0,   0, 590, 130, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	589, 129, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
254, 110, 336, 123, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  9, 106, 580, 106, 0, NULL, 0,													LO_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)SetTime4_GR,
-1
};

/*****************************************************
 * dBASE
 *****************************************************/

struct StringRecord DBase_first_SR = { 5, "      " };
struct StringRecord DBase_last_SR  = { 5, "      " };

struct GadgetRecord DBase_GR[] =
{
  0,   0, 311, 200, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	310, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 303,  10, 0, NULL, Msg_DB_1,									TEXT_REGION, NULL,
  7,  12, 303,  12, 0, NULL, 0,													LO_LINE, NULL,
  7, 177, 303, 177, 0, NULL, 0,													LO_LINE, NULL,
  7, 182,  89, 195, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
218, 182, 300, 195, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
201,  18, 260,  31, 1, NULL, Msg_DB_2,									INTEGER_GADGET, (struct GadgetRecord *)&DBase_first_SR,
201,  33, 260,  46, 1, NULL, Msg_DB_3,									INTEGER_GADGET, (struct GadgetRecord *)&DBase_last_SR,
201,  48, 260,  61, 2, NULL, Msg_DB_4,									LOBOX_REGION, NULL,
  7,  65, 301,  75, 0, NULL, Msg_DB_5,									TEXT_LEFT, NULL,
  8,  77, 279, 135, 2, NULL, 0,													BUTTON_GADGET, NULL,
284,  77, 301, 135, 0, NULL, 0,													HIBOX_REGION, NULL,
  7, 137, 301, 147, 0, NULL, Msg_DB_6,									TEXT_LEFT, NULL,
  8, 150,  24, 158, 1, NULL, Msg_DB_7,									RADIO_GADGET, NULL,
  8, 160,  24, 168, 1, NULL, Msg_DB_8,									RADIO_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Demo_GR,
-1
};

#if 0
/*****************************************************
 * FastEffect
 *****************************************************/

struct GadgetRecord FastEffect_GR[] =
{
  0,   0, 155,  94, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	154,  93, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
 65,  76, 147,  89, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,  76,  37,  89, 1, NULL, 0,													BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)DBase_GR,
-1
};
#endif

/*****************************************************
 * ClipAnim
 *****************************************************/

struct CycleRecord ClipAnim_FPS_CR		= { 30, 51, 6, NULL, Msg_FPS_Auto_60 };
struct CycleRecord ClipAnim_Loops_CR	= {  0, 31, 4, NULL, Msg_Infinite_30 };

struct GadgetRecord ClipAnim_GR[] =
{
  0,   0, 457,  54, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	456,  53, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  9,  36,  90,  49, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
366,  36, 447,  49, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
202,   3, 268,  16, 1, NULL, Msg_FramesPerSecond,				CYCLE_GADGET, (struct GadgetRecord *)&ClipAnim_FPS_CR,
202,  20, 268,  33, 1, NULL, Msg_Loops,									CYCLE_GADGET, (struct GadgetRecord *)&ClipAnim_Loops_CR,
282,   3, 297,  11, 1, NULL, Msg_PlayFromDisk,					CHECK_GADGET, NULL,
282,  15, 297,  23, 1, NULL, Msg_Transp,								CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)DBase_GR,
-1
};

/*****************************************************
 * XappInfo
 *****************************************************/

struct GadgetRecord XappInfo_GR[] =
{
  0,   0, 411, 140, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	410, 139, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7, 117, 401, 117, 0, NULL, 0,													LO_LINE, NULL,
164, 122, 245, 135, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  7, 104,  23, 112, 1, NULL, 0,													INVISIBLE_GADGET, NULL,
  8,   3, 115,  12, 0, NULL, Msg_Inter2,								TEXT_RIGHT, NULL,
  8,  14, 115,  23, 0, NULL, Msg_XI_2,									TEXT_RIGHT, NULL,
  8,  25, 115,  34, 0, NULL, Msg_XI_3,									TEXT_RIGHT, NULL,
  8,  36, 115,  45, 0, NULL, Msg_Descrip,								TEXT_RIGHT, NULL,
120,  11, 393,  11, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  22, 393,  22, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  33, 393,  33, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  44, 393,  44, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  55, 393,  55, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  66, 393,  66, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  77, 393,  77, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  88, 393,  88, 1, NULL, 0,													DOTTED_LINE, NULL,
120,  99, 393,  99, 1, NULL, 0,													DOTTED_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)ClipAnim_GR,
-1
};

/*****************************************************
 * VarPath
 *****************************************************/

struct GadgetRecord VarPath_GR[] =
{
  0,   0, 480, 120, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	479, 119, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,  97, 472,  97, 0, NULL, 0,													LO_LINE, NULL,
  8, 102,  90, 115, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
389, 102, 471, 115, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7,   1, 472,  10, 0, NULL, Msg_VarPath_1,							TEXT_REGION, NULL,
  7,  12, 472,  12, 0, NULL, 0,													LO_LINE, NULL,
147,  22, 367,  35, 1, NULL, Msg_VarPath_2,							HIBOX_REGION, NULL,
147,  43, 367,  56, 1, NULL, Msg_VarPath_3,							BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)XappInfo_GR,
-1
};

struct GadgetRecord VarPath_PopUp_GR[] =
{
  0,   0, 160,   0, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	  0,   0, 1, NULL, 0,													HIBOX_REGION, NULL,
  0,   0,   0,   0, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)VarPath_GR,
-1
};

/************************************************************************/
/**** From here the 'translator' works up to the first gadget record ****/
/************************************************************************/

struct GadgetRecord Entry_GR[] =
{
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)VarPath_PopUp_GR,
-1
};

/**********************************************************************************/
/**** DEBUG STUFF ******* THESE ARE NOT TRANSLATED ********************************/
/**********************************************************************************/

/*****************************************************
 * debug 
 *****************************************************/

struct StringRecord Debug_na1_SR 						= { 5, "      " };
struct StringRecord Debug_na2_SR 						= { 5, "      " };
struct StringRecord Debug_na3_SR 						= { 5, "      " };
struct StringRecord Debug_na4_SR 						= { 5, "      " };
struct StringRecord Debug_na5_SR 						= { 5, "      " };
struct StringRecord Debug_na6_SR 						= { 5, "      " };
struct StringRecord Debug_na7_SR 						= { 5, "      " };
struct StringRecord Debug_na8_SR 						= { 5, "      " };
struct StringRecord Debug_na9_SR 						= { 5, "      " };
struct StringRecord Debug_na10_SR 					= { 5, "      " };
struct StringRecord Debug_na11_SR 					= { 5, "      " };
struct StringRecord Debug_na12_SR 					= { 5, "      " };
struct StringRecord Debug_na13_SR 					= { 5, "      " };
struct StringRecord Debug_na14_SR 					= { 5, "      " };
struct StringRecord Debug_na15_SR 					= { 5, "      " };
struct StringRecord Debug_na16_SR 					= { 5, "      " };

struct StringRecord Debug_nodetype_SR				= { 2, "   " };

struct StringRecord Debug_startendmode_SR 	= { 2, "   " };

struct StringRecord Debug_effect_SR 				= { 4, "     " };

struct StringRecord Debug_miscflags_SR 			= { 4, "     " };

struct StringRecord Debug_daybits_SR 				= { 3, "    " };

struct StringRecord Debug_duration_SR 			= { 10, "           " };

struct StringRecord Debug_extradata1_SR 		= { 60, "                                                             " };
struct StringRecord Debug_extradata2_SR 		= { 60, "                                                             " };
struct StringRecord Debug_path_SR 					= { 60, "                                                             " };
struct StringRecord Debug_name_SR 					= { 60, "                                                             " };

struct StringRecord Debug_start_days_SR 		= { 10, "           " };
struct StringRecord Debug_start_minutes_SR 	= { 10, "           " };
struct StringRecord Debug_start_ticks_SR 		= { 10, "           " };
struct StringRecord Debug_start_frames_SR 	= { 10, "           " };

struct StringRecord Debug_end_days_SR 			= { 10, "           " };
struct StringRecord Debug_end_minutes_SR 		= { 10, "           " };
struct StringRecord Debug_end_ticks_SR 			= { 10, "           " };
struct StringRecord Debug_end_frames_SR 		= { 10, "           " };

__far struct GadgetRecord Debug_GR[] =
{
  0,   0, 640, 200, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 639, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	

560, 182, 630, 195, 0, "OK", 0,													BUTTON_GADGET, NULL,

 27,   2, 100,  14, 0, "0",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na1_SR,
 27,  16, 100,  28, 0, "1",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na2_SR,
 27,  30, 100,  42, 0, "2",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na3_SR,
 27,  44, 100,  56, 0, "3",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na4_SR,
 27,  58, 100,  70, 0, "4",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na5_SR,

133,   2, 206,  14, 0, "5",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na6_SR,
133,  16, 206,  28, 0, "6",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na7_SR,
133,  30, 206,  42, 0, "7",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na8_SR,
133,  44, 206,  56, 0, "8",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na9_SR,
133,  58, 206,  70, 0, "9",	0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na10_SR,

239,   2, 312,  14, 0, "10", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na11_SR,
239,  16, 312,  28, 0, "11", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na12_SR,
239,  30, 312,  42, 0, "12", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na13_SR,
239,  44, 312,  56, 0, "13", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na14_SR,
239,  58, 312,  70, 0, "14", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na15_SR,

345,   2, 418,  14, 0, "15", 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_na16_SR,

133,  75, 206,  87, 0, "Node type:", 0,									STRING_GADGET, (struct GadgetRecord *)&Debug_nodetype_SR,
133,  89, 206, 101, 0, "StartEnd:", 0,									STRING_GADGET, (struct GadgetRecord *)&Debug_startendmode_SR,
133, 103, 206, 115, 0, "Effect nr.", 0,									STRING_GADGET, (struct GadgetRecord *)&Debug_effect_SR,

345,  75, 418,  87, 0, "MiscFlags:", 0,									STRING_GADGET, (struct GadgetRecord *)&Debug_miscflags_SR,
345,  89, 418, 101, 0, "DayBits:", 0,										STRING_GADGET, (struct GadgetRecord *)&Debug_daybits_SR,
345, 103, 418, 115, 0, "Duration:", 0,									STRING_GADGET, (struct GadgetRecord *)&Debug_duration_SR,

 65, 122, 630, 134, 0, "ED:", 0,												STRING_GADGET, (struct GadgetRecord *)&Debug_extradata1_SR,
 65, 136, 630, 148, 0, "ED+50:", 0,											STRING_GADGET, (struct GadgetRecord *)&Debug_extradata2_SR,
 65, 150, 630, 161, 0, "Path:", 0,											STRING_GADGET, (struct GadgetRecord *)&Debug_path_SR,
 65, 164, 630, 176, 0, "Name:", 0,											STRING_GADGET, (struct GadgetRecord *)&Debug_name_SR,

478,   2, 551,  14, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_start_days_SR,
478,  16, 551,  28, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_start_minutes_SR,
478,  30, 551,  42, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_start_ticks_SR,
478,  44, 551,  56, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_start_frames_SR,
	
557,   2, 630,  14, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_end_days_SR,
557,  16, 630,  28, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_end_minutes_SR,
557,  30, 630,  42, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_end_ticks_SR,
557,  44, 630,  56, 0, NULL, 0,													STRING_GADGET, (struct GadgetRecord *)&Debug_end_frames_SR,

-1
};

/******** E O F ********/
