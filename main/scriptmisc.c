#include "nb:pre.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern struct MenuRecord **script_MR;
extern ULONG allocFlags;
extern UWORD chip gui_pattern[];
extern UWORD chip gui_pattern_lace[];
extern struct RastPort xappRP;
extern struct RastPort xappRP_2;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern BOOL ToolEnabledList[];	// index is type number
extern BOOL IconEnabledList[];	// index is index in tool bar
extern UBYTE *objectNameList[];
extern struct Document scriptDoc;
extern char *scriptCommands[];
extern int xappWdwHeight;
extern ULONG numEntries1, numDisplay1;
extern LONG topEntry1;
extern ULONG numEntries2, numDisplay2;
extern LONG topEntry2;
extern UBYTE **msgs;   
extern struct ScriptNodeRecord *editSNR;
extern struct GadgetRecord *kept_Script_GR;
extern ULONG kept_S_size;
extern BOOL blockScript;
extern TEXT *dir_scripts;

/**** static globals ****/

static struct PropInfo SliderInfo1 =
{ AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };

static struct PropInfo SliderInfo2 =
{ AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };

static struct Image Image1 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Image Image2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };

/**** globals ****/

struct Gadget ScriptSlider1 =
{	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Image1, NULL, NULL, NULL, (struct PropInfo *)&SliderInfo1, 1, NULL };

struct Gadget ScriptSlider2 =
{	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Image2, NULL, NULL, NULL, (struct PropInfo *)&SliderInfo2, 2, NULL };

/**** gadgets ****/

extern struct GadgetRecord AnimWdw_GR[];
extern struct GadgetRecord ArexxWdw_GR[];
extern struct GadgetRecord DosWdw_GR[];
extern struct GadgetRecord GlobalEventsWdw_GR[];
extern struct GadgetRecord InputSettingsWdw_GR[];
extern struct GadgetRecord LabelWdw_GR[];
extern struct GadgetRecord ObjectNameWdw_GR[];
extern struct GadgetRecord PageWdw_GR[];
extern struct GadgetRecord Prog_GR[];
extern struct GadgetRecord ProgBackup_GR[];
extern struct GadgetRecord SharedWdw_GR[];
extern struct GadgetRecord SoundWdw_GR[];
extern struct GadgetRecord TimeCodeWdw_GR[];
extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** DrawScriptScreen() ********/

void DrawScriptScreen(void)
{
	CopyMem(kept_Script_GR, Script_GR, kept_S_size);

	if ( CPrefs.userLevel < 3 )
		Script_GR[5].type = INVISIBLE_GADGET;
	else
		Script_GR[5].type = BUTTON_GADGET;

	ScaleGadgetList(scriptWindow->WScreen, AnimWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, ArexxWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, DosWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, GlobalEventsWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, InputSettingsWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, LabelWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, ObjectNameWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, PageWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, Prog_GR);
	ScaleGadgetList(scriptWindow->WScreen, ProgBackup_GR);
	ScaleGadgetList(scriptWindow->WScreen, SharedWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, SoundWdw_GR);
	ScaleGadgetList(scriptWindow->WScreen, TimeCodeWdw_GR);

	if (CPrefs.ScriptScreenModes & LACE)
	{
		Script_GR[0].y2 = scriptWindow->Height -0 -1;
		Script_GR[1].y2 = scriptWindow->Height -0 -1;

		Script_GR[2].y2 = scriptWindow->Height -92 -1;
		Script_GR[3].y2 = scriptWindow->Height -92 -1;

		Script_GR[4].y1 = scriptWindow->Height -83 -1;
		Script_GR[4].y2 = scriptWindow->Height -56 -1;

		Script_GR[5].y1 = Script_GR[4].y1;
		Script_GR[5].y2 = Script_GR[4].y2;

		Script_GR[7].y1 = scriptWindow->Height -51 -1;
		Script_GR[7].y2 = scriptWindow->Height -24 -1;

		Script_GR[8].y1 = Script_GR[7].y1;
		Script_GR[8].y2 = Script_GR[7].y2;
	}
	else
	{
		Script_GR[0].y2 = scriptWindow->Height -0 -1;
		Script_GR[1].y2 = scriptWindow->Height -0 -1;

		Script_GR[2].y2 = scriptWindow->Height -46 -1;
		Script_GR[3].y2 = scriptWindow->Height -46 -1;

		Script_GR[4].y1 = scriptWindow->Height -41 -1;
		Script_GR[4].y2 = scriptWindow->Height -28 -1;

		Script_GR[5].y1 = Script_GR[4].y1;
		Script_GR[5].y2 = Script_GR[4].y2;

		Script_GR[7].y1 = scriptWindow->Height -25 -1;
		Script_GR[7].y2 = scriptWindow->Height -12 -1;

		Script_GR[8].y1 = Script_GR[7].y1;
		Script_GR[8].y2 = Script_GR[7].y2;
	}

	/**** draw 2 prop gadgets and window gadgetery ****/

	ScriptSlider1.LeftEdge	= Script_GR[1].x1+4;
	ScriptSlider1.TopEdge		= Script_GR[1].y1+2;
	ScriptSlider1.Width			= Script_GR[1].x2-Script_GR[1].x1-7;
	ScriptSlider1.Height		= Script_GR[1].y2-Script_GR[1].y1-3;

	if ( UA_IsWindowOnLacedScreen(scriptWindow) )
	{
		ScriptSlider1.TopEdge	+= 2;
		ScriptSlider1.Height	-= 4;
	}

	ScriptSlider2.LeftEdge	= Script_GR[3].x1+4;
	ScriptSlider2.TopEdge		= Script_GR[3].y1+2;
	ScriptSlider2.Width			= Script_GR[3].x2-Script_GR[3].x1-7;
	ScriptSlider2.Height		= Script_GR[3].y2-Script_GR[3].y1-3;

	if ( UA_IsWindowOnLacedScreen(scriptWindow) )
	{
		ScriptSlider2.TopEdge	+= 2;
		ScriptSlider2.Height	-= 4;
	}

	SetFont(&scriptScreen->RastPort, smallFont);
	SetFont(scriptWindow->RPort, smallFont);

	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(scriptWindow->RPort, largeFont);
	else
		SetFont(scriptWindow->RPort, smallFont);

	UA_DrawGadgetList(scriptWindow, Script_GR);

	SetFont(scriptWindow->RPort, smallFont);

	InitPropInfo(	(struct PropInfo *)ScriptSlider1.SpecialInfo,
								(struct Image *)ScriptSlider1.GadgetRender);
	InitPropInfo(	(struct PropInfo *)ScriptSlider2.SpecialInfo,
								(struct Image *)ScriptSlider2.GadgetRender);
	AddGadget(scriptWindow, &ScriptSlider1, -1L);
	AddGadget(scriptWindow, &ScriptSlider2, -1L);

	ObjectRecord.maxObjects = (Script_GR[0].y2-Script_GR[0].y1)/20;
	numDisplay1 = ObjectRecord.maxObjects;

#ifdef USED_FOR_DEMO
ScriptGadgetsOff();
#endif
}

/******** DrawScriptScreen_2() ********/

void DrawScriptScreen_2(void)
{
	/**** get xapps ****/

	OpenToolIcons(scriptWindow, &xappWdwHeight);
	ShowToolIcons(scriptWindow, -1);

	/**** open an untitled script ****/

	stccpy(scriptDoc.title, msgs[Msg_Untitled-1], SIZE_FILENAME);
	//stccpy(scriptDoc.path, CPrefs.script_Path, SIZE_FULLPATH);
	stccpy(scriptDoc.path, dir_scripts, SIZE_FULLPATH);
	scriptDoc.modified = FALSE;
}

/******** doShowAndProgMenus() ********/

void doShowAndProgMenus(void)
{
	UnsetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);

	if ( ObjectRecord.scriptSIR.timeCodeFormat != TIMEFORMAT_HHMMSS)
		DisableMenu(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	else if ( ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
	{
		EnableMenu(script_MR[SMISC_MENU], SMISC_SHOWPROG);
		if (CPrefs.showDays)
			SetChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	}
}

/******** ShowMainEventIcons() ********/

void ShowMainEventIcons(void)
{
	DisableTool(TALK_ANIM);
	DisableTool(TALK_AREXX);
	EnableTool(TALK_BINARY);					/* binary enabled */
	DisableTool(TALK_DOS);
	EnableTool(TALK_GLOBALEVENT);			/* global enabled */
	EnableTool(TALK_INPUTSETTINGS);		/* input settings enabled */
	DisableTool(TALK_GOTO);
	DisableTool(TALK_LABEL);
	EnableTool(TALK_VARS);
	EnableTool(TALK_MAIL);						/* mail enabled */
	DisableTool(TALK_NOP);
	DisableTool(TALK_PAGE);
	DisableTool(TALK_SOUND);
	DisableTool(TALK_STARTSER);
	DisableTool(TALK_STARTPAR);
	EnableTool(TALK_TIMECODE);				/* timecode enabled */
	DisableTool(TALK_USERAPPLIC);
	ShowToolIcons(scriptWindow, -1);

	topEntry2=0L;
	UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
}

/******** ShowSerialEventIcons() ********/

void ShowSerialEventIcons(void)
{
	EnableTool(TALK_ANIM);
	EnableTool(TALK_AREXX);
	DisableTool(TALK_BINARY);
	EnableTool(TALK_DOS);
	DisableTool(TALK_GLOBALEVENT);
	DisableTool(TALK_INPUTSETTINGS);
	EnableTool(TALK_GOTO);
	EnableTool(TALK_LABEL);
	EnableTool(TALK_VARS);
	DisableTool(TALK_MAIL);
	EnableTool(TALK_NOP);
	EnableTool(TALK_PAGE);
	EnableTool(TALK_SOUND);
	EnableTool(TALK_STARTSER);
	if ( ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
		EnableTool(TALK_STARTPAR);
	else
		DisableTool(TALK_STARTPAR);
	DisableTool(TALK_TIMECODE);
	EnableTool(TALK_USERAPPLIC);
	ShowToolIcons(scriptWindow, -1);

	topEntry2=0L;
	UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
}

/******** ShowParallelEventIcons() ********/

void ShowParallelEventIcons(void)
{
	EnableTool(TALK_ANIM);
	EnableTool(TALK_AREXX);
	DisableTool(TALK_BINARY);
	EnableTool(TALK_DOS);
	DisableTool(TALK_GLOBALEVENT);
	DisableTool(TALK_INPUTSETTINGS);
	DisableTool(TALK_GOTO);
	DisableTool(TALK_LABEL);
	DisableTool(TALK_VARS);
	DisableTool(TALK_MAIL);
	DisableTool(TALK_NOP);
	EnableTool(TALK_PAGE);
	EnableTool(TALK_SOUND);
	DisableTool(TALK_STARTSER);
	DisableTool(TALK_STARTPAR);
	DisableTool(TALK_TIMECODE);
	EnableTool(TALK_USERAPPLIC);
	ShowToolIcons(scriptWindow, -1);

	topEntry2=0L;
	UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
}

/******** EnableAllEventIcons() ********/

void EnableAllEventIcons(void)
{
	EnableTool(TALK_ANIM);
	EnableTool(TALK_AREXX);
	EnableTool(TALK_BINARY);
	EnableTool(TALK_DOS);
	EnableTool(TALK_GLOBALEVENT);
	EnableTool(TALK_INPUTSETTINGS);
	EnableTool(TALK_GOTO);
	EnableTool(TALK_LABEL);
	EnableTool(TALK_VARS);
	EnableTool(TALK_MAIL);
	EnableTool(TALK_NOP);
	EnableTool(TALK_PAGE);
	EnableTool(TALK_SOUND);
	EnableTool(TALK_STARTSER);
	EnableTool(TALK_STARTPAR);
	EnableTool(TALK_TIMECODE);
	EnableTool(TALK_USERAPPLIC);
	ShowToolIcons(scriptWindow, -2);
}

/******** DisableAllEventIcons() ********/

void DisableAllEventIcons(void)
{
	DisableTool(TALK_ANIM);
	DisableTool(TALK_AREXX);
	DisableTool(TALK_BINARY);
	DisableTool(TALK_DOS);
	DisableTool(TALK_GLOBALEVENT);
	DisableTool(TALK_INPUTSETTINGS);
	DisableTool(TALK_GOTO);
	DisableTool(TALK_LABEL);
	DisableTool(TALK_VARS);
	DisableTool(TALK_MAIL);
	DisableTool(TALK_NOP);
	DisableTool(TALK_PAGE);
	DisableTool(TALK_SOUND);
	DisableTool(TALK_STARTSER);
	DisableTool(TALK_STARTPAR);
	DisableTool(TALK_TIMECODE);
	DisableTool(TALK_USERAPPLIC);
	ShowToolIcons(scriptWindow, -2);
}

/******** GetObjectName() ********/

void GetObjectName(	struct ScriptNodeRecord *this_node, STRPTR objectName,
										int lookStart, int width)
{
	if (this_node->nodeType == TALK_AREXX || this_node->nodeType == TALK_DOS)
	{
		if (this_node->extraData != NULL && this_node->extraData[0] != '\0')
		{
			if (strlen(this_node->extraData)>lookStart)
			{
				sprintf(objectName, "\"%s\"", this_node->extraData);
				UA_ShortenString(scriptWindow->RPort, objectName, width);
			}
			else
				sprintf(objectName, "\"%s\"", this_node->extraData);
		}
		else
			stccpy(objectName, msgs[Msg_Comment-1], MAX_OBJECTNAME_CHARS);
	}
	else
	{
		if (this_node->objectName[0] != '\0')
		{
			if (strlen(this_node->objectName)>lookStart)
			{
				if (this_node->nodeType == TALK_LABEL ||
						this_node->nodeType == TALK_STARTSER ||
						this_node->nodeType == TALK_STARTPAR ||
						this_node->nodeType == TALK_USERAPPLIC )
					sprintf(objectName, "\"%s\"", this_node->objectName);
				else
					stccpy(objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);
				UA_ShortenString(scriptWindow->RPort, objectName, width);
			}
			else
			{
				if (this_node->nodeType == TALK_LABEL ||
						this_node->nodeType == TALK_STARTSER ||
						this_node->nodeType == TALK_STARTPAR ||
						this_node->nodeType == TALK_USERAPPLIC )
					sprintf(objectName, "\"%s\"", this_node->objectName);
				else
					stccpy(objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);
			}
		}
		else
		{
			if ( this_node->nodeType == TALK_USERAPPLIC )
				stccpy(objectName, msgs[Msg_Comment-1], MAX_OBJECTNAME_CHARS);
			else
				stccpy(objectName, msgs[Msg_Untitled-1], MAX_OBJECTNAME_CHARS);
		}
	}

	if (this_node->nodeType == TALK_NOP)
	{
		if (strcmpi(objectName, msgs[Msg_Untitled-1])==0)
			objectName[0] = '\0';
	}
}

/******** ExpungeScript() ********/

void ExpungeScript(void)
{
	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
		DrawClosedScr();

	/**** free script memory ****/

	if ( TestBit(allocFlags, SCRIPTINMEM_FLAG) )
	{
		freeScript();
		UnSetBit(&allocFlags, SCRIPTINMEM_FLAG);
	}

	ObjectRecord.objList			= NULL;
	ObjectRecord.firstObject	= NULL;
	ObjectRecord.maxObjects		= 0;

	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
	{
		UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);		// play
		UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);		// edit
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);		// show
		if ( CPrefs.userLevel > 2 )
			UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);	// parent

		numEntries1 = 1;
		numDisplay1 = 1;
		topEntry1 = 0;
		UA_InitPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
	}

	FindSelectedIcon(-1);	/* reset Alt selecting */
}

/******** DrawClosedScr() ********/

void DrawClosedScr(void)
{
	UA_ClearButton(scriptWindow, &Script_GR[2], AREA_PEN);
	SetAPen(scriptWindow->RPort, LO_PEN);
	SetBPen(scriptWindow->RPort, AREA_PEN);
	SetDrMd(scriptWindow->RPort, JAM2);

	if ( CPrefs.ScriptScreenModes & LACE )
	{
		SetAfPt(scriptWindow->RPort, gui_pattern_lace, 2);
	}
	else
	{
		SetAfPt(scriptWindow->RPort, gui_pattern, 1);
	}

	if (CPrefs.ScriptScreenModes & LACE)
		RectFill(scriptWindow->RPort, Script_GR[0].x1+2, Script_GR[0].y1+2, Script_GR[0].x2-2, Script_GR[0].y2-2);
	else
		RectFill(scriptWindow->RPort, Script_GR[0].x1+2, Script_GR[0].y1+1, Script_GR[0].x2-2, Script_GR[0].y2-1);
	WaitBlit();
	SetAfPt(scriptWindow->RPort, NULL, 0);
	SetDrMd(scriptWindow->RPort, JAM1);
	UA_InitPropSlider(scriptWindow, &ScriptSlider1, 1, 1, 0);
	UA_InitPropSlider(scriptWindow, &ScriptSlider2, 1, 1, 0);
}

/******** FindSelectedIcon() ********/
/*
 * returns TRUE when going from *not* hilited to hilited
 *
 */

BOOL FindSelectedIcon(int top)
{
int row;
struct ScriptNodeRecord *this_node;
BOOL selected, keySHIFT, keyALT,retval=FALSE;
static int prevRow = -1;

	if (top==-1)
	{
		prevRow=-1;
		return(TRUE);
	}

	if (ObjectRecord.objList == NULL)
		return(TRUE);

	/********************
	selection goes as follows:

        if (!selected) --> select it, deselect all others

        if (selected) --> deselect it (in fact: deselect all)

				if (SHIFT) && (!selected) --> select it

				if (SHIFT) && (selected) --> deselect it

				if (ALT) && (!selected) --> select it (like shift) and at next
																		ALT click all inbetween too.

				if (ALT) && (selected) --> deselect it (like shift) and at next
																	 ALT click all inbetween too.

	********************/

	this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked(top, &row);

	/**** get info ****/

	keySHIFT = FALSE;
	keyALT = FALSE;
	selected = FALSE;

	if (	(CED.Qualifier&IEQUALIFIER_LSHIFT) ||
				(CED.Qualifier&IEQUALIFIER_RSHIFT) )
		keySHIFT=TRUE;

	/* ALT dominates SHIFT */

	if ( (CED.Qualifier&IEQUALIFIER_LALT) || (CED.Qualifier&IEQUALIFIER_RALT) )
		keyALT=TRUE;
	else
		prevRow=-1;	/* alt and then no alt means kill alt clicking */

	if (this_node->miscFlags & OBJ_SELECTED)
		selected=TRUE;

	/**** decide what to do ****/

	if (!selected && !keySHIFT && !keyALT)
	{
		/**** select object, deselect all others ****/
		DeselectAllObjects();
		SelectObject(this_node);
		RedrawVisibleObjects();
		retval=TRUE;
	}
	else if (selected && !keySHIFT && !keyALT)
	{
		/**** deselect all ****/
		DeselectAllObjects();
		RedrawVisibleObjects();
	}
	else if (!selected && keySHIFT)
	{
		/**** select object, leave others undisturbed ****/
		SelectObject(this_node);
		RedrawVisibleObjects();
		retval=TRUE;
	}
	else if (selected && keySHIFT)
	{
		/**** deselect object, leave others undisturbed ****/
		DeSelectObject(this_node);
		RedrawVisibleObjects();
	}
	else if (!selected && keyALT)
	{
		/**** select object and if prevRow!=-1 to that and inbetween too ****/
		if (prevRow != -1)
		{
			SelectMoreObjects(prevRow, row);
			prevRow=-1;
		}
		else
		{
			SelectObject(this_node);
			prevRow = row;
		}
		RedrawVisibleObjects();
		retval=TRUE;
	}
	else if (selected && keyALT)
	{
		/**** deselect object and if prevRow!=-1 to that and inbetween too ****/
		if (prevRow != -1)
		{
			DeSelectMoreObjects(prevRow, row);
			prevRow=-1;
		}
		else
		{
			DeSelectObject(this_node);
			prevRow = row;
		}
		RedrawVisibleObjects();
	}

	SetScriptMenus();

	return(retval);
}

/******** WhichObjectWasClicked() ********/

struct ScriptNodeRecord *WhichObjectWasClicked(int top, int *row)
{
int i;
struct ScriptNodeRecord *this_node;

	if (ObjectRecord.objList == NULL)
		return(NULL);

	/**** calculate which object POS we clicked on *****/

	*row = ( (CED.MouseY - Script_GR[0].y1) / 20 ) + top;

	/**** leave the show if user clicked below last object ****/

	if (*row >= ObjectRecord.numObjects)
	{
		return(NULL);
	}

	/**** with the object POS, find the object NODE ****/

	if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
		return(NULL);

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (*row>0)
	{
		for(i=0; i<*row; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	return(this_node);
}


/******** SelectAllObjects() ********/

void SelectAllObjects(void)
{
struct ScriptNodeRecord *this_node;

	for(this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
			this_node->node.ln_Succ;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
	{
		SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
		SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
	}
	//SetScriptMenus();
}

/******** DeselectAllObjects() ********/

void DeselectAllObjects(void)
{
struct ScriptNodeRecord *this_node;

	for(this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
			this_node->node.ln_Succ;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
	{
		if ( this_node->miscFlags & OBJ_SELECTED )
		{
			UnSetByteBit(&this_node->miscFlags, OBJ_SELECTED);
			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
		}
	}
	//SetScriptMenus();
}

/******** SelectObject() ********/

void SelectObject(struct ScriptNodeRecord *this_node)
{
	SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
	SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
}

/******** SelectMoreObjects() ********/

void SelectMoreObjects(int prevRow, int row)
{
struct ScriptNodeRecord *this_node;
int i;

	if (prevRow > row)
		swapInts(&prevRow, &row);

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (prevRow>0)
	{
		for(i=0; i<prevRow; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	for(i=prevRow;
			i<=row;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ,i++)
	{
		if ( !(this_node->miscFlags & OBJ_SELECTED) )
		{
			SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
		}
	}
}

/******** DeSelectObject() ********/

void DeSelectObject(struct ScriptNodeRecord *this_node)
{
	UnSetByteBit(&this_node->miscFlags, OBJ_SELECTED);
	SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
}

/******** DeSelectMoreObjects() ********/

void DeSelectMoreObjects(int prevRow, int row)
{
struct ScriptNodeRecord *this_node;
int i;

	if (prevRow > row)
		swapInts(&prevRow, &row);

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (prevRow>0)
	{
		for(i=0; i<prevRow; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	for(i=prevRow;
			i<=row;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ,i++)
	{
		if ( this_node->miscFlags & OBJ_SELECTED )
		{
			UnSetByteBit(&this_node->miscFlags, OBJ_SELECTED);
			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
		}
	}
}

/******** RedrawVisibleObjects() ********/

void RedrawVisibleObjects(void)
{
	DrawObjectList(-1, FALSE, FALSE);	/* use last top, don't draw all, don't force */
}

/******** ClearBetweenLines() ********/

void ClearBetweenLines(void)
{
int i,lines,offset;

	lines = (Script_GR[0].y2-Script_GR[0].y1) / 20;
	SetAPen(scriptWindow->RPort, AREA_PEN);

	if ( CPrefs.ScriptScreenModes & LACE)
		offset=1;
	else
		offset=0;

	for(i=0; i<lines; i++)
		RectFill(	scriptWindow->RPort,
							Script_GR[0].x1+2, Script_GR[0].y1+offset+1+20*i,
							Script_GR[0].x2-2, Script_GR[0].y1+offset+19+20*i);
	WaitBlit();
}

/******** ScriptGadgetsOn() ********/

void ScriptGadgetsOn(void)
{
#ifndef USED_FOR_DEMO
	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
	{
		OnGadget(&ScriptSlider1, scriptWindow, NULL);
		OnGadget(&ScriptSlider2, scriptWindow, NULL);
	}
#endif
}

/******** ScriptSlider2On() ********/

void ScriptSlider2On(void)
{
#ifndef USED_FOR_DEMO
	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
		OnGadget(&ScriptSlider2, scriptWindow, NULL);
#endif
}

/******** ScriptGadgetsOff() ********/

void ScriptGadgetsOff(void)
{
	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
	{
		OffGadget(&ScriptSlider1, scriptWindow, NULL);
		OffGadget(&ScriptSlider2, scriptWindow, NULL);
	}
}

/******** ReloadXapps() ********/

BOOL ReloadXapps(void)
{
	CloseToolIcons();

	if ( TestBit(allocFlags, SCRIPTWINDOW_FLAG) )
	{
		if ( !OpenToolIcons(scriptWindow, &xappWdwHeight) )
			return(FALSE);

		topEntry2=0L;
		numEntries2=(LONG)xappWdwHeight;
		numDisplay2=(LONG)Script_GR[2].y2-Script_GR[2].y1-2;
		if ( AbsInt(numDisplay2,numEntries2) < 5 )
			numEntries2 = numDisplay2;
		UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);

		if ( scriptDoc.opened )
		{	
			ShowToolIcons(scriptWindow, -1);

			if ( ObjectRecord.scriptSIR.listType == -1 )
			{
				ObjectRecord.scriptSIR.listType = 
						FindParentType( &(ObjectRecord.scriptSIR), ObjectRecord.objList);
				if ( ObjectRecord.scriptSIR.listType == -1 )
					ObjectRecord.scriptSIR.listType = TALK_STARTSER;	// root is serial
			}

			if ( ObjectRecord.scriptSIR.allLists[0] == ObjectRecord.objList )
				ShowMainEventIcons();
			else if ( ObjectRecord.scriptSIR.listType==TALK_STARTSER )
				ShowSerialEventIcons();
			else
				ShowParallelEventIcons();
		}
	}

	return(TRUE);
}

/******** DeselectAllButThisOne() ********/

void DeselectAllButThisOne(struct ScriptNodeRecord *this_node)
{
BOOL wasSelected=FALSE;

	if ( this_node->miscFlags & OBJ_SELECTED )
		wasSelected=TRUE;

	if (	!(CED.Qualifier & IEQUALIFIER_LSHIFT) &&
				!(CED.Qualifier & IEQUALIFIER_RSHIFT) )
		DeselectAllObjects();

	SetByteBit(&this_node->miscFlags, OBJ_SELECTED);

	if ( !wasSelected )
		SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);

	RedrawVisibleObjects();
}

/******** ReallyDeselectAllButThisOne() ********/

void ReallyDeselectAllButThisOne(struct ScriptNodeRecord *this_node)
{
BOOL wasSelected=FALSE;
	if ( this_node->miscFlags & OBJ_SELECTED )
		wasSelected=TRUE;
	DeselectAllObjects();
	SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
	if ( !wasSelected )
		SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
	RedrawVisibleObjects();
}

/******** E O F ********/
