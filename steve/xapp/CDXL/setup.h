/******** CDXL xapp gadgets ********/

struct CycleRecord CDXL_FPS_CR		= { 30, 50, 6, NULL, 0 };
struct CycleRecord CDXL_Loops_CR	= {  0, 31, 4, NULL, Msg_Infinite_30 };

struct GadgetRecord CDXL_GR[] =
{
  0,   0, 640,  60, 0, NULL,	0,												DIMENSIONS, NULL,	
  0,	 0,	639,  59, 0, NULL,	0,												DBL_BORDER_REGION, NULL,
 60,   3, 633,  16, 1, NULL,	0,												BUTTON_GADGET, NULL,
450,  42, 532,  55, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
551,  42, 633,  55, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
 60,  42, 171,  55, 0, NULL, Msg_CDXL_1,								INVISIBLE_GADGET, NULL,
 60,  19,  76,  27, 1, NULL, Msg_CDXL_2,								RADIO_GADGET, NULL,
 60,  29,  76,  37, 1, NULL, Msg_CDXL_3,								RADIO_GADGET, NULL,
259,  19, 325,  32, 1, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&CDXL_FPS_CR,
405,  19, 462,  32, 1, NULL, Msg_CDXL_4,								CYCLE_GADGET, (struct GadgetRecord *)&CDXL_Loops_CR,
478,  19, 493,  27, 1, NULL, Msg_CDXL_5,								CHECK_GADGET, NULL,
478,  29, 493,  37, 1, NULL, Msg_CDXL_7,								CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
