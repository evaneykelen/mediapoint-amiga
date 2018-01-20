#include "nb:pre.h"

UA_ProcessCycleGadget(window, &FormatRequester_GR[ID], &CED);
UA_SetValToCycleGadgetVal(&FormatRequester_GR[2], &val);
UA_SetCycleGadgetToVal(window, &FormatRequester_GR[1], mode1-1);
UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
UA_SetStringToGadgetString(&GR[ID],path);
UA_SetStringGadgetToString(window, &GR[11], fileName); /* filename */
UA_SetStringGadgetToVal(window, &WDef_GR[8], (int)localEW->X);
UA_SetValToStringGadgetVal(&WDef_GR[8], &my_int);

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern UBYTE **msgs;

/**** globals ****/

/**** gadgets ****/

/**** functions ****/

struct Window *window;
BOOL loop, retVal;
int ID;

	retVal = FALSE;
	loop   = TRUE;

	/**** double or halve gadgets ****/

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		window = pageWindow;
	else
		window = scriptWindow;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		if ( ???_GR[0].x1 == 0 )	// not doubled 
		{
			UA_DoubleGadgetDimensions(???_GR);
			???_GR[0].x1 = 1;
		}
	}
	else
	{
		if ( ???_GR[0].x1 == 1 )	// doubled
		{
			UA_HalveGadgetDimensions(???_GR);
			???_GR[0].x1 = 0;
		}
	}

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pagescriptWindow, ???_GR, ???COLORS);
	if (!window)
	{
		UA_WarnUser(???);
		return(FALSE);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, ???_GR);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, ???_GR, &CED);
			switch(ID)
			{
				case ???:	// OK
do_ok:
					UA_HiliteButton(window, &???_GR[???]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case ???:	// Cancel
do_cancel:
					UA_HiliteButton(window, &???_GR[???]);
					loop=FALSE;
					retVal=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&???_GR[???]))	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,???COLORS);

	return(retVal);
}

/******** E O F ********/

#if 0

ULONG signals;
BOOL takeAction=TRUE;

	/**** handle events ****/

	while(1)
	{
		if (takeAction)
		{
			if ( CED.Class == IDCMP_MOUSEBUTTONS && CED.Code == SELECTDOWN )
			{
				ID = UA_CheckGadgetList(paletteWindow, ColorAdjust_GR, &CED);
				return(NOTHING_TO_EXAMINE);
			{
			else if (CED.Class == IDCMP_MENUPICK)
				return(IDCMP_TO_EXAMINE);
			else if (CED.Class == IDCMP_RAWKEY)
				return(IDCMP_TO_EXAMINE);
			}
		}
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(window, BOOL draggable );
			takeAction=TRUE;
			if ( !(???Window->Flags & WFLG_WINDOWACTIVE) )
				return(IDCMP_TO_EXAMINE);
		}
		else
			takeAction=FALSE;
	}

	return(NOTHING_TO_EXAMINE);
}

#endif
