/******** SER xapp gadgets ********/

struct StringRecord Demo_AC_SR = { 40, "                                         " };
struct StringRecord Demo_PO_SR = { 40, "                                         " };
struct StringRecord Demo_ST_SR = { 40, "                                         " };
struct StringRecord Demo_BR_SR = {  4, "     " };

TEXT BaudRateList[] = "1200\0 2400\0 4800\0 9600\0";
struct CycleRecord Demo_BR_CR = { 0, 4, 6, BaudRateList, NULL };

struct GadgetRecord Demo_GR[] =
{
  0,   0, 411, 200, 0, NULL, 0,									DIMENSIONS, NULL,
  0,	 0,	410, 199, 0, NULL, 0,									DBL_BORDER_REGION, NULL,
  9, 182,  90, 195, 0, "OK", 0,									BUTTON_GADGET, NULL,
100, 182, 181, 195, 0, "Preview", 0,						BUTTON_GADGET, NULL,
320, 182, 401, 195, 0, "Cancel", 0,							BUTTON_GADGET, NULL,
  7, 177, 401, 177, 0, NULL, 0,									LO_LINE, NULL, 
155,  29, 364,  42, 1, "ARexx command:", 0,			STRING_GADGET, (struct GadgetRecord *)&Demo_AC_SR,
155,  46, 364,  59, 1, "Port:", 0,							STRING_GADGET, (struct GadgetRecord *)&Demo_PO_SR,
  7,  62, 401,  62, 0, NULL, 0,									DOTTED_LINE, NULL, 
  9,  81, 271,  94, 1, NULL, 0,									STRING_GADGET, (struct GadgetRecord *)&Demo_ST_SR,
285,  81, 403,  94, 1, NULL, 0,									BUTTON_GADGET, NULL,
285,  97, 403, 110, 1, NULL, 0,									CYCLE_GADGET, (struct GadgetRecord *)&Demo_BR_CR,
285, 113, 403, 126, 1, NULL, 0,									INTEGER_GADGET, (struct GadgetRecord *)&Demo_BR_SR,
  9, 100,  25, 108, 1, "AM", 0,									RADIO_GADGET, NULL,
  9, 112,  25, 120, 1, "FM", 0,									RADIO_GADGET, NULL,
 61, 103, 119, 116, 2, NULL, 0,									LOBOX_REGION, NULL,
128, 103, 186, 116, 2, NULL, 0,									HIBOX_REGION, NULL,
  9, 128,  25, 136, 1, "Stereo", 0,							CHECK_GADGET, NULL,
  9, 140,  25, 148, 1, "Dolby", 0,							CHECK_GADGET, NULL,
122, 135, 203, 148, 1, "Load", 0,								BUTTON_GADGET, NULL,
211, 135, 292, 148, 1, "Save", 0,								BUTTON_GADGET, NULL,
304, 135, 348, 148, 1, "1", 0,									BUTTON_GADGET, NULL,
360, 135, 403, 148, 1, "2", 0,									BUTTON_GADGET, NULL,
  9, 156, 403, 169, 2, NULL, 0,									LOBOX_REGION, NULL,
142,   3, 406,  12, 0, "Demo Xapp", 0, 					TEXT_RIGHT, NULL,
142,  14, 406,  23, 0, "MediaPoint› XaPP›", 0,	TEXT_RIGHT, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord PopUp_GR[] =
{
  0,   0,  58,   0, 0, NULL, 0,							DIMENSIONS, NULL,
  0,	 0,	  0,   0, 1, NULL, 0,							HIBOX_REGION, NULL,
  0,   0,   0,   0, 0, NULL, 0,							POSPREFS, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,
-1
};

/******** E O F ********/
