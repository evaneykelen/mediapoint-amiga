/********************* PREFS *********************/

struct GadgetRecord Prefs_GR[] =
{
  0,   0, 320, 200, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
143, 166, 225, 179, 0, NULL, Msg_Previous,							BUTTON_GADGET, NULL,
230, 166, 312, 179, 0, NULL, Msg_Next,									BUTTON_GADGET, NULL,
143, 182, 225, 195, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
230, 182, 312, 195, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  4,   2, 316, 163, 2, NULL, 0,													INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** page 1 ****/

struct CycleRecord Prefs_UL_CR = { 0,  3, 15, NULL, Msg_UserLevel_List };
struct CycleRecord Prefs_CS_CR = { 0, 36,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Prefs_LG_CR = { 0, 10, 14, NULL, Msg_Lang_List };
struct CycleRecord Prefs_WB_CR = { 0,  2,  6, NULL, Msg_OnOff_List };
//struct CycleRecord Prefs_SP_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
//struct CycleRecord Prefs_GP_CR = { 0,  2,  6, NULL, Msg_OnOff_List };
//struct CycleRecord Prefs_SB_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
//struct CycleRecord Prefs_PI_CR = { 0,  4, 16, NULL, Msg_PlayerInputList };
//struct CycleRecord Prefs_MP_CR = { 0,  3,  6, NULL, Msg_SpriteOnOffAutoList };
struct CycleRecord Prefs_TN_CR = { 0,  2, 10, NULL, Msg_ThumbnailSize_List };
struct CycleRecord Prefs_TC_CR = { 0,  2,  4, NULL, Msg_8_16_32 };

struct GadgetRecord Prefs_1_GR[] =
{
164,   4, 312,  17, 1, NULL, Msg_UserLevel,							CYCLE_GADGET, (struct GadgetRecord *)&Prefs_UL_CR,
164,  21, 312,  34, 1, NULL, Msg_ColorSet,							CYCLE_GADGET, (struct GadgetRecord *)&Prefs_CS_CR,
164,  38, 312,  51, 1, NULL, Msg_Lang,									CYCLE_GADGET, (struct GadgetRecord *)&Prefs_LG_CR,
164,  55, 312,  68, 1, NULL, Msg_Workbench,							CYCLE_GADGET, (struct GadgetRecord *)&Prefs_WB_CR,
164,  74, 312,  87, 1, NULL, Msg_P_ScriptMon,						HIBOX_REGION, NULL,
164,  91, 312, 104, 1, NULL, Msg_P_PageMon,							HIBOX_REGION, NULL,
164, 108, 312, 121, 1, NULL, Msg_P_PlayerMon,						HIBOX_REGION, NULL,
164, 127, 312, 140, 1, NULL, Msg_ThumbnailSize,					CYCLE_GADGET, (struct GadgetRecord *)&Prefs_TN_CR,
164, 144, 312, 157, 1, NULL, Msg_ThumbColors,						CYCLE_GADGET, (struct GadgetRecord *)&Prefs_TC_CR,
  8,  71, 312,  71, 0, NULL, 0,													DOTTED_LINE, NULL,
  8, 124, 312, 124, 0, NULL, 0,													DOTTED_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_GR,
-1
};

#if 0
164,  72, 312,  85, 1, NULL, Msg_ShowDateProg,					CYCLE_GADGET, (struct GadgetRecord *)&Prefs_SP_CR,
164,  89, 312, 102, 1, NULL, Msg_GamePort,							CYCLE_GADGET, (struct GadgetRecord *)&Prefs_GP_CR,
164, 106, 312, 119, 1, NULL, Msg_Standby,								CYCLE_GADGET, (struct GadgetRecord *)&Prefs_SB_CR,
164, 123, 312, 136, 1, NULL, Msg_PlayerInput,						CYCLE_GADGET, (struct GadgetRecord *)&Prefs_PI_CR,
164, 140, 312, 153, 1, NULL, Msg_MousePointer,					CYCLE_GADGET, (struct GadgetRecord *)&Prefs_MP_CR,
#endif

#if 0
/**** page 2 ****/

struct CycleRecord Prefs_SI_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
struct CycleRecord Prefs_TI_CR = { 0,  2,  5, NULL, Msg_YesNo_List };
struct CycleRecord Prefs_TN_CR = { 0,  2, 10, NULL, Msg_ThumbnailSize_List };
struct CycleRecord Prefs_TC_CR = { 0,  2,  4, NULL, Msg_8_16_32 };

struct GadgetRecord Prefs_2_GR[] =
{
164,   4, 312,  17, 1, NULL, Msg_P_ScriptMon,						HIBOX_REGION, NULL,
164,  21, 312,  34, 1, NULL, Msg_P_PageMon,							HIBOX_REGION, NULL,
164,  38, 312,  51, 1, NULL, Msg_P_PlayerMon,						HIBOX_REGION, NULL,
164,  65, 312,  78, 1, NULL, Msg_ThumbnailSize,					CYCLE_GADGET, (struct GadgetRecord *)&Prefs_TN_CR,
164,  82, 312,  95, 1, NULL, Msg_ThumbColors,						CYCLE_GADGET, (struct GadgetRecord *)&Prefs_TC_CR,
  8,  58, 312,  58, 0, NULL, 0,													DOTTED_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_1_GR,
-1
};
#endif

/**** page 3 ****/

struct StringRecord Prefs_F1_SR = { 11, "            " };
struct StringRecord Prefs_F2_SR = { 11, "            " };
struct StringRecord Prefs_F3_SR = { 11, "            " };
struct StringRecord Prefs_F4_SR = { 11, "            " };
struct StringRecord Prefs_F5_SR = { 11, "            " };
struct StringRecord Prefs_F6_SR = { 11, "            " };

struct GadgetRecord Prefs_3_GR[] =
{
  7,   6, 312,  15, 0, NULL, Msg_PrgTimeCodes,					TEXT_LEFT, NULL,
109,  29, 213,  42, 1, NULL, Msg_F1,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F1_SR,
109,  44, 213,  57, 1, NULL, Msg_F2,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F2_SR,
109,  59, 213,  72, 1, NULL, Msg_F3,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F3_SR,
109,  74, 213,  87, 1, NULL, Msg_F4,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F4_SR,
109,  89, 213, 102, 1, NULL, Msg_F5,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F5_SR,
109, 104, 213, 117, 1, NULL, Msg_F6,										TIME_GADGET, (struct GadgetRecord *)&Prefs_F6_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_1_GR,
-1
};

/**** page 4 ****/

struct CycleRecord Prefs_P1_CR = { 0,  7, 4, NULL, Msg_Numbers_1_40 };
struct CycleRecord Prefs_P2_CR = { 0, 10, 4, NULL, Msg_Numbers_1_40 };

struct StringRecord Prefs_P3_SR = { 30, "                               " };

struct GadgetRecord Prefs_4_GR[] =
{
  7,   6, 312,  15, 0, NULL, Msg_DefaultDirs,						TEXT_LEFT, NULL,
  7,  85, 312,  94, 0, NULL, Msg_HomeDirs,							TEXT_LEFT, NULL,
  7,  29,  55,  42, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&Prefs_P1_CR,
  7, 106,  55, 119, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&Prefs_P2_CR,
 59,  29, 312,  42, 1, NULL,	0,												LOBOX_REGION, NULL,	// def dir name
 59, 106, 312, 119, 1, NULL,	0,												STRING_GADGET, (struct GadgetRecord *)&Prefs_P3_SR,
  7,  57, 312,  70, 1, NULL,	0,												BUTTON_GADGET, NULL,	// def dir path
  7, 134, 312, 147, 1, NULL,	0,												BUTTON_GADGET, NULL,	// home dir path
  8,  79, 312,  79, 0, NULL, 0,													DOTTED_LINE, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_3_GR,
-1
};

/**** page 5 ****/

struct CycleRecord Prefs_PD1_CR = { 0,  4, 6, NULL, Msg_BaudRate_List };
struct CycleRecord Prefs_PD2_CR	= { 0, 16, 4, NULL, Msg_Numbers_0_15 };

struct StringRecord Prefs_SD_SR = { 18, "                    " };

struct GadgetRecord Prefs_5_GR[] =
{
  7,   6, 312,  15, 0, NULL, Msg_PlayerDevice,					TEXT_LEFT, NULL,
  7,  29, 312,  42, 1, NULL,	0,												BUTTON_GADGET, NULL,
126,  48, 215,  61, 1, NULL, Msg_BaudRate,							CYCLE_GADGET, (struct GadgetRecord *)&Prefs_PD1_CR,
126,  67, 312,  80, 1, NULL, Msg_SerialDevice,					STRING_GADGET, (struct GadgetRecord *)&Prefs_SD_SR, 
126,  86, 191,  99, 1, NULL, Msg_Unit,									CYCLE_GADGET, (struct GadgetRecord *)&Prefs_PD2_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_4_GR,
-1
};

/**** monitor selection ****/

struct GadgetRecord SelMon_GR[] =
{
  0,   0, 307, 149, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	306, 148, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 299,  10, 0, NULL, Msg_P_SelAMon,							TEXT_REGION, NULL,
  7,  12, 299,  12, 0, NULL, 0,													LO_LINE, NULL,
  6,  15, 277, 126, 1, NULL, 0,													BUTTON_GADGET, NULL,
281,  15, 299, 126, 0, NULL, 0,													HIBOX_REGION, NULL,
  6, 131,  88, 144, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
217, 131, 299, 144, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)Prefs_5_GR,
-1
};

/******** E O F ********/
