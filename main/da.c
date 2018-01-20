#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern UBYTE *daPathList[];
extern UBYTE *daDescList[];
extern BOOL daUsedList[];
extern UBYTE **msgs;
extern struct MenuRecord **page_MR;

/**** gadgets ****/

extern struct GadgetRecord DANames_GR[];

/**** functions ****/

/******** Alloc_DA() ********/

BOOL Alloc_DA(void)
{
int i;

	for(i=0; i<10; i++)
	{
		daPathList[i] = NULL;
		daDescList[i] = NULL;
		daUsedList[i] = FALSE;
	}

	for(i=0; i<10; i++)
	{
		daPathList[i] = (UBYTE *)AllocMem(SIZE_FULLPATH, MEMF_ANY | MEMF_CLEAR);
		if (daPathList[i] == NULL)
		{
			UA_WarnUser(205);
			return(FALSE);
		}

		daDescList[i] = (UBYTE *)AllocMem(SIZE_FILENAME, MEMF_ANY | MEMF_CLEAR);
		if (daDescList[i] == NULL)
		{
			UA_WarnUser(206);
			return(FALSE);
		}
	}

	return(TRUE);
}

/******** Free_DA() ********/

void Free_DA(void)
{
int i;

	for(i=0; i<10; i++)
	{
		if (daPathList[i] != NULL)
		{
			FreeMem(daPathList[i], SIZE_FULLPATH);
			daPathList[i]=NULL;
		}
		if (daDescList[i] != NULL)
		{
			FreeMem(daDescList[i], SIZE_FILENAME);
			daDescList[i]=NULL;
		}
	}
}

/******** Process_DA_Menu() ********/

BOOL Process_DA_Menu(int daLine)
{
struct Window *window;
BOOL loop, retVal;
int ID;
TEXT buf1[SIZE_FULLPATH], buf2[SIZE_FULLPATH], buf3[SIZE_FULLPATH];
TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME];

	/**** init vars ****/

	path[0]			=	'\0';
	filename[0]	=	'\0';
	retVal			= FALSE;
	loop				= TRUE;

	/**** double or halve gadgets ****/

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		window = pageWindow;
	else
		window = scriptWindow;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(window,DANames_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(207);
		return(FALSE);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, DANames_GR);

	/**** if line has contents copy it into buf1 and buf2 ****/

	if (daUsedList[daLine])
	{
		stccpy(buf1, daDescList[daLine], SIZE_FILENAME);
		stccpy(buf2, daPathList[daLine], SIZE_FULLPATH);
		UA_SetStringGadgetToString(window, &DANames_GR[2], buf1);
	}
	else
	{
		stccpy(buf1, msgs[Msg_Untitled-1], SIZE_FULLPATH);
		stccpy(buf2, msgs[Msg_FA_3-1], SIZE_FULLPATH);	/* 'No Program Selected' */
		UA_SetStringGadgetToString(window, &DANames_GR[2], buf1);
		UA_DisableButton(window, &DANames_GR[2], gui_pattern); /* MenuName */
		UA_DisableButton(window, &DANames_GR[5], gui_pattern); /* OK */
		UA_DisableButton(window, &DANames_GR[7], gui_pattern); /* Remove */
	}

	/**** show path ****/

	stccpy(buf3, buf2, SIZE_FULLPATH);

	UA_PrintInBox(	window, &DANames_GR[4], DANames_GR[4].x1, DANames_GR[4].y1,
									DANames_GR[4].x2, DANames_GR[4].y2, buf3, PRINT_RIGHTPART);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, DANames_GR, &CED);
			switch(ID)
			{
				case 2:	/* description */
					UA_ProcessStringGadget(window, DANames_GR, &DANames_GR[ID], &CED);
					break;

				case 3:	/* pick application */
					UA_HiliteButton(window, &DANames_GR[ID]);

					if (daUsedList[daLine])
					{
						stccpy(buf2, daPathList[daLine], SIZE_FULLPATH);
						stcgfp(path, buf2);
					}
					else
						strcpy(path,CPrefs.import_picture_Path);

					retVal = OpenAFile(	path, filename, msgs[Msg_SelectAProgram-1],
															window, DIR_OPT_ALL | DIR_OPT_NOINFO, FALSE);
					if (retVal)
					{
						UA_MakeFullPath(path, filename, buf2);
						stccpy(buf3, buf2, SIZE_FULLPATH);
						UA_EnableButton(window, &DANames_GR[2]); /* MenuName */
						UA_EnableButton(window, &DANames_GR[5]); /* OK */
						UA_EnableButton(window, &DANames_GR[7]); /* Remove */
						UA_PrintInBox(	window, &DANames_GR[4], DANames_GR[4].x1, DANames_GR[4].y1,
														DANames_GR[4].x2, DANames_GR[4].y2, buf3, PRINT_RIGHTPART);
						UA_ShortenString(window->RPort, filename, DANames_GR[2].x2-DANames_GR[2].x1-16);
						UA_SetStringGadgetToString(window, &DANames_GR[2], filename);
					}
					break;

				case 5:	/* OK */
do_ok:
					UA_HiliteButton(window, &DANames_GR[5]);
					UA_SetStringToGadgetString(&DANames_GR[2], buf3);
					UA_ShortenString(window->RPort, buf3, page_MR[DA_MENU]->width-16);
					stccpy(daDescList[daLine], buf3, SIZE_FILENAME);
					page_MR[DA_MENU]->title[daLine+1] = daDescList[daLine];
					stccpy(daPathList[daLine], buf2, SIZE_FULLPATH);
					daUsedList[daLine] = TRUE;
					/* this is a way to rewrite a menu item */
					DisableMenu(page_MR[DA_MENU], daLine+1);
					EnableMenu(page_MR[DA_MENU], daLine+1);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 6:	/* cancel */
do_cancel:
					UA_HiliteButton(window, &DANames_GR[6]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 7:	/* remove */
					UA_HiliteButton(window, &DANames_GR[ID]);
					daUsedList[daLine] = FALSE;
					ClearMenuLine(page_MR[DA_MENU], daLine+1);
					loop=FALSE;
					retVal=TRUE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE && !UA_IsGadgetDisabled(&DANames_GR[6]))	/* cancel */
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&DANames_GR[5]))	/* OK */
				goto do_ok;
		}
	}

	if (retVal)
		updateCAPSconfig(1);	// update APPLIC lines

	UA_EnableButton(window, &DANames_GR[2]); /* MenuName */
	UA_EnableButton(window, &DANames_GR[5]); /* OK */
	UA_EnableButton(window, &DANames_GR[7]); /* Remove */

	UA_CloseRequesterWindow(window,STDCOLORS);

	return(retVal);
}

/******** ExecuteDA() ********/
/*
 * daLine ranges from 0 to 9.
 */

BOOL ExecuteDA(struct Window *window, int daLine)
{
struct FileHandle	*DOSHandle;
BOOL retVal=FALSE;
TEXT buf[SIZE_FULLPATH];

	if ( CED.Qualifier&IEQUALIFIER_LSHIFT ||
			 CED.Qualifier&IEQUALIFIER_RSHIFT ||
			 !daUsedList[daLine] )
	{
		Process_DA_Menu(daLine);
		return(TRUE);
	}
	else
	{
		DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
		if (DOSHandle != NULL)
		{
			//sprintf(buf, "run >NIL: \"%s\" >NIL: <NIL:", daPathList[daLine]);
			sprintf(buf, "run \"%s\"", daPathList[daLine]);

			OpenWorkBench();
			if (WBenchToFront())
				retVal = (BOOL)Execute((UBYTE *)buf, (BPTR)NULL, (BPTR)DOSHandle);
			else
			{
				ScreenToFront(window->WScreen);
				UA_WarnUser(208);	// no wb
			}

			Close((BPTR)DOSHandle);
		}
	}

	if (!retVal)
		Message(msgs[Msg_UnableToExecute-1], daPathList[daLine]);

	return(retVal);	/* more returns */
}

/******** E O F ********/
