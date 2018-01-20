#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;
extern struct Gadget PropSlider1;

/**** gadgets ****/

extern struct GadgetRecord Help_GR[];

/**** functions ****/

/******** HelpWindow() ********/

void HelpWindow(void)
{
struct Window *window, *onWindow;
BOOL loop=TRUE;
int ID;
UBYTE *ptrs[60];	// KEEP THIS LARGE ENOUGH !!!!!!!!!!!
int topEntry=0, numEntries, numDisplay=13;
struct ScrollRecord SR;

	/**** double or halve gadgets ****/

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		onWindow = pageWindow;
	else
		onWindow = scriptWindow;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(onWindow, Help_GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Help_GR);

	PropSlider1.LeftEdge	= Help_GR[5].x1+4;
	PropSlider1.TopEdge		= Help_GR[5].y1+2;
	PropSlider1.Width			= Help_GR[5].x2-Help_GR[5].x1-7;
	PropSlider1.Height		= Help_GR[5].y2-Help_GR[5].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
	{
		// KEEP THIS LARGE ENOUGH !!!!!!!!!!!
		numEntries=53;
		for(ID=0; ID<numEntries; ID++)
			ptrs[ID] = msgs[Msg_Help_01-1+ID];
	}
	else
	{
		// KEEP THIS LARGE ENOUGH !!!!!!!!!!!
		numEntries=23;
		for(ID=0; ID<numEntries; ID++)
			ptrs[ID] = msgs[Msg_Help2_01-1+ID];
	}

	UA_InitPropSlider(window, &PropSlider1, numEntries, numDisplay, topEntry);

	/**** init scroll record ****/

	SR.GR							= &Help_GR[3];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= NULL;
	SR.entryWidth			= -1;
	SR.numDisplay			= numDisplay;
	SR.numEntries			= numEntries;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	window->RPort->TxSpacing-=1;
	UA_PrintStandardList(&SR,topEntry,ptrs);
	window->RPort->TxSpacing+=1;

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==IDCMP_MOUSEBUTTONS)
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				window->RPort->TxSpacing-=1;
				UA_ScrollStandardList(&SR, &topEntry, &PropSlider1, ptrs, &CED);
				window->RPort->TxSpacing+=1;
			}
			else if (CED.Code==SELECTDOWN)
			{
				ID = UA_CheckGadgetList(window, Help_GR, &CED);
				switch(ID)
				{
					case 4:	// OK
do_ok:
						UA_HiliteButton(window, &Help_GR[4]);
						loop=FALSE;
						break;
				}
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);
}

/******** E O F ********/
