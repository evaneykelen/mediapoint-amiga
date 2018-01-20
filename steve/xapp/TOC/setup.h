struct CycleRecord TOC_Modes_CR					= { 0,  3, 13, NULL, Msg_TOC_1 };
struct StringRecord TOC_PlayVolume_SR		= { 3, "    " };
struct CycleRecord TOC_PlayFade_CR			= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
//struct CycleRecord TOC_PlayLoops_CR			= { 0, 31,  4, NULL, Msg_Infinite_30 };
struct CycleRecord TOC_RecordFreq_CR		= { 9, 14, 10, NULL, Msg_TOC_20 };
struct CycleRecord TOC_StopFade_CR			= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord TOC_FadeIn_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord TOC_FadeOut_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord TOC_SetVol_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct StringRecord TOC_SetVolFrom_SR		= { 3, "    " };
struct StringRecord TOC_SetVolTo_SR			= { 3, "    " };

struct GadgetRecord TOC_GR[] =
{
  0,   0, 505, 157, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	504, 156, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
 10,  29, 494, 127, 2, NULL, 0,													LOBOX_REGION, NULL,
 10, 139,  92, 152, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
412, 139, 494, 152, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
117, 139, 199, 152, 0, NULL, Msg_Play,									BUTTON_GADGET, NULL,
 10, 133, 494, 133, 0, NULL, 0,													LO_LINE, NULL,
 10,  12, 181,  25, 0, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&TOC_Modes_CR,
/* play */
114,  35, 480,  48, 1, NULL, Msg_S16_4,									HIBOX_REGION, NULL,	
114,  53, 153,  66, 1, NULL, Msg_Volume,								INTEGER_GADGET, (struct GadgetRecord *)&TOC_PlayVolume_SR,
  1,   1,   3,   3, 1, NULL, Msg_Loops,									INVISIBLE_GADGET, NULL,	//CYCLE_GADGET, (struct GadgetRecord *)&TOC_PlayLoops_CR,
114,  71, 173,  84, 1, NULL, Msg_Sample_2,							CYCLE_GADGET, (struct GadgetRecord *)&TOC_PlayFade_CR,
/* record */
114,  35, 480,  48, 1, NULL, Msg_S16_4,									HIBOX_REGION, NULL,	
114,  53, 130,  61, 1, NULL, Msg_TOC_2,									RADIO_GADGET, NULL,
114,  63, 130,  71, 1, NULL, Msg_TOC_3,									RADIO_GADGET, NULL,
114,  73, 130,  81, 1, NULL, Msg_TOC_4,									RADIO_GADGET, NULL,
114,  83, 130,  91, 1, NULL, Msg_TOC_5,									RADIO_GADGET, NULL,
114,  96, 130, 104, 1, NULL, Msg_TOC_6,									CHECK_GADGET, NULL,
333,  53, 349,  61, 1, NULL, Msg_TOC_7,									RADIO_GADGET, NULL,
333,  63, 349,  71, 1, NULL, Msg_TOC_8,									RADIO_GADGET, NULL,
333,  73, 349,  81, 1, NULL, Msg_TOC_9,									RADIO_GADGET, NULL,
333,  83, 349,  91, 1, NULL, Msg_TOC_10,								RADIO_GADGET, NULL,
333,  96, 349, 104, 1, NULL, Msg_TOC_11,								CHECK_GADGET, NULL,
114, 109, 238, 122, 1, NULL, Msg_Frequency,							CYCLE_GADGET, (struct GadgetRecord *)&TOC_RecordFreq_CR,
333, 110, 480, 120, 1, NULL, Msg_TOC_12,								HIBOX_REGION, NULL,
207, 139, 289, 152, 0, NULL, Msg_Record,								BUTTON_GADGET, NULL,
440,  53, 457, 104, 1, NULL, NULL,											HIBOX_REGION, NULL,
463,  53, 480, 104, 1, NULL, NULL,											HIBOX_REGION, NULL,
/* Misc */
 29,  43,  45,  51, 1, NULL, Msg_TOC_13,								RADIO_GADGET, NULL,
 29,  62,  45,  70, 1, NULL, Msg_TOC_14,								RADIO_GADGET, NULL,
 29,  81,  45,  89, 1, NULL, Msg_TOC_15,								RADIO_GADGET, NULL,
 29, 100,  45, 108, 1, NULL, Msg_TOC_17,								RADIO_GADGET, NULL,
 22,  57, 482,  57, 1, NULL, NULL,											DOTTED_LINE, NULL,
 22,  76, 482,  76, 1, NULL, NULL,											DOTTED_LINE, NULL,
 22,  95, 482,  95, 1, NULL, NULL,											DOTTED_LINE, NULL,
/* Stop (fade out) */
209,  41, 268,  54, 1, NULL, Msg_TOC_16,								CYCLE_GADGET, (struct GadgetRecord *)&TOC_StopFade_CR,
/* Fade in */
209,  60, 268,  73, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&TOC_FadeIn_CR,
/* Fade out */
209,  79, 268,  92, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&TOC_FadeOut_CR,
/* Set vol */
163,  98, 202, 111, 1, NULL, NULL,											INTEGER_GADGET, (struct GadgetRecord *)&TOC_SetVolFrom_SR,
243,  98, 282, 111, 1, NULL, NULL,											INTEGER_GADGET, (struct GadgetRecord *)&TOC_SetVolTo_SR,
333,  98, 393, 111, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&TOC_SetVol_CR,
/* title */
140,   3, 496,  12, 0, NULL, Msg_TOC_Title,				 			TEXT_RIGHT, NULL,
140,  14, 496,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord Record_GR[] =
{
  0,   0, 320,  95, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319,  94, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,  77,  89,  90, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
 26,  18, 293,  31, 1, NULL, Msg_TOC_24,								BUTTON_GADGET, NULL,
 26,  39, 293,  52, 1, NULL, Msg_TOC_25,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
