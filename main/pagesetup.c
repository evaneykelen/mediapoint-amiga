#include "nb:pre.h"

/**** externals ****/

extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** ShowPageSetUp() ********/

void ShowPageSetUp(void)
{
BOOL retval;

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
	{
		PaletteToBack();
		//SetStandardColors(pageWindow);
	}

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( !LoadModule("PageSetUp", &retval) )
		UA_WarnUser(155);

	if ( retval )
		updateCAPSconfig(2);	// update PRINTERPREFS lines

	//if ( EHI.activeScreen == STARTSCREEN_PAGE )
	//	ResetStandardColors(pageWindow);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** E O F ********/
