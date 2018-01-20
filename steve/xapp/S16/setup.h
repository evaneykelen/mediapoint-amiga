struct CycleRecord Studio_Modes_CR					= { 0,  2, 13, NULL, Msg_S16_2 };
/* play */
struct StringRecord Studio_PlayVolume_SR		= { 3, "    " };
struct CycleRecord Studio_PlayFade_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
/* misc */
struct CycleRecord Studio_StopFade_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Studio_FadeIn_CR					= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Studio_FadeOut_CR				= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord Studio_SetVol_CR					= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct StringRecord Studio_SetVolFrom_SR		= { 3, "    " };
struct StringRecord Studio_SetVolTo_SR			= { 3, "    " };

struct GadgetRecord Studio_GR[] =
{
  0,   0, 505, 157, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	504, 156, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
140,   3, 496,  12, 0, NULL, Msg_S16_1,						 			TEXT_RIGHT, NULL,
140,  14, 496,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
 10,  29, 494, 127, 2, NULL, 0,													LOBOX_REGION, NULL,
 10, 139,  92, 152, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
412, 139, 494, 152, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
117, 139, 199, 152, 0, NULL, Msg_Play,									BUTTON_GADGET, NULL,
 10, 133, 494, 133, 0, NULL, 0,													LO_LINE, NULL,
 10,  12, 181,  25, 0, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Studio_Modes_CR,
/* play */
 17,  32, 330,  40, 0, NULL, Msg_S16_4,									TEXT_LEFT, NULL,
 17,  43, 465,  55, 1, NULL, 0,													BUTTON_GADGET, NULL,
 17,  60, 465,  72, 1, NULL, 0,													BUTTON_GADGET, NULL,
 17,  77, 465,  89, 1, NULL, 0,													BUTTON_GADGET, NULL,
 17,  94, 465, 106, 1, NULL, 0,													BUTTON_GADGET, NULL,
123, 110, 162, 123, 1, NULL, Msg_Volume,								INTEGER_GADGET, (struct GadgetRecord *)&Studio_PlayVolume_SR,
308, 110, 367, 123, 1, NULL, Msg_Sample_2,							CYCLE_GADGET, (struct GadgetRecord *)&Studio_PlayFade_CR,
/* Misc */
 29,  43,  45,  51, 1, NULL, Msg_TOC_13,								RADIO_GADGET, NULL,
 29,  62,  45,  70, 1, NULL, Msg_TOC_14,								RADIO_GADGET, NULL,
 29,  81,  45,  89, 1, NULL, Msg_TOC_15,								RADIO_GADGET, NULL,
 29, 100,  45, 108, 1, NULL, Msg_TOC_17,								RADIO_GADGET, NULL,
 22,  57, 482,  57, 1, NULL, NULL,											DOTTED_LINE, NULL,
 22,  76, 482,  76, 1, NULL, NULL,											DOTTED_LINE, NULL,
 22,  95, 482,  95, 1, NULL, NULL,											DOTTED_LINE, NULL,
/* Stop (fade out) */
209,  41, 268,  54, 1, NULL, Msg_TOC_16,								CYCLE_GADGET, (struct GadgetRecord *)&Studio_StopFade_CR,
/* Fade in */
209,  60, 268,  73, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&Studio_FadeIn_CR,
/* Fade out */
209,  79, 268,  92, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&Studio_FadeOut_CR,
/* Set vol */
163,  98, 202, 111, 1, NULL, NULL,											INTEGER_GADGET, (struct GadgetRecord *)&Studio_SetVolFrom_SR,
243,  98, 282, 111, 1, NULL, NULL,											INTEGER_GADGET, (struct GadgetRecord *)&Studio_SetVolTo_SR,
333,  98, 393, 111, 1, NULL, Msg_TOC_18,								CYCLE_GADGET, (struct GadgetRecord *)&Studio_SetVol_CR,
/* rest play */
469,  43, 487,  55, 1, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
469,  60, 487,  72, 1, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
469,  77, 487,  89, 1, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
469,  94, 487, 106, 1, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
