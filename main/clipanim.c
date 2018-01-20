#include "nb:pre.h"

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
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct GadgetRecord SmallWarning_GR[];

/**** gadgets ****/

extern struct GadgetRecord ClipAnim_GR[];

/**** functions ****/

/******** ClipAnimSettings() ********/

BOOL ClipAnimSettings(struct Window *onWindow)
{
struct Window *window;
BOOL loop, retVal;
int ID, wdw, fps, loops, bits, i;
struct EditWindow *ew = NULL;
struct CycleRecord *CR;

	wdw = FirstActiveEditWindow();
	if ( wdw != -1 )
		ew = EditWindowList[wdw];

	if ( wdw==-1 || !ew || !ew->animIsAnim )
	{
		UA_OpenGenericWindow(	onWindow, TRUE, FALSE, msgs[Msg_OK-1], NULL,
													EXCLAMATION_ICON, msgs[Msg_FirstImportAnim-1], TRUE,
													SmallWarning_GR );
		return(FALSE);
	}

	retVal = FALSE;
	loop   = TRUE;

	fps = ew->animSpeed;
	if ( GetRefreshRate( CPrefs.playerMonitorID ) < fps )	// eg. 50 Hz PAL vs 60 hz NTSC
		fps = GetRefreshRate( CPrefs.playerMonitorID );

	loops = ew->animLoops;
	if (loops==-1)
		loops=0;	// loops endless is stored as -1

	bits = ew->animFromDisk;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(onWindow, ClipAnim_GR, USECOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadget ****/

	CR = (struct CycleRecord *)ClipAnim_GR[4].ptr;
	CR->number = GetRefreshRate( CPrefs.playerMonitorID ) + 1;

	UA_DrawGadgetList(window, ClipAnim_GR);

	UA_SetCycleGadgetToVal(window, &ClipAnim_GR[4], fps);
	UA_SetCycleGadgetToVal(window, &ClipAnim_GR[5], loops);
	if ( bits & 1 )
		UA_InvertButton(window, &ClipAnim_GR[6]);
	if ( bits & 2 )
		UA_InvertButton(window, &ClipAnim_GR[7]);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, ClipAnim_GR, &CED);
			switch(ID)
			{
				case 2:	// OK
do_ok:
					UA_HiliteButton(window, &ClipAnim_GR[2]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 3:	// Cancel
do_cancel:
					UA_HiliteButton(window, &ClipAnim_GR[3]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 4:	// fps
					UA_ProcessCycleGadget(window, &ClipAnim_GR[4], &CED);
					break;

				case 5:	// loops
					UA_ProcessCycleGadget(window, &ClipAnim_GR[5], &CED);
					break;

				case 6:	// play from disk
					UA_InvertButton(window, &ClipAnim_GR[6]);
					if ( bits & 1 )
						bits = bits & ~1;
					else
						bits = bits | 1;
					break;

				case 7:	// transparent
					UA_InvertButton(window, &ClipAnim_GR[7]);
					if ( bits & 2 )
						bits = bits & ~2;
					else
						bits = bits | 2;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	if ( retVal )	// set all windows with anims to these settings
	{
		UA_SetValToCycleGadgetVal(&ClipAnim_GR[4],&fps);
		UA_SetValToCycleGadgetVal(&ClipAnim_GR[5],&loops);

		if (loops==0)
			loops=-1;	// loops endless is stored as -1

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (	EditWindowList[i] && EditWindowList[i]->animIsAnim &&
						EditSupportList[i]->Active )
			{
				EditWindowList[i]->animLoops = loops;
				EditWindowList[i]->animSpeed = fps;
				EditWindowList[i]->animFromDisk = bits;
			}
		}
	}

	UA_CloseRequesterWindow(window,USECOLORS);

	return(retVal);
}

/******** E O F ********/
