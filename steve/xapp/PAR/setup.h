//struct StringRecord PAR_In_SR		= { 11, "            " };
//struct StringRecord PAR_Out_SR	= { 11, "            " };
struct StringRecord PAR_S16_SR	= { 11, "            " };

struct GadgetRecord PAR_GR[] =
{
  0,   0, 411, 180, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	410, 179, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
140,   3, 402,  12, 0, NULL, Msg_PAR_Title,				 			TEXT_RIGHT, NULL,
140,  14, 402,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
  9, 162,  90, 175, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
320, 162, 401, 175, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
100, 162, 181, 175, 0, NULL, Msg_Play,									BUTTON_GADGET, NULL,
  7, 157, 403, 157, 0, NULL, 0,													LO_LINE, NULL,
  9,  39, 401,  52, 1, NULL, 0,                         BUTTON_GADGET, NULL,
 72,  63, 176,  76, 2, NULL, Msg_PAR_In,								LOBOX_REGION, NULL, /* (struct GadgetRecord *)&PAR_In_SR,*/
 72,  80, 176,  93, 2, NULL, Msg_PAR_Out,								LOBOX_REGION, NULL, /*TIME_GADGET, (struct GadgetRecord *)&PAR_Out_SR,*/
248,  68, 264,  76, 1, NULL, Msg_PAR_Cue,								CHECK_GADGET, NULL,
248,  80, 352,  93, 1, NULL, 0,													TIME_GADGET, (struct GadgetRecord *)&PAR_S16_SR,
  9, 105,  35, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
 39, 105,  65, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
 69, 105,  95, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
 99, 105, 125, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
129, 105, 155, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
159, 105, 185, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
189, 105, 215, 117, 1, NULL, 0,													BUTTON_GADGET, NULL,
219, 105, 245, 117, 1, NULL, 0,													INVISIBLE_GADGET, NULL,
254, 105, 401, 117, 2, NULL, 0,													LOBOX_REGION, NULL,
  9, 121, 401, 133, 0, NULL, 0,													BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct PropInfo PI1 = { AUTOKNOB | FREEHORIZ | PROPBORDERLESS, 0,0,0,0, };
struct Image Im = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
struct Gadget PropSlider =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im, NULL, NULL, NULL, (struct PropInfo *)&PI1, 1, NULL
};

/******** E O F ********/
