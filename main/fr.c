#include "nb:pre.h"

/**** defines ****/

#define DO_FILES 		1
#define DO_DEVICES	2
#define DO_ASSIGNS	3
#define DO_HOMES		4

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct CapsPrefs CPrefs;
extern struct EventData CED;
extern struct Library *medialinkLibBase;
extern struct FileListInfo FLI;
extern struct eventHandlerInfo EHI;
extern UWORD chip gui_pattern[];
extern UBYTE **msgs;
extern UBYTE *homeDirs;
extern UBYTE *homePaths;
extern struct Gadget PropSlider1;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;

/***** statics ****/

BOOL save_as_iff = FALSE;

/**** gadgets ****/

extern struct GadgetRecord FileRequester_GR[];
extern struct GadgetRecord SaveFile_GR[];

/**** functions ****/

/******** CreateFileRequester() ********/
/*
 * type == 1 for open and 2 for save
 *
 */

struct Window *CreateFileRequester(int type, struct Window *onWindow, STRPTR title)
{
struct GadgetRecord *GR;
struct Window *window;

	/**** double or halve gadgets if necessary ****/

	if ( UA_IsWindowOnLacedScreen(onWindow) )
	{
		if (type==1)
		{
			if ( FileRequester_GR[0].x1 == 0 )	/* not doubled */
			{
				UA_DoubleGadgetDimensions(FileRequester_GR);
				FileRequester_GR[0].x1 = 1;
			}
		}
		else if (type==2)
		{
			if ( SaveFile_GR[0].x1 == 0 )	/* not doubled */
			{
				UA_DoubleGadgetDimensions(SaveFile_GR);
				SaveFile_GR[0].x1 = 1;
			}
		}
	}
	else
	{
		if (type==1)
		{
			if ( FileRequester_GR[0].x1 == 1 )	/* doubled */
			{
				UA_HalveGadgetDimensions(FileRequester_GR);
				FileRequester_GR[0].x1 = 0;
			}
		}
		else if (type==2)
		{
			if ( SaveFile_GR[0].x1 == 1 )	/* doubled */
			{
				UA_HalveGadgetDimensions(SaveFile_GR);
				SaveFile_GR[0].x1 = 0;
			}
		}
	}

	/**** open a window ****/

	if (type==1)
		GR = FileRequester_GR;
	else
		GR = SaveFile_GR;

	window = UA_OpenRequesterWindow(onWindow,GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(116);
		return(NULL);
	}

	/**** attach an Intuition gadget to the window ****/

	if (type==1)
	{
		PropSlider1.LeftEdge	= FileRequester_GR[8].x1+4;
		PropSlider1.TopEdge		= FileRequester_GR[8].y1+2;
		PropSlider1.Width			= FileRequester_GR[8].x2-FileRequester_GR[8].x1-7;
		PropSlider1.Height		= FileRequester_GR[8].y2-FileRequester_GR[8].y1-3;
		if ( UA_IsWindowOnLacedScreen(onWindow) )
		{
			PropSlider1.TopEdge	+= 2;
			PropSlider1.Height	-= 4;
		}
	}
	else
	{
		PropSlider1.LeftEdge	= SaveFile_GR[8].x1+4;
		PropSlider1.TopEdge		= SaveFile_GR[8].y1+2;
		PropSlider1.Width			= SaveFile_GR[8].x2-SaveFile_GR[8].x1-7;
		PropSlider1.Height		= SaveFile_GR[8].y2-SaveFile_GR[8].y1-3;
		if ( UA_IsWindowOnLacedScreen(onWindow) )
		{
			PropSlider1.TopEdge	+= 2;
			PropSlider1.Height	-= 4;
		}
	}

	if (type==1)
		UA_DrawGadgetList(window, FileRequester_GR);
	else
		UA_DrawGadgetList(window, SaveFile_GR);

	if (type==1 && title!=NULL)
		UA_DrawSpecialGadgetText(window,&FileRequester_GR[13],title,SPECIAL_TEXT_LEFT);
	else if (type==2 && title!=NULL)
		UA_DrawSpecialGadgetText(window,&SaveFile_GR[12],title,SPECIAL_TEXT_LEFT);

	InitPropInfo((struct PropInfo *)PropSlider1.SpecialInfo, (struct Image *)PropSlider1.GadgetRender);
	
	AddGadget(window,&PropSlider1,-1L);

	OffGadget(&PropSlider1,window,NULL);

	return(window);
}

/******** CloseFileRequester() ********/

void CloseFileRequester(struct Window *window)
{
	UA_CloseRequesterWindow(window,STDCOLORS);
}

/******** InitPropInfo() ********/
/*
 * This functions should eliminate the problems with prop gadgets
 * which are used in non-lace and lace environments and write over
 * the container in which they live.
 *
 */

void InitPropInfo(struct PropInfo *PI, struct Image *IM)
{
	PI->VertPot = 0;
	PI->VertBody = 0;
	PI->CHeight = 0;
	PI->VPotRes = 0;

	IM->Height = 0;
	IM->Depth = 0;
	IM->ImageData = NULL;
	IM->PlanePick	= 0x0000;
	IM->PlaneOnOff = 0x0000;
	IM->NextImage = NULL;
}

/******** OpenAFile() ********/
/*
 * multiple is TRUE for selection of more than 1 file
 *
 */

BOOL OpenAFile(	STRPTR path, STRPTR fileName, STRPTR title, 
								struct Window *onWindow, int opts, BOOL multiple)
{
BOOL retVal=FALSE;
struct Window *window;

	// SPECIAL TRICK TO CALL UPDATE DIR CACHE FROM E.G. XAPPS
	if ( !fileName && !title && !onWindow && opts==-1 )
	{
		UpdateDirCache(path);
		return(TRUE);
	}

	PaletteToBack();

	window = CreateFileRequester(1, onWindow, title);
	if (!window)
		return(FALSE);

	FileRequester_GR[1].type = HIBOX_REGION;
	UA_DrawGadget(window, &FileRequester_GR[1]);
	UA_PrintInBox(	window, &FileRequester_GR[1],
									FileRequester_GR[1].x1, FileRequester_GR[1].y1,
									FileRequester_GR[1].x2, FileRequester_GR[1].y2,
									path, PRINT_RIGHTPART);
	FileRequester_GR[1].type = SPECIAL_STRING_GADGET;

	UA_DrawDefaultButton(window,&FileRequester_GR[7]);	// open

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( OpenDir(path, opts) )
	{
		if ( GetDevicesAndAssigns() )
		{
			retVal = MonitorFileRequester(1, path, fileName, FileRequester_GR,
																		opts, multiple, window);
			FreeDevicesAndAssigns();
		}
		if (!multiple)
			CloseDir();
	}

	CloseFileRequester(window);

	return(retVal);
}

/******** SaveAFile() ********/

BOOL SaveAFile(	STRPTR path, STRPTR fileName, STRPTR title, 
								struct Window *onWindow, int opts)
{
BOOL retVal=FALSE;
struct Window *window;

	PaletteToBack();

	if ( opts & DIR_OPT_THUMBS )
		SaveFile_GR[14].type = CHECK_GADGET;

	window = CreateFileRequester(2, onWindow, title);
	if (!window)
		return(FALSE);

	SaveFile_GR[1].type = HIBOX_REGION;
	UA_DrawGadget(window,&SaveFile_GR[1]);
	UA_PrintInBox(	window, &SaveFile_GR[1],
									SaveFile_GR[1].x1, SaveFile_GR[1].y1,
									SaveFile_GR[1].x2, SaveFile_GR[1].y2,
									path, PRINT_RIGHTPART);
	SaveFile_GR[1].type = SPECIAL_STRING_GADGET;

	UA_DrawDefaultButton(window,&SaveFile_GR[7]);	// save

	if ( save_as_iff && SaveFile_GR[14].type==CHECK_GADGET )
		UA_InvertButton(window,&SaveFile_GR[14]);

	UA_SetStringGadgetToString(window, &SaveFile_GR[11], fileName);	// filename - NEW NEW

	SetSpriteOfActWdw(SPRITE_BUSY);

	if ( OpenDir(path, opts) )
	{
		if ( GetDevicesAndAssigns() )
		{
			retVal = MonitorFileRequester(2, path, fileName, SaveFile_GR,
																		opts, FALSE, window);
			FreeDevicesAndAssigns();
		}
		CloseDir();
	}

	CloseFileRequester(window);

	SaveFile_GR[14].type = INVISIBLE_GADGET;

	return(retVal);
}

/******** MonitorFileRequester() ********/
/*
 * type == 1 for open and 2 for save
 *
 */

BOOL MonitorFileRequester(int type, STRPTR path, STRPTR fileName, 
													struct GadgetRecord *GR, int opts, BOOL multiple,
													struct Window *window)
{
BOOL loop=TRUE, retVal=FALSE, events=TRUE;
int ID, top=0, line=-1, i,j;
char action;
TEXT oldPath[SIZE_FULLPATH], oldFileName[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
BOOL anyChosen, shifted=FALSE, OkToDblClick=TRUE, mouseMoved=FALSE, SHIFTQUAL=FALSE;
struct FileLock *FL;
int retcode;
struct StringRecord *SR_ptr;
ULONG signals;
struct IntuiMessage *message;
BOOL mouseDown=FALSE, toggle;
ULONG prev_Seconds = 0L;
ULONG prev_Micros  = 0L;
struct ScrollRecord SR;
WORD ascii;

	// START NEW
	GR[1].type = HIBOX_REGION;
	UA_DrawGadget(window, &GR[1]);
	UA_PrintInBox(window, &GR[1],	GR[1].x1, GR[1].y1, GR[1].x2, GR[1].y2, path, PRINT_RIGHTPART);
	GR[1].type = SPECIAL_STRING_GADGET;
	// END NEW

	SR_ptr = (struct StringRecord *)GR[1].ptr;
	strcpy(SR_ptr->buffer,path);

	/**** init vars ****/

	action = DO_FILES;
	stccpy(oldPath, path, SIZE_FULLPATH);
	stccpy(oldFileName, fileName, SIZE_FILENAME);

	/**** prepare display ****/

	/**** init scroll record ****/

	SR.GR							= &GR[2];
	SR.window					= window;
	SR.list						= FLI.fileList;
	SR.sublist				= NULL;
	SR.selectionList	= &(FLI.selectionList[0]);
	SR.entryWidth			= SIZE_FILENAME;
	SR.numDisplay			= 10;
	SR.numEntries			= FLI.numFiles;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	top=0;
	UA_PrintStandardList(&SR,top,NULL);

	UA_InitPropSlider(window, &PropSlider1, (ULONG)FLI.numFiles, 10, (ULONG)top);

	if ( type==1 && !(opts&DIR_OPT_ONLYDIRS) )
		UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */

	if ( !multiple && type==1 )
		UA_DisableButton(window, &GR[11], gui_pattern);	/* select all */

	if (type==2)
		UA_SetStringGadgetToString(window, &GR[11], fileName); /* filename */

	if (	type==1 &&
				!(	(opts & DIR_OPT_ILBM) || (opts & DIR_OPT_THUMBS) ||
						(opts & DIR_OPT_ANIM)) )
		UA_DisableButton(window, &GR[12], gui_pattern);	/* thumbs */

	OnGadget(&PropSlider1, window, NULL);

	SetSpriteOfActWdw(SPRITE_NORMAL);
	
	/**** handle events ****/

	ActivateWindow(window);

	while(loop)
	{
		events=TRUE;
		toggle=FALSE;

		while(events)
		{
			signals = Wait(SIGNALMASK);
			if (signals & SIGNALMASK)
			{
				mouseMoved=FALSE;
				while(events && (message = (struct IntuiMessage *)GetMsg(capsPort)))
				{
					CED.Class			= message->Class;
					CED.Code			= message->Code;
					CED.MouseX		= message->MouseX;
					CED.MouseY		= message->MouseY;
					CED.Qualifier	= message->Qualifier;
					CED.Seconds		= message->Seconds;
					CED.Micros		= message->Micros;
					ReplyMsg((struct Message *)message);

					if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
						SHIFTQUAL=TRUE;	// save it before its killed
					else
						SHIFTQUAL=FALSE;

					if (!multiple)
						SHIFTQUAL=FALSE;

					if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code!=MENUDOWN && CED.Code==SELECTDOWN)
						UA_CheckIfDragged(window,&CED);

					CED.extraClass = NULL;
					if ( CED.Class == IDCMP_MOUSEBUTTONS && CED.Code == SELECTDOWN )
					{
						if ( MyDoubleClick(prev_Seconds, prev_Micros, CED.Seconds, CED.Micros) )
							CED.extraClass = DBLCLICKED;
						else
						{
							prev_Seconds = CED.Seconds;
							prev_Micros  = CED.Micros;
						}
					}
					if (CED.Class != IDCMP_MOUSEBUTTONS)
					{
						prev_Seconds = 0L;
						prev_Micros  = 0L;
					}

					switch(CED.Class)
					{
						case IDCMP_MOUSEBUTTONS:
							if ( CED.Code==SELECTDOWN )
							{
								if (EHI.activeScreen==STARTSCREEN_PAGE)
									CheckIfDepthWasClicked(pageWindow);
								else if (EHI.activeScreen==STARTSCREEN_SCRIPT)
									CheckIfDepthWasClicked(scriptWindow);

								if ( SHIFTQUAL )
								{
									UA_SwitchMouseMoveOn(window);
									mouseDown=TRUE;
									UA_SelectStandardListLine(&SR,top,TRUE,&CED,FALSE,FALSE);
									if ( UA_IsGadgetDisabled(&GR[7]) )
										UA_EnableButton(window, &GR[7]);	// OK/Save
								}
								else
								{
									UA_SwitchMouseMoveOff(window);
									mouseDown=FALSE;
									events=FALSE;
								}
							}
							else if ( CED.Code==SELECTUP )
							{
								if ( SHIFTQUAL )
								{
									if (toggle)
										toggle=FALSE;
									else
										toggle=TRUE;
									mouseDown=FALSE;
								}
								else
								{
									UA_SwitchMouseMoveOff(window);
									mouseDown=FALSE;
									events=FALSE;
								}
							}
							break;

						case IDCMP_RAWKEY:
							events=FALSE;
							break;

						case IDCMP_MOUSEMOVE:
							mouseMoved=TRUE;
							break;

						case IDCMP_GADGETDOWN:
						case IDCMP_GADGETUP:
							CED.extraClass = CED.Class;
							CED.Class = IDCMP_MOUSEBUTTONS;
							events=FALSE;
							break;
					}
				}
				if ( mouseMoved && (action == DO_FILES) && mouseDown )
				{
					UA_SelectStandardListLine(&SR,top,TRUE,&CED,toggle,!toggle);
					if ( UA_IsGadgetDisabled(&GR[7]) )
						UA_EnableButton(window, &GR[7]);	// OK/Save
				}
			}
		}

		while(message = (struct IntuiMessage *)GetMsg(capsPort))
			ReplyMsg((struct Message *)message);

		UA_SwitchMouseMoveOff(window);

		if ( window != IntuitionBase->ActiveWindow )
		{
			CED.Class = 0L;
			CED.Code = 0;
		}

		switch(CED.Class)
		{
			case IDCMP_RAWKEY:
				ascii = RawKeyToASCII(CED.Code);
				//if ( CED.Qualifier&IEQUALIFIER_RCOMMAND && CED.Code==0x20 ) // raw A
				if ( CED.Qualifier&IEQUALIFIER_RCOMMAND && ascii=='a' ) // raw A
					goto select_all;
				else if ( CED.Code==RAW_RETURN )
					goto do_OK;
				else if ( CED.Code==RAW_ESCAPE )
					goto do_Cancel;
				break;

			case IDCMP_MOUSEBUTTONS:
				if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
				{
					if ( action == DO_FILES )
						UA_ScrollStandardList(&SR,&top,&PropSlider1,NULL,&CED);
					else if ( action == DO_ASSIGNS )
						UA_ScrollStandardList(&SR,&top,&PropSlider1,NULL,&CED);
					else if ( action == DO_DEVICES )
						UA_ScrollStandardList(&SR,&top,&PropSlider1,NULL,&CED);
					else if ( action == DO_HOMES )
						UA_ScrollStandardList(&SR,&top,&PropSlider1,NULL,&CED);
					CED.Code = 0;
					break;
				}
				else if (CED.Code==SELECTDOWN)  
				{
					if ( SHIFTQUAL )
						shifted=TRUE;
					else
						shifted=FALSE;

					ID = UA_CheckGadgetList(window, GR, &CED);
					switch(ID)
					{
						case 1: // path                         
							strcpy(SR_ptr->buffer,path);
							UA_DrawGadget(window, &GR[ID]);
							// make it a string gadget again
							UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
							UA_SetStringToGadgetString(&GR[ID],path);
							UA_ValidatePath(path);
							UA_TurnAssignIntoDir(path);
							UA_ValidatePath(path);
							UpdateDirCache(path);
							LoadTheNewDir(path, path, opts, &top, GR, window, &SR);
							break;

						case 2:	/* scroll area */
							if ( action == DO_FILES )
								line = UA_SelectStandardListLine(&SR,top,shifted,&CED,FALSE,FALSE);
							else if ( action == DO_ASSIGNS )
								line = UA_SelectStandardListLine(&SR,top,FALSE,&CED,FALSE,FALSE);
							else if ( action == DO_DEVICES )
								line = UA_SelectStandardListLine(&SR,top,FALSE,&CED,FALSE,FALSE);
							else if ( action == DO_HOMES )
								line = UA_SelectStandardListLine(&SR,top,FALSE,&CED,FALSE,FALSE);
							else
								line=-1;

							if (line != -1 && type==1 && UA_IsGadgetDisabled(&GR[7]))
								UA_EnableButton(window, &GR[7]);	/* OK/Save */

							if (type==1)	// open a file
							{
								anyChosen = FALSE;
								for(j=0,i=0; i<MAX_FILE_LIST_ENTRIES; i++)
								{
									if ( *(FLI.selectionList+i) == 1 )
									{
										anyChosen = TRUE;
										j++;
										if (j>1)
											break;
									}
								}
								if ( action==DO_FILES && opts&DIR_OPT_ONLYDIRS )
									UA_EnableButton(window, &GR[7]);	/* OK/Save */
								else
								{
									if ( anyChosen )
									{
										if (UA_IsGadgetDisabled(&GR[7]))
											UA_EnableButton(window, &GR[7]);	/* OK/Save */
										if (j==1)
											OkToDblClick=TRUE;
									}
									else
									{
										UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
										OkToDblClick=FALSE;
									}
								}
							}

							if ( type==2 && line != -1 && action==DO_FILES )
							{
								if ( *(FLI.fileList+((top+line)*SIZE_FILENAME)) == FILE_PRECODE )
								{
									stccpy(	fileName,
													FLI.fileList+((top+line)*SIZE_FILENAME)+1,
													SIZE_FILENAME);
									UA_SetStringGadgetToString(window, &GR[11], fileName); /* filename */
								}
							}

							if (CED.extraClass == DBLCLICKED || CED.Code==SELECTDOWN)
							{
pretend_dblclick:
								if (line != -1 )
								{
									if ( action == DO_FILES )
									{
										if (!processClickOnFileOrDir(window, &top, line, path, opts, GR, &SR))
											line=-1;	/* new dir is shown with nothing selected */
										else if (CED.extraClass == DBLCLICKED)
										{
											if (OkToDblClick && type==1)
												goto do_OK;
											if ( (type==2 && UA_IsGadgetDisabled(&GR[7])) || (opts&DIR_OPT_ONLYDIRS) )
												UA_EnableButton(window, &GR[7]);	// Save
										}
									}
									else if (action == DO_ASSIGNS)
									{
										processClickOnAssign(window, &top, line, path, opts, GR, &SR);
										line=-1;	/* new dir is shown with nothing selected */
										UA_EnableButtonQuiet(&GR[1]);	// path string
										UA_EnableButton(window, &GR[3]);		// parent
										action = DO_FILES;
										if ( (type==2 && UA_IsGadgetDisabled(&GR[7])) || (opts&DIR_OPT_ONLYDIRS) )
											UA_EnableButton(window, &GR[7]);	// Save
									}
									else if (action == DO_DEVICES)
									{
										processClickOnDevice(window, &top, line, path, opts, GR, &SR);
										line=-1;	/* new dir is shown with nothing selected */
										UA_EnableButtonQuiet(&GR[1]);	// path string
										UA_EnableButton(window, &GR[3]);		// parent
										action = DO_FILES;
										if ( (type==2 && UA_IsGadgetDisabled(&GR[7])) || (opts&DIR_OPT_ONLYDIRS) )
											UA_EnableButton(window, &GR[7]);	// Save
									}
									else if (action == DO_HOMES)
									{
										processClickOnHome(window, &top, line, path, opts, GR, &SR);
										line=-1;	/* new dir is shown with nothing selected */
										UA_EnableButtonQuiet(&GR[1]);	// path string
										UA_EnableButton(window, &GR[3]);		// parent
										action = DO_FILES;
										if (type==2 && UA_IsGadgetDisabled(&GR[7]))
											UA_EnableButton(window, &GR[7]);	// Save
									}

									if ( line==-1 && type==1 && !(opts&DIR_OPT_ONLYDIRS) )
										UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
									if ( type==1 && multiple )
										UA_EnableButton(window, &GR[11]);	// select all
									if (	type==1 &&
												((opts & DIR_OPT_ILBM) || (opts & DIR_OPT_THUMBS) ||
												(opts & DIR_OPT_ANIM)) )
										UA_EnableButton(window, &GR[12]);	// show thumbs
								}
							}
							break;

						case 3: /* parent */
							UA_HiliteButton(window, &GR[ID]);
							processClickOnParent(window, &top, line, path, opts, GR, &SR);
							line=-1;	/* new dir is shown with nothing selected */
							if ( type==1 && !(opts&DIR_OPT_ONLYDIRS) )
								UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
							break;

						case 4: /* disks */
							top=0;
							UA_HiliteButton(window, &GR[ID]);
							action = DO_DEVICES;
							if ( !UA_IsGadgetDisabled(&GR[1]) )
								UA_ClearButton(window, &GR[1], AREA_PEN);	/* title */
							UA_ClearButton(window, &GR[2], AREA_PEN);	/* scroll area */
							for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
								*(FLI.selectionList+i) = 0;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							SR.list			 			= FLI.deviceList;
							SR.numEntries 		= FLI.numDevices;
							SR.selectionList	= &(FLI.selectionList[0]);
							UA_PrintStandardList(&SR,top,NULL);
							UA_InitPropSlider(window, &PropSlider1, (ULONG)FLI.numDevices, 10, (ULONG)top);
							UA_DisableButton(window, &GR[1], gui_pattern);	/* path string */
							UA_DisableButton(window, &GR[3], gui_pattern);	/* parent */
							UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
							if (type==1)
							{
								UA_DisableButton(window, &GR[11], gui_pattern);	/* select all */
								if (	(opts & DIR_OPT_ILBM) || (opts & DIR_OPT_THUMBS) ||
											(opts & DIR_OPT_ANIM) )
									UA_DisableButton(window, &GR[12], gui_pattern);	/* show thumbs */
							}
							break;

						case 5: /* assigns */
							UA_HiliteButton(window, &GR[ID]);
							top=0;
							action = DO_ASSIGNS;
							if ( !UA_IsGadgetDisabled(&GR[1]) )
								UA_ClearButton(window, &GR[1], AREA_PEN);	/* title */
							UA_ClearButton(window, &GR[2], AREA_PEN);	/* scroll area */
							for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
								*(FLI.selectionList+i) = 0;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							SR.list			 			= FLI.assignList;
							SR.numEntries 		= FLI.numAssigns;
							SR.selectionList	= &(FLI.selectionList[0]);
							UA_PrintStandardList(&SR,top,NULL);
							UA_InitPropSlider(window, &PropSlider1, (ULONG)FLI.numAssigns, 10, (ULONG)top);
							UA_DisableButton(window, &GR[1], gui_pattern);	/* path string */
							UA_DisableButton(window, &GR[3], gui_pattern);	/* parent */
							UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
							if (type==1)
							{
								UA_DisableButton(window, &GR[11], gui_pattern);	/* select all */
								if (	(opts & DIR_OPT_ILBM) || (opts & DIR_OPT_THUMBS) ||
											(opts & DIR_OPT_ANIM) )
									UA_DisableButton(window, &GR[12], gui_pattern);	/* show thumbs */
							}
							break;

						case 13: /* save homes */
						case 14: /* load homes */
							if ( (type==1 && ID==14) || (type==2 && ID==13) )
							{
								UA_HiliteButton(window, &GR[ID]);
								top=0;
								action = DO_HOMES;
								if ( !UA_IsGadgetDisabled(&GR[1]) )
									UA_ClearButton(window, &GR[1], AREA_PEN);	/* title */
								UA_ClearButton(window, &GR[2], AREA_PEN);	/* scroll area */
								for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
									*(FLI.selectionList+i) = 0;
								UA_PrintStandardList(NULL,-1,NULL);	// init static
								SR.list			 			= FLI.homeList;
								SR.numEntries 		= FLI.numHomes;
								SR.selectionList	= &(FLI.selectionList[0]);
								UA_PrintStandardList(&SR,top,NULL);
								UA_InitPropSlider(window, &PropSlider1, (ULONG)FLI.numHomes, 10, (ULONG)top);
								UA_DisableButton(window, &GR[1], gui_pattern);	/* path string */
								UA_DisableButton(window, &GR[3], gui_pattern);	/* parent */
								UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
								if (type==1)
								{
									UA_DisableButton(window, &GR[11], gui_pattern);	/* select all */
									if (	(opts & DIR_OPT_ILBM) || (opts & DIR_OPT_THUMBS) ||
												(opts & DIR_OPT_ANIM) )
										UA_DisableButton(window, &GR[12], gui_pattern);	/* show thumbs */
								}
							}
							else if ( type==2 && ID==14 )	// save as IFF toggle
							{
								UA_InvertButton(window, &GR[ID]);
								if ( save_as_iff )
									save_as_iff = FALSE;
								else
									save_as_iff = TRUE;
							}
							break;

						case 6:	/* Cancel */
do_Cancel:
							if (!UA_IsGadgetDisabled(&GR[6]))
							{
								UA_HiliteButton(window, &GR[6]);
								loop=FALSE;
								retVal=FALSE;
							}
							break;

						case 7:	/* Open/Save */
do_OK:
							if (!UA_IsGadgetDisabled(&GR[7]))
							{
								UA_HiliteButton(window, &GR[7]);
								if (type==1) /* open */
								{
									if ( line!=-1 && action!=DO_FILES )
										goto pretend_dblclick;
									else if ( action == DO_FILES )
									{
										if (!processClickOnFileOrDir(window, &top, line, path, opts, GR, &SR))
											line=-1;	/* new dir is shown with nothing selected */
										else
										{
											loop=FALSE;
											retVal=TRUE;
										}
										if ( loop && line == -1 && type==1 && !(opts&DIR_OPT_ONLYDIRS) )
											UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
									}
								}
								else if (type==2) /* save */
								{
									if (action == DO_FILES)
									{
										loop=FALSE;
										retVal=TRUE;
									}
								}
							}
							break;

						case 11: /* select all or filename */
select_all:
							if (type==1 && !UA_IsGadgetDisabled(&GR[11]))
							{
								UA_HiliteButton(window, &GR[11]);
								processClickOnSelectAll(window,top,&SR);

								anyChosen = FALSE;
								for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
								{
									if ( *(FLI.selectionList+i) == 1 )
									{
										anyChosen = TRUE;
										break;
									}
								}
								if (anyChosen && UA_IsGadgetDisabled(&GR[7]))
									UA_EnableButton(window, &GR[7]);	/* OK/Save */
								else if ( !anyChosen )
									UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */
							}
							else if (type==2)
							{
								UA_ProcessStringGadget(window, GR, &GR[11], &CED);
								UA_SetStringToGadgetString(&GR[11], fileName);
							}
							break;

						case 12: /* show thumbs */
							if (type==1)	// open a file
							{
								UA_HiliteButton(window, &GR[12]);
								if ( multiple )
									retcode = ShowThumbNails(	path, FLI.selectionList,
																						SELECT_MULTIPLE_FILES,
																						FLI.fileList, FLI.numFiles);
								else
									retcode = ShowThumbNails(	path, FLI.selectionList,
																						SELECT_ONE_FILE,
																						FLI.fileList, FLI.numFiles);

								UA_PrintStandardList(NULL,-1,NULL);	// init static
								UA_PrintStandardList(&SR,top,NULL);

								anyChosen = FALSE;
								for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
								{
									if ( *(FLI.selectionList+i) == 1 )
									{
										anyChosen = TRUE;
										break;
									}
								}
								if ( anyChosen && UA_IsGadgetDisabled(&GR[7]) )
									UA_EnableButton(window, &GR[7]);	/* OK/Save */
								else if ( !anyChosen )
									UA_DisableButton(window, &GR[7], gui_pattern);	/* OK/Save */

								if ( anyChosen && retcode==5 )
									goto do_OK; //pretend_dblclick;
							}
							break;
					}
				}

				break;
		}
	}

	UA_SwitchMouseMoveOff(window);

	if ( retVal && type==1 && !(opts&DIR_OPT_ONLYDIRS) )
	{
		for(i=0; i<FLI.numFiles; i++)
		{
			if ( *(FLI.selectionList+i) == 1 )
			{
				stccpy(fileName, FLI.fileList+i*SIZE_FILENAME+1, SIZE_FILENAME);
				break;
			}
		}
	}

	/**** ask user if an existing file should be overwritten ****/

	if (retVal && type==2)	// save a file
	{
		UA_MakeFullPath(path, fileName, fullPath);
		FL = (struct FileLock *)Lock((STRPTR)fullPath, (LONG)ACCESS_READ);
		if (FL != NULL)
		{
			UnLock((BPTR)FL);
			/**** (mis)use fullPath to hold question string ****/
			sprintf(fullPath, msgs[Msg_Replace-1], fileName);
			if ( !UA_OpenGenericWindow(	window, TRUE, TRUE, msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																	QUESTION_ICON, fullPath, TRUE, NULL) )
				retVal=FALSE;
		}
	}

	if (!retVal)
	{
		stccpy(path, oldPath, SIZE_FULLPATH);
		stccpy(fileName, oldFileName, SIZE_FILENAME);
	}

	UA_EnableButtonQuiet(&GR[1]);	// path string
	UA_EnableButtonQuiet(&GR[3]);
	UA_EnableButtonQuiet(&GR[7]);
	UA_EnableButtonQuiet(&GR[11]);
	UA_EnableButtonQuiet(&GR[12]);

	return(retVal);
}

/******** processClickOnFileOrDir() ********/

BOOL processClickOnFileOrDir(	struct Window *window, int *top, int line,
															STRPTR path, int opts,
															struct GadgetRecord *GR, struct ScrollRecord *SR)
{
TEXT fullPath[SIZE_FULLPATH];
int realLine;

	realLine = line + *top;

	if ( *(FLI.fileList+(realLine*SIZE_FILENAME)) == DIR_PRECODE &&
			 *(FLI.selectionList+realLine) == 1 )
	{
		/**** create new path ****/

		UA_MakeFullPath(path, (FLI.fileList+(realLine*SIZE_FILENAME)+1), fullPath);
		UA_ValidatePath(fullPath);

		LoadTheNewDir(path, fullPath, opts, top, GR, window, SR);

		return(FALSE);
	}

	return(TRUE);
}

/******** processClickOnAssign() ********/

void processClickOnAssign(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR)
{
int realLine;

	realLine = line + *top;

	/**** create new path ****/

	stccpy(path, FLI.assignList+(realLine*SIZE_FILENAME), SIZE_FULLPATH);
	UA_ValidatePath(path);
	UA_TurnAssignIntoDir(path);
	UA_ValidatePath(path);

	LoadTheNewDir(path, path, opts, top, GR, window, SR);
}

/******** processClickOnDevice() ********/

void processClickOnDevice(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR)
{
int realLine;

	realLine = line + *top;

	/**** create new path ****/

	stccpy(path, FLI.deviceList+(realLine*SIZE_FILENAME), SIZE_FULLPATH);
	UA_ValidatePath(path);
	UA_TurnAssignIntoDir(path);
	UA_ValidatePath(path);

	LoadTheNewDir(path, path, opts, top, GR, window, SR);
}

/******** processClickOnHome() ********/

void processClickOnHome(	struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR )
{
int realLine;

	realLine = line + *top;

	/**** create new path ****/

	stccpy(path, homePaths+(realLine*SIZE_FULLPATH), SIZE_FULLPATH);
	UA_ValidatePath(path);
	UA_TurnAssignIntoDir(path);
	UA_ValidatePath(path);

	LoadTheNewDir(path, path, opts, top, GR, window, SR);
}

/******** processClickOnParent() ********/

void processClickOnParent(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR)
{
int realLine;

	realLine = line + *top;

	GetParentOf(path);

	LoadTheNewDir(path, path, opts, top, GR, window, SR);
}

/******** processClickOnSelectAll() ********/

void processClickOnSelectAll(struct Window *window, int top, struct ScrollRecord *SR)
{
int i;
BOOL someUnselected=FALSE;

	for(i=0; i<FLI.numFiles; i++)
	{
		if (	(*(FLI.fileList+i*SIZE_FILENAME) == FILE_PRECODE) &&
					*(FLI.selectionList+i) == 0 )
		{
			someUnselected=TRUE;
			break;
		}
	}

	if (someUnselected)
	{
		for(i=0; i<FLI.numFiles; i++)
		{
			if ( *(FLI.fileList+i*SIZE_FILENAME) == FILE_PRECODE )
				*(FLI.selectionList+i) = 1;
			else
				*(FLI.selectionList+i) = 0;
		}
	}
	else	/* all are selected, then deselect all */
	{
		for(i=0; i<FLI.numFiles; i++)
			*(FLI.selectionList+i) = 0;
	}

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(SR,top,NULL);
}

/******** LoadTheNewDir() ********/

void LoadTheNewDir(	STRPTR oldPath, STRPTR newPath, int opts, int *top,
										struct GadgetRecord *GR, struct Window *window,
										struct ScrollRecord *SR)
{
int i;

	SetSpriteOfActWdw(SPRITE_BUSY);

	OffGadget(&PropSlider1, window, NULL);

	/**** load dir ****/

	CloseDir();
	if ( !OpenDir(newPath, opts) )
		OpenDir(oldPath, opts);	/* get old path */
	else
		stccpy(oldPath, newPath, SIZE_FULLPATH);

	SR->list			 		= FLI.fileList;
	SR->numEntries 		= FLI.numFiles;
	SR->selectionList	= &(FLI.selectionList[0]);

	/**** init vars ****/

	*top=0;

	for(i=0; i<MAX_FILE_LIST_ENTRIES; i++)
		*(FLI.selectionList+i) = 0;

	/**** rework the display ****/

	UA_ClearButton(window, &GR[1], AREA_PEN);	/* title */
	GR[1].type = HIBOX_REGION;
	UA_DrawGadget(window, &GR[1]);
	UA_PrintInBox(window, &GR[1], GR[1].x1, GR[1].y1, GR[1].x2, GR[1].y2, oldPath, PRINT_RIGHTPART);
	GR[1].type = SPECIAL_STRING_GADGET;
	UA_ClearButton(window, &GR[2], AREA_PEN);	/* scroll area */
	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(SR,*top,NULL);
	UA_InitPropSlider(window, &PropSlider1, (ULONG)FLI.numFiles, 10, (ULONG)*top);

	OnGadget(&PropSlider1, window, NULL);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** E O F ********/
