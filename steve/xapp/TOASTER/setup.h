/******** Toaster xapp gadgets ********/

TEXT Toaster_CR1_List[] =
{ "Perform transition\0  Load framestore\0     Save framestore\0     " };
 /*----------------------++++++++++++++++++++++----------------------*/
struct CycleRecord Toaster_CR1_CR = { 0, 2, 20, Toaster_CR1_List };

TEXT Toaster_CR2_List[] =
{ "1\0    2\0    3\0    4\0    DV1\0  DV2\0  DV3\0  " };
 /*-------+++++++-------+++++++-------+++++++-------*/
struct CycleRecord Toaster_CR2_CR = { 0, 7, 6, Toaster_CR2_List };

TEXT Toaster_CR3_List[] =
{ "Slow\0 Med\0  Fast\0 " };
 /*-------+++++++-------*/
struct CycleRecord Toaster_CR3_CR = { 0, 3, 6, Toaster_CR3_List };

TEXT Toaster_CR4_List[] =
{ "#\0 A\0 B\0 C\0 D\0 E\0 F\0 G\0 H\0 I\0 " };
 /*----++++----++++----++++----++++----++++*/
struct CycleRecord Toaster_CR4_CR = { 0, 10, 3, Toaster_CR4_List };

TEXT Toaster_CR5_List[] =
{ "DV1\0  DV2\0  " };
 /*-------+++++++*/
struct CycleRecord Toaster_CR5_CR = { 0, 2, 6, Toaster_CR5_List };
struct CycleRecord Toaster_CR6_CR = { 0, 2, 6, Toaster_CR5_List };

// BUTTON AND STUFF COMMON TO ALL PAGES OF TOASTER XAPP

struct GadgetRecord Toaster_1_GR[] =
{
  0,   0, 582, 164, NULL, 0,												DIMENSIONS, NULL,
  0,	 0,	581, 163, NULL, 0,												DBL_BORDER_REGION, NULL,
150,   3, 577,  12, "Toaster controller", 0,				TEXT_RIGHT, NULL,
150,  14, 577,  23, "MediaPoint� XaPP�", 0,					TEXT_RIGHT, NULL,
  9, 146,  90, 159, "OK", 0,												BUTTON_GADGET, NULL,
100, 146, 181, 159, "Preview", 0,										BUTTON_GADGET, NULL,
491, 146, 572, 159, "Cancel",	0,										BUTTON_GADGET, NULL,
 10,  12, 273,  25, NULL,	0,												CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR1_CR,
 10,  29, 571, 134, NULL, 0,												LOBOX_REGION, NULL,
 13,  68, 566, 132, NULL, 0,												INVISIBLE_GADGET, NULL,
-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

/************************* PAGE 1 ******************************/

// BUTTON SET 1 OF PAGE 1

struct GadgetRecord Toaster_2_GR[] =
{
185,  32, 264,  45, "Preview source:", 0,						CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR2_CR,
185,  49, 264,  62, "Transition bank:",	0,					CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR4_CR,
448,  49, 555,  62, "Transition speed:", 0,					CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR3_CR,
-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

// SMALL BUTTONS OF PAGE 1

struct GadgetRecord Toaster_3_GR[] =
{
182,  79, 207,  89, NULL, 0,												BUTTON_GADGET, NULL,
212,  79, 237,  89, NULL, 0,												BUTTON_GADGET, NULL,
242,  79, 267,  89, NULL, 0,												BUTTON_GADGET, NULL,
272,  79, 297,  89, NULL, 0,												BUTTON_GADGET, NULL,
302,  79, 327,  89, NULL, 0,												BUTTON_GADGET, NULL,
332,  79, 357,  89, NULL, 0,												BUTTON_GADGET, NULL,
362,  79, 387,  89, NULL, 0,												BUTTON_GADGET, NULL,
392,  79, 417,  89, NULL, 0,												BUTTON_GADGET, NULL,

182,  92, 207, 102, NULL, 0,												BUTTON_GADGET, NULL,
212,  92, 237, 102, NULL, 0,												BUTTON_GADGET, NULL,
242,  92, 267, 102, NULL, 0,												BUTTON_GADGET, NULL,
272,  92, 297, 102, NULL, 0,												BUTTON_GADGET, NULL,
302,  92, 327, 102, NULL, 0,												BUTTON_GADGET, NULL,
332,  92, 357, 102, NULL, 0,												BUTTON_GADGET, NULL,
362,  92, 387, 102, NULL, 0,												BUTTON_GADGET, NULL,
392,  92, 417, 102, NULL, 0,												BUTTON_GADGET, NULL,

182, 105, 207, 115, NULL, 0,												BUTTON_GADGET, NULL,
212, 105, 237, 115, NULL, 0,												BUTTON_GADGET, NULL,
242, 105, 267, 115, NULL, 0,												BUTTON_GADGET, NULL,
272, 105, 297, 115, NULL, 0,												BUTTON_GADGET, NULL,
302, 105, 327, 115, NULL, 0,												BUTTON_GADGET, NULL,
332, 105, 357, 115, NULL, 0,												BUTTON_GADGET, NULL,
362, 105, 387, 115, NULL, 0,												BUTTON_GADGET, NULL,
392, 105, 417, 115, NULL, 0,												BUTTON_GADGET, NULL,

182, 118, 207, 128, NULL, 0,												BUTTON_GADGET, NULL,
212, 118, 237, 128, NULL, 0,												BUTTON_GADGET, NULL,
242, 118, 267, 128, NULL, 0,												BUTTON_GADGET, NULL,
272, 118, 297, 128, NULL, 0,												BUTTON_GADGET, NULL,
302, 118, 327, 128, NULL, 0,												BUTTON_GADGET, NULL,
332, 118, 357, 128, NULL, 0,												BUTTON_GADGET, NULL,
362, 118, 387, 128, NULL, 0,												BUTTON_GADGET, NULL,
392, 118, 417, 128, NULL, 0,												BUTTON_GADGET, NULL,

-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

// LARGE BUTTONS OF PAGE 2

struct GadgetRecord Toaster_4_GR[] =
{
 29,  87,  90, 128, NULL, 0,												BUTTON_GADGET, NULL,
 95,  87, 156, 128, NULL, 0,												BUTTON_GADGET, NULL,
161,  87, 222, 128, NULL, 0,												BUTTON_GADGET, NULL,
227,  87, 288, 128, NULL, 0,												BUTTON_GADGET, NULL,
293,  87, 354, 128, NULL, 0,												BUTTON_GADGET, NULL,
359,  87, 420, 128, NULL, 0,												BUTTON_GADGET, NULL,
425,  87, 486, 128, NULL, 0,												BUTTON_GADGET, NULL,
491,  87, 552, 128, NULL, 0,												BUTTON_GADGET, NULL,

-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

/************************* PAGE 2 ******************************/

// BUTTONS OF PAGE 2

//struct StringRecord Toaster_SR1 = {  3, "    " };
//struct StringRecord Toaster_SR1b = { 30, "                                " };

struct GadgetRecord Toaster_5_GR[] =
{
143,  35, 222,  48, "From:", 0,											CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR5_CR,
237,  52, 521,  65, NULL, 0,												LOBOX_REGION, NULL,
143,  52, 224,  65, "Save", 0,											BUTTON_GADGET, NULL,
/*143,  69, 208,  82, "F.S.#:", 0,										INTEGER_GADGET, (struct GadgetRecord *)&Toaster_SR1,*/
-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

/************************* PAGE 3 ******************************/

// BUTTONS OF PAGE 3

//struct StringRecord Toaster_SR2 = { 3, "    " };

//143,  35, 222,  48, "From:", 0,											CYCLE_GADGET, (struct GadgetRecord *)&Toaster_CR5_CR,

struct GadgetRecord Toaster_6_GR[] =
{
237,  35, 521,  48, NULL, 0,												LOBOX_REGION, NULL,
143,  35, 224,  48, "Load", 0,											BUTTON_GADGET, NULL,
-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

/* OBSOLETE
 66,  35, 147,  48, "Load", 0,											BUTTON_GADGET, NULL,
160,  35, 444,  48, NULL,	0,												LOBOX_REGION, NULL,
 66,  52, 131,  65, "F.S.#:", 0,										INTEGER_GADGET, (struct GadgetRecord *)&Toaster_SR2,
*/

/************************* IMAGES ******************************/

/*----- bitmap : w = 528, h = 42 ------ */
/*------ plane # 0: --------*/
UWORD plane0[1386] = { 
	0x0000,0x0000,0x0000,0x0007,0xc000,0x0000,0x0000,0x0001,0xf000,0x0000,0x0000,0x0000,0x7c00,0x0000,0x0000,0x0000,0x1f00,0x0000,0x0000,0x0000,0x07c0,0x0000,0x0000,0x0000,0x01f0,0x0000,0x0000,0x0000,0x007c,0x0000,0x0000,0x0000,0x001f,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,
	0x7fff,0xffff,0xffff,0xffff,0xdfff,0xffff,0xffff,0xffff,0xf7ff,0xffff,0xffff,0xffff,0xfdff,0xffff,0xffff,0xffff,0xff7f,0xffff,0xffff,0xffff,0xffdf,0xffff,0xffff,0xffff,0xfff7,0xffff,0xffff,0xffff,0xfffd,0xffff,0xffff,0xffff,0xffff,
	};
/*------ plane # 1: --------*/
UWORD plane1[1386] = { 
	0xffff,0xffff,0xffff,0xfffb,0xffff,0xffff,0xffff,0xfffe,0xffff,0xffff,0xffff,0xffff,0xbfff,0xffff,0xffff,0xffff,0xefff,0xffff,0xffff,0xffff,0xfbff,0xffff,0xffff,0xffff,0xfeff,0xffff,0xffff,0xffff,0xffbf,0xffff,0xffff,0xffff,0xffef,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffc0,0xffff,0xff3f,0xffe0,0x3fff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xfffc,0x0b47,0xffff,0xf3ff,0xffff,0xf01f,0xffff,0xfcff,0xffff,0xffc0,0xffff,0xff3f,0xffe0,0x3fff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xe000,0xffff,0x3fff,0xff80,0x03ff,0xffff,0xcfff,0xfffd,0xfff7,0xffff,0xf3ff,0xffff,0xf81f,0xffff,0xfcff,0xffff,0xffc0,0xffff,0xff3f,0xffe0,0x3fff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0x9ffe,0xffff,0x3fff,0xffbf,0xfdff,0xffff,0xcfff,0xffff,0xc077,0xffff,0xf3ff,0xffff,0xfc3f,0xffff,0xfcff,0xffff,0xffe1,0xffff,0xff3f,0xffe0,0x3fff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xbc7f,0xffff,0x3fff,0xfffc,0x0eff,0xffff,0xcfff,0xffff,0x803f,0xffff,0xf3ff,0xffff,0xfc3f,0xffff,0xfcff,0xffff,0xffff,0xcfff,0xff3f,0xffe0,0x0fff,0xffff,0xffcf,
	0xfff8,0x001f,0xffff,0xfff3,0xffff,0xffff,0xc001,0xfffc,0xffff,0xfffe,0x600d,0xffff,0x3fff,0xffd8,0x037f,0xffff,0xcfff,0xffff,0x803b,0xffff,0xf3ff,0xffff,0xfc7f,0xffff,0xfcff,0xffff,0xffff,0xfe1f,0xff3f,0xffe0,0x0fff,0xffff,0xffcf,
	0xfff9,0xffdf,0xffff,0xfff3,0xffff,0xffff,0xfffd,0xfffc,0xffff,0xfffd,0xe009,0xffff,0x3fff,0xffec,0x07bf,0xffff,0xcfff,0xfffb,0x803b,0xffff,0xf3ff,0xffff,0xfc7f,0xffff,0xfcff,0xffff,0xffff,0xff8f,0xff3f,0xffe0,0x0fff,0xffff,0xffcf,
	0xfffb,0x83df,0xffff,0xfff3,0xffff,0xfff7,0xd804,0xfffc,0xffff,0xfffb,0xfabf,0xffff,0x3fff,0xfff6,0x0f9f,0xffff,0xcfff,0xfffb,0xe17b,0xffff,0xf3ff,0xffff,0xfe7f,0xffff,0xfcff,0xffff,0xffff,0xff87,0xff3f,0xffe0,0x1fe3,0xffff,0xffcf,
	0xfffb,0x00ff,0xffff,0xfff3,0xffff,0xfff1,0xd806,0xfffc,0xffff,0xfff8,0x17f7,0xffff,0x3fff,0xfff7,0xfc1f,0xffff,0xcfff,0xfffa,0xe141,0xffff,0xf3ff,0xffff,0xfe7f,0xffff,0xfcff,0xffff,0xffff,0xf00f,0xff3f,0xffe0,0x1fc3,0xffff,0xffcf,
	0xffff,0x009f,0xfffb,0xfff3,0xffff,0xfff8,0xd806,0xfffc,0xffff,0xffff,0x02f7,0xffff,0x3fff,0xfff8,0x3bff,0xffff,0xcfff,0xfff8,0x0003,0xffff,0xf3ff,0xffff,0xff7f,0xffff,0xfcff,0xffff,0xfff8,0x001f,0xff3f,0xffe0,0x3f00,0x007f,0xffcf,
	0xfff7,0x00b0,0x2ff9,0xfff3,0xffff,0xfd00,0xc80e,0xfffc,0xffff,0xffff,0xffcf,0xffff,0x3fff,0xfff9,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xf007,0xffff,0xfcff,0xffff,0xf400,0x03ff,0xff3f,0xffe0,0x3f00,0x001f,0xffcf,
	0xfff7,0x00b0,0x0071,0xfff3,0xfffd,0x4008,0xc806,0xfffc,0xffff,0xffff,0xefff,0xffff,0x3fff,0xffff,0xf07f,0xffff,0xcfff,0xffff,0xfbff,0xffff,0xf3ff,0xffff,0xf817,0xffff,0xfcff,0xffff,0x8000,0x7fff,0xff3f,0xffe0,0xffe3,0xff83,0xffcf,
	0xfff1,0x7fbf,0xc001,0xfff3,0xfffc,0x01f9,0xcfe4,0xfffc,0xffff,0xffff,0x1fff,0xffff,0x3fff,0xffff,0xf03f,0xffff,0xcfff,0xffff,0xfbff,0xffff,0xf3ff,0xffff,0xff7f,0xffff,0xfcff,0xfffc,0x01ff,0xffff,0xff3f,0xffe0,0xfff3,0xffc1,0xffcf,
	0xfff0,0x173f,0xfc01,0xfff3,0xfffc,0x07f9,0xee03,0xfffc,0xffff,0xfffe,0x3fff,0xffff,0x3fff,0xffff,0xfeff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xfff8,0x07ff,0xffff,0xff3f,0xffe1,0xffff,0xfff8,0x7fcf,
	0xffff,0xfa3f,0xff00,0xfff3,0xfffc,0x1ffb,0xe3ff,0xfffc,0xffff,0xfffc,0x3fff,0xffff,0x3fff,0xffff,0xff7f,0xffff,0xcfff,0xffff,0xf9ff,0xffff,0xf3ff,0xffff,0x0000,0x7fff,0xfcff,0xfff8,0x1fbf,0xffff,0xff3f,0xffef,0xffff,0xfe0c,0x3fcf,
	0xffff,0xffff,0xffe0,0xfff3,0xfffe,0xbfff,0xffff,0xfffc,0xffff,0xfff8,0x7fff,0xffff,0x3fff,0xffff,0xff7f,0xffff,0xcfff,0xffff,0xf8ff,0xffff,0xf3ff,0xffff,0x3ebf,0x7fff,0xfcff,0xfff8,0x1f1f,0xffff,0xff3f,0xffff,0xffff,0xfc00,0x3fcf,
	0xffff,0xffff,0xffe1,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xfff8,0x7fff,0xffff,0x3fff,0xffff,0xff9f,0xffff,0xcfff,0xffff,0xf8ff,0xffff,0xf3ff,0xffff,0x7d7f,0xffff,0xfcff,0xfff8,0x1f1f,0xffff,0xff3f,0xffff,0xffff,0xc078,0x7fcf,
	0xffff,0xffff,0xffe3,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xfff0,0x7fff,0xffff,0x3fff,0xffff,0xff9f,0xffff,0xcfff,0xffff,0xf0ff,0xffff,0xf3ff,0xffff,0x7007,0xffff,0xfcff,0xfff8,0x001f,0xffff,0xff3f,0xffff,0xffff,0xc1ff,0xffcf,
	0xffff,0xffff,0xffe7,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xfff0,0x7fff,0xffff,0x3fff,0xffff,0xff87,0xffff,0xcfff,0xffff,0xe0ff,0xffff,0xf3ff,0xffff,0xf007,0xffff,0xfcff,0xfffc,0x001f,0xffff,0xff3f,0xffff,0xffff,0x07ff,0xffcf,
	0xffff,0xffff,0xffef,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffe0,0x7fff,0xffff,0x3fff,0xffff,0xffc7,0xffff,0xcfff,0xffff,0xe07f,0xffff,0xf3ff,0xffff,0xb006,0xffff,0xfcff,0xffff,0x001f,0xffff,0xff3f,0xffff,0xffff,0x0fff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffe0,0xffff,0xffff,0x3fff,0xffff,0xffc3,0xffff,0xcfff,0xffff,0x0007,0xffff,0xf3ff,0xffff,0xb81e,0xffff,0xfcff,0xffff,0xc01f,0xffff,0xff3f,0xffff,0xffff,0x1fff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xfe00,0x1fff,0xffff,0x3fff,0xffff,0xffc3,0xffff,0xcfff,0xffff,0x801f,0xffff,0xf3ff,0xffff,0x9ffe,0xffff,0xfcff,0xffff,0xc1ff,0xffff,0xff3f,0xffff,0xffff,0x1fff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xff80,0x7fff,0xffff,0x3fff,0xffff,0xffc1,0xffff,0xcfff,0xffff,0xe0ff,0xffff,0xf3ff,0xffff,0x8000,0xffff,0xfcff,0xffff,0xcfff,0xffff,0xff3f,0xffff,0xffff,0x0fff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffc0,0x7fff,0xffff,0x3fff,0xffff,0xffc1,0xffff,0xcfff,0xffff,0xf8ff,0xffff,0xf3ff,0xffff,0xd54f,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0xffff,0xffff,0xffff,0xfff3,0xffff,0xffff,0xffff,0xfffc,0xffff,0xffff,0xffff,0xffff,0x3fff,0xffff,0xffff,0xffff,0xcfff,0xffff,0xffff,0xffff,0xf3ff,0xffff,0xffff,0xffff,0xfcff,0xffff,0xffff,0xffff,0xff3f,0xffff,0xffff,0xffff,0xffcf,
	0x8000,0x0000,0x0000,0x0003,0xe000,0x0000,0x0000,0x0000,0xf800,0x0000,0x0000,0x0000,0x3e00,0x0000,0x0000,0x0000,0x0f80,0x0000,0x0000,0x0000,0x03e0,0x0000,0x0000,0x0000,0x00f8,0x0000,0x0000,0x0000,0x003e,0x0000,0x0000,0x0000,0x000f,
	};

/******** E O F ********/