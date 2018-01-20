struct CycleRecord Nep_page_CR = { 0, 3, 12, NULL, Msg_Nep_Page };

struct GadgetRecord NEP_GR[] =
{
  0,   0, 540, 140, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	539, 139, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  9,  32, 530, 111, 2, NULL, 0,													LOBOX_REGION, NULL,
  9, 122,  90, 135, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
100, 122, 181, 135, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,
449, 122, 530, 135, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
142,   3, 535,  12, 0, NULL, Msg_Nep_Title,				 			TEXT_RIGHT, NULL,
142,  14, 535,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
  7, 117, 530, 117, 0, NULL, 0,													LO_LINE, NULL,
  9,  15, 163,  28, 0, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&Nep_page_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

struct GadgetRecord NEP_Page1_GR[] =
{
 18,  38, 264,  69, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page1_GR[1],
 51,  34,  67,  42, 1, NULL, Msg_Nep_Video,							CHECK_GADGET, NULL,
 28,  45,  44,  53, 1, NULL, Msg_Nep_Normal,						RADIO_GADGET, NULL,
 28,  57,  44,  65, 1, NULL, Msg_Nep_Invert,						RADIO_GADGET, NULL,

276,  38, 522,  69, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page1_GR[5],
309,  34, 325,  42, 1, NULL, Msg_Nep_Computer,					CHECK_GADGET, NULL,
286,  45, 302,  53, 1, NULL, Msg_Nep_Genlock,						RADIO_GADGET, NULL,
286,  57, 302,  65, 1, NULL, Msg_Nep_Amiga,							RADIO_GADGET, NULL,

 18,  76, 264, 107, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page1_GR[9],
 51,  72,  67,  80, 1, NULL, Msg_Nep_Overlay,						CHECK_GADGET, NULL,
 28,  83,  44,  91, 1, NULL, Msg_Nep_Normal,						RADIO_GADGET, NULL,
 28,  95,  44, 103, 1, NULL, Msg_Nep_Alpha,							RADIO_GADGET, NULL,

0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

struct GadgetRecord NEP_Page2_GR[] =
{
 18,  38, 264,  69, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page2_GR[1],
 51,  34,  67,  42, 1, NULL, Msg_Nep_Video,							CHECK_GADGET, NULL,
 28,  45,  44,  53, 1, NULL, Msg_Nep_FadeIn,						RADIO_GADGET, NULL,
 28,  57,  44,  65, 1, NULL, Msg_Nep_FadeOut,						RADIO_GADGET, NULL,

276,  38, 522,  69, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page2_GR[5],
309,  34, 325,  42, 1, NULL, Msg_Nep_Computer,					CHECK_GADGET, NULL,
286,  45, 302,  53, 1, NULL, Msg_Nep_FadeIn,						RADIO_GADGET, NULL,
286,  57, 302,  65, 1, NULL, Msg_Nep_FadeOut,						RADIO_GADGET, NULL,

0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

struct StringRecord NEP_Dur1_SR		= { 5, "      " };
struct StringRecord NEP_Dur2_SR		= { 5, "      " };

struct StringRecord NEP_From1_SR	= { 3, "    " };
struct StringRecord NEP_To1_SR		= { 3, "    " };

struct StringRecord NEP_From2_SR	= { 3, "    " };
struct StringRecord NEP_To2_SR		= { 3, "    " };

struct GadgetRecord NEP_Page3_GR[] =
{
 18,  38, 264, 107, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page3_GR[1],
 51,  34,  67,  42, 1, NULL, Msg_Nep_Video,							CHECK_GADGET, NULL,
111,  46, 170,  59, 1, NULL, Msg_CDTV_3,								STRING_GADGET, (struct GadgetRecord *)&NEP_Dur1_SR,
 26,  67,  42,  76, 1, NULL, NULL,											CHECK_GADGET, NULL,
111,  65, 150,  78, 1, NULL, Msg_Nep_From,							INTEGER_GADGET, (struct GadgetRecord *)&NEP_From1_SR,
175,  65, 255,  78, 1, NULL, Msg_Nep_Get,								BUTTON_GADGET, NULL,
 26,  86,  42,  95, 1, NULL, NULL,											CHECK_GADGET, NULL,
111,  84, 150,  97, 1, NULL, Msg_Nep_To,								INTEGER_GADGET, (struct GadgetRecord *)&NEP_To1_SR,
175,  84, 255,  97, 1, NULL, Msg_Nep_Get,								BUTTON_GADGET, NULL,
276,  38, 522, 107, 0, NULL, 0,													COMBOBOX_REGION, (struct GadgetRecord *)&NEP_Page3_GR[10],
309,  34, 325,  42, 1, NULL, Msg_Nep_Computer,					CHECK_GADGET, NULL,
369,  46, 428,  59, 1, NULL, Msg_CDTV_3,								STRING_GADGET, (struct GadgetRecord *)&NEP_Dur2_SR,
284,  67, 300,  76, 1, NULL, NULL,											CHECK_GADGET, NULL,
369,  65, 408,  78, 1, NULL, Msg_Nep_From,							INTEGER_GADGET, (struct GadgetRecord *)&NEP_From2_SR,
433,  65, 513,  78, 1, NULL, Msg_Nep_Get,								BUTTON_GADGET, NULL,
284,  86, 300,  95, 1, NULL, NULL,											CHECK_GADGET, NULL,
369,  84, 408,  97, 1, NULL, Msg_Nep_To,								INTEGER_GADGET, (struct GadgetRecord *)&NEP_To2_SR,
433,  84, 513,  97, 1, NULL, Msg_Nep_Get,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

/******** E O F ********/
