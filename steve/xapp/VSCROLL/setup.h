/******** Vscroll xapp gadgets ********/

struct CycleRecord VSC_bgnd_CR		= { 4, 16, 10, NULL, Msg_VSC_ColorsList };
struct CycleRecord VSC_text_CR		= { 1, 16, 10, NULL, Msg_VSC_ColorsList };
struct CycleRecord VSC_shad_CR		= { 0, 16, 10, NULL, Msg_VSC_ColorsList };

struct CycleRecord VSC_speed_CR		= { 0, 30,  4, NULL, Msg_Numbers_1_40 };
struct CycleRecord VSC_weight_CR	= { 0, 16,  4, NULL, Msg_Numbers_0_15 };
struct CycleRecord VSC_stype_CR		= { 0,  2, 10, NULL, Msg_VSC_SType };
struct CycleRecord VSC_lspc_CR		= { 0, 40,  4, NULL, Msg_Numbers_1_40 };

struct GadgetRecord VSC_GR[] =
{
  0,   0, 521, 200, 0, NULL, 0,													DIMENSIONS, NULL,	
  0,	 0,	520, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  9, 182,  90, 195, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
430, 182, 511, 195, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
100, 182, 181, 195, 0, NULL, Msg_X_2,										BUTTON_GADGET, NULL,
100,   3, 507,  12, 0, NULL, Msg_VSC_Title,				 			TEXT_RIGHT, NULL,
100,  14, 507,  23, 0, NULL, Msg_X_1,										TEXT_RIGHT, NULL,
  7, 177, 511, 177, 0, NULL, 0,													LO_LINE, NULL, 
 17,   8,  98,  21, 0, NULL, Msg_LoadPalette,						BUTTON_GADGET, NULL,
112,   8, 193,  21, 0, NULL, Msg_SavePalette,						BUTTON_GADGET, NULL,
 17,  31, 503, 110, 2, NULL, 0,													LOBOX_REGION, NULL,
 69, 115, 294, 128, 1, NULL, Msg_CrawlFont,							HIBOX_REGION, NULL,
140, 130, 247, 143, 1, NULL, Msg_VSC_Bgnd,							CYCLE_GADGET, (struct GadgetRecord *)&VSC_bgnd_CR,
140, 145, 247, 158, 1, NULL, Msg_VSC_Text,							CYCLE_GADGET, (struct GadgetRecord *)&VSC_text_CR,
140, 160, 247, 173, 1, NULL, Msg_WDefShadow,						CYCLE_GADGET, (struct GadgetRecord *)&VSC_shad_CR,
381, 115, 488, 128, 1, NULL, Msg_Speed,									CYCLE_GADGET, (struct GadgetRecord *)&VSC_speed_CR,
381, 145, 488, 158, 1, NULL, Msg_VSC_Weight,						CYCLE_GADGET, (struct GadgetRecord *)&VSC_weight_CR,
381, 160, 488, 173, 1, NULL, Msg_VSC_Type,							CYCLE_GADGET, (struct GadgetRecord *)&VSC_stype_CR,
231,   8, 257,  21, 0, "?",  0,													BUTTON_GADGET, NULL,
272,   8, 298,  21, 0, NULL, Msg_Char_Cross,						BUTTON_GADGET, NULL,
381, 130, 488, 143, 1, NULL, Msg_Style_LSpc_List,				CYCLE_GADGET, (struct GadgetRecord *)&VSC_lspc_CR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/*****************************************************
 * font selector
 *****************************************************/

struct StringRecord FontSelect_SR = { 3, "    " };

struct GadgetRecord FontSelect_GR[] =
{
  0,   0, 320, 172, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 171, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   3, 224,  60, 1, NULL, 0,													BUTTON_GADGET, NULL,	// name scroll area
227,   3, 245,  60, 0, NULL, 0,													HIBOX_REGION, NULL,		// prop gad
252,   3, 291,  45, 1, NULL, 0,													BUTTON_GADGET, NULL,	// size scroll area
294,   3, 312,  45, 0, NULL, 0,													HIBOX_REGION, NULL,		// prop gad
252,  47, 291,  60, 0, NULL, 0,													INTEGER_GADGET, (struct GadgetRecord *)&FontSelect_SR,
  7,  63, 312, 149, 0, NULL, 0,													BUTTON_GADGET, NULL,		// sample
  7, 154,  89, 167, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
230, 154, 312, 167, 0, NULL, Msg_Cancel,								BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
