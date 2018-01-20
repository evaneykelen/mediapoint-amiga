/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include "nb:mpplayer/dongle/dongle_protos.h"

#define SCRIPT_LINESIZE 2048L

struct GenericFuncs
{
	void (*func)(APTR, APTR);
};

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct ScriptFuncs scriptFuncs[];
extern ULONG allocFlags;
extern struct Window *scriptWindow;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct Library *medialinkLibBase;
extern ULONG numEntries1, numDisplay1;
extern LONG topEntry1;
extern ULONG numEntries2, numDisplay2;
extern LONG topEntry2;
extern int xappWdwHeight;
extern struct Gadget ScriptSlider1;
extern struct Gadget ScriptSlider2;
extern struct List **clipLists;
extern struct ObjectInfo ObjectRecord;
extern BOOL EditMenuStates[];
extern UBYTE **msgs;   
extern struct MenuRecord **script_MR;
extern UWORD STD_NON_OBJ_PAL;
extern UWORD STD_LACE_OBJ_PAL;
extern UWORD STD_NON_OBJ_NTSC;
extern UWORD STD_LACE_OBJ_NTSC;
extern UWORD chip gui_pattern[];
extern struct Process *process;
extern TEXT mainName[];

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** ReadScript() ********/

BOOL ReadScript(STRPTR path, STRPTR fileName, char **scriptCommands)
{
struct ParseRecord *PR;
TEXT fullPath[SIZE_FULLPATH];
int instruc, line;
BOOL validScript=TRUE;
BOOL headerPassed=FALSE;
SNRPTR this_node;
UBYTE *buffer;
BOOL InputSettingsSeen=FALSE;
BOOL ProgramSeen=FALSE;
BOOL dongle;

	/**** init vars ****/

	if (path!=NULL)
		UA_MakeFullPath(path, fileName, fullPath);

	InitScriptInfoRecord(&(ObjectRecord.scriptSIR));

	instruc = -1;
	line=0;

	/**** open script file ****/

	if (path!=NULL)
	{
		PR = (struct ParseRecord *)OpenParseFile(scriptCommands, fullPath);
		if (PR==NULL)
		{
			Message(msgs[Msg_UnableToReadScript-1], fileName);
			return(FALSE);
		}
	}

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_ANY | MEMF_CLEAR);
	if ( buffer==NULL )
	{
		CloseParseFile(PR);
		Message(msgs[Msg_UnableToReadScript-1], fileName);
		return(FALSE);
	}

	/**** reset these ****/

	CPrefs.objectPreLoading = 30;	// b+1 scheme
	CPrefs.playOptions			= 3;	// 1=auto, 2=manual, 3=auto+manual
	CPrefs.scriptTiming			= 0;	// 0=normal, 1=precise
	CPrefs.bufferOptions		= 1;	// 0=keep, 1=flush

	mainName[0]='\0';

	/**** parse all lines ****/

	dongle = CheckDongle3();

	while(path!=NULL && instruc!=PARSE_STOP)
	{
		/**** get one line of source ****/

		instruc = GetParserLine((struct ParseRecord *)PR, buffer);

		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buffer);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				if (line>=MAX_PARSER_NODES)	// check for too large script
				{
					Message(msgs[Msg_ScriptTooLarge-1]);
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
				else if (line==0 && PR->commandCode!=TALK_SCRIPTTALK)	/* check for script validity */
				{
					Message(msgs[Msg_ScriptUnreadable-1]);
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
				else
				{
					if ( PR->commandCode == TALK_SCRIPTTALK )
						headerPassed = TRUE;
					else if ( dongle && PR->commandCode == TALK_INPUTSETTINGS )
						InputSettingsSeen=TRUE;
					else if ( PR->commandCode == TALK_PROGRAM )
						ProgramSeen=TRUE;

					PR->sourceLine = line+1;
					PerfFunc((struct GenericFuncs *)scriptFuncs, PR, &(ObjectRecord.scriptSIR));
					if (PR->commandCode == PRINTERROR_CODE)	/* an error was reported so stop parsing */
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
					else if ( PR->commandCode == TALK_SCRIPTTALK )
					{
						/**** update user level ****/
						if (	ObjectRecord.scriptSIR.revision != 0 &&
									ObjectRecord.scriptSIR.revision > CPrefs.userLevel )
						{
							CPrefs.userLevel = ObjectRecord.scriptSIR.revision;
							// START NEW
							if ( CPrefs.userLevel > 3 )
								CPrefs.userLevel = 3;
							// END NEW

#ifndef USED_FOR_PLAYER
							ChangeUserLevel();
#endif
						}
					}
				}
			}
			else
			{
				/* catch weird command after line 0, line 0 is treated above */
				if (line>0 && PR->commandCode==-1)	/* command not valid */
				{
					sprintf(buffer, msgs[Msg_IllegalCommandInLine-1], line+1);
					printError(PR, buffer);
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
			}
		}
		line++;	/* keeps track of number of parsed lines */
	}

	FreeMem(buffer,SCRIPT_LINESIZE);

	if (path!=NULL)
	{
		CloseParseFile(PR);

		if (!headerPassed)
			validScript=FALSE;

		if (!validScript)
			return(FALSE);
	}

	SetBit(&allocFlags, SCRIPTINMEM_FLAG);

	if ( CheckDongle4() )
	{
		if ( ProgramSeen )
			CPrefs.showDays = TRUE;
	}
	else
		CPrefs.standBy = TRUE;

	/**** links every global event with its associated node ****/

	if (path!=NULL)
		ProcessGlobalEvents(&(ObjectRecord.scriptSIR));

	/**** links all documents BUTTONS with LOCALEVENTS in script ****/

	GetAllLEInfos(&(ObjectRecord.scriptSIR));

	/**** clear scrolling area ****/

#ifndef USED_FOR_PLAYER

	if ( allocFlags & SCRIPTSCREEN_FLAG )
	{
		InitObjectArea();
		/**** disable icons ****/
		ShowSerialEventIcons();
	}

#endif

	/**** initialize list and node start ****/

	ObjectRecord.objList = ObjectRecord.scriptSIR.allLists[1];
	ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;

	ObjectRecord.maxObjects = (Script_GR[0].y2-Script_GR[0].y1)/20;

	GetNumObjects();

#ifndef USED_FOR_PLAYER

	if ( allocFlags & SCRIPTSCREEN_FLAG )
	{
		if (CPrefs.ScriptScreenModes & LACE)
			SetFont(scriptWindow->RPort, largeFont);

		UA_EnableButton(scriptWindow, &Script_GR[4]);									// play
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);		// show
		if ( CPrefs.userLevel > 2 )
			UA_EnableButton(scriptWindow, &Script_GR[5]);								// parent

		SetFont(scriptWindow->RPort, smallFont);
	}

#endif

	/**** copy script name to STARTSER of root IF NOT FILLED YET ****/

	this_node = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[0]->lh_TailPred;
	if ( this_node->objectName[0] == '\0' )
	{
		stccpy(this_node->objectName, msgs[Msg_Root-1], MAX_OBJECTNAME_CHARS);
		strcpy(mainName,this_node->objectName);
	}

#ifndef USED_FOR_PLAYER

	if ( allocFlags & SCRIPTSCREEN_FLAG )
	{
		/**** print sub branch name ****/
		if ( this_node )
			PrintSubBranchName(this_node->objectName);
		else
			PrintSubBranchName(NULL);

		/**** init scroll bar values ****/
		topEntry2=0L;
		numEntries2=(LONG)xappWdwHeight;
		numDisplay2=(LONG)Script_GR[2].y2-Script_GR[2].y1-2;

		if ( AbsInt(numDisplay2,numEntries2) < 5 )
			numEntries2 = numDisplay2;

		topEntry1=0L;
		numEntries1=ObjectRecord.numObjects;
		numDisplay1=ObjectRecord.maxObjects;

		/**** init script screen ornaments ****/

		UA_InitPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
		UA_InitPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
	
		DrawObjectList(0, TRUE, TRUE);	/* top, draw all, force */
	}

#endif

	/**** if this script has global events, show tool in list ****/

	if ( ObjectRecord.scriptSIR.globallocalEvents[0] != NULL )
		createGlobalEntry();

	/**** if this script has an input settings object ****/

	if ( InputSettingsSeen )
		createInputSettings();

	/**** if this script has variables, show tool in list ****/

	if (	ObjectRecord.scriptSIR.VIList.lh_TailPred !=
				(struct Node *)&(ObjectRecord.scriptSIR.VIList) )
		createVarsEntry();

	/**** fill in label SNR's that could not be resolved while loading script ****/

	if ( dongle )
		FillInLabelPointers(&(ObjectRecord.scriptSIR));

	/**** set numericalArgs[0] to defer or continue ****/

	if ( dongle )
		UpdateDeferCont(&(ObjectRecord.scriptSIR));

#ifndef USED_FOR_PLAYER

	if ( allocFlags & SCRIPTSCREEN_FLAG )
	{
		/**** dim menus ****/
		doShowAndProgMenus();

		for(line=0; line<CPrefs.MaxNumLists; line++)
		{
			if ( clipLists[line] != NULL )
			{
				EnableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
				break;
			}
		}
	
		if (ObjectRecord.numObjects==0)
			DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
		else
			EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);

		DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}

#endif

	return(TRUE);
}

/******** freeScript() ********/

void freeScript(void)
{
	FreeScriptInfoRecord(&(ObjectRecord.scriptSIR));
}

#ifndef USED_FOR_PLAYER

/******** InitObjectArea() ********/

void InitObjectArea(void)
{
	/**** clear scroll area ****/

	SetAPen(scriptWindow->RPort, AREA_PEN);
	SetDrMd(scriptWindow->RPort, JAM1);
	if (CPrefs.ScriptScreenModes & LACE)
		RectFill(scriptWindow->RPort, Script_GR[0].x1+2, Script_GR[0].y1+2, Script_GR[0].x2-2, Script_GR[0].y2-2);
	else
		RectFill(scriptWindow->RPort, Script_GR[0].x1+2, Script_GR[0].y1+1, Script_GR[0].x2-2, Script_GR[0].y2-1);
	WaitBlit();

	/**** draw dotted lines ****/

	DrawDottedLines();
}

/******** DrawDottedLines() ********/

void DrawDottedLines(void)
{
int i,lines,offset;

	lines = (Script_GR[0].y2-Script_GR[0].y1) / 20;
	SetAPen(scriptWindow->RPort, LO_PEN);
	SetDrMd(scriptWindow->RPort, JAM1);
	SetDrPt(scriptWindow->RPort, 0xaaaa);

	if ( CPrefs.ScriptScreenModes & LACE)
		offset=1;
	else
		offset=0;

	for(i=1; i<lines; i++)
	{
		Move(scriptWindow->RPort, Script_GR[0].x1+2, Script_GR[0].y1+offset+20*i);
		Draw(scriptWindow->RPort, Script_GR[0].x2-2, Script_GR[0].y1+offset+20*i);
	}

	SetDrPt(scriptWindow->RPort, 0xffff);
}

#endif

/******** GetNumObjects() ********/

void GetNumObjects(void)
{
SNRPTR this_node;

	ObjectRecord.numObjects=0;
	if (ObjectRecord.objList->lh_TailPred != (struct Node *)ObjectRecord.objList)
	{
		for(this_node=ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
		{
			ObjectRecord.numObjects++;
		}
	}
}

/******** GetMemSize() ********/
/*
 * appName is without .info !
 *
 */

void GetMemSize(STRPTR appName, int *memSize)
{
struct DiskObject *diskObj;
char *s;

	*memSize = USERAPPLIC_MEMSIZE;

	diskObj = GetDiskObject(appName);
	if ( diskObj==NULL )
		return;

	s = (char *)FindToolType(diskObj->do_ToolTypes, "MEMSIZE");
	if ( s!=NULL )
		sscanf(s, "%d", memSize);

	FreeDiskObject(diskObj);
}

/******** PerfFunc() ********/
/* input:    pointer to an array of pointers to functions, a pointer to
 *           a filled ParseRecord which holds the currently parsed command
 *           and a pointer to the current ScriptInfoRecord.
 * output:   -
 * function: jumps to a function, carrying along the PR and SIR pointers.
 */

void PerfFunc(struct GenericFuncs *FuncList, struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
void (*func)(APTR, APTR);

	func = FuncList[PR->commandCode].func;
	if (func!=NO_FUNCTION)
		(*(func))(PR,SIR);
}

/******** createGlobalEntry() ********/

void createGlobalEntry(void)
{
SNRPTR new_node, this_node;

	/**** allocate new node ****/

	new_node = (struct ScriptNodeRecord *)AllocateNode();
	if (new_node==NULL)
		return;

	new_node->nodeType = TALK_GLOBALEVENT;
	stccpy(new_node->objectName, msgs[Msg_GlobalEvents-1], MAX_OBJECTNAME_CHARS);

	/**** we're attaching things to the root list ****/

	this_node = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[0]->lh_TailPred;
	this_node = (struct ScriptNodeRecord *)this_node->node.ln_Pred;

	Insert((struct List *)ObjectRecord.scriptSIR.allLists[0], (struct Node *)new_node, (struct Node *)this_node);
}

/******** createInputSettings() ********/

void createInputSettings(void)
{
SNRPTR new_node, this_node;

	/**** allocate new node ****/

	new_node = (struct ScriptNodeRecord *)AllocateNode();
	if (new_node==NULL)
		return;

	new_node->nodeType = TALK_INPUTSETTINGS;
	stccpy(new_node->objectName, msgs[Msg_InputSettings-1], MAX_OBJECTNAME_CHARS);

	/**** we're attaching things to the root list ****/

	this_node = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[0]->lh_TailPred;
	this_node = (struct ScriptNodeRecord *)this_node->node.ln_Pred;

	Insert((struct List *)ObjectRecord.scriptSIR.allLists[0], (struct Node *)new_node, (struct Node *)this_node);
}

/******** createVarsEntry() ********/

void createVarsEntry(void)
{
SNRPTR new_node, this_node;

	/**** allocate new node ****/

	new_node = (struct ScriptNodeRecord *)AllocateNode();
	if (new_node==NULL)
		return;

	new_node->nodeType = TALK_VARS;
	stccpy(new_node->objectName, msgs[Msg_Vars-1], MAX_OBJECTNAME_CHARS);

	new_node->list = (struct List *)AllocMem(sizeof(struct List),MEMF_CLEAR|MEMF_ANY);
	NewList( new_node->list );

	/**** we're attaching things to the root list ****/

	this_node = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[0]->lh_TailPred;
	this_node = (struct ScriptNodeRecord *)this_node->node.ln_Pred;

	Insert((struct List *)ObjectRecord.scriptSIR.allLists[0], (struct Node *)new_node, (struct Node *)this_node);
}

/******** E O F ********/
