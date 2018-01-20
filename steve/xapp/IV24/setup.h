/******** IV-24 (fye) xapp gadgets ********/

struct CycleRecord FYE_CR	= { 0, 9, 23, NULL, Msg_IV_1_2 };

struct GadgetRecord FYE_GR[] =
{
  0,   0, 505, 157, 0, NULL,	0,												DIMENSIONS, NULL,
  0,	 0,	504, 156, 0, NULL,	0,												DBL_BORDER_REGION, NULL,
300,   3, 500,  12, 0, NULL, Msg_IV_1_1,								TEXT_RIGHT, NULL,			// xapp name
300,  14, 500,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,			// MP xapp
 10, 139,  92, 152, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,	// preview
325, 139, 407, 152, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
412, 139, 494, 152, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 10, 118, 494, 118, 0, NULL,	0,												LO_LINE, NULL,
 10,  29, 494,  84, 2, NULL,	0,												LOBOX_REGION, NULL,
 10,  87, 494, 115, 2, NULL,	0,												LOBOX_REGION, NULL,
 10,  12, 273,  25, 0, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&FYE_CR,
 16,  89, 487,  98, 2, NULL,	0,												TEXT_REGION, NULL,
 16, 101, 487, 110, 2, NULL,	0,												TEXT_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: PIP ****/

struct CycleRecord A1_Size_CR		= { 0, 4, 13, NULL, Msg_IV_1_4				};
struct CycleRecord A1_PIP_CR		= { 0, 3, 19, NULL, Msg_IV_1_6				};
struct CycleRecord A1_Mode_CR		= { 1, 6,  4, NULL, Msg_Numbers_1_40	};
struct CycleRecord A1_Speed_CR	= { 1, 6,  4, NULL, Msg_Numbers_1_40	};

struct StringRecord A1_X_SR = { 3, "      " };
struct StringRecord A1_Y_SR = { 3, "      " };

struct GadgetRecord A1_GR[] =
{
 92,  33, 249,  46, 1, NULL, Msg_IV_1_3,								CYCLE_GADGET, (struct GadgetRecord *)&A1_Size_CR,
 92,  49, 294,  62, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A1_PIP_CR,
 92,  65, 167,  78, 1, NULL, Msg_IV_1_9,								CYCLE_GADGET, (struct GadgetRecord *)&A1_Mode_CR,
392,  65, 453,  78, 1, NULL, Msg_IV_1_10,								CYCLE_GADGET, (struct GadgetRecord *)&A1_Speed_CR,
256,  36, 272,  44, 1, NULL, Msg_IV_1_5,								CHECK_GADGET, NULL,
325,  49, 374,  62, 1, NULL, Msg_IV_1_7,								INTEGER_GADGET, (struct GadgetRecord *)&A1_X_SR,
404,  49, 453,  62, 1, NULL, Msg_IV_1_8,								INTEGER_GADGET, (struct GadgetRecord *)&A1_Y_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Full Screen Video ****/

struct CycleRecord A2_Mode_CR = { 0, 4, 4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord A2_GR[] =
{
102,  33, 177,  46, 1, NULL, Msg_IV_1_9,								CYCLE_GADGET, (struct GadgetRecord *)&A2_Mode_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Framebuffer ****/

struct StringRecord A3_Delay_SR	= { 3, "    " };

struct GadgetRecord A3_GR[] =
{
102,  33, 395,  46, 1, NULL, Msg_IV_2_1,								BUTTON_GADGET, NULL,
102,  53, 141,  66, 1, NULL, Msg_IV_2_2,								INTEGER_GADGET, (struct GadgetRecord *)&A3_Delay_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Control panel ****/
 
struct CycleRecord A4_COut1_CR	= { 0, 3,  8, NULL, Msg_IV_3_3 };
struct CycleRecord A4_COut2_CR	= { 0, 2,  7, NULL, Msg_IV_3_4 };
struct CycleRecord A4_COut3_CR	= { 0, 2,  7, NULL, Msg_IV_3_5 };

struct CycleRecord A4_ROut1_CR	= { 0, 3,  8, NULL, Msg_IV_3_3 };
struct CycleRecord A4_ROut2_CR	= { 0, 2,  7, NULL, Msg_IV_3_4 };
struct CycleRecord A4_ROut3_CR	= { 0, 2,  7, NULL, Msg_IV_3_5 };

struct CycleRecord A4_Scan_CR		= { 0, 2, 13, NULL, Msg_IV_3_6 };
struct CycleRecord A4_Cols_CR		= { 0, 2, 11, NULL, Msg_IV_3_7 };

struct GadgetRecord A4_GR[] =
{
193,  33, 298,  46, 1, NULL, Msg_IV_3_1,								CYCLE_GADGET, (struct GadgetRecord *)&A4_COut1_CR,
306,  33, 390,  46, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_COut2_CR,
398,  33, 482,  46, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_COut3_CR,
193,  49, 298,  62, 1, NULL, Msg_IV_3_2,								CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut1_CR,
306,  49, 390,  62, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut2_CR,
398,  49, 482,  62, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_ROut3_CR,
 24,  65, 176,  78, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_Scan_CR,
193,  65, 345,  78, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A4_Cols_CR,
370,  67, 386,  75, 1, NULL, Msg_IV_3_8,								CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** ACTION: Composite keyer ****/

struct StringRecord A5_From_SR	= { 3, "      " };
struct StringRecord A5_To_SR		= { 3, "      " };

struct CycleRecord A5_Fade_CR = { 0, 3, 13, NULL, Msg_IV_4_1 };

struct GadgetRecord A5_GR[] =
{
 44,  33, 242,  46, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&A5_Fade_CR,
303,  33, 353,  46, 1, NULL, Msg_IV_4_2,								INTEGER_GADGET, (struct GadgetRecord *)&A5_From_SR,
393,  33, 442,  46, 1, NULL, Msg_IV_4_3,								INTEGER_GADGET, (struct GadgetRecord *)&A5_To_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
