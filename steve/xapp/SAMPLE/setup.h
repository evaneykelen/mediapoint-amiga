/******** sample xapp gadgets ********/

struct CycleRecord Sample1_Action_CR	= { 0,  5, 13, NULL, Msg_Sample_1 };
struct CycleRecord Sample1_Fade_CR		= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Sample1_Loops_CR		= { 0, 31,  4, NULL, Msg_Infinite_30 };
struct CycleRecord Sample1_Track_CR		= { 0,  2,  4, NULL, Msg_Numbers_1_40 };

struct StringRecord Sample1_Freq_SR		= { 5, "      " };
struct StringRecord Sample1_Volume_SR	= { 3, "    " };

/**** Play ****/

struct GadgetRecord Sample1_GR[] =
{
  0,   0, 640,  88, 0, NULL,	0,												DIMENSIONS, NULL,
  0,	 0,	639,  87, 0, NULL,	0,												DBL_BORDER_REGION, NULL,
 60,   3, 186,  16, 0, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&Sample1_Action_CR,
129,  21, 186,  34, 1, NULL, Msg_Loops,									CYCLE_GADGET, (struct GadgetRecord *)&Sample1_Loops_CR,
129,  37, 168,  50, 1, NULL, Msg_Volume,								INTEGER_GADGET, (struct GadgetRecord *)&Sample1_Volume_SR,
271,  21, 330,  34, 1, NULL, Msg_Frequency,							INTEGER_GADGET, (struct GadgetRecord *)&Sample1_Freq_SR,
271,  37, 330,  50, 1, NULL, Msg_Sample_2,							CYCLE_GADGET, (struct GadgetRecord *)&Sample1_Fade_CR,
509,  23, 622,  34, 1, NULL, Msg_Sample_3,							HIBOX_REGION, NULL,
 60,  70, 171,  83, 0, NULL, Msg_Play,									BUTTON_GADGET, NULL,
450,  70, 532,  83, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
551,  70, 633,  83, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
191,   3, 633,  16, 1, NULL,	0,												HIBOX_REGION, NULL,
  4,  18, 635,  66, 2, NULL,	0,												INVISIBLE_GADGET, NULL,
509,  39, 568,  52, 1, NULL, Msg_Sample_6,							CYCLE_GADGET, (struct GadgetRecord *)&Sample1_Track_CR,
 60,  54,  75,  62, 1, NULL, Msg_PlayFromDisk, 					CHECK_GADGET, NULL,
271,  54, 286,  62, 1, NULL, Msg_Filter,								CHECK_GADGET, NULL,
 37,  54,  52,  62, 2, NULL, 0,													INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** Fade out ****/

struct CycleRecord Sample2_Fade_CR	= { 0, 16, 4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Sample2_Track_CR	= { 0,  2,  4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord Sample2_GR[] =
{
230,   3, 287,  16, 1, NULL, Msg_Sample_4,							CYCLE_GADGET, (struct GadgetRecord *)&Sample2_Fade_CR,
129,  21, 186,  34, 1, NULL, Msg_Sample_6,							CYCLE_GADGET, (struct GadgetRecord *)&Sample2_Track_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** Fade in ****/

struct CycleRecord Sample3_Fade_CR	= { 0, 16, 4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Sample3_Track_CR	= { 0,  2, 4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord Sample3_GR[] =
{
230,   3, 287,  16, 1, NULL, Msg_Sample_4,							CYCLE_GADGET, (struct GadgetRecord *)&Sample3_Fade_CR,
129,  21, 186,  34, 1, NULL, Msg_Sample_6,							CYCLE_GADGET, (struct GadgetRecord *)&Sample3_Track_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** Stop ****/

struct CycleRecord Sample5_Track_CR = { 0,  2,  4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord Sample5_GR[] =
{
129,  21, 186,  34, 1, NULL, Msg_Sample_6,							CYCLE_GADGET, (struct GadgetRecord *)&Sample5_Track_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** Set volume ****/

struct StringRecord Sample4_Volume_SR	= { 3, "    " };
struct CycleRecord Sample4_Track_CR		= { 0,  2,  4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord Sample4_GR[] =
{
230,   3, 269,  16, 1, NULL, Msg_Sample_5,							INTEGER_GADGET, (struct GadgetRecord *)&Sample4_Volume_SR,
129,  21, 186,  34, 1, NULL, Msg_Sample_6,							CYCLE_GADGET, (struct GadgetRecord *)&Sample4_Track_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
