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
extern UBYTE **msgs;   
extern struct Gadget PropSlider1;
extern UWORD chip gui_pattern[];
extern struct MenuRecord **script_MR;

/**** gadgets ****/

extern struct GadgetRecord InputSettingsWdw_GR[];

/**** functions ****/

/******** MonitorInputSettings() ********/

void MonitorInputSettings(struct ScriptNodeRecord *this_node)
{
struct Window *window;
BOOL loop=TRUE, retVal=TRUE;
int ID, value;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow, InputSettingsWdw_GR, USECOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	UA_DrawGadgetList(window,InputSettingsWdw_GR);

	/**** set gadgets ****/

	// PLAYER INPUT

	if ( CPrefs.mousePointer & 1 )							
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[8], 0);	// cursor
	else if ( CPrefs.mousePointer & 2 )				
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[8], 1);	// mouse
	else if ( CPrefs.mousePointer & 4 )				
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[8], 2);	// cursor+mouse
	else if ( CPrefs.mousePointer & 8 )				
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[8], 3);	// none

	// MOUSE POINTER

	if ( CPrefs.mousePointer & 16 )						
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[9], 0);	// on
	else if ( CPrefs.mousePointer & 32 )				
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[9], 1);	// off
	else if ( CPrefs.mousePointer & 64 )				
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[9], 2);	// auto

	// ASYNC CLICKING

	if ( CPrefs.mousePointer & 128 )
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[10], 1);	// no
	else
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[10], 0);	// yes -- default

	// SCHEDULING

	if (CPrefs.showDays)
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[11], 0);	// yes
	else
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[11], 1);	// no

	// GAMEPORT

	if ( CPrefs.gameport_used )								
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[12], 0);	// on
	else																									
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[12], 1);	// off

	// STANDBY

	if ( CPrefs.standBy )
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[13], 0);	// yes
	else
		UA_SetCycleGadgetToVal(window, &InputSettingsWdw_GR[13], 1);	// no

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, InputSettingsWdw_GR, &CED);
			switch(ID)
			{
				case 2:	// OK
do_ok:
					UA_HiliteButton(window, &InputSettingsWdw_GR[2]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 3:	// Cancel
do_cancel:
					UA_HiliteButton(window, &InputSettingsWdw_GR[3]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
					UA_ProcessCycleGadget(window, &InputSettingsWdw_GR[ID], &CED);
					break;

				case 14:
					UA_InvertButton(window, &InputSettingsWdw_GR[ID]);
					UA_OpenGenericWindow(	window, TRUE, FALSE, msgs[Msg_OK-1], NULL,
																NO_ICON, msgs[Msg_AC_Help-1], TRUE, NULL);
					UA_InvertButton(window, &InputSettingsWdw_GR[ID]);
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	if ( retVal )
	{
		// PLAYER INPUT

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[8], &value);

		CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
		if ( value==0 )				// cursor
			CPrefs.mousePointer |= 1;
		else if ( value==1 )	// mouse
			CPrefs.mousePointer |= 2;
		else if ( value==2 )	// cursor+mouse
			CPrefs.mousePointer |= 4;
		else									// none
			CPrefs.mousePointer |= 8;

		// MOUSE POINTER

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[9], &value);

		CPrefs.mousePointer = CPrefs.mousePointer & ~(16+32+64);
		if ( value==0 )				// on
			CPrefs.mousePointer |= 16;
		else if ( value==1 )	// off
			CPrefs.mousePointer |= 32;
		else									// auto
			CPrefs.mousePointer |= 64;

		// ASYNC CLICKING

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[10], &value);

		if (value==0)	// yes
			CPrefs.mousePointer = CPrefs.mousePointer & ~(128);
		else					// no
			CPrefs.mousePointer |= 128;

		// SCHEDULING

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[11], &value);

		if (value==0)
			CPrefs.showDays = TRUE;
		else
			CPrefs.showDays = FALSE;

		// GAMEPORT

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[12], &value);

		if ( value==0 )	// on
			CPrefs.gameport_used = TRUE;
		else	
			CPrefs.gameport_used = FALSE;

		// STANDBY

		UA_SetValToCycleGadgetVal(&InputSettingsWdw_GR[13], &value);

		if ( value==0 )	// yes
			CPrefs.standBy = 1;
		else
			CPrefs.standBy = 0;

		// Change other stuff

		if ( ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
		{
			if (CPrefs.showDays)
				SetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
			else
				UnsetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
			DeselectAllObjects();
			ClearBetweenLines();
			DrawObjectList(-1, TRUE, TRUE);
			doShowAndProgMenus();
		}
	}

	UA_CloseRequesterWindow(window,USECOLORS);
}

/******** E O F ********/
