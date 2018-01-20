/******** IV-24 (fye) globals *******/

#define ACTION_PIP						1	// PIP
#define ACTION_FSV						2	// full screen video
#define ACTION_FB							3	// frame buffer
#define ACTION_CP							4	// control panel
#define ACTION_CK							5	// composite keyer
#define ACTION_IPF						6	// internal pip freeze
#define ACTION_EPF						7	// external pip freeze
#define ACTION_FA							8	// freeze all
#define ACTION_UA							9	// unfreeze all

#define PIP_ACTION_PLACE			0
#define PIP_ACTION_MOVEDOWN		1
#define PIP_ACTION_MOVEUP			2

#define SCAN_HISCAN						1
#define SCAN_VIDEOSCAN				2

#define COLORS_16M						1
#define COLORS_4096						2

#define COMPFADE_AMIGA				1
#define COMPFADE_EXT					2
#define COMPFADE_CROSS				3

#define SET_EXTERN						1
#define SET_INTERN						2

#define SET_POSITIVE					1
#define SET_NEGATIVE					2

#define MODE_AMIGA						1
#define MODE_KEYER						2
#define MODE_EXTERN						3

struct IV24_actions
{
	int action;

	/* PIP */

	int pip_size;
	int pip_action;
	int pip_display_mode;
	BOOL pip_close;
	int pip_x, pip_y;
	int pip_speed;

	/* FSV */

	int fsv_display_mode;

	/* Framebuffer */

	TEXT fileName[SIZE_FULLPATH];
	int delay;

	/* Control panel */

	int comp_output[3];
	int rgb_output[3];
	int misc_output[3];

	/* Composite keyer */

	int comp_fade;
	int comp_from, comp_to;
};	

/******** E O F ********/
