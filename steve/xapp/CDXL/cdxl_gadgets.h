/******** CDXL xapp gadgets ********/

#define STP (char *)	// scalar to pointer

struct CycleRecord CDXL_FPS_CR		= { 30, 50, 6, NULL };
struct CycleRecord CDXL_Loops_CR	= {  0, 31, 4, STP Msg_Infinite_30 };

struct GadgetRecord CDXL_GR[] =
{
  0,   0, 640,  60, NULL,											DIMENSIONS, NULL,	
  0,	 0,	639,  59, NULL,											DBL_BORDER_REGION, NULL,
 60,   3, 633,  16, NULL,											BUTTON_GADGET, NULL,
450,  42, 532,  55, STP Msg_OK,								BUTTON_GADGET, NULL,
551,  42, 633,  55, STP Msg_Cancel,						BUTTON_GADGET, NULL,
 60,  42, 171,  55, STP Msg_CDXL_1,						BUTTON_GADGET, NULL,
 60,  19,  76,  27, STP Msg_CDXL_2,						RADIO_GADGET, NULL,
 60,  29,  76,  37, STP Msg_CDXL_3,						RADIO_GADGET, NULL,
259,  19, 337,  32, NULL,											CYCLE_GADGET, (struct GadgetRecord *)&CDXL_FPS_CR,
405,  19, 462,  32, STP Msg_CDXL_4,						CYCLE_GADGET, (struct GadgetRecord *)&CDXL_Loops_CR,
478,  19, 493,  27, STP Msg_CDXL_5,						CHECK_GADGET, NULL,
478,  29, 493,  37, STP Msg_CDXL_7,						CHECK_GADGET, NULL,
0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct StringRecord FR_SR = { 40, "                                         " };

struct GadgetRecord FileRequester_GR[] =
{
  0,   0, 317, 134, NULL,										DIMENSIONS, NULL,
  6,  13, 235,  26, NULL,										SPECIAL_STRING_GADGET, (struct GadgetRecord *)&FR_SR,
  6,  28, 215, 131, NULL,										BUTTON_GADGET, NULL,
241,  13, 311,  26, "Parent",								BUTTON_GADGET, NULL,
241,  63, 311,  76, "Disks",								BUTTON_GADGET, NULL,
241,  79, 311,  92, "Assigns",							BUTTON_GADGET, NULL,
241, 100, 311, 113, "Cancel",								BUTTON_GADGET, NULL,
241, 118, 311, 131, "Open",									BUTTON_GADGET, NULL,
218,  28, 235, 131, NULL,										HIBOX_REGION, NULL,
  0,	 0,	316, 133, NULL,										DBL_BORDER_REGION, NULL,
241,  96, 311,  96, NULL,										LO_LINE, NULL,
254,  36, 276,  47, "ç",										BUTTON_GADGET, NULL,	/* select all */
281,  36, 303,  47, "é",										BUTTON_GADGET, NULL,	/* thumbnails */
  5,   2, 312,  11, NULL,										TEXT_REGION, NULL,		/* 'Select a file' */
-1
};

/******** E O F ********/
