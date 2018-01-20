#include "nb:pre.h"

/**** externals ****/

extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** ShowAbout() ********/

void ShowAbout(void)
{
BOOL retval;

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
	{
		PaletteToBack();
		//SetStandardColors(pageWindow);
	}

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( !LoadModule("About", &retval) )
		UA_WarnUser(156);

	//if ( EHI.activeScreen == STARTSCREEN_PAGE )
	//	ResetStandardColors(pageWindow);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** E O F ********/
