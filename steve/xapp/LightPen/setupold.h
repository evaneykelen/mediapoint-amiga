/******** LightPen xapp gadgets ********/

struct StringRecord A1_X_SR = { 3, "      " };
struct StringRecord A1_Y_SR = { 3, "      " };

struct GadgetRecord Light_GR[] =
{
  0,   0, 411, 140, 0, NULL, 0,												DIMENSIONS, NULL,	
  0,	 0,	410, 139, 0, NULL, 0,												DBL_BORDER_REGION, NULL,
 320, 50, 380,  66, 1, NULL, Msg_IV_1_7,							INTEGER_GADGET, (struct GadgetRecord *)&A1_X_SR,
 320, 70, 380,  86, 1, NULL, Msg_IV_1_8,							INTEGER_GADGET, (struct GadgetRecord *)&A1_Y_SR,
  9, 122,  90, 135, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
320, 122, 401, 135, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/

