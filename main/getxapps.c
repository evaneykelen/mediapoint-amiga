#include "nb:pre.h"
#include "nb:xapp_names.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern ULONG allocFlags;
extern struct Window *scriptWindow;
extern struct TextFont *smallFont;
extern UWORD chip gui_pattern[];
extern UWORD chip gui_pattern_lace[];
extern TEXT *dir_xapps;
extern TEXT *dir_system;
extern struct RastPort xappRP;
extern struct RastPort xappRP_2;
extern int kept_totalHeight;
extern UBYTE *objectNameList[];
extern int objectTypeList[];
extern int objectXPosList[];
extern int objectYPosList[];
extern BOOL IconEnabledList[];	// index is index in tool bar
extern BOOL ToolEnabledList[];	// index is type number
extern int standardXPosList[];
extern int standardYPosList[];
extern int numLevelTools;
extern ULONG numEntries2, numDisplay2;
extern LONG topEntry2;
extern UBYTE **msgs;
extern struct Library *medialinkLibBase;
extern struct FileListInfo FLI;

/**** static globals ****/

static struct BitMap xappBM;
static struct BitMap xappBM_2;
static UBYTE *temp_objectNameList[MAXTOOLS];
static int temp_objectTypeList[MAXTOOLS];

/**** globals ****/

int clipLine = 0;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** OpenToolIcons() ********/

BOOL OpenToolIcons(struct Window *window, int *tHeight)
{
int i;

	*tHeight = 0;

	/**** init static arrays ****/

	for(i=0; i<MAXTOOLS; i++)
	{
		objectNameList[i] 			= NULL;
		temp_objectNameList[i] 	= NULL;
		objectTypeList[i] 			= -1;
		temp_objectTypeList[i]	= -1;
		objectXPosList[i] 			= 0;
		objectYPosList[i] 			= 0;
		standardXPosList[i] 		= 0;
		standardYPosList[i] 		= 0;
		IconEnabledList[i] 			= TRUE;
		ToolEnabledList[i] 			= TRUE;
	}

	/*************** SCAN 1st DIRECTORY (XAPPS) *****************/

	if ( !OpenDir(dir_xapps, DIR_OPT_ALL | DIR_OPT_NODIRS) )
	{
		Message(msgs[Msg_NoXappsDir-1], msgs[Msg_AppName-1]);
		return(FALSE);
	}

	if ( !SniffXapps(FLI.numFiles, 1) )
	{
		CloseDir();
		return(FALSE);
	}
	CloseDir();

	/*************** SCAN 2nd DIRECTORY (SYSTEM) *****************/

	if ( !OpenDir(dir_system, DIR_OPT_ALL | DIR_OPT_NODIRS) )
	{
		Message(msgs[Msg_NoXappsDir-1], msgs[Msg_AppName-1]);
		return(FALSE);
	}

	if ( !SniffXapps(FLI.numFiles, 2) )
	{
		CloseDir();
		return(FALSE);
	}
	CloseDir();

	/*************** PLACE ICONS IN BITMAP *****************/

	if ( !PlaceIconsInBitMap() )
		return(FALSE);

	*tHeight = numEntries2 = kept_totalHeight;

	SetBit(&allocFlags,XAPPSLOADED_FLAG);

	return(TRUE);
}

/******** SniffXapps() ********/
/*
 * mode 1 -> scan xapps
 * mode 2 -> scan system
 *
 */

BOOL SniffXapps(int numFiles, UBYTE mode)
{
int i,j,xappLevel,objType;
struct DiskObject *dobj;
TEXT filename[SIZE_FILENAME], path[SIZE_FULLPATH];
char *s;

	for(i=0; i<numFiles; i++)
	{
		stccpy(filename, FLI.fileList+i*SIZE_FILENAME+1, SIZE_FILENAME); 
		if ( UA_FindString(filename,".info")==-1 )
		{
			if ( mode==1 )
				UA_MakeFullPath(dir_xapps, filename, path);
			else
				UA_MakeFullPath(dir_system, filename, path);

			dobj = GetDiskObject(path);
			if (dobj!=NULL)
			{
				/**** when the user level is at 3, the xapps with USERLEVEL	****/
				/**** 1,2,3 must be loaded.																	****/

				s = (char *)FindToolType(dobj->do_ToolTypes, "USERLEVEL");
				if ( MatchToolValue(s, "1") )
					xappLevel=1;
				else if ( MatchToolValue(s, "2") )
					xappLevel=2;
				else if ( MatchToolValue(s, "3") )
					xappLevel=3;
				else if ( MatchToolValue(s, "4") )
					xappLevel=4;
				else
					xappLevel=1;

				if ( xappLevel <= CPrefs.userLevel )
				{
					s = (char *)FindToolType(dobj->do_ToolTypes, "TYPE");
					if ( MatchToolValue(s, "XAPP") )
						objType = TALK_USERAPPLIC;
					else if ( MatchToolValue(s, "SYSTEM") )
					{
						if ( !stricmp(filename, ANIMATION_XAPP) )
							objType = TALK_ANIM;
						else if ( !stricmp(filename, AREXX_XAPP) )
							objType = TALK_AREXX;
						else if ( !stricmp(filename, DOS_XAPP) )
							objType = TALK_DOS;
						else if ( !stricmp(filename, MUSIC_XAPP) )
							objType = TALK_SOUND;
						else if ( !stricmp(filename, TRANSITIONS_XAPP) )
							objType = TALK_PAGE;
						else if ( !stricmp(filename, VARS_XAPP) )
							objType = TALK_VARS;
					}						
					else if ( MatchToolValue(s, "CONTROLLER") )
					{
						if ( !stricmp(filename, INPUT_XAPP) )
							objType = TALK_GLOBALEVENT;
						else if ( !stricmp(filename, PARALLEL_XAPP) )
							objType = TALK_STARTPAR;
						else if ( !stricmp(filename, SERIAL_XAPP) )
							objType = TALK_STARTSER;
					}						
					else if ( MatchToolValue(s, "ICON") )
					{
						if ( !stricmp(filename, GOTO_XAPP) )
							objType = TALK_GOTO;
						else if ( !stricmp(filename, TIMECODE_XAPP) )
							objType = TALK_TIMECODE;
						else if ( !stricmp(filename, NOP_XAPP) )
							objType = TALK_NOP;
						else if ( !stricmp(filename, MAIL_XAPP) )
							objType = TALK_MAIL;
						else if ( !stricmp(filename, DATA_XAPP) )
							objType = TALK_BINARY;
						else if ( !stricmp(filename, LABEL_XAPP) )
							objType = TALK_LABEL;
						else if ( !stricmp(filename, INPUTSETTINGS_XAPP) )
							objType = TALK_INPUTSETTINGS;
					}
					else
						objType = -1;

					if ( objType != -1 )
					{
						j=0;
						while( temp_objectNameList[j] != NULL )	// find empty slot
							j++;
						temp_objectNameList[j] = AllocMem(SIZE_FILENAME, MEMF_ANY | MEMF_CLEAR);
						if (temp_objectNameList[j]==NULL)
						{
							FreeDiskObject(dobj);
							UA_WarnUser(128);
							return(FALSE);
						}
						else
						{
							stccpy(temp_objectNameList[j],filename,SIZE_FILENAME-1);
							temp_objectTypeList[j] = objType;
						}
					}
				}
				FreeDiskObject(dobj);
			}
		}
	}

	return(TRUE);
}

/******** PlaceIconsInBitMap() ********/

BOOL PlaceIconsInBitMap(void)
{
int i, j, pos, numTools=0, padNum, width, height, h;

	/**** count number of xapps ****/

	for(i=0; i<MAXTOOLS; i++)
	{
		if ( temp_objectNameList[i] != NULL )
		{
			numTools++;
		}
	}

	padNum = numTools;

	if ( (padNum%3) != 0 )
	{
		padNum -= (padNum % 3);
		padNum += 3;
	}

	h = padNum / 3;
	height = (h*ICONHEIGHT) + (h-1)*4;	// 4 in width between xapps

	if ( height < (Script_GR[2].y2 - Script_GR[2].y1) )
		height = Script_GR[2].y2 - Script_GR[2].y1;
	else if ( height > (Script_GR[2].y2 - Script_GR[2].y1) )
		height += 4;

	width = Script_GR[2].x2-Script_GR[2].x1-3;

	numLevelTools = numTools;		// globals
	kept_totalHeight = height;	// globals

	/**** allocate xapps bitmap ****/

	InitBitMap(&xappBM, (LONG)TOOLSDEPTH, (LONG)width, (LONG)height);

	for(i=0; i<TOOLSDEPTH; i++)
		xappBM.Planes[i] = NULL;

	for(i=0; i<TOOLSDEPTH; i++)
	{
		xappBM.Planes[i] = (PLANEPTR)AllocRaster((LONG)width, (LONG)height);
		if (xappBM.Planes[i]==NULL)
		{
			for(i=0; i<TOOLSDEPTH; i++)
				FreeRaster(xappBM.Planes[i], (LONG)width, (LONG)height);
			UA_WarnUser(127);	
			return(FALSE);
		}
	}

	InitRastPort(&xappRP);
	xappRP.BitMap = (struct BitMap *)&xappBM;

	Move(&xappRP, 0L, 0L);
	SetRast(&xappRP, UA_GetRightPen(scriptWindow,&Script_GR[2],AREA_PEN));
	WaitBlit();
	SetFont(&xappRP, smallFont);

	/**** allocate undo xapps bitmap ****/

	InitBitMap(&xappBM_2, (LONG)TOOLSDEPTH, (LONG)width, (LONG)height);

	for(i=0; i<TOOLSDEPTH; i++)
		xappBM_2.Planes[i] = NULL;

	for(i=0; i<TOOLSDEPTH; i++)
	{
		xappBM_2.Planes[i] = (PLANEPTR)AllocRaster((LONG)width, (LONG)height);
		if (xappBM_2.Planes[i]==NULL)
		{
			for(i=0; i<TOOLSDEPTH; i++)
				FreeRaster(xappBM_2.Planes[i], (LONG)width, (LONG)height);
			UA_WarnUser(127);	
			return(FALSE);
		}
	}

	InitRastPort(&xappRP_2);
	xappRP_2.BitMap = (struct BitMap *)&xappBM_2;

	/**** place icons in bitmap ****/

	j=0;

	/**** 1st row ****/

	pos = GetIconIndex(TALK_PAGE);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_ANIM);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_SOUND);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	for(i=0; i<MAXTOOLS; i++)
	{
		if (	temp_objectNameList[i] != NULL &&
					temp_objectTypeList[i] == TALK_USERAPPLIC &&
					!strnicmp(temp_objectNameList[i],"sample",5) )
		{
			PutIconAt(i, 2, j++);
			break;
		}
	}

	/**** 2nd row ****/

	pos = GetIconIndex(TALK_AREXX);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_DOS);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_GLOBALEVENT);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	/**** 3rd row ****/

	pos = GetIconIndex(TALK_STARTSER);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_STARTPAR);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_NOP);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	/**** 4th row ****/

	pos = GetIconIndex(TALK_GOTO);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_LABEL);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_TIMECODE);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	/**** 5th row ****/

	pos = GetIconIndex(TALK_VARS);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_INPUTSETTINGS);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	/**** these xapps are not available ****/
	/*
	pos = GetIconIndex(TALK_BINARY);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	

	pos = GetIconIndex(TALK_MAIL);
	if (pos!=-1)
		PutIconAt(pos, 1, j++);	
	*/

	/**** last rows ****/

	for(i=0; i<MAXTOOLS; i++)
	{
		if (	temp_objectNameList[i] && temp_objectTypeList[i] == TALK_USERAPPLIC &&
					strnicmp(temp_objectNameList[i],"sample",5) )
		{
			PutIconAt(i, 2, j++);
		}
	}

	ClipBlit(&xappRP,0,0,&xappRP_2,0,0,width,height,0xc0L);

	return(TRUE);
}

/******** GetIconIndex() ********/

int GetIconIndex(int type)
{
int i;

	for(i=0; i<MAXTOOLS; i++)
		if ( temp_objectNameList[i] != NULL && temp_objectTypeList[i] == type )
			return(i);
	return(-1);	
}

/******** PutIconAt() ********/
	
void PutIconAt(int obj, UBYTE mode, int pos)
{
TEXT name[SIZE_FILENAME], path[SIZE_FULLPATH];
struct DiskObject *dobj;
struct Image *wbImage;
struct Gadget *wbGadget;

	strcpy(name, temp_objectNameList[obj]);

	if ( mode==1 )
		UA_MakeFullPath(dir_system, name, path);
	else
		UA_MakeFullPath(dir_xapps, name, path);

	dobj = GetDiskObject(path);
	if (dobj==NULL)
		Message(msgs[Msg_XappMissesInfo-1],name);
	else
	{
		objectNameList[pos] = temp_objectNameList[obj];
		objectTypeList[pos] = temp_objectTypeList[obj];

		wbGadget = &(dobj->do_Gadget);
		wbImage = wbGadget->GadgetRender;

		objectXPosList[pos] = 9 + ((pos%3) * 51);
		objectYPosList[pos] = 1 + (pos/3) * (ICONHEIGHT+4);

		standardXPosList[ objectTypeList[pos] ] = objectXPosList[pos];
		standardYPosList[ objectTypeList[pos] ] = objectYPosList[pos];

		if (wbImage->Width <= ICONWIDTH && wbImage->Height <= ICONHEIGHT)
			DrawImage(&xappRP, wbImage, objectXPosList[pos], objectYPosList[pos]);
		else
		{
			SetAPen(&xappRP, LO_PEN);
			SetDrMd(&xappRP, JAM1);
			Move(&xappRP, objectXPosList[pos], objectYPosList[pos]);
			Text(&xappRP, "š", 1L);
			Message(msgs[Msg_IconTooLarge-1],name);
		}
		FreeDiskObject(dobj);
	}
}

/******** ShowToolIcons() ********/

void ShowToolIcons(struct Window *window, int line)
{
int height;

	if (line==-1)
	{
		clipLine = 0;
		line = 0;
	}
	else if (line!=-2)	/* if line==-2, use static clipLine */
		clipLine = line;	/* remember line */

	height = Script_GR[2].y2-Script_GR[2].y1-3;
	if ( kept_totalHeight < height )
		height = kept_totalHeight;

	WaitTOF();
	ClipBlit(	&xappRP, 0, (LONG)clipLine, window->RPort,
						(LONG)Script_GR[2].x1+2, (LONG)Script_GR[2].y1+2,
						(LONG)Script_GR[2].x2-Script_GR[2].x1-3, (LONG)height,
						0xc0L);
}

/******** CloseToolIcons() ********/

void CloseToolIcons(void)
{
int i;

	if ( TestBit(allocFlags,XAPPSLOADED_FLAG) )
	{
		for(i=0; i<MAXTOOLS; i++)
		{
			if (objectNameList[i] != NULL)
				FreeMem(objectNameList[i], SIZE_FILENAME);
			objectNameList[i] = NULL;
		}

		for(i=0; i<TOOLSDEPTH; i++)
		{
			if (xappBM.Planes[i] != NULL)
				FreeRaster(xappBM.Planes[i], (LONG)xappBM.BytesPerRow*8, (LONG)xappBM.Rows);
			xappBM.Planes[i] = NULL;

			if (xappBM_2.Planes[i] != NULL)
				FreeRaster(xappBM_2.Planes[i], (LONG)xappBM_2.BytesPerRow*8, (LONG)xappBM_2.Rows);
			xappBM_2.Planes[i] = NULL;
		}
	}
	UnSetBit(&allocFlags,XAPPSLOADED_FLAG);
}

/******** DrawToolDots() ********/

void DrawToolDots(int type, BOOL flag)
{
int i;

	SetDrMd(&xappRP, JAM1);

	if ( CPrefs.ScriptScreenModes & LACE )
	{
		SetAfPt(&xappRP, gui_pattern_lace, 2);
	}
	else
	{
		SetAfPt(&xappRP, gui_pattern, 1);
	}

	for(i=0; i<MAXTOOLS; i++)
	{
		if (objectTypeList[i] == type)
		{
			if ( IconEnabledList[i] && !flag )	// is enabled, must *not* be enabled
			{																		// make it ghosted
				RectFill(	&xappRP,
									(LONG)objectXPosList[i]+2,
									(LONG)objectYPosList[i]+1,
									(LONG)objectXPosList[i]+ICONWIDTH-3,
									(LONG)objectYPosList[i]+ICONHEIGHT-2);
				IconEnabledList[i] = flag;
				ToolEnabledList[type] = flag;
			}
			else if ( !IconEnabledList[i] && flag )		// is not enabled, must be enabled
			{																					// get original
				ClipBlit(	&xappRP_2,
									(LONG)objectXPosList[i]+2, (LONG)objectYPosList[i]+1,
									&xappRP,
									(LONG)objectXPosList[i]+2, (LONG)objectYPosList[i]+1,
									(LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0L);
				IconEnabledList[i] = flag;
				ToolEnabledList[type] = flag;
			}
		}
	}

	SetAfPt(&xappRP, NULL, 0);
}

/******** DisableTool() ********/

void DisableTool(int type)
{
	DrawToolDots(type, FALSE);
}

/******** EnableTool() ********/

void EnableTool(int type)
{
	DrawToolDots(type, TRUE);
}

/******** E O F ********/
