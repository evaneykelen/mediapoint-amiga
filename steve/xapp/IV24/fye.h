/******** IV-24 (fye) xapp gadgets ********/

#define STP (char *)	// scalar to pointer

struct CycleRecord FYE_CR	= { 0, 9, 23, STP Msg_IV_1_2 };

struct GadgetRecord FYE_GR[] =
{
  0,   0, 505, 157, NULL,													DIMENSIONS, NULL,
  0,	 0,	504, 156, NULL,													DBL_BORDER_REGION, NULL,
300,   3, 500,  12, STP Msg_IV_1_1,								TEXT_RIGHT, NULL,			// xapp name
300,  14, 500,  23, STP Msg_X_1,									TEXT_RIGHT, NULL,			// MP xapp
 10, 139,  92, 152, STP Msg_X_2,									BUTTON_GADGET, NULL,	// preview
325, 139, 407, 152, STP Msg_OK,										BUTTON_GADGET, NULL,
412, 139, 494, 152, STP Msg_Cancel,								BUTTON_GADGET, NULL,
 10, 118, 494, 118, NULL,													LO_LINE, NULL,
 10,  29, 494,  84, NULL,													LOBOX_REGION, NULL,
 10,  87, 494, 115, NULL,													LOBOX_REGION, NULL,
 10,  12, 273,  25, NULL,													CYCLE_GADGET, (struct GadgetRecord *)&FYE_CR,
 16,  89, 487,  98, NULL,													TEXT_REGION, NULL,
 16, 101, 487, 110, NULL,													TEXT_REGION, NULL,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: PIP ****/

struct CycleRecord A1_Size_CR		= { 0, 4, 13, STP Msg_IV_1_4				};
struct CycleRecord A1_PIP_CR		= { 0, 3, 19, STP Msg_IV_1_6				};
struct CycleRecord A1_Mode_CR		= { 1, 6,  4, STP Msg_Numbers_1_40	};
struct CycleRecord A1_Speed_CR	= { 1, 6,  4, STP Msg_Numbers_1_40	};

struct StringRecord A1_X_SR = { 3, "      " };
struct StringRecord A1_Y_SR = { 3, "      " };

struct GadgetRecord A1_GR[] =
{
 92,  33, 249,  46, STP Msg_IV_1_3,					CYCLE_GADGET, (struct GadgetRecord *)&A1_Size_CR,
 92,  49, 294,  62, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A1_PIP_CR,
 92,  65, 167,  78, STP Msg_IV_1_9,					CYCLE_GADGET, (struct GadgetRecord *)&A1_Mode_CR,
392,  65, 453,  78, STP Msg_IV_1_10,				CYCLE_GADGET, (struct GadgetRecord *)&A1_Speed_CR,
256,  36, 272,  44, STP Msg_IV_1_5,					CHECK_GADGET, NULL,
325,  49, 374,  62, STP Msg_IV_1_7,					INTEGER_GADGET, (struct GadgetRecord *)&A1_X_SR,
404,  49, 453,  62, STP Msg_IV_1_8,					INTEGER_GADGET, (struct GadgetRecord *)&A1_Y_SR,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Full Screen Video ****/

struct CycleRecord A2_Mode_CR = { 0, 4, 4, STP Msg_Numbers_1_40 };

struct GadgetRecord A2_GR[] =
{
102,  33, 177,  46, STP Msg_IV_1_3,					CYCLE_GADGET, (struct GadgetRecord *)&A2_Mode_CR,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Framebuffer ****/

struct StringRecord A3_Delay_SR	= { 3, "    " };

struct GadgetRecord A3_GR[] =
{
102,  33, 395,  46, STP Msg_IV_2_1,					BUTTON_GADGET, NULL,
102,  53, 141,  66, STP Msg_IV_2_2,					INTEGER_GADGET, (struct GadgetRecord *)&A3_Delay_SR,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Control panel ****/
 
struct CycleRecord A4_COut1_CR	= { 0, 3,  8, STP Msg_IV_3_3 };
struct CycleRecord A4_COut2_CR	= { 0, 2,  7, STP Msg_IV_3_4 };
struct CycleRecord A4_COut3_CR	= { 0, 2,  7, STP Msg_IV_3_5 };

struct CycleRecord A4_ROut1_CR	= { 0, 3,  8, STP Msg_IV_3_3 };
struct CycleRecord A4_ROut2_CR	= { 0, 2,  7, STP Msg_IV_3_4 };
struct CycleRecord A4_ROut3_CR	= { 0, 2,  7, STP Msg_IV_3_5 };

struct CycleRecord A4_Scan_CR		= { 0, 2, 13, STP Msg_IV_3_6 };
struct CycleRecord A4_Cols_CR		= { 0, 2, 11, STP Msg_IV_3_7 };

struct GadgetRecord A4_GR[] =
{
193,  33, 298,  46, STP Msg_IV_3_1,					CYCLE_GADGET, (struct GadgetRecord *)&A4_COut1_CR,
306,  33, 390,  46, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_COut2_CR,
398,  33, 482,  46, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_COut3_CR,
193,  49, 298,  62, STP Msg_IV_3_2,					CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut1_CR,
306,  49, 390,  62, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut2_CR,
398,  49, 482,  62, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut3_CR,
 24,  65, 176,  78, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_Scan_CR,
193,  65, 345,  78, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A4_Cols_CR,
370,  67, 386,  75, STP Msg_IV_3_8,					CHECK_GADGET, NULL,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Composite keyer ****/

struct StringRecord A5_From_SR	= { 3, "      " };
struct StringRecord A5_To_SR		= { 3, "      " };

struct CycleRecord A5_Fade_CR = { 0, 3, 13, STP Msg_IV_4_1 };

struct GadgetRecord A5_GR[] =
{
 44,  33, 242,  46, NULL,										CYCLE_GADGET, (struct GadgetRecord *)&A5_Fade_CR,
303,  33, 353,  46, STP Msg_IV_4_2,					INTEGER_GADGET, (struct GadgetRecord *)&A5_From_SR,
393,  33, 442,  46, STP Msg_IV_4_3,					INTEGER_GADGET, (struct GadgetRecord *)&A5_To_SR,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/*
TEXT mode_id_a1[] = "Full screen video";
TEXT mode_id_b1[] = "Amiga in PIP";

TEXT mode_id_a2[] = "Full screen Amiga";
TEXT mode_id_b2[] = "Video in PIP";

TEXT mode_id_a3[] = "Keyed video and Amiga";
TEXT mode_id_b3[] = "Amiga in PIP";

TEXT mode_id_a4[] = "Negative keyed video and Amiga";
TEXT mode_id_b4[] = "Amiga in PIP";

TEXT mode_id_a5[] = "Full screen Amiga";
TEXT mode_id_b5[] = "Keyed video and Amiga in PIP";

TEXT mode_id_a6[] = "Full screen Amiga";
TEXT mode_id_b6[] = "Negative keyed video and Amiga in PIP";
*/

UWORD WaitPointer[] =
{
0x0000, 0x0000,
0x0400, 0x07C0,
0x0000, 0x07C0,
0x0100, 0x0380,
0x0000, 0x07E0,
0x07C0, 0x1FF8,
0x1FF0, 0x3FEC,
0x3FF8, 0x7FDE,
0x3FF8, 0x7FBE,
0x7FFC, 0xFF7F,
0x7EFC, 0xFFFF,
0x7FFC, 0xFFFF,
0x3FF8, 0x7FFE,
0x3FF8, 0x7FFE,
0x1FF0, 0x3FFC,
0x07C0, 0x1FF8,
0x0000, 0x07E0,
0x0000, 0x0000
};

struct StringRecord FR_SR = { 40, "                                         " };

struct GadgetRecord FileRequester_GR[] =
{
  0,   0, 317, 134, NULL,										DIMENSIONS, NULL,
  6,  13, 235,  26, NULL,										SPECIAL_STRING_GADGET, (struct GadgetRecord *)&FR_SR,
  6,  28, 215, 131, NULL,										BUTTON_GADGET, NULL,
241,  13, 311,  26, "Parent",								BUTTON_GADGET, NULL,
241,  63, 311,  76, "Disks",								BUTTON_GADGET, NULL,
241,  79, 311,  92, "Assigns",							BUTTON_GADGET, NULL,
241, 100, 311, 113, "Cancel",								BUTTON_GADGET, NULL,
241, 118, 311, 131, "Open",									BUTTON_GADGET, NULL,
218,  28, 235, 131, NULL,										HIBOX_REGION, NULL,
  0,	 0,	316, 133, NULL,										DBL_BORDER_REGION, NULL,
241,  96, 311,  96, NULL,										LO_LINE, NULL,
254,  36, 276,  47, "ç",										BUTTON_GADGET, NULL,	/* select all */
281,  36, 303,  47, "é",										BUTTON_GADGET, NULL,	/* thumbnails */
  5,   2, 312,  11, NULL,										TEXT_REGION, NULL,		/* 'Select a file' */
-1
};

/******** E O F ********/
