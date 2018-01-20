/*****************************************************
 * print
 *****************************************************/

struct StringRecord Print_Copies_SR = { 3, "    " };

struct GadgetRecord Print_GR[] =
{
  0,   0, 320, 147, 0, NULL,	0,												DIMENSIONS, NULL,	
  0,   0, 319, 146, 0, NULL,	0,												DBL_BORDER_REGION, NULL,	
 11,   2, 308,  12, 0, NULL,	0,												TEXT_REGION, NULL,
 10,  24, 310,  32, 0, NULL, Msg_Dest,									TEXT_LEFT, NULL,
 11,  38,  27,  46, 1, NULL, Msg_Printer1,							RADIO_GADGET, NULL,
 11,  50,  27,  58, 1, NULL, Msg_Printer2,							RADIO_GADGET, NULL,
 11,  62,  27,  70, 1, NULL, Msg_Printer3,							RADIO_GADGET, NULL,
 11,  74,  27,  82, 1, NULL, Msg_Printer4,							RADIO_GADGET, NULL,
 11,  88, 310,  97, 0, NULL, Msg_Options,								TEXT_LEFT, NULL,
 11, 101,  26, 109, 1, NULL, Msg_TextOnly,							CHECK_GADGET, NULL,
 86, 124, 125, 137, 1, NULL, Msg_Copies,								INTEGER_GADGET, (struct GadgetRecord *)&Print_Copies_SR,
228, 111, 310, 124, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
228, 129, 310, 142, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
 11,  13, 308,  13, 0, NULL,	0,												LO_LINE, NULL,
 11, 112,  26, 120, 1, NULL, Msg_MultipleFiles,					CHECK_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord PrintWindow_GR[] =
{
  0,   0, 320,  95, 0, NULL,	0,												DIMENSIONS, NULL,
  0,	 0,	319,  94, 0, NULL,	0,												DBL_BORDER_REGION, NULL,
230,  77, 312,  90, 0, NULL,	0,												BUTTON_GADGET, NULL,
-1
};

/******** E O F ********/
