#include "nb:pre.h"

#define CUT_EFFECT 0

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Window *scriptWindow;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern BOOL ToolEnabledList[];	// index is type number
extern int objectTypeList[];
extern UBYTE *objectNameList[];
extern BOOL IconEnabledList[];
extern struct ObjectInfo ObjectRecord;
extern struct RastPort scratchRP;
extern struct Gadget ScriptSlider1;
extern struct RastPort gfxRP;
extern struct RastPort xappRP;
extern struct RastPort xappRP_2;
extern struct MenuRecord **script_MR;
extern ULONG numEntries1, numDisplay1;
extern LONG topEntry1;
extern TEXT *dir_xapps;
extern int numLevelTools;
extern UBYTE **msgs;
extern int clipLine;
extern struct FileListInfo FLI;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/******** StartIconDragging() ********/
/*
 * mode==1 means drag objects, 2 means paste objects
 *
 */

void StartIconDragging(int top1, int top2, int mode)
{
ULONG signals;
BOOL loop, mouseMoved, doIt=FALSE;
struct IntuiMessage *message;
int start_x, start_y, icon, iconwidth, xOffset, yOffset, dx, dy, drawn=0;
int col,row;

	if (mode==2 && ObjectRecord.numObjects==0)
	{
		PasteClipboard(&(ObjectRecord.scriptSIR), top1, 0, Script_GR[2].y1);
		if (ObjectRecord.numObjects>0)
			EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
		return;
	}

	UA_SwitchMouseMoveOn(scriptWindow);

	CED.MouseX = scriptWindow->MouseX;
	CED.MouseY = scriptWindow->MouseY;

	start_x = CED.MouseX;
	start_y = CED.MouseY;

	if (mode==1)
	{
		start_x -= Script_GR[2].x1;
		start_y -= Script_GR[2].y1;

		iconwidth = TOOLSWIDTH;
		iconwidth /= 3;
		col = start_x / iconwidth;

		row = (start_y + clipLine) / (ICONHEIGHT+4);
		icon = (row*3) + col;

		if ( icon > numLevelTools-1 )
		{
			UA_SwitchMouseMoveOff(scriptWindow);
			return;
		}

		if ( !IconEnabledList[icon] )
		{
			UA_SwitchMouseMoveOff(scriptWindow);
			return;
		}

		xOffset = objectXPosList[icon] - start_x + 2;
		yOffset = objectYPosList[icon] - start_y + 2;

		yOffset -= clipLine;
	}
	else
	{
		icon=-1;
		xOffset = -(ICONWIDTH/2);
		yOffset = -(ICONHEIGHT/2);
	}

	/**** copy area under mouse pointer to off-screen area ****/

	start_x=scriptWindow->MouseX;
	start_y=scriptWindow->MouseY;
	ClipBlit(	scriptWindow->RPort, start_x, start_y,
						&scratchRP,0,0,ICONWIDTH,ICONHEIGHT,0xc0);

	if (mode==2)
	{
		DrawScriptIcon(	icon, CED.MouseX+xOffset, CED.MouseY+yOffset,
										start_x, start_y);

		start_x=CED.MouseX+xOffset;
		start_y=CED.MouseY+yOffset;
	}
	if (mode==1)
	{
		dx = CED.MouseX + xOffset;
		dy = CED.MouseY + yOffset;
		SetDrMd(scriptWindow->RPort,JAM2|COMPLEMENT);
		DrawSimpleBox(scriptWindow->RPort, dx-1, dy-1, dx+ICONWIDTH, dy+ICONHEIGHT );
		DrawSimpleBox(scriptWindow->RPort, dx-2, dy-2, dx+ICONWIDTH+1, dy+ICONHEIGHT+1 );
		SetDrMd(scriptWindow->RPort,JAM1);
	}

	/**** event handler ****/

	loop=TRUE;
	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class		= message->Class;
				CED.Code		= message->Code;
				CED.MouseX	= message->MouseX+xOffset;
				CED.MouseY	= message->MouseY+yOffset;

				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
						{
							loop=FALSE;
							doIt=TRUE;
						}
						else if (CED.Code == MENUUP | CED.Code == MENUDOWN)
							loop=FALSE;
						break;

					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}
			if (mouseMoved)
			{
				if (mode==1 && drawn==0)
				{
					SetDrMd(scriptWindow->RPort,JAM2|COMPLEMENT);
					DrawSimpleBox(scriptWindow->RPort, dx-1, dy-1, dx+ICONWIDTH, dy+ICONHEIGHT );
					DrawSimpleBox(scriptWindow->RPort, dx-2, dy-2, dx+ICONWIDTH+1, dy+ICONHEIGHT+1 );
					SetDrMd(scriptWindow->RPort,JAM1);
					drawn=1;
				}
				DrawScriptIcon(icon, CED.MouseX, CED.MouseY, start_x, start_y);
				start_x=CED.MouseX;
				start_y=CED.MouseY;
			}
		}
	}

	if (mode==1 && drawn==0)
	{
		SetDrMd(scriptWindow->RPort,JAM2|COMPLEMENT);
		DrawSimpleBox(scriptWindow->RPort, dx-1, dy-1, dx+ICONWIDTH, dy+ICONHEIGHT );
		DrawSimpleBox(scriptWindow->RPort, dx-2, dy-2, dx+ICONWIDTH+1, dy+ICONHEIGHT+1 );
		SetDrMd(scriptWindow->RPort,JAM1);
	}

	/**** copy off-screen area back to window ****/

	ClipBlit(	&scratchRP,0,0,
						scriptWindow->RPort,start_x,start_y,ICONWIDTH,ICONHEIGHT,0xc0);

	if (doIt)
	{
		if ( start_x<=(Script_GR[0].x2-ICONWIDTH) )
		{
			if (mode==1)
				PasteDraggedIcon(icon, top1, start_x, start_y);
			else
			{
				PasteClipboard(&(ObjectRecord.scriptSIR), top1, start_x, start_y);
				if (ObjectRecord.numObjects>0)
					EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
			}
		}
	}

	UA_SwitchMouseMoveOff(scriptWindow);
}

/******** DrawScriptIcon() ********/

void DrawScriptIcon(int icon, int mouseX, int mouseY, int oldX, int oldY)
{
	/**** copy off-screen area back to previously obscured window part ****/

	ClipBlit(	&scratchRP, 0,0,
						scriptWindow->RPort, oldX,oldY, ICONWIDTH,ICONHEIGHT, 0xc0);

	/**** copy area under new mouse pointer to off-screen area ****/

	ClipBlit(	scriptWindow->RPort, mouseX,mouseY,
						&scratchRP, 0,0, ICONWIDTH,ICONHEIGHT, 0xc0);

	/**** copy icon to area under mouse pointer ****/

	if (icon!=-1)
		ClipBlit(	&xappRP_2,
							objectXPosList[icon],
							objectYPosList[icon],
							scriptWindow->RPort, mouseX,mouseY, ICONWIDTH,ICONHEIGHT, 0xc0);
	else	// render scissors
	{
		ClipBlit(	&gfxRP, GFX_SCIS_X, GFX_SCIS_Y,
							scriptWindow->RPort, mouseX,mouseY, ICONWIDTH,ICONHEIGHT, 0xc0);
	}
}

/******** PasteDraggedIcon() ********/

void PasteDraggedIcon(int icon, int top, int start_x, int start_y)
{
SNRPTR this_node, new_node;
int i, row, where, iconwidth, col;
BOOL retval, skip, noFR=FALSE;
TEXT path[256], filename[100];

	if (ObjectRecord.objList == NULL)
		return;

	/**** calculate which object POS we clicked on *****/

	this_node=NULL;
	new_node=NULL;
	where=-1;

	if ( start_x==-1 && start_y==-1 )	// paste to end
	{
		where = ADD_TO_TAIL;
		iconwidth = TOOLSWIDTH;
		iconwidth /= 3;
		col = (CED.MouseX-Script_GR[2].x1)/iconwidth;
		row = ((CED.MouseY-Script_GR[2].y1)+clipLine)/(ICONHEIGHT+4);

		if ( icon==-1 )	// if not -1, icon# is supplied by caller
			icon = (row*3)+col;
		else
			noFR = TRUE;

		if ( icon > numLevelTools-1 )
			return;
		if ( !IconEnabledList[icon] )
			return;
		if ( ObjectRecord.numObjects==0 )
			where = ADD_TO_HEAD;
	}
	else
	{
		if ( (start_y < Script_GR[0].y1) || ObjectRecord.numObjects==0)
		{
			this_node = NULL;
			where = ADD_TO_HEAD;
		}
		else
		{
			start_y += ( (ICONHEIGHT/2)-11 );
			row = ( ( (start_y - Script_GR[0].y1) / 20 ) + top );

			if (row<ObjectRecord.numObjects)
			{
				/**** with the object POS, find the object NODE ****/

				if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
					return;

				this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

				if (row>0)
				{
					for(i=0; i<row; i++)
					{
						if (this_node!=NULL)
							this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
					}
				}

				if (this_node==NULL ||
						this_node==(struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred)
					where = ADD_TO_TAIL;
				else
					where = ADD_TO_MIDDLE;
			}
			else
				where = ADD_TO_TAIL;
		}
	}

	if (where==-1)
	{
		UA_WarnUser(158);
		return;
	}

	/**** add the object to the list ****/

	new_node = (struct ScriptNodeRecord *)AddObjectToList(where, icon, this_node, NULL, NULL);
	if (new_node == NULL)
		return;

	/**** do already a refresh display ****/

	GetNumObjects();
	numEntries1=ObjectRecord.numObjects;

	if (where == ADD_TO_TAIL || ( (start_y+ICONHEIGHT) > Script_GR[0].y2) )
	{
		if (ObjectRecord.numObjects > ObjectRecord.maxObjects)
			topEntry1++;
	}

	if ( start_x!=-1 )
		UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);

	if (where == ADD_TO_TAIL || ((start_y+ICONHEIGHT) > Script_GR[0].y2) )
		DrawObjectList(topEntry1, TRUE, TRUE);	/* new top, draw all, force */
	else
		DrawObjectList(-1, TRUE, TRUE);	/* use last top, draw all, force */

	if (where == ADD_TO_HEAD)
		where = ADD_TO_MIDDLE;	/* else several icons get stuck at start */

	/**** add more objects ****/

	retval=FALSE;	/* IF TRUE LIST IS REDRAWN */

	if ( !noFR )
	{
		switch(objectTypeList[icon])
		{
			case TALK_ANIM:
			case TALK_BINARY:
			case TALK_MAIL:
			case TALK_PAGE:
			case TALK_SOUND:

				if (objectTypeList[icon]==TALK_ANIM)
				{
					stccpy(path, CPrefs.anim_Path, 256);
					retval = OpenAFile(	path, filename, msgs[Msg_SelectAnims-1], scriptWindow,
															DIR_OPT_ANIM | DIR_OPT_NOINFO, TRUE );
					if (retval)
					{
						stccpy(CPrefs.anim_Path, path, SIZE_PATH);
						UA_ValidatePath(path);
					}
				}
				else if (objectTypeList[icon]==TALK_BINARY)
				{
					stccpy(path, CPrefs.import_picture_Path, 256);
					retval = OpenAFile(	path, filename, msgs[Msg_SelectFiles-1], scriptWindow,
															DIR_OPT_ALL | DIR_OPT_NOINFO, TRUE );
					if (retval)
					{
						stccpy(CPrefs.import_picture_Path, path, SIZE_PATH);
						UA_ValidatePath(path);
					}
				}
				else if (objectTypeList[icon]==TALK_MAIL)
				{
					stccpy(path, CPrefs.import_picture_Path, 256);
					retval = OpenAFile(	path, filename, msgs[Msg_SelectMailFiles-1], scriptWindow,
															DIR_OPT_ALL | DIR_OPT_NOINFO, TRUE );
					if (retval)
					{
						stccpy(CPrefs.import_picture_Path, path, SIZE_PATH);
						UA_ValidatePath(path);
					}
				}
				else if (objectTypeList[icon]==TALK_PAGE)
				{
					stccpy(path, CPrefs.import_picture_Path, 256);
					retval = OpenAFile(	path, filename, msgs[Msg_SelectDocsOrPics-1], scriptWindow,
															DIR_OPT_ILBM | DIR_OPT_THUMBS | DIR_OPT_NOINFO, TRUE );
					if (retval)
					{
						stccpy(CPrefs.import_picture_Path, path, SIZE_PATH);
						UA_ValidatePath(path);
					}
				}
				else if (objectTypeList[icon]==TALK_SOUND)
				{
					stccpy(path, CPrefs.music_Path, 256);
					retval = OpenAFile(	path, filename, msgs[Msg_SelectSoundFiles-1], scriptWindow,
															DIR_OPT_MUSIC | DIR_OPT_NOINFO, TRUE );
					if (retval)
					{
						stccpy(CPrefs.music_Path, path, SIZE_PATH);
						UA_ValidatePath(path);
					}
				}

				if (FLI.selectionList!=NULL)
				{
					SetSpriteOfActWdw(SPRITE_BUSY);

					if (retval)
					{
						/**** one object already has been placed. This is ****/
						/**** new_node. Only now I know its name, so fill ****/
						/**** it in. ****/

						PrepareAddedNode((struct ScriptNodeRecord *)new_node, path, filename, icon);

						/**** get next filename, skip first ****/

						skip=TRUE;
						for(i=0; i<(int)MAX_FILE_LIST_ENTRIES; i++)
						{
					 		if ( *(FLI.selectionList+i) == 1 && skip)
								skip=FALSE;
							else if ( *(FLI.selectionList+i) == 1 && !skip)
								new_node = (SNRPTR)AddObjectToList(	where, icon, new_node,
																				path, FLI.fileList+1+SIZE_FILENAME*i);
						}
					}

					SetSpriteOfActWdw(SPRITE_NORMAL);

					CloseDir();
				}
				break;

			case TALK_AREXX:
			case TALK_DOS:
			case TALK_GOTO:
			case TALK_USERAPPLIC:
				processDblClick(0, new_node);
				break;

			case TALK_GLOBALEVENT:
				DisableTool(TALK_GLOBALEVENT);
				ShowToolIcons(scriptWindow, -2);
				processDblClick(0, new_node);
				break;

			case TALK_INPUTSETTINGS:
				DisableTool(TALK_INPUTSETTINGS);
				ShowToolIcons(scriptWindow, -2);
				processDblClick(0, new_node);
				break;

			case TALK_TIMECODE:
				DisableTool(TALK_TIMECODE);
				ShowToolIcons(scriptWindow, -2);
				processDblClick(0, new_node);
				break;

			case TALK_VARS:
				if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] )
				{
					DisableTool(TALK_VARS);
					ShowToolIcons(scriptWindow, -2);
				}
				processDblClick(0, new_node);
				break;

			case TALK_LABEL:
				DoObjectName(new_node->objectName, msgs[Msg_UniqueLabel-1], new_node->nodeType);
				retval=TRUE; /* redraw list */
				break;

			case TALK_STARTPAR:
				DoObjectName(new_node->objectName, msgs[Msg_NameParallel-1], new_node->nodeType);
				retval=TRUE; /* redraw list */
				break;

			case TALK_STARTSER:
				DoObjectName(new_node->objectName, msgs[Msg_NameSerial-1], new_node->nodeType);
				retval=TRUE; /* redraw list */
				break;
		}
	}
	else
		retval=TRUE;

	/**** refresh display ****/

	if (retval)
	{
		GetNumObjects();
		numEntries1 = ObjectRecord.numObjects;

		if ( start_x!=-1 )
			UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);

		if (where == ADD_TO_TAIL || ((start_y+ICONHEIGHT) > Script_GR[0].y2))
			DrawObjectList(topEntry1, TRUE, TRUE);	/* new top, draw all, force */
		else
			DrawObjectList(-1, TRUE, TRUE);	/* use last top, draw all, force */

		doShowAndProgMenus();
	}

	if ( new_node )
		SetScriptMenus();
}

/******** AddObjectToList() ********/

struct ScriptNodeRecord *AddObjectToList(int where, int icon, struct ScriptNodeRecord *this_node, STRPTR path, STRPTR filename)
{
struct ScriptNodeRecord *new_node;

	/**** allocate new node ****/

	new_node = (struct ScriptNodeRecord *)AllocateNode();
	if (new_node==NULL)
		return(NULL);

	new_node->nodeType = objectTypeList[icon];

	/**** add node to list ****/

	if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] )
	{
		/**** we're attaching things to the root list ****/
		if (ObjectRecord.numObjects>1)
		{
			this_node = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred;
			this_node = (struct ScriptNodeRecord *)this_node->node.ln_Pred;
		}
		else
		{
			this_node = (struct ScriptNodeRecord *)ObjectRecord.firstObject;
			this_node = (struct ScriptNodeRecord *)this_node->node.ln_Pred;
			ObjectRecord.firstObject = (struct ScriptNodeRecord *)new_node;
		}
		where = ADD_TO_MIDDLE;
	}

	if (where == ADD_TO_HEAD)
	{
		AddHead((struct List *)ObjectRecord.objList, (struct Node *)new_node);
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
	}
	else if (where == ADD_TO_TAIL)
	{
		AddTail((struct List *)ObjectRecord.objList, (struct Node *)new_node);
	}
	else	/* ADD_TO_MIDDLE */
	{
		Insert((struct List *)ObjectRecord.objList, (struct Node *)new_node, (struct Node *)this_node);
	}

	/**** fill info ****/

	PrepareAddedNode((struct ScriptNodeRecord *)new_node, path, filename, icon);

	return(new_node);
}

/******** PrepareAddedNode() ********/

void PrepareAddedNode(struct ScriptNodeRecord *this_node, STRPTR path,
											STRPTR filename, int icon)
{
int i, memSize, fps, numFrames;
struct List *list;
TEXT fullPath[SIZE_FULLPATH];
ULONG type;

	if (path!=NULL && filename!=NULL)
	{
		stccpy(this_node->objectPath, path, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, filename, MAX_OBJECTNAME_CHARS);
	}

	if (this_node->nodeType == TALK_ANIM)
	{
		/* scriptTiming = 0 for defer and 1 for continu */
		this_node->numericalArgs[0] = CPrefs.scriptTiming+1;

		this_node->numericalArgs[1] = 0;		// not used
		this_node->numericalArgs[2] = CUT_EFFECT;		// wipe nr.
		this_node->numericalArgs[3] = 20;		// speed
		this_node->numericalArgs[4] = 0;		// thickness
		this_node->numericalArgs[5] = 0;		// variation
		this_node->numericalArgs[6] = 0;		// xpos
		this_node->numericalArgs[7] = 0;		// ypos
		this_node->numericalArgs[8] = 0;		// wipe out
		this_node->numericalArgs[9] = 0;		// speed (0 = use file setting)
		this_node->numericalArgs[10] = 1;		// rot
		this_node->numericalArgs[11] = 0;		// bit 0 looping, bit 1 disk anim
		this_node->numericalArgs[12] = 0;		// color cycle

		if (path!=NULL && filename!=NULL)
		{
			GetAnimSpeed(path, filename, &fps, &numFrames);
			this_node->numericalArgs[9] = fps;
		}
	}
	else if (this_node->nodeType == TALK_AREXX)
	{
		this_node->numericalArgs[0] = ARGUMENT_DEFER;
		this_node->numericalArgs[1] = ARGUMENT_COMMAND; //ARGUMENT_SCRIPT;
		/**** this alloc also appears in ScriptTalk.c ****/
		this_node->extraData = (UBYTE *)AllocMem(AREXX_MEMSIZE, MEMF_PUBLIC | MEMF_CLEAR);
		if (this_node->extraData==NULL)
			UA_WarnUser(159);
		this_node->extraDataSize = AREXX_MEMSIZE;
	}
	else if (this_node->nodeType == TALK_DOS)
	{
		this_node->numericalArgs[0] = ARGUMENT_DEFER;
		this_node->numericalArgs[1] = ARGUMENT_COMMAND;
		this_node->numericalArgs[2] = 4096;	// stack size
		/**** this alloc also appears in ScriptTalk.c ****/
		this_node->extraData = (UBYTE *)AllocMem(DOS_MEMSIZE, MEMF_PUBLIC | MEMF_CLEAR);
		if (this_node->extraData==NULL)
			UA_WarnUser(160);
		this_node->extraDataSize = DOS_MEMSIZE;
	}
	else if (this_node->nodeType == TALK_GLOBALEVENT)
	{
		stccpy(this_node->objectName, msgs[Msg_GlobalEvents-1], MAX_OBJECTNAME_CHARS);
	}
	else if (this_node->nodeType == TALK_INPUTSETTINGS)
	{
		stccpy(this_node->objectName, msgs[Msg_InputSettings-1], MAX_OBJECTNAME_CHARS);
	}
	else if (this_node->nodeType == TALK_VARS)
	{
		this_node->numericalArgs[0] = TG_CONDITIONJUMP;

		stccpy(this_node->objectName, msgs[Msg_Vars-1], MAX_OBJECTNAME_CHARS);

		this_node->list = (struct List *)AllocMem(sizeof(struct List),MEMF_ANY|MEMF_CLEAR);
		if ( this_node->list )
			NewList( this_node->list );
	}
	else if (this_node->nodeType == TALK_PAGE)
	{
		/* scriptTiming = 0 for defer and 1 for continu */
		this_node->numericalArgs[0] = CPrefs.scriptTiming+1;

		this_node->numericalArgs[1] = 0;		// not used
		this_node->numericalArgs[2] = CUT_EFFECT;		// wipe nr.
		this_node->numericalArgs[3] = 20;		// speed
		this_node->numericalArgs[4] = 0;		// thickness
		this_node->numericalArgs[5] = 0;		// variation
		this_node->numericalArgs[6] = 0;		// xpos
		this_node->numericalArgs[7] = 0;		// ypos
		this_node->numericalArgs[8] = 0;		// wipe out
		this_node->numericalArgs[9] = 0;		// not used ??????????
		this_node->numericalArgs[12] = 0;		// color cycle

		if (path!=NULL && filename!=NULL)
		{
			UA_MakeFullPath(path, filename, fullPath);
			type = checkFileType(fullPath, NULL);
			if (type==ILBM)
				this_node->numericalArgs[15] = 2;	// ILBM IFF
			else
			{
				this_node->numericalArgs[15] = 1;	// pagetalk document
				//GetLEInfo(this_node,fullPath,&(ObjectRecord.scriptSIR));
				UpdateLEInfo(this_node,fullPath,&(ObjectRecord.scriptSIR));
			}
		}
		else
			this_node->numericalArgs[15] = 0;
	}
	else if (this_node->nodeType == TALK_TIMECODE)
	{
		stccpy(this_node->objectName, msgs[Msg_TimeCode-1], MAX_OBJECTNAME_CHARS);
	}
	else if (this_node->nodeType == TALK_USERAPPLIC)
	{
		this_node->numericalArgs[0] = 1;	// DEFER

		stccpy(this_node->objectPath, objectNameList[icon], MAX_PARSER_CHARS);

		/**** the list scroller needs to find the XaPP icons fast.			****/
		/**** In order to do so, I (mis)use a numericalArgs field to		****/
		/**** store a value which points to where the scroller can find ****/
		/**** the icon.																									****/

		i=0;
		while ( objectNameList[i] != NULL )
		{
			if (strcmpi(objectNameList[i],this_node->objectPath)==0)
			{
				this_node->numericalArgs[MAX_PARSER_ARGS-1] = i;
				break;
			}
			i++;
		}

		/**** this alloc also appears in ScriptTalk.c ****/

		UA_MakeFullPath(dir_xapps, this_node->objectPath, fullPath);
		GetMemSize(fullPath, &memSize);
		this_node->extraData = (UBYTE *)AllocMem(memSize, MEMF_ANY | MEMF_CLEAR);
		if (this_node->extraData==NULL)
			UA_WarnUser(161);
		this_node->extraDataSize = memSize;
	}
	else if (	this_node->nodeType == TALK_STARTSER || this_node->nodeType == TALK_STARTPAR )
	{
		list = (struct List *)InitScriptList();
		if (list==NULL)
			UA_WarnUser(162);
		else
		{
			/**** find a place to store the new list ****/

			this_node->list = list;

			for(i=0; i<CPrefs.MaxNumLists; i++)
			{
				if (ObjectRecord.scriptSIR.allLists[i] == NULL)	/* put new branch into SIR */
				{
					ObjectRecord.scriptSIR.allLists[i] = list;
					break;
				}
			}
		}
	}
	else if (this_node->nodeType == TALK_SOUND)
	{
		this_node->numericalArgs[0] = 2;		// default is NOT to wait but to loop
		this_node->numericalArgs[1] = 0;		// default is NOT to stop
	}
	else if (this_node->nodeType == TALK_GOTO)
	{
		this_node->numericalArgs[0] = TG_GOTO;
	}

	/**** general info ****/

	if (ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
	{
		this_node->duration	= DEFAULT_DELAY*10;	/* 10 seconds */
		this_node->dayBits	= 127;
	}
	else
	{
		/**** see also list.c (also sets timecodes) ****/

		this_node->Start.TimeCode.HH = 00;
		this_node->Start.TimeCode.MM = 00;
		this_node->Start.TimeCode.SS = 00;
		this_node->Start.TimeCode.FF = 00;

		this_node->End.TimeCode.HH = 00;
		this_node->End.TimeCode.MM = 00;
		this_node->End.TimeCode.SS = 00;
		this_node->End.TimeCode.FF = 00;
	}

	this_node->miscFlags = OBJ_SELECTED;
}

/******** GetAnimSpeed() ********/

void GetAnimSpeed(STRPTR path, STRPTR filename, int *fps, int *numFrames)
{
DPAnimChunk DPAN_chunk;
ULONG HB;
TEXT fullPath[256];

	UA_MakeFullPath(path, filename, fullPath);

	if ( GetInfoOnForm(fullPath,DPAN,(UBYTE *)&DPAN_chunk,sizeof(DPAN_chunk),NULL) )
	{
		*numFrames = DPAN_chunk.nframes;		

		HB = DPAN_chunk.flags;
		HB = HB >> 24;
		*fps = (int)HB;
		if (*fps<1 || *fps>50)
			*fps = DEFAULT_ANIM_SPEED;
	}
	else
	{
		*numFrames = -1;	// unknown
		*fps = DEFAULT_ANIM_SPEED;
	}

	//this_node->numericalArgs[9] = fps;
}

/******** E O F ********/
