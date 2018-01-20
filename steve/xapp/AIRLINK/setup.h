struct StringRecord AIR_Port_SR = { 20, "                     " };

struct GadgetRecord AIR_GR[] =
{
  0,   0, 411, 140, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	410, 139, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
142,   3, 406,  12, 0, NULL, Msg_Air_1,						 			TEXT_RIGHT, NULL,
142,  14, 406,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
  9, 122,  90, 135, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
320, 122, 401, 135, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
100, 122, 181, 135, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,
  7, 117, 401, 117, 0, NULL, 0,													LO_LINE, NULL, 
 92,  39, 320,  52, 1, NULL, Msg_Port,									STRING_GADGET, (struct GadgetRecord *)&AIR_Port_SR,
 92,  60, 320,  73, 1, NULL, Msg_Air_2,									BUTTON_GADGET, NULL,
 92,  81, 320,  94, 1, NULL, 0,													BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

struct GadgetRecord PopUp_GR[] =
{
  0,   0, 168,   0, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	  0,   0, 1, NULL, 0,													HIBOX_REGION, NULL,	//DBL_BORDER_REGION, NULL,
  0,   0,   0,   0, 0, NULL, 0,													POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

/******** E O F ********/
