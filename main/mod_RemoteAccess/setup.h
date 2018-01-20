UWORD chip mypattern1[] = { 0x5555, 0xaaaa };

struct GadgetRecord DiskReq_GR[] =
{
  7,  77,  89,  90, 0, NULL, Msg_OK,			BUTTON_GADGET, NULL,
230,  77, 312,  90, 0, NULL, Msg_Cancel,	BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA1_GR[] =
{
  0,   0, 640, 200, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 639, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 628,  12, 0, NULL, Msg_RA_Remote_Access,			TEXT_REGION, NULL,
 12,  15, 628,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12, 135, 628, 135, 0, NULL, 0,													LO_LINE, NULL,								
 12, 175, 628, 175, 0, NULL, 0,													LO_LINE, NULL,								
 12, 140, 187, 153, 1, NULL, Msg_RA_Open_Session,				BUTTON_GADGET, NULL, 
 12, 157, 187, 170, 1, NULL, Msg_RA_Save_Session,				BUTTON_GADGET, NULL, 
194, 140, 456, 153, 2, NULL, 0,													LOBOX_REGION, NULL, 
503, 140, 628, 153, 1, NULL, Msg_RA_Options,						BUTTON_GADGET, NULL, 
 12, 181, 132, 194, 0, NULL, Msg_RA_Upload,							BUTTON_GADGET, NULL, 
546, 181, 628, 194, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL, 
  5,  17, 630, 132, 2, NULL, 0,													INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA1_A_GR[] =
{
 14,  19, 150,  28, 0, NULL, Msg_RA_Scripts,						TEXT_LEFT, NULL,
150,  19, 291,  28, 0, NULL, Msg_RA_Swap,								TEXT_LEFT, NULL,
328,  19, 464,  28, 0, NULL, Msg_RA_Carrier,						TEXT_LEFT, NULL,
464,  19, 605,  28, 0, NULL, Msg_RA_Connection,					TEXT_LEFT, NULL,
 12,  30, 291, 113, 1, NULL, 0,													BUTTON_GADGET, NULL, 
296,  30, 313, 113, 0, NULL, 0,													HIBOX_REGION, NULL,
326,  30, 605, 113, 1, NULL, 0,													BUTTON_GADGET, NULL, 
610,  30, 627, 113, 0, NULL, 0,													HIBOX_REGION, NULL,
 12, 117,  92, 130, 1, NULL, Msg_RA_Add,								BUTTON_GADGET, NULL, 
104, 117, 184, 130, 1, NULL, Msg_RA_Delete,							BUTTON_GADGET, NULL, 
197, 117, 277, 130, 1, NULL, Msg_RA_Edit,								BUTTON_GADGET, NULL, 
326, 117, 406, 130, 1, NULL, Msg_RA_Add,								BUTTON_GADGET, NULL, 
418, 117, 498, 130, 1, NULL, Msg_RA_Delete,							BUTTON_GADGET, NULL, 
511, 117, 591, 130, 1, NULL, Msg_RA_Edit,								BUTTON_GADGET, NULL, 
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA1_B_GR[] =
{
155,  33, 484,  46, 1, NULL, 0,													BUTTON_GADGET, NULL, 
155,  63, 484,  76, 1, NULL, 0,													BUTTON_GADGET, NULL, 
155,  93, 484, 106, 1, NULL, 0,													BUTTON_GADGET, NULL, 
155, 115, 171, 123, 1, NULL, Msg_RA_Swap_immediately,		CHECK_GADGET, NULL,
493,  93, 563, 106, 1, NULL, Msg_RA_Edit,								BUTTON_GADGET, NULL,	
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA2_GR[] =
{
  0,   0, 600, 154, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 599, 153, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  7,   1, 592,  10, 0, NULL, Msg_RA_Remote_Access2,			TEXT_REGION, NULL,
  7,  12, 592,  12, 0, NULL, 0,													LO_LINE, NULL,								
  8,  18, 569, 118, 2, NULL, 0,													LOBOX_REGION, NULL,
259, 136, 341, 149, 0, NULL, Msg_RA_Abort,							BUTTON_GADGET, NULL,
  8, 122, 591, 131, 2, NULL, 0,													LOBOX_REGION, NULL,
574,  18, 591, 118, 0, NULL, 0,													HIBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA3_GR[] =
{
  0,   0, 505, 115, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 504, 114, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 493,  12, 0, NULL, Msg_RA_Upload_Options,			TEXT_REGION, NULL,
 12,  15, 493,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12,  90, 493,  90, 0, NULL, 0,													LO_LINE, NULL,								
 12,  96,  93, 109, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
411,  96, 493, 109, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 43,  20,  59,  28, 1, NULL, Msg_RA_Upload_all_files,		CHECK_GADGET, NULL,
 43,  32,  59,  40, 1, NULL, Msg_RA_Delayed_upload,			CHECK_GADGET, NULL,
 43,  44,  59,  52, 1, NULL, Msg_RA_Skip_system_files,	CHECK_GADGET, NULL,
 43,  56,  59,  64, 1, NULL, Msg_RA_Upload_multiple_scripts, CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA4_GR[] =
{
  0,   0, 505, 115, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 504, 114, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 493,  12, 0, NULL, Msg_RA_Countdown,					TEXT_REGION, NULL,
 12,  15, 493,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12,  90, 493,  90, 0, NULL, 0,													LO_LINE, NULL,								
211,  96, 293, 109, 0, NULL, Msg_RA_Abort,							BUTTON_GADGET, NULL,
 57,  30, 447,  39, 0, NULL, Msg_RA_Upload_starts,			TEXT_REGION, NULL,
 57,  60, 447,  69, 0, NULL, Msg_RA_from_now,						TEXT_REGION, NULL,
 57,  42, 447,  55, 2, NULL, 0,													LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA5_GR[] =
{
  0,   0, 505, 115, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 504, 114, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 493,  12, 0, NULL, Msg_RA_Select_Script,			TEXT_REGION, NULL,
 12,  15, 493,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12,  90, 493,  90, 0, NULL, 0,													LO_LINE, NULL,								
 12,  96,  93, 109, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
411,  96, 493, 109, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 27,  34, 356,  47, 1, NULL, 0,													BUTTON_GADGET, NULL,
 27,  54,  43,  62, 1, NULL, Msg_RA_Swap_immediately, 	CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord RA6_GR[] =
{
  0,   0, 505, 115, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 504, 114, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 493,  12, 0, NULL, Msg_RA_Select_Carrier_and_Connection, TEXT_REGION, NULL,
 12,  15, 493,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12,  90, 493,  90, 0, NULL, 0,													LO_LINE, NULL,								
 12,  96,  93, 109, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
411,  96, 493, 109, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 27,  34, 356,  47, 1, NULL, 0,													BUTTON_GADGET, NULL,
 27,  64, 356,  77, 1, NULL, 0,													BUTTON_GADGET, NULL,
365,  64, 435,  77, 1, NULL, Msg_RA_Edit,								BUTTON_GADGET, NULL,	
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct StringRecord RA7_Date_SR	= { 8, "         " };
struct StringRecord RA7_Time_SR	= { 8, "         " };

struct GadgetRecord RA7_GR[] =
{
  0,   0, 505, 115, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 504, 114, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 12,   3, 493,  12, 0, NULL, Msg_RA_Upload_delay,				TEXT_REGION, NULL,
 12,  15, 493,  15, 0, NULL, 0,													LO_LINE, NULL,								
 12,  90, 493,  90, 0, NULL, 0,													LO_LINE, NULL,								
 12,  96,  93, 109, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
411,  96, 493, 109, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 25,  22, 277,  31, 0, NULL, Msg_RA_Start_to_upload,		TEXT_LEFT, NULL,
 27,  37,  43,  45, 1, NULL, 0,													RADIO_GADGET, NULL,
 27,  57,  43,  65, 1, NULL, Msg_RA_Immediately,				RADIO_GADGET, NULL,
120,  35, 205,  48, 1, NULL, Msg_RA_Date,								DATE_GADGET, (struct GadgetRecord *)&RA7_Date_SR,
276,  35, 355,  48, 1, NULL, Msg_RA_Time,								TIME_GADGET, (struct GadgetRecord *)&RA7_Time_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct StringRecord RA8_BR_SR = { 6, "       " };
struct StringRecord RA8_BS_SR = { 6, "       " };
struct StringRecord RA8_DP_SR = { 60, "                                                                " };
struct StringRecord RA8_PW_SR = { 9, "         " };
struct StringRecord RA8_DE_SR = { 30, "                               " };
struct StringRecord RA8_RS_SR = { 30, "                               " };
struct StringRecord RA8_DI_SR = { 30, "                               " };
struct StringRecord RA8_PN_SR = { 30, "                               " };

struct CycleRecord RA8_CC_CR = { 0, 10, 18, NULL, Msg_CDF_9 };
struct CycleRecord RA8_UN_CR = { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord RA8_HS_CR = { 0,  2, 10, NULL, Msg_CDF_10 };

struct GadgetRecord RA8_GR[] =
{
  0,   0, 600, 191, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 599, 190, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
  7,   1, 592,  10, 0, NULL, Msg_CDF_1,									TEXT_REGION, NULL,
  7,  12, 592,  12, 0, NULL, 0,													LO_LINE, NULL,								
  7, 166, 592, 166, 0, NULL, 0,													LO_LINE, NULL,								
 12, 172,  93, 185, 0, NULL, Msg_Use,										BUTTON_GADGET, NULL,
506, 172, 588, 185, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 12,  46, 587, 152, 2, NULL, 0,													LOBOX_REGION, NULL,
 12,  28, 260,  41, 0, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&RA8_CC_CR,
301,  28, 441,  41, 0, NULL, Msg_CDF_7,									BUTTON_GADGET, NULL,
447,  28, 587,  41, 0, NULL, Msg_CDF_8,									BUTTON_GADGET, NULL,
159,  54, 282,  67, 1, NULL, Msg_BaudRate,							INTEGER_GADGET, (struct GadgetRecord *)&RA8_BR_SR,
159,  70, 282,  83, 1, NULL, Msg_Unit,									CYCLE_GADGET, (struct GadgetRecord *)&RA8_UN_CR,
159,  86, 282,  99, 1, NULL, Msg_CDF_3,									CYCLE_GADGET, (struct GadgetRecord *)&RA8_HS_CR,
159, 102, 282, 115, 1, NULL, Msg_CDF_4,									INTEGER_GADGET, (struct GadgetRecord *)&RA8_BS_SR,
301,  59, 549,  72, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_DP_SR,
301,  84, 549,  97, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_PW_SR,
301, 109, 549, 122, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_DE_SR,
159, 118, 282, 131, 1, NULL, Msg_CDF_11,								SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_RS_SR,
159, 134, 282, 147, 1, NULL, Msg_CDF_13,								SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_DI_SR,
301, 134, 549, 147, 1, NULL, 0,													SPECIAL_STRING_GADGET, (struct GadgetRecord *)&RA8_PN_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
