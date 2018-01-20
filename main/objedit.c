#include "nb:pre.h"
#include "nb:xapp_names.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct ObjectInfo ObjectRecord;
extern struct RastPort xappRP;
extern struct RastPort xappRP_2;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern TEXT *dir_system;
extern UBYTE **msgs;   

/**** gadgets ****/

extern struct GadgetRecord SharedWdw_GR[];
extern struct GadgetRecord ObjectNameWdw_GR[];
extern struct GadgetRecord ArexxWdw_GR[];
extern struct GadgetRecord DosWdw_GR[];
extern struct GadgetRecord AnimWdw_GR[];
extern struct GadgetRecord PageWdw_GR[];
extern struct GadgetRecord SoundWdw_GR[];

/**** functions ****/

/******** DoObjectName() ********/

void DoObjectName(STRPTR objectName, STRPTR title, int nodeType)
{
int ID;
BOOL loop=TRUE, retval;
struct ScriptNodeRecord *foundNode;
TEXT tempName[40];
struct Window *window;

	UA_WipeStringGadget(&ObjectNameWdw_GR[3]);	// fill string with zeros

	window = Open_A_Request_Window(scriptWindow, ObjectNameWdw_GR);
	if ( !window )
		return;

	/**** print what kinda object name we need ****/

	UA_DrawSpecialGadgetText(window, &ObjectNameWdw_GR[2], title, SPECIAL_TEXT_LEFT);

	/**** set string gadget to current object name ****/

	UA_SetStringGadgetToString(window, &ObjectNameWdw_GR[3], objectName);

	/**** active string gadget ****/

	UA_ProcessStringGadget(	window, ObjectNameWdw_GR,	&ObjectNameWdw_GR[3], &CED);

	/**** copy entered string to tempName ****/

	UA_SetStringToGadgetString(&ObjectNameWdw_GR[3], tempName);

	/**** check if entered object name is valid ****/

	ValidateLabelName(tempName);
	UA_SetStringGadgetToString(window, &ObjectNameWdw_GR[3],	tempName);

	/**** check if entered object name doesn't already exist ****/

	if (	nodeType==TALK_LABEL ||
				nodeType==TALK_STARTSER ||
				nodeType==TALK_STARTPAR )	/* they must have unique names */
	{
		if ( stricmp(objectName, tempName)!=0 )
		{
			foundNode = FindLabel(&(ObjectRecord.scriptSIR), tempName);
			if (foundNode != NULL || tempName[0]==' ')
			{
				DisplayBeep(scriptScreen);
				UA_DisableButton(window, &ObjectNameWdw_GR[4], gui_pattern);	/* OK */
			}
		}
	}

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				switch(CED.Code)
				{
					case SELECTDOWN:
						ID = UA_CheckGadgetList(window, ObjectNameWdw_GR, &CED);
						switch(ID)
						{
							case 3:
								UA_ProcessStringGadget(window, ObjectNameWdw_GR, &ObjectNameWdw_GR[3], &CED);
								UA_SetStringToGadgetString(&ObjectNameWdw_GR[3], tempName);
								ValidateLabelName(tempName);
								UA_SetStringGadgetToString(window, &ObjectNameWdw_GR[3], tempName);

								if (	nodeType==TALK_LABEL ||
											nodeType==TALK_STARTSER ||
											nodeType==TALK_STARTPAR )	/* they must have unique names */
								{
									if ( stricmp(objectName, tempName)!=0 )
									{
										foundNode = FindLabel(&(ObjectRecord.scriptSIR), tempName);
										if (foundNode != NULL || tempName[0]==' ')
										{
											DisplayBeep(scriptScreen);
											UA_DisableButton(window, &ObjectNameWdw_GR[4], gui_pattern);	/* OK */
										}
										else
											UA_EnableButton(window, &ObjectNameWdw_GR[4]);	/* OK */
									}
									else
										UA_EnableButton(window, &ObjectNameWdw_GR[4]);	/* OK */
								}
								break;				

							case 4:	// OK
do_ok:
								UA_HiliteButton(window, &ObjectNameWdw_GR[4]);
								loop=FALSE;
								retval=TRUE;
								break;

							case 5:	// Cancel
do_cancel:
								UA_HiliteButton(window, &ObjectNameWdw_GR[5]);
								loop=FALSE;
								retval=FALSE;
								break;
						}
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)
					goto do_cancel;
				else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&ObjectNameWdw_GR[4]))
					goto do_ok;
				break;
		}
	}

	UA_EnableButton(window, &ObjectNameWdw_GR[4]);	// OK

	Close_A_Request_Window(window);

	if (retval)
	{
		if (	nodeType==TALK_LABEL ||
					nodeType==TALK_STARTSER ||
					nodeType==TALK_STARTPAR )	/* they must have unique names */
		{
			/**** change all objectNames into tempNames ****/
			ChangeGotosAndGlobals(&(ObjectRecord.scriptSIR), objectName, tempName);
		}

		UA_SetStringToGadgetString(&ObjectNameWdw_GR[3], objectName);

		if (objectName[0]==' ')
			objectName[0]='\0';
	}
}

/******** ValidateLabelName() ********/

void ValidateLabelName(STRPTR str)
{
int i;

	for(i=0; i<strlen(str); i++)
		if (str[i]=='\"' || str[i]=='\'')
			str[i]=' ';
}

/******** Open_A_Request_Window() ********/

struct Window *Open_A_Request_Window(struct Window *onWindow, struct GadgetRecord *GR)
{
struct Window *window;

	window = UA_OpenRequesterWindow(onWindow,GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(177);
		return(NULL);
	}
	UA_DrawGadgetList(window,GR);

	return(window);
}

/******** Close_A_Request_Window() ********/

void Close_A_Request_Window(struct Window *window)
{
	UA_CloseRequesterWindow(window,STDCOLORS);
}

/******** Build_SmallScriptWindow() ********/
/*
 * returns TRUE if edit button was hit
 *
 */

BOOL Build_SmallScriptWindow(struct GadgetRecord *GR, struct ScriptNodeRecord *this_node)
{
struct Window *window;
struct CycleRecord *CR;
BOOL result=FALSE;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow,SharedWdw_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(178);
		return(result);
	}

	/**** clear string gadget ****/

	UA_WipeStringGadget(&SharedWdw_GR[2]);

	UA_DrawGadgetList(window, SharedWdw_GR);
	UA_DrawGadgetList(window, GR);

	/**** copy object icon ****/

	if (this_node->nodeType == TALK_USERAPPLIC)
		ClipBlit(	&xappRP_2,
							(LONG)objectXPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
							(LONG)objectYPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
							window->RPort,
							9, 3, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);			
	else
		ClipBlit(	&xappRP_2,
							(LONG)standardXPosList[this_node->nodeType],
							(LONG)standardYPosList[this_node->nodeType],
							window->RPort,
							9, 3, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);

	/**** monitor user, nodeType defines behaviour ****/

	CR = (struct CycleRecord *)AnimWdw_GR[2].ptr;
/*
	if (CPrefs.PlayerPalNtsc == PAL_MODE)
		CR->number = 51;
	else
		CR->number = 61;
*/
	CR->number = GetRefreshRate( CPrefs.playerMonitorID ) + 1;

	switch(this_node->nodeType)
	{
		case TALK_AREXX:	editobject_AREXX_and_DOS(window, this_node); break;
		case TALK_DOS:		editobject_AREXX_and_DOS(window, this_node); break;
		case TALK_ANIM:		editobject_ANIM(window,this_node); break;
		case TALK_PAGE:		result = editobject_PAGE(window,this_node); break;
		case TALK_SOUND:	editobject_SOUND(window,this_node); break;
	}
	
	/**** close the shop again ****/

	UA_CloseRequesterWindow(window,STDCOLORS);

	return(result);
}

/******** editobject_AREXX_and_DOS() ********/

void editobject_AREXX_and_DOS(struct Window *window, struct ScriptNodeRecord *this_node)
{
int ID;
BOOL loop=TRUE, retval;
TEXT path[MAXSCANDEPTH];
int cmd_or_script, wait, stack;
TEXT OLD_objectPath[MAX_PARSER_CHARS];
TEXT OLD_objectName[MAX_OBJECTNAME_CHARS];
struct ScriptNodeRecord temp_node;

	stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);
	wait = this_node->numericalArgs[0];
	cmd_or_script = this_node->numericalArgs[1];
	stccpy(OLD_objectPath, this_node->objectPath, MAX_PARSER_CHARS);
	stccpy(OLD_objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);

	stack = this_node->numericalArgs[2];
	if ( stack<=0 )
		stack = 4096;

	/**** draw a button or a string gadget depending on mode ****/

	if ( cmd_or_script == ARGUMENT_COMMAND)
	{
		SharedWdw_GR[2].type = STRING_GADGET;
		UA_DrawGadget(window, &SharedWdw_GR[2]);
		UA_SetStringGadgetToString(window, &SharedWdw_GR[2], this_node->objectPath);
		if (this_node->nodeType==TALK_AREXX)
			UA_SetStringGadgetToString(window, &ArexxWdw_GR[6],	&(this_node->extraData[50]));	// port
		UA_DisableButton(window, &ArexxWdw_GR[5], gui_pattern);
	}
	else	/* SCRIPT */
	{
		SharedWdw_GR[2].type = BUTTON_GADGET;
		UA_DrawGadget(window, &SharedWdw_GR[2]);
		if (this_node->objectPath[0]!='\0')
		{
			UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
			UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
		}
		if ( this_node->objectName[0]=='@' )
			strcpy(path, msgs[Msg_VarPath_5-1]);
		UA_DrawText(window, &SharedWdw_GR[2], path);
		if ( this_node->objectName[0]=='@' )
		{
			UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);
			UA_DisableButton(window, &ArexxWdw_GR[4], gui_pattern);
			UA_DisableButton(window, &ArexxWdw_GR[5], gui_pattern);
		}
		if (this_node->nodeType==TALK_AREXX)
		{
			UA_ClearButton(window, &ArexxWdw_GR[6], AREA_PEN);
			UA_WipeStringGadget(&ArexxWdw_GR[6]);
			UA_DisableButton(window, &ArexxWdw_GR[6], gui_pattern);
		}
	}

	/**** highlight gadgets ****/

	if ( wait == ARGUMENT_DEFER)	// is a wait
		UA_InvertButton(window, &ArexxWdw_GR[1]);

	if ( cmd_or_script == ARGUMENT_COMMAND)
		UA_InvertButton(window, &ArexxWdw_GR[2]);
	else if ( cmd_or_script == ARGUMENT_SCRIPT)
		UA_InvertButton(window, &ArexxWdw_GR[3]);

	if (this_node->nodeType==TALK_DOS)
		UA_SetStringGadgetToVal(window,&DosWdw_GR[6],stack);

	/**** monitor user ****/

	if ( cmd_or_script == ARGUMENT_COMMAND)
		UA_ProcessStringGadget(window, SharedWdw_GR, &SharedWdw_GR[2], &CED);

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		retval = checkOKandCancel(window);
		if (retval != -1)
		{
			loop=FALSE;
			break;
		}

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code == SELECTDOWN)
				{
					if ( UA_CheckGadgetList(window, SharedWdw_GR, &CED) == 2)
					{
						if ( cmd_or_script == ARGUMENT_COMMAND )
							UA_ProcessStringGadget(window, SharedWdw_GR, &SharedWdw_GR[2], &CED);
						else
						{
							if (selectAFile(window,this_node))
							{
								UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
								UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
								UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
								UA_DrawText(window, &SharedWdw_GR[2], path);
								UA_EnableButton(window, &ArexxWdw_GR[5]);
							}
						}
					}
					else
					{
						ID = UA_CheckGadgetList(window, ArexxWdw_GR, &CED);
						switch(ID)
						{
							case 1:	// wait
								UA_InvertButton(window, &ArexxWdw_GR[1]);
								if ( wait == ARGUMENT_DEFER )
									wait = ARGUMENT_CONTINUE;
								else
									wait = ARGUMENT_DEFER;
								break;

							case 2:	/* command */
							case 3:	/* script */
								if (	(cmd_or_script==ARGUMENT_COMMAND && ID==3) || 
											(cmd_or_script==ARGUMENT_SCRIPT && ID==2) )
								{
									UA_InvertButton(window, &ArexxWdw_GR[2]);	// command
									UA_InvertButton(window, &ArexxWdw_GR[3]);	// script
									if (ID==2)
									{
										cmd_or_script = ARGUMENT_COMMAND;
										/**** transform button into string gadget ****/
										SharedWdw_GR[2].type = STRING_GADGET;
										UA_DrawGadget(window, &SharedWdw_GR[2]);
										if (this_node->nodeType==TALK_AREXX)
											UA_EnableButton(window, &ArexxWdw_GR[6]);	// port
										UA_EnableButton(window, &ArexxWdw_GR[4]);	// view
										UA_DisableButton(window, &ArexxWdw_GR[5], gui_pattern);	// edit
										stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);
										this_node->objectPath[0] = '\0';
										this_node->objectName[0] = '\0';
									}
									else
									{
										cmd_or_script = ARGUMENT_SCRIPT;
										/**** transform string gadget into button ****/
										SharedWdw_GR[2].type = BUTTON_GADGET;
										UA_DrawGadget(window, &SharedWdw_GR[2]);
										UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
										stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);
										this_node->objectPath[0] = '\0';
										this_node->objectName[0] = '\0';
										if ( this_node->objectName[0]=='@' )
											strcpy(path, msgs[Msg_VarPath_5-1]);
										UA_DrawText(window, &SharedWdw_GR[2], path);
										if (this_node->nodeType==TALK_AREXX)
											UA_DisableButton(window, &ArexxWdw_GR[6], gui_pattern);	// port
										UA_DisableButton(window, &ArexxWdw_GR[5], gui_pattern);		// edit
										if ( this_node->objectName[0]=='@' )
										{
											UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);
											UA_DisableButton(window, &ArexxWdw_GR[4], gui_pattern);
										}
									}
								}
								break;

							case 4:	/* view */
								UA_InvertButton(window, &ArexxWdw_GR[4]);
								CopyMem(this_node, &temp_node, sizeof(struct ScriptNodeRecord));
								this_node->numericalArgs[0] = wait;
								this_node->numericalArgs[1] = cmd_or_script;
								if ( cmd_or_script == ARGUMENT_COMMAND)
								{
									UA_SetStringToGadgetString(&SharedWdw_GR[2], this_node->objectPath);
									if (this_node->nodeType==TALK_AREXX)
										UA_SetStringToGadgetString(&ArexxWdw_GR[6], &(this_node->extraData[50]));
								}
								do_small_play(this_node);
								CopyMem(&temp_node, this_node, sizeof(struct ScriptNodeRecord));
								UA_InvertButton(window, &ArexxWdw_GR[4]);
								break;

							case 5:	/* edit */
								UA_HiliteButton(window, &ArexxWdw_GR[ID]);
								ExecuteTextEditor(this_node);
								ActivateWindow(window);
								break;

							case 6:	// port
								if (this_node->nodeType==TALK_AREXX)
									UA_ProcessStringGadget(window, ArexxWdw_GR, &ArexxWdw_GR[ID], &CED);
								else if (this_node->nodeType==TALK_DOS)
								{
									UA_ProcessStringGadget(window, DosWdw_GR, &DosWdw_GR[ID], &CED);
									UA_SetValToStringGadgetVal(&DosWdw_GR[ID],&stack);
									if ( stack<4096 || stack>32767 )
									{
										DisplayBeep(NULL);
										if ( stack<4096 )
											stack = 4096;
										else
											stack = 32767;
										UA_SetStringGadgetToVal(window,&DosWdw_GR[ID],stack);
									}
								}
								break;
						}
					}
				}
				break;
		}
	}

	UA_EnableButton(window, &ArexxWdw_GR[4]);		// view
	UA_EnableButton(window, &ArexxWdw_GR[5]);		// edit
	if (this_node->nodeType==TALK_AREXX)
		UA_EnableButton(window, &ArexxWdw_GR[6]);	// port
	UA_EnableButton(window, &SharedWdw_GR[2]);	// path

	if (retval)
	{
		this_node->numericalArgs[0] = wait;
		this_node->numericalArgs[1] = cmd_or_script;
		if ( cmd_or_script == ARGUMENT_COMMAND)
		{
			UA_SetStringToGadgetString(&SharedWdw_GR[2], this_node->objectPath);
			if (this_node->nodeType==TALK_AREXX)
				UA_SetStringToGadgetString(&ArexxWdw_GR[6], &(this_node->extraData[50]));
		}
		if (this_node->nodeType==TALK_DOS)
			this_node->numericalArgs[2] = stack;
	}
	else
	{
		stccpy(this_node->objectPath, OLD_objectPath, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, OLD_objectName, MAX_OBJECTNAME_CHARS);
	}	
}

/******** editobject_ANIM() ********/

void editobject_ANIM(struct Window *window, struct ScriptNodeRecord *this_node)
{
BOOL loop=TRUE, retval, diskAnim, loopFrames;
TEXT path[MAX_PARSER_CHARS], OLD_objectPath[MAX_PARSER_CHARS];
TEXT OLD_objectName[MAX_OBJECTNAME_CHARS],str[15];
int ID, fps, loops, numFrames, dummy;
struct ScriptNodeRecord temp_node;

	/**** save values ****/

	stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);
	stccpy(OLD_objectPath, this_node->objectPath, MAX_PARSER_CHARS);
	stccpy(OLD_objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);

	/**** the loops list goes from 1 to 30 and then an infinite symbol ****/
	/**** appears. So, when num args is 0, loops should be on 30 ****/

	loops = this_node->numericalArgs[10];
	if (loops<0)	// undefined
		loops=0;	// infinite character

	fps = this_node->numericalArgs[9];

	/**** init file select button, get info from ANIM file ****/ 

	SharedWdw_GR[2].type = BUTTON_GADGET;
	UA_DrawGadget(window, &SharedWdw_GR[2]);
	if (this_node->objectPath[0] != '\0')
	{
		UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
		if ( fps == -1 )
		{
			GetAnimSpeed(this_node->objectPath, this_node->objectName, &fps, &numFrames);
#if 0
			if ( GetInfoOnForm(path,DPAN,(UBYTE *)&DPAN_chunk,sizeof(DPAN_chunk),NULL) )
			{
				HB = DPAN_chunk.flags;
				HB = HB >> 24;
				fps = (int)HB;
				if (fps<1 || fps>50)
					fps = DEFAULT_ANIM_SPEED;
			}
			else
				fps = DEFAULT_ANIM_SPEED;
#endif
		}
		else
			GetAnimSpeed(this_node->objectPath, this_node->objectName, &dummy, &numFrames);

		/**** print number of frames ****/
		if ( numFrames!=-1 )
		{
			sprintf(str,"%d fr.",numFrames);
			UA_ClearButton(window, &AnimWdw_GR[6], AREA_PEN);
			UA_DrawSpecialGadgetText(window, &AnimWdw_GR[6],str,SPECIAL_TEXT_CENTER);
		}
	}
	else
	{
		UA_DisableButton(window, &AnimWdw_GR[1], gui_pattern);	/* show */
		fps = DEFAULT_ANIM_SPEED;
	}

	UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);

	if ( this_node->objectName[0]=='@' )
		strcpy(path, msgs[Msg_VarPath_5-1]);

	UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
	UA_DrawText(window, &SharedWdw_GR[2], path);

	if ( this_node->objectName[0]=='@' )
	{
		UA_DisableButton(window, &AnimWdw_GR[1], gui_pattern);	// show
		UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);	// path
	}

	if ( this_node->numericalArgs[11] & 2 )
		diskAnim=TRUE;
	else
		diskAnim=FALSE;

	if ( this_node->numericalArgs[11] & 1 )
		loopFrames=TRUE;
	else
		loopFrames=FALSE;

	/**** highlight gadgets ****/

	if ( GetRefreshRate( CPrefs.playerMonitorID ) < fps )	// eg. 50 Hz PAL vs 60 hz NTSC
	{
		fps = GetRefreshRate( CPrefs.playerMonitorID );
		this_node->numericalArgs[9] = fps;
	}

	UA_SetCycleGadgetToVal(window, &AnimWdw_GR[2], fps);
	UA_SetCycleGadgetToVal(window, &AnimWdw_GR[3], loops);
	if ( loopFrames )
		UA_InvertButton(window, &AnimWdw_GR[4]);	// add loop frames
	if ( diskAnim )
		UA_InvertButton(window, &AnimWdw_GR[5]);	// play from disk

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		retval = checkOKandCancel(window);
		if (retval != -1)
		{
			loop=FALSE;
			break;
		}

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code == SELECTDOWN)
				{
					if ( UA_CheckGadgetList(window, SharedWdw_GR, &CED) == 2)
					{
						if (selectAFile(window,this_node))
						{
							UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
							UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
							UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
							UA_DrawText(window, &SharedWdw_GR[2], path);
							UA_EnableButton(window, &AnimWdw_GR[1]);	/* show */
							UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);

							GetAnimSpeed(this_node->objectPath, this_node->objectName, &fps, &numFrames);
							UA_SetCycleGadgetToVal(window, &AnimWdw_GR[2], fps);

		/**** print number of frames ****/
		if ( numFrames!=-1 )
		{
			sprintf(str,"%d fr.",numFrames);
			UA_ClearButton(window, &AnimWdw_GR[6], AREA_PEN);
			UA_DrawSpecialGadgetText(window, &AnimWdw_GR[6],str,SPECIAL_TEXT_CENTER);
		}

#if 0
							if ( GetInfoOnForm(path,DPAN,(UBYTE *)&DPAN_chunk,sizeof(DPAN_chunk),NULL) )
							{
								HB = DPAN_chunk.flags;
								HB = HB >> 24;
								fps = (int)HB;
								if (fps<1 || fps>50)
									fps = DEFAULT_ANIM_SPEED;
								UA_SetCycleGadgetToVal(window, &AnimWdw_GR[2], fps);
							}
#endif
						}
					}
					else
					{
						ID = UA_CheckGadgetList(window, AnimWdw_GR, &CED);
						switch(ID)
						{
							case 1:		// show
								UA_InvertButton(window, &AnimWdw_GR[ID]);
								CopyMem(this_node, &temp_node, sizeof(struct ScriptNodeRecord));

								UA_SetValToCycleGadgetVal(&AnimWdw_GR[2], &fps);
								UA_SetValToCycleGadgetVal(&AnimWdw_GR[3], &loops);
								this_node->numericalArgs[9] = fps;
								this_node->numericalArgs[10] = loops;
								if (loops==0)	// infinite chosen
									this_node->numericalArgs[10] = -1;	// this is infinite for Cees
								this_node->numericalArgs[11] = 0;
								if ( loopFrames )
									this_node->numericalArgs[11] = 1;
								if ( diskAnim )
									this_node->numericalArgs[11] |= 2;

								do_small_play(this_node);

								CopyMem(&temp_node, this_node, sizeof(struct ScriptNodeRecord));
								UA_InvertButton(window, &AnimWdw_GR[ID]);
								break;

							case 2:
							case 3:
									UA_ProcessCycleGadget(window, &AnimWdw_GR[ID], &CED);
									break;

							case 4:
								UA_InvertButton(window, &AnimWdw_GR[4]);
								if ( loopFrames )
									loopFrames = FALSE;
								else
									loopFrames = TRUE;
								break;

							case 5:
								UA_InvertButton(window, &AnimWdw_GR[5]);
								if ( diskAnim )
									diskAnim = FALSE;
								else
									diskAnim = TRUE;
								break;
						}
					}
				}
				break;
		}
	}

	UA_EnableButton(window, &AnimWdw_GR[1]);
	UA_EnableButton(window, &SharedWdw_GR[2]);	// path

	if (!retval)
	{
		stccpy(this_node->objectPath, OLD_objectPath, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, OLD_objectName, MAX_OBJECTNAME_CHARS);
	}
	else
	{
		UA_SetValToCycleGadgetVal(&AnimWdw_GR[2], &fps);
		UA_SetValToCycleGadgetVal(&AnimWdw_GR[3], &loops);
		this_node->numericalArgs[9] = fps;
		this_node->numericalArgs[10] = loops;
		if (loops==0)	// infinite chosen
			this_node->numericalArgs[10] = -1;	// this is infinite for Cees
		this_node->numericalArgs[11] = 0;
		if ( loopFrames )
			this_node->numericalArgs[11] = 1;
		if ( diskAnim )
			this_node->numericalArgs[11] |= 2;
	}
}	

/******** editobject_PAGE() ********/

BOOL editobject_PAGE(struct Window *window, struct ScriptNodeRecord *this_node)
{
int ID;
BOOL loop=TRUE, retval, result=FALSE, cycle;
TEXT path[MAX_PARSER_CHARS];
TEXT OLD_objectPath[MAX_PARSER_CHARS];
TEXT OLD_objectName[MAX_OBJECTNAME_CHARS];
ULONG type;
WORD old15;

	/**** save values ****/

	stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);
	stccpy(OLD_objectPath, this_node->objectPath, MAX_PARSER_CHARS);
	stccpy(OLD_objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);
	old15 = this_node->numericalArgs[15];

	/**** init file select button ****/ 

	SharedWdw_GR[2].type = BUTTON_GADGET;
	UA_DrawGadget(window, &SharedWdw_GR[2]);
	if (this_node->objectPath[0] != '\0')
	{
		UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
		UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
	}
	else
	{
		UA_DisableButton(window, &PageWdw_GR[1], gui_pattern);	/* show */
		//UA_DisableButton(window, &PageWdw_GR[2], gui_pattern);	/* edit */
	}

	if ( this_node->objectName[0]=='@' )
		strcpy(path, msgs[Msg_VarPath_5-1]);

	UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
	UA_DrawText(window, &SharedWdw_GR[2], path);

	if ( this_node->objectName[0]=='@' )
	{
		UA_DisableButton(window, &PageWdw_GR[1], gui_pattern);	// show
		UA_DisableButton(window, &PageWdw_GR[2], gui_pattern);	// edit
		UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);	// path
	}

	cycle = this_node->numericalArgs[12];
	if ( cycle )
		UA_InvertButton(window, &PageWdw_GR[3]);	// color cycle

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		retval = checkOKandCancel(window);
		if (retval != -1)
		{
			loop=FALSE;
			break;
		}

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code == SELECTDOWN)
				{
					if ( UA_CheckGadgetList(window, SharedWdw_GR, &CED) == 2)
					{
						if (selectAFile(window,this_node))
						{
							UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
							UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
							UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
							UA_DrawText(window, &SharedWdw_GR[2], path);
							UA_EnableButton(window, &PageWdw_GR[1]);	/* show */
							//UA_EnableButton(window, &PageWdw_GR[2]);	/* edit */
							type = checkFileType(path, NULL);
							if ( type==ILBM )
								this_node->numericalArgs[15] = 2;	// ILBM IFF
							else
								this_node->numericalArgs[15] = 1;	// pagetalk document

							UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
							//GetLEInfo(this_node, path, &(ObjectRecord.scriptSIR));
							UpdateLEInfo(this_node, path, &(ObjectRecord.scriptSIR));
						}
					}
					else
					{
						ID = UA_CheckGadgetList(window, PageWdw_GR, &CED);
						switch(ID)
						{
							case 1:	// show
								UA_InvertButton(window,&PageWdw_GR[1]);
								do_small_play(this_node);
								UA_InvertButton(window,&PageWdw_GR[1]);
								break;
							case 2:	// edit
								UA_HiliteButton(window,&PageWdw_GR[2]);
								loop=FALSE;
								result=TRUE;
								break;
							case 3:	// color cycle
								UA_InvertButton(window,&PageWdw_GR[3]);
								if ( cycle )
									cycle=FALSE;
								else
									cycle=TRUE;
								break;
						}
					}
				}
				break;
		}
	}

	UA_EnableButton(window, &PageWdw_GR[1]);
	UA_EnableButton(window, &PageWdw_GR[2]);
	UA_EnableButton(window, &SharedWdw_GR[2]);

	if (!retval)
	{
		stccpy(this_node->objectPath, OLD_objectPath, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, OLD_objectName, MAX_OBJECTNAME_CHARS);
		this_node->numericalArgs[15] = old15;
	}
	else
		this_node->numericalArgs[12] = cycle;

	return(result);
}

/********	checkOKandCancel() ********/

BOOL checkOKandCancel(struct Window *window)
{
int ID;

	switch(CED.Class)
	{
		case IDCMP_MOUSEBUTTONS:
			switch(CED.Code)
			{
				case SELECTDOWN:
					ID = UA_CheckGadgetList(window, SharedWdw_GR, &CED);
					if (ID==3) /* OK */
					{
do_ok:
						UA_HiliteButton(window, &SharedWdw_GR[3]);
						return(TRUE);
					}						
					else if (ID==4) /* Cancel */
					{
do_cancel:
						UA_HiliteButton(window, &SharedWdw_GR[4]);
						return(FALSE);
					}						
					break;
			}
			break;

		case IDCMP_RAWKEY:
			if (CED.Code==RAW_ESCAPE)	/* cancel */
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&SharedWdw_GR[3]))	/* OK */
				goto do_ok;
			break;
	}

	return(-1);
}

/******** selectAFile() ********/

BOOL selectAFile(struct Window *window, struct ScriptNodeRecord *this_node)
{
TEXT title[80];
BOOL retVal;
int flags;
TEXT path[MAX_PARSER_CHARS];
TEXT filename[MAX_OBJECTNAME_CHARS];
BOOL has_path;

	UA_HiliteButton(window, &SharedWdw_GR[2]);

	if ( this_node->objectPath[0] != '\0' )
	{
		stccpy(path, this_node->objectPath, MAX_PARSER_CHARS);
		has_path=TRUE;
	}
	else
	{
		stccpy(path, CPrefs.import_picture_Path, MAX_PARSER_CHARS);
		has_path=FALSE;
	}

	switch(this_node->nodeType)
	{
		case TALK_ANIM:		if ( this_node->objectPath[0] == '\0' )
												stccpy(path, CPrefs.anim_Path, MAX_PARSER_CHARS);
											stccpy(title, msgs[Msg_SelectAnAnim-1], 80);
											flags = DIR_OPT_ANIM | DIR_OPT_NOINFO;
											break;
		case TALK_AREXX:	if ( this_node->objectPath[0] == '\0' )
												stccpy(path, CPrefs.import_text_Path, MAX_PARSER_CHARS);
											stccpy(title, msgs[Msg_SelectAnArexxScript-1], 80);
											flags = DIR_OPT_ALL | DIR_OPT_NOINFO;
											break;
		case TALK_DOS:		if ( this_node->objectPath[0] == '\0' )
												stccpy(path, CPrefs.import_text_Path, MAX_PARSER_CHARS);
											stccpy(title, msgs[Msg_SelectADosScript-1], 80);
											flags = DIR_OPT_ALL | DIR_OPT_NOINFO;
											break;
		case TALK_BINARY: stccpy(title, msgs[Msg_SelectAFile-1], 80);
											flags = DIR_OPT_ALL;
											break;
		case TALK_MAIL:		stccpy(title, msgs[Msg_SelectAMailFile-1], 80);
											flags = DIR_OPT_ALL | DIR_OPT_NOINFO;
											break;
		case TALK_PAGE:		stccpy(title, msgs[Msg_SelectADocOrAPic-1], 80);
											flags = DIR_OPT_ILBM | DIR_OPT_THUMBS | DIR_OPT_NOINFO;
											break;
		case TALK_SOUND:	if ( this_node->objectPath[0] == '\0' )
												stccpy(path, CPrefs.music_Path, MAX_PARSER_CHARS);
											stccpy(title, msgs[Msg_SelectASoundFile-1], 80);
											flags = DIR_OPT_MUSIC | DIR_OPT_NOINFO;
											break;
		default:					UA_WarnUser(179);
											break;
	}

	retVal = OpenAFile(path, filename, title, window, flags, FALSE);
	if (retVal)
	{
		if ( this_node->nodeType==TALK_ANIM )
			stccpy(CPrefs.anim_Path, path, SIZE_PATH);
		else if ( this_node->nodeType==TALK_SOUND )
			stccpy(CPrefs.music_Path, path, SIZE_PATH);
		else if ( this_node->nodeType==TALK_AREXX || this_node->nodeType==TALK_DOS )
			stccpy( CPrefs.import_text_Path, path, SIZE_PATH);
		else
			stccpy(CPrefs.import_picture_Path, path, SIZE_PATH);
		stccpy(this_node->objectPath, path, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, filename, MAX_OBJECTNAME_CHARS);
	}

	return(retVal);
}

/******** editobject_SOUND() ********/

void editobject_SOUND(struct Window *window, struct ScriptNodeRecord *this_node)
{
BOOL loop=TRUE, retval;
TEXT path[MAX_PARSER_CHARS], OLD_objectPath[MAX_PARSER_CHARS];
TEXT OLD_objectName[MAX_OBJECTNAME_CHARS];
int ID, val;
WORD old5;
struct ScriptNodeRecord temp_node;

	stccpy(path, msgs[Msg_NoFileSelectedYet-1], MAX_PARSER_CHARS);

	/**** save values ****/

	stccpy(OLD_objectPath, this_node->objectPath, MAX_PARSER_CHARS);
	stccpy(OLD_objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);
	old5 = this_node->numericalArgs[5];

	/**** init file select button, get file type ****/

	SharedWdw_GR[2].type = BUTTON_GADGET;
	UA_DrawGadget(window, &SharedWdw_GR[2]);

	if (this_node->objectPath[0] != '\0')
	{
		/**** get file type ****/
		PrintSoundType(window, this_node);
		UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
	}
	else
		UA_DisableButton(window, &SoundWdw_GR[1], gui_pattern);	// play

	UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
	UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);

	if ( this_node->objectName[0]=='@' )
		strcpy(path, msgs[Msg_VarPath_5-1]);

	UA_DrawText(window, &SharedWdw_GR[2], path);

	if ( this_node->numericalArgs[1] == 1 )						// STOP
	{
		UA_SetCycleGadgetToVal(window, &SoundWdw_GR[2], 2);
		UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);	// no filename on stop
	}
	else
	{
		if ( this_node->numericalArgs[0] == 1 )					// WAIT
			UA_SetCycleGadgetToVal(window, &SoundWdw_GR[2], 0);
		else if ( this_node->numericalArgs[0] == 2 )		// LOOP
			UA_SetCycleGadgetToVal(window, &SoundWdw_GR[2], 1);
	}

	if ( this_node->objectName[0]=='@' )
	{
		UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);	// path
		UA_DisableButton(window, &SoundWdw_GR[1], gui_pattern);	// play
	}

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		retval = checkOKandCancel(window);
		if (retval != -1)
		{
			loop=FALSE;
			break;
		}

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code == SELECTDOWN)
				{
					if ( UA_CheckGadgetList(window, SharedWdw_GR, &CED) == 2) // fr
					{
						if (selectAFile(window,this_node))
						{
							UA_MakeFullPath(this_node->objectPath, this_node->objectName, path);
							UA_ShortenString(window->RPort, path, (SharedWdw_GR[2].x2-SharedWdw_GR[2].x1)-16);
							UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
							UA_DrawText(window, &SharedWdw_GR[2], path);
							UA_EnableButton(window, &SoundWdw_GR[1]);	// play
							PrintSoundType(window, this_node);
						}
					}
					else
					{
						ID = UA_CheckGadgetList(window, SoundWdw_GR, &CED);
						switch(ID)
						{
							case 1:	// play
								UA_InvertButton(window, &SoundWdw_GR[1]);
								CopyMem(this_node, &temp_node, sizeof(struct ScriptNodeRecord));

								UA_SetValToCycleGadgetVal(&SoundWdw_GR[2], &val);
								if (val==2)				// STOP
								{
									this_node->numericalArgs[0] = 1;	// 2; ???????????
									this_node->numericalArgs[1] = 1;
								}
								else if (val==0)	// WAIT
								{
									this_node->numericalArgs[0] = 1;
									this_node->numericalArgs[1] = 0;
								}		
								else if (val==1)	// LOOP
								{
									this_node->numericalArgs[0] = 2;
									this_node->numericalArgs[1] = 0;
								}		

								do_small_play(this_node);

								CopyMem(&temp_node, this_node, sizeof(struct ScriptNodeRecord));
								UA_InvertButton(window, &SoundWdw_GR[1]);
								break;

							case 2:	// cycle
								UA_ProcessCycleGadget(window, &SoundWdw_GR[2], &CED);
								UA_SetValToCycleGadgetVal(&SoundWdw_GR[2], &ID);
								if (ID==2)			// STOP
									UA_DisableButton(window, &SharedWdw_GR[2], gui_pattern);	// no filename on stop
								else
								{
									if ( this_node->objectName[0]!='@' )
									{
										UA_EnableButton(window, &SharedWdw_GR[2]);	// no filename on stop
										UA_ClearButton(window, &SharedWdw_GR[2], AREA_PEN);
										UA_DrawText(window, &SharedWdw_GR[2], path);
									}
								}
								break;
						}
					}
				}
				break;
		}
	}

	UA_EnableButton(window, &SoundWdw_GR[1]);
	UA_EnableButton(window, &SharedWdw_GR[2]);

	if (!retval)
	{
		stccpy(this_node->objectPath, OLD_objectPath, MAX_PARSER_CHARS);
		stccpy(this_node->objectName, OLD_objectName, MAX_OBJECTNAME_CHARS);
		this_node->numericalArgs[5] = old5;
	}
	else
	{
		UA_SetValToCycleGadgetVal(&SoundWdw_GR[2], &val);
		if (val==2)				// STOP
		{
			this_node->numericalArgs[0] = 1;	// 2; ???????????
			this_node->numericalArgs[1] = 1;

			this_node->objectPath[0] = '\0';
			this_node->objectName[0] = '\0';
		}
		else if (val==0)	// WAIT
		{
			this_node->numericalArgs[0] = 1;
			this_node->numericalArgs[1] = 0;
		}		
		else if (val==1)	// LOOP
		{
			this_node->numericalArgs[0] = 2;
			this_node->numericalArgs[1] = 0;
		}		
	}
}

/******** PrintSoundType() ********/

void PrintSoundType(struct Window *window, SNRPTR this_node)
{
TEXT fullPath[SIZE_FULLPATH];

	/**** create e.g. "work:medialink/xapps/system/music" ****/

	SetSpriteOfActWdw(SPRITE_BUSY);

	UA_MakeFullPath(dir_system, MUSIC_XAPP, fullPath);
	this_node->numericalArgs[5]=-1;
	this_node->numericalArgs[4]=1;	// get file type
	if ( !InitXaPP(fullPath, this_node, TRUE) )	// true means tiny
		Message("TEMPORARY ERROR MESSAGE: cannot load XaPP");
	this_node->numericalArgs[4]=0;	// play, don't get file type

	UA_ClearButton(window, &SoundWdw_GR[3], AREA_PEN);

	switch( this_node->numericalArgs[5] )
	{
		case 1:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_MarkII-1], SPECIAL_TEXT_CENTER);
			break;
		case 2:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_DSS-1], SPECIAL_TEXT_CENTER);
			break;
		case 3:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_ST-1], SPECIAL_TEXT_CENTER);
			break;
		case 4:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_SNPro-1], SPECIAL_TEXT_CENTER);
			break;
		case 5:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_FC13-1], SPECIAL_TEXT_CENTER);
			break;
		case 6:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_FC14-1], SPECIAL_TEXT_CENTER);
			break;
		case 7:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_Jam-1], SPECIAL_TEXT_CENTER);
			break;
		case 8:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_SM-1], SPECIAL_TEXT_CENTER);
			break;
		default:
			UA_DrawSpecialGadgetText(window, &SoundWdw_GR[3], msgs[Msg_Mod_Unknown-1], SPECIAL_TEXT_CENTER);
			break;
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** E O F ********/
