#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern TEXT codedStr[];	// see initglobals.c

/**** gadgets ****/

extern struct GadgetRecord Demo_GR[];

/**** functions ****/

#ifdef USED_FOR_DEMO

/******** ShowDemoMsg() ********/

void ShowDemoMsg(void)
{
struct Window *window;
BOOL loop;
int ID,y,i;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow, Demo_GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Demo_GR);

	y = window->RPort->TxBaseline + 10;
	PrintAt(LO_PEN, window->RPort, 20, y, "This is a demo version of MediaPoint");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "  ");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "It has the following limitations:");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "· Some characters in the text editor don't work.");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "· The script list can't be scrolled.");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "· Scripts can't be saved.");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "· The player can only be controlled manually.");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "  ");
	y = y + window->RPort->TxHeight;
	PrintAt(LO_PEN, window->RPort, 20, y, "MediaPoint requires at least 3 MB.");

	/**** Do a personalized message ****/

	ShowPersonalized();
/*
	len = strlen(codedStr);
	for(i=0; i<len; i++)
		codedStr[i] = ~codedStr[i];					// ALT SPACE AFTER COLON!
	Message( "This program is personalized for: %s", codedStr );
*/

	/**** monitor user ****/

	loop = TRUE;
	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Demo_GR, &CED);
			switch(ID)
			{
				case 2:	// OK
do_ok:
					UA_HiliteButton(window, &Demo_GR[2]);
					loop=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);
}

#endif

/******** ShowPersonalized() ********/

void ShowPersonalized(void)
{
#if 0
int i,len;
	len = strlen(codedStr);
	for(i=0; i<len; i++)
		codedStr[i] = ~codedStr[i];					// ALT SPACE AFTER COLON!
	Message( "This program is personalized for: %s", codedStr );
#endif
}

/******** ShowBeta() ********/

void ShowBeta(void)
{
#if 0
	Message( "Remember: this is a PERSONALIZED BETA version!" );
#endif
}

/******** E O F ********/
