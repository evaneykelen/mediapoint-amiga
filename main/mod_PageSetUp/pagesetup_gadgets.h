/*****************************************************
 * page setup
 *****************************************************/

struct CycleRecord PageSetup_Ori_CR = { 0, 4, 17, NULL, Msg_ScaleAndOri_List };

struct GadgetRecord PageSetup_GR[] =
{
  0,   0, 320, 147, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,   0, 319, 146, 0, NULL, 0,													DBL_BORDER_REGION, NULL,	
 11,   2, 308,  12, 0, NULL, Msg_PageSetup,							TEXT_REGION, NULL,
 10,  23, 217,  31, 0, NULL, Msg_ScaleAndOri,						TEXT_LEFT, NULL,
 11,  41, 189,  54, 1, NULL, 0,													CYCLE_GADGET, (struct GadgetRecord *)&PageSetup_Ori_CR,
222,  24, 285,  54, 2, NULL,	0,												LOBOX_REGION, NULL,
 10,  63, 274,  72, 0, NULL, Msg_Quality,								TEXT_LEFT, NULL,
 11,  79,  27,  87, 1, NULL, Msg_Draft,									RADIO_GADGET, NULL,
 11,  91,  27,  99, 1, NULL, Msg_Letter,								RADIO_GADGET, NULL,
228, 111, 310, 124, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
228, 129, 310, 142, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
 11,  13, 308,  13, 0, NULL,	0,												LO_LINE, NULL,
 11, 129, 218, 142, 0, NULL, Msg_GetWBPrefs,						BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
