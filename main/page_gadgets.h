/******** PAGE_GADGETS.H ********/

/*****************************************************
 * window definition
 *****************************************************/

struct StringRecord WDef_X_SR 			= { 3, "    " };
struct StringRecord WDef_Y_SR 			= { 3, "    " };
struct StringRecord WDef_Width_SR		= { 3, "    " };
struct StringRecord WDef_Height_SR 	= { 3, "    " };

TEXT WDef_FillList[] = { TL_FillTypes } ;

TEXT WDef_BWidthList[] = {
 "1\0  2\0  3\0  4\0  5\0  6\0  7\0  8\0  9\0  10\0 11\0 12\0 13\0 14\0 15\0 \
16\0 17\0 18\0 19\0 20\0 21\0 22\0 23\0 24\0 25\0 26\0 27\0 28\0 29\0 30\0 \
31\0 32\0 33\0 34\0 35\0 36\0 37\0 38\0 39\0 40\0 41\0 42\0 43\0 44\0 45\0 \
46\0 47\0 48\0 49\0 50\0 " };

struct CycleRecord WDef_Fill_CR 	= { 0,  3, 8, (STRPTR)&WDef_FillList };
struct CycleRecord WDef_BWidth_CR	= { 0, 30, 4, (STRPTR)&WDef_BWidthList };

struct GadgetRecord WDef_GR[] =
{
  0,   0, 320, 112, NULL,                   DIMENSIONS, NULL,
  0,	 0,	319, 111, NULL,										DBL_BORDER_REGION, NULL,
 42,  79,  81,  92, TL_X,										INTEGER_GADGET, (struct GadgetRecord *)&WDef_X_SR, 
 42,  94,  81, 107, TL_Y,										INTEGER_GADGET, (struct GadgetRecord *)&WDef_Y_SR, 
160,  79, 199,  92, TL_Width,								INTEGER_GADGET, (struct GadgetRecord *)&WDef_Width_SR, 
160,  94, 199, 107, TL_Height,							INTEGER_GADGET, (struct GadgetRecord *)&WDef_Height_SR, 
130,  19, 184,  32, TL_BorderWidth,					CYCLE_GADGET, (struct GadgetRecord *)&WDef_BWidth_CR,
 86,  39, 184,  52, TL_FillType,						CYCLE_GADGET, (struct GadgetRecord *)&WDef_Fill_CR,
229,  19, 246,  27, NULL,										CHECK_GADGET, NULL,
265,  36, 282,  45, NULL,										CHECK_GADGET, NULL,
229,  54, 246,  62, NULL,										CHECK_GADGET, NULL,
193,  36, 210,  45, NULL,										CHECK_GADGET, NULL,
217,  28, 258,  35, NULL,										0, NULL,
256,  33, 263,  48, NULL,										0, NULL,
217,  46, 258,  53, NULL,										0, NULL,
212,  33, 219,  48, NULL,										0, NULL,
217,  33, 258,  48, NULL,										0, NULL,
230,  78, 312,  91, TL_Cancel,							BUTTON_GADGET, NULL,
230,  94, 312, 107, TL_OK,									BUTTON_GADGET, NULL,
  6,  12, 312,  12, NULL,										LO_LINE, NULL,
  6,   2, 312,  11, NULL,										TEXT_REGION, NULL,
  5,  67, 172,  76, NULL,										TEXT_REGION, NULL,
-1
};

/*****************************************************
 * color adjust
 *****************************************************/

struct GadgetRecord ColorAdjust_GR[] =
{
  0,   0, 640,  78, NULL,                   DIMENSIONS, NULL,
  0,	 0,	639,  77, NULL,										DBL_BORDER_REGION, NULL,
  4,   2,  21,  11, "àá",										BUTTON_GADGET, NULL,
 28,  33,  40,  41, TL_R,										TEXT_REGION, NULL,
 28,  49,  40,  57, TL_G,										TEXT_REGION, NULL,
 28,  65,  40,  73, TL_B,										TEXT_REGION, NULL,
 42,  30, 316,  43, NULL,										BUTTON_GADGET, NULL,
 42,  46, 316,  59, NULL,										BUTTON_GADGET, NULL,
 42,  62, 316,  75, NULL,										BUTTON_GADGET, NULL,
320,  30, 339,  75, NULL,										BUTTON_GADGET, NULL,
345,  30, 382,  75, NULL,										LOBOX_REGION, NULL,
347,  33, 380,  41, NULL,										TEXT_REGION, NULL,
347,  49, 380,  57, NULL,										TEXT_REGION, NULL,
347,  65, 380,  73, NULL,										TEXT_REGION, NULL,
387,  30, 453,  43, TL_Spread,							BUTTON_GADGET, NULL,
457,  30, 516,  43, TL_Copy,								BUTTON_GADGET, NULL,
387,  46, 453,  59, TL_Swap,								BUTTON_GADGET, NULL,
457,  46, 516,  59, TL_Undo,								BUTTON_GADGET, NULL,
387,  62, 516,  75, TL_Extra_color,					BUTTON_GADGET, NULL,
520,  30, 635,  43, TL_Load_palette,				BUTTON_GADGET, NULL,
520,  46, 635,  59, TL_Save_palette,				BUTTON_GADGET, NULL,
520,  62, 635,  75, TL_Harmonize,						BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * desk accessories
 *****************************************************/

struct StringRecord DANames_SR = { SIZE_FILENAME,
										"                                                   " };

struct GadgetRecord DANames_GR[] =
{
  0,   0, 317,  79, NULL,										DIMENSIONS, NULL,
  0,	 0,	316,  78, NULL,										DBL_BORDER_REGION, NULL,
139,   3, 258,  16, TL_Menu_name,						STRING_GADGET, (struct GadgetRecord *)&DANames_SR,
 89,  19, 228,  33, TL_Select_program,			BUTTON_GADGET, NULL,
  4,  36, 312,  49, NULL,										LOBOX_REGION, NULL,
  4,  62,  87,  76, TL_OK,									BUTTON_GADGET, NULL,
229,  62, 312,  76, TL_Cancel,							BUTTON_GADGET, NULL,
142,  62, 225,  76, TL_Remove,							BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * font selector
 *****************************************************/

struct StringRecord FontSelect_SR = { 3, "    " };

struct GadgetRecord FontSelect_GR[] =
{
  0,   0, 248,  80, NULL,										DIMENSIONS, NULL,
/*  4,   2, 110,  15, TL_Select_path,					BUTTON_GADGET, NULL, */
  4,   2,  96,  15, TL_Show,								BUTTON_GADGET, NULL,
182,   2, 221,  15, NULL,										INTEGER_GADGET, (struct GadgetRecord *)&FontSelect_SR,
  4,  19, 153,  76, NULL,										BUTTON_GADGET, NULL,
156,  19, 174,  76, NULL,										HIBOX_REGION, NULL,
183,  19, 220,  76, NULL,										BUTTON_GADGET, NULL,
223,  19, 241,  76, NULL,										HIBOX_REGION, NULL,
-1
};

/*****************************************************
 * font sample
 *****************************************************/

struct GadgetRecord FontSample_GR[] =
{
  0,   0, 317,  79, NULL,										DIMENSIONS, NULL,
-1
};

/*****************************************************
 * style selector
 *****************************************************/
 
struct CycleRecord StyleSelect_ShadowDepth_CR	= { 0, 30, 4, (STRPTR)&WDef_BWidthList };

struct GadgetRecord StyleSelect_GR[] =
{
  0,   0, 296,  61, NULL,										DIMENSIONS, NULL,
  4,   2,  63,  15, TL_Plain,								BUTTON_GADGET, NULL,
	4,  19,  23,  29, TL_Bold,								CHECK_GADGET, NULL,
	4,  33,  23,  43, TL_Italic,							CHECK_GADGET, NULL,
	4,  47,  23,  57, TL_Underline,						CHECK_GADGET, NULL,
109,  47, 128,  57, TL_Shadow,							CHECK_GADGET, NULL,
207,  47, 226,  57, TL_Outline,							CHECK_GADGET, NULL,
 85,   2, 104,  12, NULL,										CHECK_GADGET, NULL,
107,   2, 126,  12, NULL,										CHECK_GADGET, NULL,
129,   2, 148,  12, NULL,										CHECK_GADGET, NULL,
129,  14, 148,  24, NULL,										CHECK_GADGET, NULL,
129,  26, 148,  36, NULL,										CHECK_GADGET, NULL,
107,  26, 126,  36, NULL,										CHECK_GADGET, NULL,
 85,  26, 104,  36, NULL,										CHECK_GADGET, NULL,
 85,  14, 104,  24, NULL,										CHECK_GADGET, NULL,
230,   2, 289,  15, TL_Depth,								CYCLE_GADGET, (struct GadgetRecord *)&StyleSelect_ShadowDepth_CR,
154,  17, 170,  25, TL_Cast,								RADIO_GADGET, NULL,
154,  28, 170,  36, TL_Solid,								RADIO_GADGET, NULL,
230,  23, 289,  36, TL_Show,								BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * Open file requester
 *****************************************************/
 
struct GadgetRecord FileRequester_GR[] =
{
  0,   0, 317, 134, NULL,										DIMENSIONS, NULL,
  6,  13, 235,  26, NULL,										LOBOX_REGION, NULL,
  6,  28, 215, 131, NULL,										BUTTON_GADGET, NULL,
241,  13, 311,  26, TL_Parent,							BUTTON_GADGET, NULL,
241,  63, 311,  76, TL_Disks,								BUTTON_GADGET, NULL,
241,  79, 311,  92, TL_Assigns,							BUTTON_GADGET, NULL,
241, 100, 311, 113, TL_Cancel,							BUTTON_GADGET, NULL,
241, 118, 311, 131, TL_Open,								BUTTON_GADGET, NULL,
218,  28, 235, 131, NULL,										HIBOX_REGION, NULL,
  0,	 0,	316, 133, NULL,										DBL_BORDER_REGION, NULL,
241,  96, 311,  96, NULL,										LO_LINE, NULL,
254,  36, 276,  47, "ç",										BUTTON_GADGET, NULL,	/* select all */
281,  36, 303,  47, "é",										BUTTON_GADGET, NULL,	/* thumbnails */
  5,   2, 312,  11, NULL,										TEXT_REGION, NULL,		/* 'Select a file' */
-1
};

/*****************************************************
 * Save file requester
 *****************************************************/

struct StringRecord SaveFile_SR = { 30, "                               " };

struct GadgetRecord SaveFile_GR[] =
{
  0,   0, 317, 164, NULL,										DIMENSIONS, NULL,
  6,  13, 235,  26, NULL,										LOBOX_REGION, NULL,
  6,  28, 215, 131, NULL,										BUTTON_GADGET, NULL,
241,  13, 311,  26, TL_Parent,							BUTTON_GADGET, NULL,
241,  93, 311, 106, TL_Disks,								BUTTON_GADGET, NULL,
241, 109, 311, 122, TL_Assigns,							BUTTON_GADGET, NULL,
241, 130, 311, 143, TL_Cancel,							BUTTON_GADGET, NULL,
241, 148, 311, 161, TL_Save,								BUTTON_GADGET, NULL,
218,  28, 235, 131, NULL,										HIBOX_REGION, NULL,
  0,	 0,	316, 163, NULL,										DBL_BORDER_REGION, NULL,
241, 126, 311, 126, NULL,										LO_LINE, NULL,
  6, 148, 235, 161, NULL,										STRING_GADGET, (struct GadgetRecord *)&SaveFile_SR,
  5, 135, 235, 144, NULL,										TEXT_REGION, NULL,		/* 'Select a file' */
-1
};

/*****************************************************
 * format requester
 *****************************************************/

TEXT Format_FormatList[]		= { TL_FormatTypes };
TEXT Format_OverscanList[]	= { TL_OverscanTypes };
TEXT Format_ColorList1[]		= { TL_ColorTypes };
TEXT Format_ColorList2[]		= { TL_NewColorTypes };
TEXT Format_ModeList[]			= { TL_ModeTypes };
TEXT Format_SpecialList[]		= { TL_SpecialTypes };
 
struct CycleRecord Format_Format_CR		= { 3,  8, 11, (STRPTR)&Format_FormatList };
struct CycleRecord Format_Overscan_CR	= { 0,  3,  4, (STRPTR)&Format_OverscanList };
struct CycleRecord Format_Color_CR1		= { 3,  7,  5, (STRPTR)&Format_ColorList1 };
struct CycleRecord Format_Color_CR2		= { 3, 11,  5, (STRPTR)&Format_ColorList2 };
struct CycleRecord Format_Mode_CR			= { 0,  2,  5, (STRPTR)&Format_ModeList };
struct CycleRecord Format_Special_CR	= { 0,  6,  7, (STRPTR)&Format_SpecialList };

struct GadgetRecord FormatRequester_GR[] =
{
  0,   0, 225, 119, NULL,										DIMENSIONS, NULL,
 10,  14, 127,  27, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Format_Format_CR,
134,  14, 214,  27, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Format_Overscan_CR,
 10,  45, 127,  58, NULL,										CYCLE_GADGET, NULL,
134,  45, 214,  58, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Format_Mode_CR,
 10, 101, 104, 114, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Format_Special_CR,
 10,  65, 214,  78, NULL,										LOBOX_REGION, NULL,
132,  85, 214,  98, TL_Cancel,							BUTTON_GADGET, NULL,
132, 101, 214, 114, TL_OK,									BUTTON_GADGET, NULL,
  0,	 0,	224, 118, NULL,										DBL_BORDER_REGION, NULL,
-1
};

/*****************************************************
 * don't save, cancel save requester
 *****************************************************/

struct GadgetRecord DontSaveRequester_GR[] =
{
  0,   0, 300, 100, NULL,										DIMENSIONS, NULL,
 10,  80, 108,  94, TL_DontSave,						BUTTON_GADGET, NULL,
142,  80, 213,  94, TL_Cancel,							BUTTON_GADGET, NULL,
218,  80, 289,  94, TL_Save,								BUTTON_GADGET, NULL,
  0,	 0,	299,  99, NULL,										DBL_BORDER_REGION, NULL,
-1
};

/*****************************************************
 * about
 *****************************************************/

struct GadgetRecord About_GR[] =
{
  0,   0, 500, 190, NULL,										DIMENSIONS, NULL,
  0,	 0,	499, 189, NULL,										DBL_BORDER_REGION, NULL,
340,   4, 485,  16, NULL,										TEXT_REGION, NULL,	/* version */
424,  21, 485,  33, TL_More,								BUTTON_GADGET, NULL, 
 15,  97, 240, 108, NULL,										TEXT_REGION, NULL,	/* left 1 */
 15, 112, 240, 123, NULL,										TEXT_REGION, NULL,	/* left 2 */
 15, 127, 240, 138, NULL,										TEXT_REGION, NULL,	/* left 3 */
281,  97, 485, 108, NULL,										TEXT_REGION, NULL,	/* right 1 */
281, 112, 485, 123, NULL,										TEXT_REGION, NULL,	/* right 2 */
281, 127, 485, 138, NULL,										TEXT_REGION, NULL,	/* right 3 */
 15, 142, 485, 153, NULL,										TEXT_REGION, NULL,	/* bottom 1 */
 15, 157, 485, 168, NULL,										TEXT_REGION, NULL,	/* bottom 2 */
 15, 172, 485, 183, NULL,										TEXT_REGION, NULL,	/* bottom 3 */
-1
};

/*****************************************************
 * thumbnail
 *****************************************************/

struct GadgetRecord Thumbnail_GR[] =
{
  5, 186,  88, 199, THUMBTEXT2,							BUTTON_GADGET, NULL,	/* prev */
 96, 186, 179, 199, TL_NextPage,						BUTTON_GADGET, NULL,	/* next */
460, 186,	543, 199, TL_Cancel,							BUTTON_GADGET, NULL,
551, 186,	634, 199, TL_OK,									BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * import requester
 *****************************************************/

struct GadgetRecord ImportType_GR[] =
{
  0,   0, 286,  99, NULL,										DIMENSIONS, NULL,
  0,	 0,	285,  98, NULL,										DBL_BORDER_REGION, NULL,
  4,   2, 282,  15, TL_SelectImportType,		TEXT_REGION, NULL,
 11,  26,  27,  34, TL_Picture,							RADIO_GADGET, NULL,
 11,  43,  27,  51, TL_Screen,							RADIO_GADGET, NULL,
 11,  60,  27,  68, TL_Text,								RADIO_GADGET, NULL,
 11,  82,  93,  95, TL_OK,									BUTTON_GADGET, NULL,
193,  82, 275,  95, TL_Cancel,							BUTTON_GADGET, NULL,
136,  25, 155,  35, TL_ResizeToFit,					CHECK_GADGET, NULL,
121,  25, 121,  68, NULL,										LO_LINE, NULL,
122,  25, 122,  68, NULL,										LO_LINE, NULL,
136,  55, 230,  68, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Format_Special_CR,
136,  38, 155,  48, TL_Remap,								CHECK_GADGET, NULL,
  6,  14, 279,  14, NULL,										LO_LINE, NULL,
-1
};

/*****************************************************
 * warning window
 *****************************************************/

struct GadgetRecord Warning_GR[] =
{
  0,   0, 320,  95, NULL,										DIMENSIONS, NULL,
  0,	 0,	319,  94, NULL,										DBL_BORDER_REGION, NULL,
-1
};

/*****************************************************
 * warning window, left button
 *****************************************************/

struct GadgetRecord Left_button_GR[] =
{
  7,  77,  89,  90, NULL,										BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 *  warning window, right button
 *****************************************************/

struct GadgetRecord Right_button_GR[] =
{
230,  77, 312,  90, NULL,										BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * specials
 *****************************************************/

struct GadgetRecord Specials_GR[] =
{
  0,   0, 320, 158, NULL,										DIMENSIONS, NULL,
  0,	 0,	319, 157, NULL,										DBL_BORDER_REGION, NULL,
  4,   2, 315,  11, TL_SPECIALS8,						TEXT_REGION, NULL,
  4,  13, 315,  13, NULL,										LO_LINE, NULL,
  9,  20,  28,  30, TL_SPECIALS1,						CHECK_GADGET, NULL,
  9,  35,  28,  45, TL_SPECIALS2,						CHECK_GADGET, NULL,
  9,  50,  28,  60, TL_SPECIALS3,						CHECK_GADGET, NULL,
  9,  65,  28,  75, TL_SPECIALS4,						CHECK_GADGET, NULL,
  9,  80,  28,  90, TL_SPECIALS5,						CHECK_GADGET, NULL,
  9,  95,  28, 105, TL_SPECIALS6,						CHECK_GADGET, NULL,
  9, 110,  28, 120, TL_SPECIALS7,						CHECK_GADGET, NULL,
  9, 141,  90, 152, TL_OK,									BUTTON_GADGET, NULL,
229, 141, 310, 152, TL_Cancel,							BUTTON_GADGET, NULL,
-1
};

/*****************************************************
 * distributor
 *****************************************************/

struct CycleRecord Distri_1_CR = { 0, 2, 24, TL_Distribute };
struct CycleRecord Distri_2_CR = { 0, 2, 14, TL_Center };

struct GadgetRecord Distributor_GR[] =
{
  0,   0, 320, 126, NULL,										DIMENSIONS, NULL,
  0,	 0,	319, 125, NULL,										DBL_BORDER_REGION, NULL,
  8,   1, 311,  10, TL_Distri1,							TEXT_REGION, NULL,
 10,  16,  26,  24, TL_Distri2,							RADIO_GADGET, NULL,
 10,  47,  26,  55, TL_Distri3,							RADIO_GADGET, NULL,
 38,  27, 297,  40, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Distri_1_CR,
 38,  58, 297,  72, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&Distri_2_CR,
 38,  81, 160, 121, NULL,										LOBOX_REGION, NULL,
240,  90, 310, 103, TL_Cancel,							BUTTON_GADGET, NULL,
240, 108, 310, 121, TL_OK,									BUTTON_GADGET, NULL,
  8,  12, 311,  12, NULL,										LO_LINE, NULL,
-1
};

/*****************************************************
 * duplicator
 *****************************************************/

struct StringRecord Dupli_times_SR = { 2, "   " };
struct StringRecord Dupli_x_SR = { 3, "    " };
struct StringRecord Dupli_y_SR = { 3, "    " };

struct GadgetRecord Duplicator_GR[] =
{
  0,   0, 320,  74, NULL,										DIMENSIONS, NULL,
  0,	 0,	319,  73, NULL,										DBL_BORDER_REGION, NULL,
  8,   1, 311,  10, TL_Dupli1,							TEXT_REGION, NULL,
 91,  19, 121,  30, TL_Dupli2,							INTEGER_GADGET, (struct GadgetRecord *)&Dupli_times_SR,
127,  20, 204,  30, TL_Dupli3,							TEXT_LEFT, NULL,
 56,  37, 102,  48, TL_Dupli4,							INTEGER_GADGET, (struct GadgetRecord *)&Dupli_x_SR,
 56,  52, 102,  63, TL_Dupli4,							INTEGER_GADGET, (struct GadgetRecord *)&Dupli_y_SR,
107,  38, 192,  48, TL_Dupli5,							TEXT_LEFT, NULL,
107,  52, 192,  62, TL_Dupli6,							TEXT_LEFT, NULL,
240,  38, 310,  51, TL_Cancel,							BUTTON_GADGET, NULL,
240,  56, 310,  69, TL_OK,									BUTTON_GADGET, NULL,
  8,  12, 311,  12, NULL,										LO_LINE, NULL,
-1
};

/******** E O F ********/
