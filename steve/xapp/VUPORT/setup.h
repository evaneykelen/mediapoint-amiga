struct StringRecord VP_Arg1_SR = { 7, "        " };
struct StringRecord VP_Arg2_SR = { 7, "        " };

struct CycleRecord VP_Commands_CR = { 0, 20, 24, NULL, Msg_VU_Cmds };
struct CycleRecord VP_Unit_CR     = { 0,  9,  8, NULL, Msg_VU_Units };

struct GadgetRecord VP_GR[] =
{
  0,   0, 411, 140, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	410, 139, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
114,  38, 397,  51, 1, NULL, Msg_VU_Cmd,								CYCLE_GADGET, (struct GadgetRecord *)&VP_Commands_CR,
114,  72, 198,  85, 1, NULL, Msg_Unit2,									CYCLE_GADGET, (struct GadgetRecord *)&VP_Unit_CR,
  9, 122,  90, 135, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
320, 122, 401, 135, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
100, 122, 181, 135, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,
  7, 117, 401, 117, 0, NULL, 0,													LO_LINE, NULL, 
142,   3, 406,  12, 0, NULL, Msg_VU_Name,					 			TEXT_RIGHT, NULL,
142,  14, 406,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
114,  55, 193,  68, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&VP_Arg1_SR,
206,  55, 285,  68, 1, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&VP_Arg2_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
