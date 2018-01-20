#include "nb:pre.h"
#include "fyedefs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"

/**** functions ****/

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct IV24_actions *IV24)
{
int action=-1;
int values[16];
TEXT path[SIZE_FULLPATH];

	sscanf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, "%d", &action);
	
	switch(action)
	{
		case ACTION_PIP:
			IV24->action = action;
			sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d %d %d %d %d",
							&values[0], &values[1], &values[2], &values[3], &values[4],
							&values[5], &values[6], &values[7] );
			IV24->pip_size					= values[1];
			IV24->pip_action				= values[2];
			IV24->pip_display_mode	= values[3];
			IV24->pip_close					= values[4];
			IV24->pip_x							= values[5];
			IV24->pip_y							= values[6];
			IV24->pip_speed					= values[7];
			break;

		case ACTION_FSV:
			IV24->action = action;
			sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d", &values[0], &values[1] );
			IV24->fsv_display_mode	= values[1];
			break;

		case ACTION_FB:
			IV24->action = action;
			sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %s %d", &values[0], path, &values[1] );
			ScriptToStr(path, IV24->fileName);
			RemoveQuotes(IV24->fileName);
			IV24->delay	= values[1];
			break;

		case ACTION_CP:
			IV24->action = action;
			sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d %d %d %d %d %d %d",
							&values[0], &values[1], &values[2], &values[3], &values[4],
							&values[5], &values[6], &values[7], &values[8], &values[9] );
			IV24->comp_output[0] 	= values[1];
			IV24->comp_output[1] 	= values[2];
			IV24->comp_output[2] 	= values[3];
			IV24->rgb_output[0]		= values[4];
			IV24->rgb_output[1]		= values[5];
			IV24->rgb_output[2]		= values[6];
			IV24->misc_output[0]	= values[7];
			IV24->misc_output[1]	= values[8];
			IV24->misc_output[2]	= values[9];
			break;

		case ACTION_CK:
			IV24->action = action;
			sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d",
							&values[0], &values[1], &values[2], &values[3] );
			IV24->comp_fade	= values[1];
			IV24->comp_from	= values[2];
			IV24->comp_to		= values[3];
			break;

		case ACTION_IPF:
		case ACTION_EPF:
		case ACTION_FA:
		case ACTION_UA:
			IV24->action = action;
			break;
	}
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, struct IV24_actions *IV24)
{
TEXT path[SIZE_FULLPATH];

	switch(IV24->action)
	{
		case ACTION_PIP:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d %d %d %d %d",
							IV24->action,
							IV24->pip_size,
							IV24->pip_action,
							IV24->pip_display_mode,
							IV24->pip_close,
							IV24->pip_x,
							IV24->pip_y,
							IV24->pip_speed);
			break;

		case ACTION_FSV:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d",
							IV24->action,
							IV24->fsv_display_mode);
			break;

		case ACTION_FB:
			StrToScript(IV24->fileName, path);
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d \\\"%s\\\" %d",
							IV24->action,
							path, IV24->delay);
			break;

		case ACTION_CP:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d %d %d %d %d %d %d",
							IV24->action,
							IV24->comp_output[0],
							IV24->comp_output[1],
							IV24->comp_output[2],
							IV24->rgb_output[0],
							IV24->rgb_output[1],
							IV24->rgb_output[2],
							IV24->misc_output[0],
							IV24->misc_output[1],
							IV24->misc_output[2] );
			break;

		case ACTION_CK:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d %d",
							IV24->action,
							IV24->comp_fade,
							IV24->comp_from,
							IV24->comp_to );
			break;

		case ACTION_IPF:
		case ACTION_EPF:
		case ACTION_FA:
		case ACTION_UA:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, "%d", IV24->action);
			break;
	}
}

/******** E O F ********/
