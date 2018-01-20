/******** LightPen xapp gadgets ********/

struct StringRecord A1_X_SR = { 4, "     " };
struct StringRecord A1_Y_SR = { 4, "     " };

struct GadgetRecord Light_GR[] =
{
  0,   0, 326, 129, 0, NULL, 0,									DIMENSIONS, NULL,	
  0,	 0,	325, 128, 0, NULL, 0,									DBL_BORDER_REGION, NULL,
  9,  32, 317,  98, 2, NULL, 0,									LOBOX_REGION, NULL,
	7, 106, 318, 106, 0, NULL, 0,									LO_LINE, NULL, 
168,   3, 321,  12, 0, "Lightpen Control", 0,		TEXT_RIGHT, NULL,
138,  14, 321,  23, 0, NULL, Msg_X_1,						TEXT_RIGHT, NULL,
 20,  38, 197,  48, 0, "Position lightpen and", 0,TEXT_REGION, NULL,
 20,  52, 197,  62, 0, "use the cursor keys", 0,	TEXT_REGION, NULL,
 20,  66, 197,  76, 0, "to change the offset", 0,	TEXT_REGION, NULL,
243,  38, 320,  48, 0, "Offset:", 0,						TEXT_LEFT, NULL,
243,  50, 294,  63, 1, NULL, Msg_IV_1_7,				INTEGER_GADGET, (struct GadgetRecord *)&A1_X_SR,
243,  68, 294,  81, 1, NULL, Msg_IV_1_8,				INTEGER_GADGET, (struct GadgetRecord *)&A1_Y_SR,
  7, 110,  92, 124, 0, NULL, Msg_OK,						BUTTON_GADGET, NULL,
238, 110, 318, 124, 0, NULL, Msg_Cancel,				BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/******** E O F ********/

