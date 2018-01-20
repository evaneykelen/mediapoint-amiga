STATIC TEXT Commands[]	= { "Play\0    Stop\0    Rewind\0  " };
													 /*----------++++++++++----------*/
STATIC TEXT Unit[]			= { "All\0 1\0   2\0   3\0   4\0   5\0   6\0   7\0   8\0   " };
													 /*++++++------++++++------++++++------++++++------++++++*/

struct CycleRecord VP_Commands_CR = { 0, 3, 9, (UBYTE *)Commands, 0 };
struct CycleRecord VP_Unit_CR = { 0, 9, 5, (UBYTE *)Unit, 0 };

struct GadgetRecord VP_GR[] =
{
  0,   0, 411, 140, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	410, 139, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
137,  38, 362,  51, 1, "Command:", 0,										CYCLE_GADGET, (struct GadgetRecord *)&VP_Commands_CR,
137,  55, 221,  68, 1, "Unit:", 0,											CYCLE_GADGET, (struct GadgetRecord *)&VP_Unit_CR,
  9, 122,  90, 135, 0, "OK", 0,													BUTTON_GADGET, NULL,
320, 122, 401, 135, 0, "Cancel", 0,											BUTTON_GADGET, NULL,
100, 122, 181, 135, 0, "Preview", 0,										BUTTON_GADGET, NULL,
  7, 117, 401, 117, 0, NULL, 0,													LO_LINE, NULL, 
142,   3, 406,  12, 0, "Selectra VuPort", 0,			 			TEXT_RIGHT, NULL,
142,  14, 406,  23, 0, "MediaPoint› XaPP›", 0,					TEXT_RIGHT, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/
