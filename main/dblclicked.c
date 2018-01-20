#include "nb:pre.h"
#include "xapp_names.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct ObjectInfo ObjectRecord;
extern ULONG numEntries1, numDisplay1;
extern LONG topEntry1;
extern struct Gadget ScriptSlider1;
extern BOOL EditMenuStates[];
extern struct MenuRecord **script_MR;
extern UWORD chip gui_pattern[];
extern TEXT *dir_xapps;
extern UBYTE **msgs;
extern struct Document scriptDoc;
extern ULONG allocFlags;
extern struct FER FontEntryRecord;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];
extern struct GadgetRecord ArexxWdw_GR[];
extern struct GadgetRecord DosWdw_GR[];
extern struct GadgetRecord AnimWdw_GR[];
extern struct GadgetRecord PageWdw_GR[];
extern struct GadgetRecord SoundWdw_GR[];
extern struct GadgetRecord BinaryWdw_GR[];
extern struct GadgetRecord MailWdw_GR[];
extern struct GadgetRecord LabelWdw_GR[];
extern struct GadgetRecord GlobalEventsWdw_GR[];
extern struct GadgetRecord InputSettingsWdw_GR[];
extern struct GadgetRecord TimeCodeWdw_GR[];

/**** functions ****/

/******** processDblClick() ********/

BOOL processDblClick(int top, SNRPTR this_node)
{
int row;
TEXT fullPath[SIZE_FULLPATH];
BOOL result=FALSE;
struct Window *reqWindow;
struct IntuiMessage *message;

	if (this_node==NULL)	/* not supplied, then search self */
	{
		this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked(top, &row);
		if (this_node == NULL)
			return(FALSE);
	}

	DeselectAllButThisOne(this_node);

	switch(this_node->nodeType)
	{
		case TALK_ANIM:
			Build_SmallScriptWindow(AnimWdw_GR, this_node);
			break;

		case TALK_AREXX:
			Build_SmallScriptWindow(ArexxWdw_GR, this_node);
			break;

		case TALK_BINARY:
			OpenAFile(this_node->objectPath, this_node->objectName,
								msgs[Msg_SelectFiles-1], scriptWindow, DIR_OPT_ALL | DIR_OPT_NOINFO,
								FALSE);
			break;

		case TALK_MAIL:
			OpenAFile(this_node->objectPath, this_node->objectName,
								msgs[Msg_SelectMailFiles-1], scriptWindow, DIR_OPT_ALL | DIR_OPT_NOINFO,
								FALSE);
			break;

		case TALK_USERAPPLIC:
			UA_MakeFullPath(dir_xapps, this_node->objectPath, fullPath);
			SetSpriteOfActWdw(SPRITE_BUSY);

			/***** LOAD FONTS LIST ****/
			if (	!strcmpi(this_node->objectPath,CREDIT_XAPP) &&
						!TestBit(allocFlags,FONTS_SCANNED_FLAG) )
			{
				struct Window *panel;
					panel = (struct Window *)UA_OpenMessagePanel(scriptWindow, msgs[Msg_GettingFonts-1]);
					if ( panel )
					{
						SetSpriteOfActWdw(SPRITE_BUSY);
						if ( !UA_ScanFontsDir(&FontEntryRecord) )
							UA_WarnUser(218);
						else
							SetBit(&allocFlags, FONTS_SCANNED_FLAG);
					}
					Delay(15L);
					UA_CloseMessagePanel(panel);
			}
			/***** LOAD FONTS LIST ****/

			if ( !InitXaPP(fullPath, this_node, TRUE) )	// TRUE means tiny
				Message(msgs[Msg_UnableToLaunchXapp-1], this_node->objectPath);
			SetSpriteOfActWdw(SPRITE_NORMAL);
			break;

		case TALK_DOS:
			Build_SmallScriptWindow(DosWdw_GR, this_node);
			break;

		case TALK_GLOBALEVENT:
			if ( ValidateSER(&(ObjectRecord.scriptSIR),FALSE,FALSE) )
			{
				reqWindow = Open_A_Request_Window(scriptWindow, GlobalEventsWdw_GR);
				if ( reqWindow )
				{
					Monitor_GlobalLabels(reqWindow);
					Close_A_Request_Window(reqWindow);
				}
			}
			break;

		case TALK_INPUTSETTINGS:
			MonitorInputSettings(this_node);
			break;

		case TALK_LABEL:
			DoObjectName(this_node->objectName, msgs[Msg_UniqueLabel-1], this_node->nodeType);
			break;

		case TALK_NOP:
			DoObjectName(this_node->objectName, msgs[Msg_ObjectComment-1], this_node->nodeType);
			break;

		case TALK_STARTSER:
		case TALK_STARTPAR:
			processScriptSerPar(this_node);
			break;

		case TALK_SOUND:
			Build_SmallScriptWindow(SoundWdw_GR, this_node);
			break;

		case TALK_PAGE:
			result = Build_SmallScriptWindow(PageWdw_GR, this_node);
			break;

		case TALK_GOTO:
			standAlonePickLabel(scriptWindow, this_node);
			break;

		case TALK_TIMECODE:
			reqWindow = Open_A_Request_Window(scriptWindow, TimeCodeWdw_GR);
			if ( reqWindow )
			{
				Monitor_TimeCode(reqWindow);
				Close_A_Request_Window(reqWindow);
				doShowAndProgMenus();
				ClearBetweenLines();
				DrawObjectList(-1, TRUE, TRUE);	/* use last top, draw all, force */
			}
			break;

		case TALK_VARS:
			MonitorVariables(this_node);
			break;
	}

	SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
	DrawObjectList(-1, FALSE, TRUE);

	/**** drain messages ****/

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	SetScriptMenus();

	return(result);
}

/******** processScriptSerPar() ********/
/*
 * Dive deeper into script
 *
 */

void processScriptSerPar(SNRPTR this_node)
{
int where,i;
BOOL fromRoot=FALSE;

	if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] ) /* root encountered */
		fromRoot=TRUE;

	/**** first remember current scroll position ****/

	where = InWhichListAreWe(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
	if (where != -1)
		ObjectRecord.scriptSIR.scrollPos[where] = topEntry1;

	/**** dive into new list ****/

	DeselectAllObjects();

	ClearBetweenLines();

	/**** initialize list and node start ****/

	ObjectRecord.objList = this_node->list;
	ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;

	GetNumObjects();

	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(scriptWindow->RPort, largeFont);

	if ( CPrefs.userLevel > 2 )
	{
		UA_EnableButton(scriptWindow, &Script_GR[5]);	// parent
//		UA_EnableButton(scriptWindow, &Script_GR[7]);	// edit
//		UA_EnableButton(scriptWindow, &Script_GR[8]);	// show
	}

	SetFont(scriptWindow->RPort, smallFont);

	if (this_node->nodeType == TALK_STARTSER)
		ShowSerialEventIcons();
	else if (this_node->nodeType == TALK_STARTPAR)
		ShowParallelEventIcons();

	/**** draw new script ****/

	ObjectRecord.scriptSIR.listType = -1;

	where = InWhichListAreWe(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
	if (where != -1)
	{
		topEntry1 = ObjectRecord.scriptSIR.scrollPos[where];
		DrawObjectList(topEntry1, TRUE, TRUE);
	}
	else
	{
		DrawObjectList(0, TRUE, TRUE);
		topEntry1=0L;
	}

	numEntries1=ObjectRecord.numObjects;
	UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);

	/**** set edit menus ****/

	if (ObjectRecord.numObjects==0)
		DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
	else
		EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);

	CountNumSelected(ObjectRecord.firstObject, &where);
	if (where>=1)
	{
		EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}
	else
	{
		DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}

	if (fromRoot)
	{
		for(i=0; i<6; i++)
		{
			if (EditMenuStates[i])	/* TRUE IF DISABLED */
				DisableMenu(script_MR[EDIT_MENU], i);
			else
				EnableMenu(script_MR[EDIT_MENU], i);
		}
	}

	FindSelectedIcon(-1);	/* reset Alt selecting */

	/**** print sub branch name ****/

	PrintSubBranchName(this_node->objectName);

	SetScriptMenus();
}

/******** processScriptParent() ********/

void processScriptParent(void)
{
struct List *newList;
int nodeType, where;
int i;
struct ScriptNodeRecord *this_node;

	if ( CPrefs.userLevel < 3 )	// user level too low
		return;

	/**** first remember current scroll position ****/

	where = InWhichListAreWe(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
	if (where != -1)
		ObjectRecord.scriptSIR.scrollPos[where] = topEntry1;

	DeselectAllObjects();

	newList = (struct List *)FindParent(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
	if (newList == NULL)
		return;

	ClearBetweenLines();

	if ( newList == ObjectRecord.scriptSIR.allLists[0] ) /* root encountered */
	{
		ObjectRecord.scriptSIR.listType = -1;

		ShowMainEventIcons();

		UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);	// parent
		//UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);	// edit
		//UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);	// show

		/**** initialize list and node start ****/

		ObjectRecord.objList = newList;
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;

		/**** in root don't show very first object (it's a startser) ****/
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.firstObject->node.ln_Succ;
	}
	else
	{
		nodeType = FindParentType(&(ObjectRecord.scriptSIR), newList);
		if (nodeType == -1)
			return;
		if (nodeType==TALK_STARTSER)
			ShowSerialEventIcons();
		else
			ShowParallelEventIcons();

ObjectRecord.scriptSIR.listType = nodeType;

		/**** initialize list and node start ****/

		ObjectRecord.objList = newList;
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
	}

	GetNumObjects();

	/**** draw new script ****/

	ObjectRecord.scriptSIR.listType = -1;

	where = InWhichListAreWe(&(ObjectRecord.scriptSIR), ObjectRecord.objList);
	if (where != -1)
	{
		topEntry1 = ObjectRecord.scriptSIR.scrollPos[where];
		DrawObjectList(topEntry1, TRUE, TRUE);
	}
	else
	{
		DrawObjectList(0, TRUE, TRUE);
		topEntry1=0L;
	}

	numEntries1=ObjectRecord.numObjects;
	UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);

	/**** remember edit menus ****/

	for(i=0; i<6; i++)
		EditMenuStates[i] = script_MR[EDIT_MENU]->disabled[i];	/* TRUE IF DISABLED */

	/**** set edit menus ****/

	if (ObjectRecord.numObjects==0)
		DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
	else
		EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);

	CountNumSelected(ObjectRecord.firstObject, &where);
	if (where>=1)
	{
		EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}
	else
	{
		DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}

	/**** disable cut 'n paste functions in root ****/

	if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] ) /* root encountered */
	{
		DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
		DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}

	FindSelectedIcon(-1);	/* reset Alt selecting */

	/**** print sub branch name ****/

	this_node = FindParentNode(&(ObjectRecord.scriptSIR), newList);
	if ( this_node)
		PrintSubBranchName(this_node->objectName);
	else
		PrintSubBranchName(NULL);

	SetScriptMenus();
}

/******** PrintSubBranchName() ********/

void PrintSubBranchName(STRPTR name)
{
TEXT str[80];

	SetDrMd(scriptWindow->RPort, JAM1);
	SetAPen(scriptWindow->RPort, BGND_PEN);
	RectFill(scriptWindow->RPort, 0, 0, 320, Script_GR[0].y1-1);	// above scroll area

	if (name)
		sprintf(str, "%s - %s", scriptDoc.title, name);
	else
		sprintf(str, "%s - Root", scriptDoc.title);

	UA_ShortenString(scriptWindow->RPort, str, 320-16);
	SetAPen(scriptWindow->RPort, HI_PEN);
	Move(scriptWindow->RPort, 2, 8);
	Text(scriptWindow->RPort, str, strlen(str));
}

/******** E O F ********/
