#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"

/**** externals ****/

extern struct GadgetRecord CDTV_GR[];
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** ControlCDTV() ********/

BOOL ControlCDTV(struct CDTV_record *CDTV_rec)
{
BOOL retVal=TRUE;

	switch(CDTV_rec->action)
	{
		case 0:	// DO_PLAYTRACK
			retVal = CDTV_PlayTrack(CDTV_rec);
			break;
		case 1: // DO_PLAYFROMTO
			retVal = CDTV_PlayTrackFromTo(CDTV_rec);
			break;
		case 2: // DO_PLAYSTARTEND
			CDTV_PlayTrackStartEnd(CDTV_rec);
			break;
		case 3:
			switch(CDTV_rec->command)
			{
				case 0:	// DO_FADE
					retVal = CDTV_Fade(CDTV_rec, 1, 2);	// slow fade out
					break;

				case 1:	// DO_FADE
					retVal = CDTV_Fade(CDTV_rec, 2, 2);	// fast fade out
					break;

				case 2:	// DO_FRONTPANEL
					retVal = CDTV_FrontPanel(CDTV_rec, 1);	// on
					break;

				case 3:	// DO_FRONTPANEL
					retVal = CDTV_FrontPanel(CDTV_rec, 2);	// off
					break;

				case 4:	// DO_MUTE
					retVal = CDTV_Mute(CDTV_rec, 1);	// on
					break;

				case 5:	// DO_MUTE
					retVal = CDTV_Mute(CDTV_rec, 2);	// off
					break;

				case 6:	// DO_PAUSE
					retVal = CDTV_Pause(CDTV_rec);
					break;

				case 7:	// DO_RESET
					retVal = CDTV_Reset(CDTV_rec);
					break;

				case 8:	// DO_STOP
					retVal = CDTV_Stop(CDTV_rec);
					break;
			}
			break;
	}

	return(retVal);
}

/******** ShowTrack() ********/

void ShowTrack(struct Window *window, struct CDTV_record *CDTV_rec)
{
TEXT duration[30], track[30], info[30];

	duration[0] = '\0';
	track[0] = '\0';
	info[0] = '\0';

	UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);

	CDTV_IsPlaying(CDTV_rec, TRUE, track, duration);
	sprintf(info, "%s: %s", track, duration);

	UA_DrawSpecialGadgetText(window, &CDTV_GR[26], info, SPECIAL_TEXT_CENTER);
}

/******** GetNewCD() ********/

void GetNewCD(struct Window *window, struct CDTV_record *CDTV_rec)
{
TEXT duration[30], numTracks[30];

	UA_InvertButton(window, &CDTV_GR[15]);

	CDTV_Stop(CDTV_rec);

	CDTV_GetCDInfo(CDTV_rec, numTracks, duration);
	UA_ClearButton(window, &CDTV_GR[2], AREA_PEN);
	UA_ClearButton(window, &CDTV_GR[3], AREA_PEN);
	UA_DrawSpecialGadgetText(window, &CDTV_GR[2], numTracks, SPECIAL_TEXT_CENTER);
	UA_DrawSpecialGadgetText(window, &CDTV_GR[3], duration, SPECIAL_TEXT_CENTER);

	UA_InvertButton(window, &CDTV_GR[15]);
}

/******** E O F ********/
