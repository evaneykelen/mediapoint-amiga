#define STP (char *)	// scalar to pointer

struct GadgetRecord SM_Std_GR[] =
{
  0,   0, 640, 134, NULL,										DIMENSIONS, NULL,	
  0,   0, 639, 133, NULL,										DBL_BORDER_REGION, NULL,	
  8,   3, 631,  12, "Script Manager",				TEXT_REGION, NULL,
  8,  14, 631,  14, NULL,										LO_LINE, NULL,
  5,  16, 634, 114, NULL,										INVISIBLE_GADGET, NULL,
  4, 114,  36, 127, NULL,										INVISIBLE_GADGET, NULL,
549, 116, 630, 129, "Exit",									BUTTON_GADGET, NULL,
-1
};

// 0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'

struct GadgetRecord SM_1_GR[] =
{
203,  27, 436,  38, "Check script",					BUTTON_GADGET, NULL,
203,  44, 436,  55, "Create run-time disks",BUTTON_GADGET, NULL,
-1
};

// 0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'

struct GadgetRecord SM_2_GR[] =
{
  8,  20, 149,  29, "Search on:",						TEXT_LEFT, NULL,	
  9,  32,  23,  38, "                 ",		CHECK_GADGET, NULL,
  9,  44,  23,  50, "                 ",		CHECK_GADGET, NULL,
  9,  56,  23,  62, "                 ",		CHECK_GADGET, NULL,
  9,  68,  23,  74, "                 ",		CHECK_GADGET, NULL,
  9,  80,  23,  86, "                 ",		CHECK_GADGET, NULL,
  9,  92,  23,  98, "                 ",		CHECK_GADGET, NULL,
  9, 104,  23, 110, "                 ",		CHECK_GADGET, NULL,
  9, 116,  23, 122, "                 ",		CHECK_GADGET, NULL,
157,  25, 630,  95, NULL,                   LOBOX_REGION, NULL,
353,  79, 434,  91, "Scan",									BUTTON_GADGET, NULL,
157,  25, 630,  76, NULL,                   TEXT_REGION, NULL,
-1
};

// 0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'

struct GadgetRecord SM_3_GR[] =
{
 21,  20, 215,  29, "Create run-time on:",	TEXT_LEFT, NULL,	
247,  20, 491,  29, "Copy:",								TEXT_LEFT, NULL,	
 22,  31,  38,  39, "Floppy",								RADIO_GADGET, NULL,
 22,  43,  38,  51, "Other device",					RADIO_GADGET, NULL,
248,  33, 262,  39, "Fonts",								CHECK_GADGET, NULL,
248,  45, 262,  51, "Player",								CHECK_GADGET, NULL,
248,  57, 262,  63, "Amiga libraries/devices",CHECK_GADGET, NULL,
 22,  69, 146,  81, "Start",								BUTTON_GADGET, NULL,
-1
};

// 0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'

struct GadgetRecord SM_4_GR[] =
{
  8,  20, 631, 105, NULL,                   LOBOX_REGION, NULL,
458, 116, 539, 129, "Abort",								BUTTON_GADGET, NULL,
-1
};

// 0,0,0,0,NULL,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'

/******** E O F ********/
