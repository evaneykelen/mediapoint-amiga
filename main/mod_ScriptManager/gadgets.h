/**** template screen ****/

struct GadgetRecord SM1_GR[] =
{
  0,   0, 600, 154, 0, NULL,	0,												DIMENSIONS, NULL,	
  0,   0, 599, 153, 0, NULL,	0,												DBL_BORDER_REGION, NULL,	
  7,   1, 592,  10, 0, NULL, Msg_SM_1,									TEXT_REGION, NULL,
  7,  12, 592,  12, 0, NULL, 0,													LO_LINE, NULL,								
  8,  18, 591, 151, 2, NULL, NULL,											INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** choose between create run-time, find missing files etc. ****/

struct GadgetRecord SM2_GR[] =
{
 16,  21,  32,  29, 1, NULL, Msg_SM_2,									RADIO_GADGET, NULL,
 16,  33,  32,  41, 1, NULL, Msg_SM_3,									RADIO_GADGET, NULL,
 16,  45,  32,  53, 1, NULL, Msg_SM_4,									RADIO_GADGET, NULL,
 11, 136,  93, 149, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
506, 136, 588, 149, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** create run-time ****/

struct GadgetRecord SM3_GR[] =
{
 18,  21, 575,  30, 0, NULL, Msg_SM_5,									TEXT_LEFT, NULL,
114,  36, 498,  49, 1, NULL, Msg_SM_6,									HIBOX_REGION, NULL,
 16,  65,  32,  73, 1, NULL, Msg_SM_8,									RADIO_GADGET, NULL,
 16,  77,  32,  85, 1, NULL, Msg_SM_9,									RADIO_GADGET, NULL,
 11, 136,  93, 149, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
506, 136, 588, 149, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** look for missing files ****/

struct GadgetRecord SM4_GR[] =
{
 18,  21, 575,  30, 0, NULL, Msg_SM_10,									TEXT_LEFT, NULL,
114,  36, 498,  49, 1, NULL, Msg_SM_11,									HIBOX_REGION, NULL,
 16,  65, 278, 127, 1, NULL, NULL,											BUTTON_GADGET, NULL,
283,  65, 300, 127, 0, NULL, NULL,											BUTTON_GADGET, NULL,
 11, 136,  93, 149, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
506, 136, 588, 149, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** look for missing files ****/

struct GadgetRecord SM5_GR[] =
{
  8,  18, 591, 118, 2, NULL, NULL,											LOBOX_REGION, NULL,
259, 136, 341, 149, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  8, 122, 591, 131, 2, NULL, NULL,											LOBOX_REGION, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
