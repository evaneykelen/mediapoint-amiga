/******** SER xapp gadgets ********/

struct CycleRecord Ser_HS_CR = { 0,  3, 10, NULL, Msg_Ser_HS };
struct CycleRecord Ser_PA_CR = { 0,  5, 10, NULL, Msg_Ser_PA };
struct CycleRecord Ser_BC_CR = { 0,  2,  3, NULL, Msg_Ser_BC };
struct CycleRecord Ser_SB_CR = { 0,  2,  3, NULL, Msg_Ser_SB };
struct CycleRecord Ser_UN_CR = { 0, 16,  4, NULL, Msg_Numbers_0_15 };

struct StringRecord Ser_BR_SR	= { 6, "       " };
struct StringRecord Ser_PA_SR	= { 6, "       " };
struct StringRecord Ser_DN_SR	= { 30, "                               " };

struct GadgetRecord Ser_GR[] =
{
  0,   0, 411, 200, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	410, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  9, 182,  90, 195, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
100, 182, 181, 195, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,
320, 182, 401, 195, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
  7, 177, 401, 177, 0, NULL, 0,													LO_LINE, NULL, 
142,   3, 406,  12, 0, NULL, Msg_Ser1,						 			TEXT_RIGHT, NULL,
142,  14, 406,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
  7,  36, 403,  49, 1, NULL, 0,													SPECIAL_STRING_GADGET, NULL,
130,  54, 239,  67, 1, NULL, Msg_Ser2,									INTEGER_GADGET, (struct GadgetRecord *)&Ser_BR_SR,
130,  69, 270,  82, 1, NULL, Msg_Ser3,									CYCLE_GADGET, (struct GadgetRecord *)&Ser_HS_CR,
130,  84, 270,  97, 1, NULL, Msg_Ser4,									CYCLE_GADGET, (struct GadgetRecord *)&Ser_PA_CR,
130,  99, 270, 112, 1, NULL, Msg_Ser5,									CYCLE_GADGET, (struct GadgetRecord *)&Ser_BC_CR,
130, 114, 270, 127, 1, NULL, Msg_Ser6,									CYCLE_GADGET, (struct GadgetRecord *)&Ser_SB_CR,
130, 129, 270, 142, 1, NULL, Msg_Ser7,									CYCLE_GADGET, (struct GadgetRecord *)&Ser_UN_CR,
130, 144, 270, 157, 1, NULL, Msg_Ser8,									INTEGER_GADGET, (struct GadgetRecord *)&Ser_PA_SR,
284,  54, 402, 106, 2, NULL, 0,													LOBOX_REGION, NULL,
130, 159, 270, 172, 1, NULL, Msg_Device,								STRING_GADGET, (struct GadgetRecord *)&Ser_DN_SR,
243,  54, 270,  67, 1, "?",  0,													BUTTON_GADGET, NULL,												
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord PopUp_GR[] =
{
  0,   0,  49,   0, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	  0,   0, 1, NULL, 0,													HIBOX_REGION, NULL,
  0,   0,   0,   0, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

/******** E O F ********/
