#include "nb:pre.h"
#include "xapp_names.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern UBYTE **msgs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct TextFont *tiny_smallFont;
extern struct TextFont *tiny_largeFont;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern struct RastPort xappRP_2;
extern TEXT *dir_xapps;
extern TEXT *dir_system;

STATIC void datestampToTime_V2(ULONG minute, ULONG tick, STRPTR str);
STATIC BOOL FindVersion(STRPTR fileName, STRPTR verStr);
STATIC long Skip[256];

/**** gadgets ****/

extern struct GadgetRecord XappInfo_GR[];

/**** functions ****/

/******** ShowXappInfo() ********/

void ShowXappInfo(struct ScriptNodeRecord *this_node)
{
struct Window *window;
BOOL loop=TRUE;
int ID,y,type=0,i;
char *str;
BPTR lock;
struct FileInfoBlock __aligned fib;
TEXT path[SIZE_FULLPATH], date[32], time[32], info[256];

	// OPEN WINDOW

	window = UA_OpenRequesterWindow(scriptWindow, XappInfo_GR, USECOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	// DRAW GADGETS

	UA_DrawGadgetListRange(window, XappInfo_GR, 0, 3);

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		SetFont(window->RPort, tiny_largeFont);
	else
		SetFont(window->RPort, tiny_smallFont);

	UA_DrawGadgetListRange(window, XappInfo_GR, 5, 17);

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		y=66*2;
	else
		y=66;

	// SHOW OBJECT'S ICON

	if (this_node->nodeType == TALK_USERAPPLIC)
		ClipBlit(	&xappRP_2,
							(LONG)objectXPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
							(LONG)objectYPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
							window->RPort,
							39, y, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);			
	else
		ClipBlit(	&xappRP_2,
							(LONG)standardXPosList[this_node->nodeType],
							(LONG)standardYPosList[this_node->nodeType],
							window->RPort,
							39, y, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);

	// PRINT OBJECT'S NAME

	SetAPen(window->RPort, 4L);
	SetDrMd(window->RPort, JAM1);

	str = NULL;
	if (this_node->nodeType == TALK_USERAPPLIC)
		str = this_node->objectPath;
	else
	{
		switch( this_node->nodeType )
		{
			case TALK_PAGE:					str = TRANSITIONS_XAPP;	break;
			case TALK_ANIM:					str = ANIMATION_XAPP;		break;
			case TALK_SOUND:				str = MUSIC_XAPP;				break;
			case TALK_AREXX:				str = AREXX_XAPP;				break;
			case TALK_DOS:					str = DOS_XAPP;					break;

			case TALK_GLOBALEVENT:	str = INPUT_XAPP;				break;
			case TALK_STARTSER:			str = SERIAL_XAPP;			break;
			case TALK_STARTPAR:			str = PARALLEL_XAPP;		break;

			case TALK_NOP:					str = NOP_XAPP;					break;
			case TALK_GOTO:					str = GOTO_XAPP;				break;
			case TALK_LABEL:				str = LABEL_XAPP;				break;
			case TALK_INPUTSETTINGS:str = INPUTSETTINGS_XAPP;break;

			case TALK_TIMECODE:			str = TIMECODE_XAPP;		break;
			case TALK_VARS:					str = VARS_XAPP;				break;

			case TALK_BINARY:				break;
			case TALK_MAIL:					break;
		}
	}

	// PRINT OBJECT'S LAST CHANGED DATE

	if( str )
	{
		// TRY XAPPS
		UA_MakeFullPath(dir_xapps,str,path);
		lock=Lock(path,SHARED_LOCK);
		if (lock)
		{
			if( Examine(lock,&fib) )
			{
				type=1;	// means we're dealing with a xapp
				datestampToTime_V2(fib.fib_Date.ds_Minute, fib.fib_Date.ds_Tick, time);
				datestampToDate(fib.fib_Date.ds_Days, date);
				if ( fib.fib_Size > 100 )
					sprintf(path,"%s - %s - size %d bytes", date, time, fib.fib_Size);
				else
					sprintf(path,"%s - %s", date, time);
				Move(window->RPort, 120, XappInfo_GR[10].y1-1);
				Text(window->RPort,path,strlen(path));
			}
			UnLock(lock);
		}
		// TRY SYSTEM
		UA_MakeFullPath(dir_system,str,path);
		lock=Lock(path,SHARED_LOCK);
		if (lock)
		{
			if( Examine(lock,&fib) )
			{
				type=2;	// means we're dealing with a system xapp
				datestampToTime_V2(fib.fib_Date.ds_Minute, fib.fib_Date.ds_Tick, time);
				datestampToDate(fib.fib_Date.ds_Days, date);
				if ( fib.fib_Size > 100 )
					sprintf(path,"%s - %s - size %d bytes", date, time, fib.fib_Size);
				else
					sprintf(path,"%s - %s", date, time);
				Move(window->RPort, 120, XappInfo_GR[10].y1-1);
				Text(window->RPort,path,strlen(path));
			}
			UnLock(lock);
		}
	}

	if ( str )
	{
		strcpy(info,str);
		if ( type==1 )
			UA_MakeFullPath(dir_xapps,str,path);
		else
			UA_MakeFullPath(dir_system,str,path);
		date[0]='\0';
		if ( FindVersion(path,date) )
			sprintf(info, "%s - version %s", str, date);
		else
			sprintf(info, "%s - version unknown", str);
		Move(window->RPort, 120, XappInfo_GR[9].y1-1);
		Text(window->RPort,info,strlen(info));
	}

	// GET INFO FROM TOOL TYPES

	if ( str )
	{
		if ( type==1 )
			UA_MakeFullPath(dir_xapps,str,path);
		else
			UA_MakeFullPath(dir_system,str,path);
		for(i=1; i<=7; i++)	// 7 lines (1 for author, 6 for description)
		{
			sprintf(date,"INFO%d",i);
			if ( GetToolTypeStr(path, date, info) )
			{
				Move(window->RPort, 120, XappInfo_GR[10+i].y1-1);
				Text(window->RPort, info, strlen(info));
			}
		}
	}

	// EVENT LOOP

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, XappInfo_GR, &CED);
			switch(ID)
			{
				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &XappInfo_GR[3]);
					loop=FALSE;
					break;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
	}

	UA_CloseRequesterWindow(window,USECOLORS);
}

/******** GetToolTypeStr() ********/

BOOL GetToolTypeStr(STRPTR path, STRPTR tag, STRPTR answer)
{
struct DiskObject *diskObj;
char *s;
BOOL retval=FALSE;

	diskObj = GetDiskObject(path);
	if ( diskObj )
	{
		s = (char *)FindToolType(diskObj->do_ToolTypes,tag);
		if ( s )
		{
			retval=TRUE;
			strcpy(answer, s);
		}
		FreeDiskObject(diskObj);
	}

	return(retval);
}

/******** datestampToTime() ********/

STATIC void datestampToTime_V2(ULONG minute, ULONG tick, STRPTR str)
{
int h,m,s;

	h = (int)(minute/60L);
	m = (int)(minute%60L);
	s = (int)(tick/50L);
	tick = tick - (s*50);
	sprintf(str, "%02d:%02d:%02d", h,m,s);
}

/******** init_skip() ********/

void init_skip( char *pat )
{
	long i,j,M = strlen( pat );

	for( i = 0; i < 256; Skip[ i++ ] = M );

	for( i = 0, j = M-1; j >=0; j--, i++ )
		if( Skip[ pat[ j ] ] == M )
			Skip[ pat[ j ] ] = i;
}

/******** BM_Search() ********/

long BM_Search( char *dat, char *pat, long size )
{
	long i, j, M = strlen( pat), t;

	for( i = M - 1, j = M - 1; j >= 0; i--, j-- )
		while( dat[i] != pat[j] )
		{
			t = Skip[ (unsigned char )dat[ i ] ];
			i += ( M - j > t ) ? M - j : t;
			if( i >= size )
				return 0;
			j = M - 1;
		}
	return i;
}

/******** find_string() ********/

char *find_string( char *dat, char *pat, long size )
{
	long i = 0;

	init_skip( pat );
	if( ( i = BM_Search( dat, pat, size ) ) != 0  )
		return dat + i + 1;
	else
		return NULL;
}

/******** FindVersion() ********/

STATIC BOOL FindVersion(STRPTR fileName, STRPTR verStr)
{
struct FileHandle *FH;
UBYTE *mem;
ULONG realSize;
char *p;
BOOL retVal=FALSE;

	mem = (UBYTE *)AllocMem(200000L, MEMF_PUBLIC|MEMF_CLEAR);
	if (mem)
	{
		FH = (struct FileHandle *)Open(fileName, (LONG)MODE_OLDFILE);
		if (FH)
		{
			realSize = Read((BPTR)FH, mem, 200000L);
			if (realSize!=-1)
			{
				p = find_string(mem, "$VER: ", realSize);
				if ( p )
				{
					strcpy(verStr,p+6);
					retVal=TRUE;
				}
			}
			Close((BPTR)FH);
		}
		FreeMem(mem, 200000L);
	}

	return(retVal);
}

/******** E O F ********/
