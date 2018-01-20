/******** GVR xapp gadgets ********/

enum {
GVR_FAST_REWIND, GVR_FAST_FORWARD, GVR_FIND, GVR_INITIALIZE, GVR_LOOP_ON,
GVR_LOOP_OFF, GVR_PAUSE, GVR_PLAY, GVR_PLAYTO, GVR_PLAYSEG, GVR_SCAN,
GVR_SCAN_REVERSE, GVR_SEEK, GVR_SLOW, GVR_STEP, GVR_STOP, GVR_WAIT
};

#ifndef GUI_DEFS
#define GUI_DEFS

// This list contains 0,1,2 depending on how many string gadgets should
// be enabled (start and end frame string gadgets).
// This list follows the list as listed above!!!!!

BYTE EnDisAbleList[] = {
0, 0, 1, 0, 0,
0, 0, 0, 1, 2, 0,
0, 0, 0, 0, 0, 0
};

struct StringRecord GVR_start_SR	     = { 11, "00:00:00:00 " };
struct StringRecord GVR_end_SR		= { 11, "00:00:00:00 " };
struct StringRecord GVR_cur_SR		= { 11, "00:00:00:00 " };     

TEXT GVR_CmdList[] = {
 "\
Fast rewind\0  \
Fast forward\0 \
Find\0         \
Initialize\0   \
Loop on\0      \
Loop off\0     \
Pause\0        \
Play\0         \
Play To\0      \
Play Segment\0 \
Scan\0         \
Scan reverse\0 \
Seek\0         \
Slow\0         \
Step\0         \
Stop\0         \
Wait\0" };

// Below numbers WERE 7,15,14																	/* active, 	num,	width */
struct CycleRecord GVR_cmds_CR = { 7,		17,		14, GVR_CmdList };

struct GadgetRecord GVR_GR[] =
{
  0,   0, 411, 188, NULL, 0,												DIMENSIONS, NULL,
  0,	 0,	410, 187, NULL, 0,												DBL_BORDER_REGION, NULL,
150,   3, 406,  12, "Sanyo GVR-S950   ", 0,  TEXT_RIGHT, NULL,
150,  14, 406,  23, "MediaPoint� XaPP�", 0,  TEXT_RIGHT, NULL,
  9, 171,  90, 183, "OK", 0,												BUTTON_GADGET, NULL,
100, 171, 181, 183, "Preview", 0,										BUTTON_GADGET, NULL,
320, 171, 401, 183, "Cancel",	0,										BUTTON_GADGET, NULL,
 21,  44,  37,  52, "Video", 0,											CHECK_GADGET, NULL,
 21,  56,  37,  64, "Audio", 0,											CHECK_GADGET, NULL,
 21,  98, 158, 110, "Controller", 0,								BUTTON_GADGET, NULL,
236,  44, 389,  57, "Action:", 0,										CYCLE_GADGET, (struct GadgetRecord *)&GVR_cmds_CR,
202,  67, 269,  79, "Start", 0,											BUTTON_GADGET, NULL,
202,  83, 269,  95, "End", 0,												BUTTON_GADGET, NULL,
285,  67, 389,  79, NULL, 0,												INTEGER_GADGET, (struct GadgetRecord *)&GVR_start_SR,
285,  83, 389,  95, NULL, 0,												INTEGER_GADGET, (struct GadgetRecord *)&GVR_end_SR,
 21, 118, 389, 118, NULL, 0,												LO_LINE, NULL,
 21, 127,  37, 135, "Blank screen after showing",0,	CHECK_GADGET, NULL,
 21, 141,  37, 149, "Initialize player before start",0,CHECK_GADGET, NULL,
-1	// ALWAYS END WITH THIS!!!!!!!!!!!!
};

struct GadgetRecord Controller_GR[] =
{
  0,   0, 251,  45, NULL,	0, DIMENSIONS, NULL,
  3,   2,  29,  14, NULL,	0, BUTTON_GADGET, NULL,
 33,   2,  59,  14, NULL,	0, BUTTON_GADGET, NULL,
 63,   2,  89,  14, NULL,	0, BUTTON_GADGET, NULL,
 93,   2, 119,  14, NULL,	0, BUTTON_GADGET, NULL,
123,   2, 149,  14, NULL,	0, BUTTON_GADGET, NULL,
153,   2, 179,  14, NULL,	0, BUTTON_GADGET, NULL,
183,   2, 209,  14, NULL,	0, BUTTON_GADGET, NULL,
213,   2, 239,  14, NULL,	0, BUTTON_GADGET, NULL,
  3,  17, 239,  28, NULL,	0, LOBOX_REGION, NULL,
-1
};

USHORT ImageDataController[] = {
	0x0000,0x003C,0x0000,0x00F0,0x0000,0x03C0,0x0000,0x0F00,
	0x0000,0x3C00,0x0000,0xF000,0x0003,0xC000,0x000F,0x3FFF,
	0xFFFC,0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,
	0xFCFF,0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,
	0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,
	0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,0xFFFF,
	0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,0xFFFF,
	0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,0xFFFF,0xFFF3,
	0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,0xFFFF,0xF3FF,
	0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,0xFFFF,0xFFF3,0xFFFF,
	0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,0xFFFF,0xF3FF,0xFFFF,
	0xCFFF,0xFFFF,0x3FFF,0xFFFC,0xFFFF,0xFFF3,0xFFFF,0xFFCF,
	0xFFFF,0xFF3F,0xFFFF,0xFCFF,0xFFFF,0xF3FF,0xFFFF,0xCFFF,
	0xFFFF,0x3FFF,0xFFFC,0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,
	0xFF3F,0xFFFF,0xFCFF,0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,
	0x3FFF,0xFFFC,0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,
	0xFFFF,0xFCFF,0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,
	0xFFFC,0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,
	0xFCFF,0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,
	0xFFFF,0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,
	0xFFFF,0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x3FFF,0xFFFC,0xFFFF,
	0xFFF3,0xFFFF,0xFFCF,0xFFFF,0xFF3F,0xFFFF,0xFCFF,0xFFFF,
	0xF3FF,0xFFFF,0xCFFF,0xFFFF,0x7FFF,0xFFFD,0xFFFF,0xFFF7,
	0xFFFF,0xFFDF,0xFFFF,0xFF7F,0xFFFF,0xFDFF,0xFFFF,0xF7FF,
	0xFFFF,0xDFFF,0xFFFF,0xFFFF,0xFFDF,0xFFFF,0xFF7F,0xFFFF,
	0xFDFF,0xFFFF,0xF7FF,0xFFFF,0xDFFF,0xFFFF,0x7FFF,0xFFFD,
	0xFFFF,0xFFF7,0xFFFF,0xFF9F,0xFFFF,0xFE7F,0xFFFF,0xF9FF,
	0xFFFF,0xE7FF,0xFFFF,0x9FFF,0xFFFE,0x7FFF,0xFFF9,0xFFFF,
	0xFFE7,0xFFFF,0xFF9F,0xFFFC,0xFE7F,0xF9FF,0xF9FF,0xFE70,
	0x67F8,0x39FF,0x9FFF,0x9FCE,0x7F3F,0x9FF9,0xFF83,0x83E7,
	0xFE00,0x0F9F,0xFFF0,0xFE7F,0xF87F,0xF9FF,0xF870,0x67F8,
	0x387F,0x9FFE,0x1F0E,0x7F0F,0x87F9,0xFF83,0x83E7,0xFE00,
	0x0F9F,0xFFC0,0xFE7F,0xF81F,0xF9FF,0xE070,0x67F8,0x381F,
	0x9FF8,0x1C0E,0x7F03,0x81F9,0xFF83,0x83E7,0xFE00,0x0F9F,
	0xFF00,0xFE7F,0xF807,0xF9FF,0x8070,0x67F8,0x3807,0x9FE0,
	0x100E,0x7F00,0x8079,0xFF83,0x83E7,0xFE00,0x0F9F,0xFC00,
	0xFE7F,0xF801,0xF9FE,0x0070,0x67F8,0x3801,0x9F80,0x000E,
	0x7F00,0x0019,0xFF83,0x83E7,0xFE00,0x0F9F,0xFF00,0xFE7F,
	0xF807,0xF9FF,0x8070,0x67F8,0x3807,0x9FE0,0x100E,0x7F00,
	0x8079,0xFF83,0x83E7,0xFE00,0x0F9F,0xFFC0,0xFE7F,0xF81F,
	0xF9FF,0xE070,0x67F8,0x381F,0x9FF8,0x1C0E,0x7F03,0x81F9,
	0xFF83,0x83E7,0xFE00,0x0F9F,0xFFF0,0xFE7F,0xF87F,0xF9FF,
	0xF870,0x67F8,0x387F,0x9FFE,0x1F0E,0x7F0F,0x87F9,0xFF83,
	0x83E7,0xFFFF,0xFF9F,0xFFFC,0xFE7F,0xF9FF,0xF9FF,0xFE70,
	0x67F8,0x39FF,0x9FFF,0x9FCE,0x7F3F,0x9FF9,0xFF83,0x83E7,
	0xFFFF,0xFF9F,0xFFFF,0xFE7F,0xFFFF,0xF9FF,0xFFFF,0xE7FF,
	0xFFFF,0x9FFF,0xFFFE,0x7FFF,0xFFF9,0xFFFF,0xFFE7,0x8000,
	0x001E,0x0000,0x0078,0x0000,0x01E0,0x0000,0x0780,0x0000,
	0x1E00,0x0000,0x7800,0x0001,0xE000,0x0007
};

struct Image ImageController = {
	0,0,
	240,13,
	2,
	ImageDataController,
	0x0003,0x0000,
	NULL
};

#endif

/******** E O F ********/