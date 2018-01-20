/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include "nb:mpplayer/dongle/dongle_protos.h"

/**** defines ****/

#define PLAYER_UNITS "devs:player-units"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern UBYTE *daPathList[];
extern UBYTE *daDescList[];
extern BOOL daUsedList[];
extern struct FileInfoBlock *FIB;
extern UBYTE **msgs;
extern UBYTE *homeDirs;												
extern UBYTE *homePaths;												
extern struct Library *medialinkLibBase;
extern struct EditWindow prefsEW;
extern ULONG allocFlags;
extern TEXT MRO_Script[];
extern TEXT MRO_Page[];
extern TEXT *dir_system;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;

/**** globals ****/

char *CAPSconfigList[] = {
"PAGE", "SCRIPT", "PLAYER", "COLORS", "WORKBENCH",
"IMPORT_PICTURE_PATH", "IMPORT_TEXT_PATH", "DOCUMENT_PATH", "SCRIPT_PATH",
"ANIM_PATH", "MUSIC_PATH", "SAMPLE_PATH",
"USERLEVEL", "SHOWPROG", "TEXTEDITOR", "THUMBNAILS",
"F1_TIMECODE", "F2_TIMECODE", "F3_TIMECODE", "F4_TIMECODE", "F5_TIMECODE", "F6_TIMECODE",
"PRINTERPREFS", "NOGENLOCKING", "LANGUAGE", "STANDBY", "INPUT", "POINTER", "WINDOW",
"HOMEDIR", "APPLIC", "GAMEPORT", "FASTMENUS", "FORMAT", "STYLE",
"INPUT_DELAY", "FASTICONS",
NULL }; /* ALWAYS END WITH NULL! */

enum {
PARSE_PAGE, PARSE_SCRIPT, PARSE_PLAYER, PARSE_COLORS, PARSE_WORKBENCH,
PARSE_IMPORT_PICTURE_PATH, PARSE_IMPORT_TEXT_PATH, PARSE_DOCUMENT_PATH, PARSE_SCRIPT_PATH,
PARSE_ANIM_PATH, PARSE_MUSIC_PATH, PARSE_SAMPLE_PATH, 
PARSE_USERLEVEL, PARSE_SHOWPROG, PARSE_TEXTEDITOR, PARSE_THUMBNAILS,
PARSE_F1_TIMECODE, PARSE_F2_TIMECODE, PARSE_F3_TIMECODE, PARSE_F4_TIMECODE, PARSE_F5_TIMECODE, PARSE_F6_TIMECODE,
PARSE_PRINTERPREFS, PARSE_NOGENLOCKING, PARSE_LANGUAGE, PARSE_STANDBY, PARSE_INPUT, PARSE_POINTER, PARSE_WINDOW,
PARSE_HOMEDIR, PARSE_APPLIC, PARSE_GAMEPORT, PARSE_FASTMENUS, PARSE_FORMAT, PARSE_STYLE,
PARSE_INPUT_DELAY, PARSE_FASTICONS,
};

UWORD IconColorMap[] =
{
	0x100, 0x8fe, 0xfe6, 0xe0f, 0xee0, 0x0fe, 0x08f, 0xe08,
	0x0f0, 0x00f, 0xe00, 0x778,	0x880, 0x007, 0x070, 0xfef,
};

UWORD GrayMap[] =	// for 16 color thumbnails
{
	0xe11, 0x11e, 0x1e1, 0x2ee, 0xee2, 0xe2e, 0x555, 0xbbb,
};												//  ^^^^^ yellow!

UWORD GrayMap2[] =	// for 32 color thumbnails
{
	0x80f, 0x9f2, 0xfc9, 0xf66, 0xff0, 0xf08, 0x08f, 0x6f0, 
	0x0f6, 0xf60, 0xf0f, 0xf00, 0x00e, 0xff6, 0x888, 0x880, 
	0xd02, 0x068, 0x608, 0x080, 0x006, 0x700, 0x0ff, 0xf8f, 
};

UWORD allColors[]		= {	0x0000, 0x0999, 0x0888, 0x0777,	/*  0 - 3  */
												0x0555, 0x0444, 0x0333, 0x0222,	/*  4 - 7  */
												0x0d00, 0x0d80, 0x0dd0, 0x0080, /*  8 - 11 */
												0x000d, 0x055f, 0x0666, 0x0ddd, /* 12 - 15 */
												0x0f08, 0x0f66, 0x0700, 0x0fb9,	/* 16 SPRITE 17-19 */
												0x0608, 0x0f8f, 0x0f0f, 0x0ff6, /* 20 - 23 */
												0x0006, 0x066f, 0x008f, 0x00ff, /* 24 - 27 */
												0x04f9, 0x00f6, 0x09f2, 0x0880, /* 28 - 31 */
											};

UWORD palettes[] =
{ 
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0dfd,0x09b9,0x0797,0x0353,

0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0ffd,0x0bb9,0x0797,0x0353,

0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0fdd,0x0b99,0x0797,0x0353,

0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0fdf,0x0b9b,0x0797,0x0353,

0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0ddf,0x099b,0x0797,0x0353,

0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0779,0x0335,
0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0979,0x0535,
0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0799,0x0355,
0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0997,0x0553,
0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0977,0x0533,
0x0666,0x0000,0x0ddd,0x0888,0x0dff,0x09bb,0x0797,0x0353,
};

UWORD STD_NON_OBJ_PAL		= 12;	// number of script objects
UWORD STD_LACE_OBJ_PAL	= 24;
UWORD STD_NON_OBJ_NTSC	=	9;
UWORD STD_LACE_OBJ_NTSC	=	18;

TEXT path1[SIZE_FULLPATH];
TEXT path2[SIZE_FULLPATH];
TEXT path3[SIZE_FULLPATH];
TEXT path4[SIZE_FULLPATH];
TEXT path5[SIZE_FULLPATH];
TEXT path6[SIZE_FULLPATH];
TEXT path7[SIZE_FULLPATH];

/**** gadgets ****/

extern struct GadgetRecord FormatRequester_GR[];

/**** functions ****/

/******** GetConfigFile() ********/

BOOL GetConfigFile(void)
{
struct ParseRecord *PR;
TEXT buffer[MAXSCANDEPTH];
WORD args[MAX_PARSER_ARGS];
int instruc,i,len,DA_line;
TEXT fontname[50];
BOOL dongle;

	dongle = CheckDongle1();

	/**** init vars ****/

	DA_line=0;

	/**** open 's:medialink.config' ****/

	PR = (struct ParseRecord *)OpenParseFile(CAPSconfigList, CONFIG_TEXT);
	if (PR!=NULL)
	{
		instruc = -1;
		while(instruc != PARSE_STOP)
		{
			instruc = GetParserLine((struct ParseRecord *)PR, buffer);
			if (instruc == PARSE_INTERPRET)
			{
				passOneParser((struct ParseRecord *)PR, buffer);
				if (passTwoParser((struct ParseRecord *)PR))
				{
					switch(PR->commandCode)
					{
						case PARSE_PAGE:
							if (PR->numArgs==2)
								strcpy(CPrefs.pageMonName,PR->argString[1]);
							break;

						case PARSE_SCRIPT:
							if (PR->numArgs==4)
							{
								GetNumericalArgs(PR, args);
								strcpy(CPrefs.scriptMonName,PR->argString[1]);
								CPrefs.ScriptScreenWidth = *(args+1);
								CPrefs.ScriptScreenHeight = *(args+2);
							}
							break;

						case PARSE_PLAYER:
							if (PR->numArgs==2)
								strcpy(CPrefs.playerMonName,PR->argString[1]);
							break;

						case PARSE_COLORS:
							if (PR->numArgs==2)
							{
								GetNumericalArgs(PR, args);
#ifndef USED_FOR_PLAYER
								for(i=0; i<32; i++)
									SetColorCM4(CPrefs.PageCM, allColors[i], i);
#endif
								CPrefs.colorSet = (*args)-1;
							}
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_APPLIC:
							if ( PR->numArgs==3 && DA_line<=4 )	// 0...4 may be filled
							{
								len = strlen(PR->argString[1]);
								if (len>0)
									PR->argString[1][len-1] = '\0';
								len = strlen(PR->argString[2]);
								if (len>0)
									PR->argString[2][len-1] = '\0';
								stccpy(daPathList[DA_line], &PR->argString[1][1], SIZE_FULLPATH-1);
								stccpy(daDescList[DA_line], &PR->argString[2][1], SIZE_FILENAME-1);
								daUsedList[DA_line]=TRUE;
								DA_line++;
							}
							break;

#endif

						case PARSE_WORKBENCH:
							if (PR->numArgs==2)
							{
								if (strcmpi(PR->argString[1],"ON")==0)
									CPrefs.WorkBenchOn = TRUE;
								else if (strcmpi(PR->argString[1],"OFF")==0)
									CPrefs.WorkBenchOn = FALSE;
							}
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_IMPORT_PICTURE_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.import_picture_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.import_picture_Path);
							}
							break;

						case PARSE_IMPORT_TEXT_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.import_text_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.import_text_Path);
							}
							break;

						case PARSE_DOCUMENT_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.document_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.document_Path);
							}
							break;

						case PARSE_SCRIPT_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.script_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.script_Path);
							}
							break;

						case PARSE_ANIM_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.anim_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.anim_Path);
							}
							break;

						case PARSE_MUSIC_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.music_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.music_Path);
							}
							break;

						case PARSE_SAMPLE_PATH:
							if (PR->numArgs==2)
							{
								stccpy(CPrefs.sample_Path, &PR->argString[1][1], strlen(PR->argString[1])-1);
								UA_TurnAssignIntoDir(CPrefs.sample_Path);
							}
							break;
#endif

						case PARSE_USERLEVEL:
							if (PR->numArgs==2)
							{
								GetNumericalArgs(PR, args);
								CPrefs.userLevel = args[0];
							}
							break;

						case PARSE_SHOWPROG:
							if (PR->numArgs==2)
							{
								if (strcmpi(PR->argString[1],"ON")==0)
									CPrefs.showDays = TRUE;
								else if (strcmpi(PR->argString[1],"OFF")==0)
									CPrefs.showDays = FALSE;
							}
							break;

						case PARSE_TEXTEDITOR:
							if (PR->numArgs==2)
								stccpy(CPrefs.textEditor, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_THUMBNAILS:
							if (PR->numArgs==3)
							{
								if (strcmpi(PR->argString[1],"SMALL")==0)
									CPrefs.thumbnailSize = SMALL_THUMBNAILS;
								else if (strcmpi(PR->argString[1],"LARGE")==0)
									CPrefs.thumbnailSize = LARGE_THUMBNAILS;
								GetNumericalArgs(PR, args);
								CPrefs.ThumbnailScreenDepth = args[1];
							}
							break;

						case PARSE_F1_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F1_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_F2_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F2_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_F3_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F3_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_F4_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F4_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_F5_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F5_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_F6_TIMECODE:
							if (PR->numArgs==2)
								stccpy(CPrefs.F6_TIMECODE_STR, &PR->argString[1][1], strlen(PR->argString[1])-1);
							break;

						case PARSE_PRINTERPREFS:
							if (PR->numArgs==6)
							{
								GetNumericalArgs(PR, args);
								CPrefs.printerDest				= *(args);
								if (strcmpi(PR->argString[2],"TRUE")==0)
									CPrefs.printerTextOnly	= TRUE;
								else
									CPrefs.printerTextOnly	= FALSE;
								CPrefs.printerCopies			= *(args+2);
								CPrefs.printerScaleAndOri = *(args+3);
								CPrefs.printerQuality			= *(args+4);
							}
							break;

#endif

						case PARSE_LANGUAGE:
							if (PR->numArgs==2)
							{
								GetNumericalArgs(PR, args);
								CPrefs.lanCode = *(args);
								MakeLanExt(CPrefs.lanCode,CPrefs.lanExtension);
							}
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_HOMEDIR:
							if (PR->numArgs==4)
							{
								GetNumericalArgs(PR, args);
								if ( *(args) <= 10 )
								{
									RemoveQuotes(PR->argString[2]);	// e.g. MP-Modules
									RemoveQuotes(PR->argString[3]);	// e.g. work:mediapoint/sounds/modules
									strcpy( homeDirs  + ((*(args)-1)*SIZE_FILENAME), PR->argString[2] );
									strcpy( homePaths + ((*(args)-1)*SIZE_FULLPATH), PR->argString[3] );
								}
							}
							break;

#endif

						case PARSE_STANDBY:
							if (PR->numArgs==2)
							{
								if (strcmpi(PR->argString[1],"ON")==0)
									CPrefs.standBy = TRUE;
								else if (strcmpi(PR->argString[1],"OFF")==0)
									CPrefs.standBy = FALSE;
							}
							break;

						case PARSE_INPUT:
							if (PR->numArgs==2)
							{
								CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
								if (strcmpi(PR->argString[1],"CURSOR")==0)
									CPrefs.mousePointer |= 1;
								else if (strcmpi(PR->argString[1],"MOUSE")==0)
									CPrefs.mousePointer |= 2;
								else if (strcmpi(PR->argString[1],"CURSOR|MOUSE")==0)
									CPrefs.mousePointer |= 4;
								else
									CPrefs.mousePointer |= 8;	// NONE
							}
							break;

						case PARSE_POINTER:
							if (PR->numArgs==2)
							{
								CPrefs.mousePointer = CPrefs.mousePointer & ~(16+32+64);
								if (strcmpi(PR->argString[1],"ON")==0)
									CPrefs.mousePointer |= 16;
								else if (strcmpi(PR->argString[1],"OFF")==0)
									CPrefs.mousePointer |= 32;
								else
									CPrefs.mousePointer |= 64;	// AUTO
							}
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_WINDOW:
							if (PR->numArgs==17)
							{
								GetNumericalArgs(PR, args);

								if ( *(args+0) != -1 ) 	// top border color
								{
									prefsEW.BorderColor[0] = *(args+0);
									prefsEW.Border |= BORDER_TOP;
								}

								if ( *(args+1) != -1 )	// right border color
								{
									prefsEW.BorderColor[1] = *(args+1);
									prefsEW.Border |= BORDER_RIGHT;
								}

								if ( *(args+2) != -1 )	// bottom border color
								{
									prefsEW.BorderColor[2] = *(args+2);
									prefsEW.Border |= BORDER_BOTTOM;
								}

								if ( *(args+3) != -1 )	// left border color
								{
									prefsEW.BorderColor[3] = *(args+3);
									prefsEW.Border |= BORDER_LEFT;
								}

								prefsEW.BorderWidth = *(args+4);
								prefsEW.BackFillColor = *(args+5);
								prefsEW.BackFillType = *(args+6)-1;
	
								prefsEW.flags	= *(args+7);

								prefsEW.TopMargin = *(args+ 8);
								prefsEW.RightMargin = *(args+ 9);
								prefsEW.BottomMargin = *(args+10);
								prefsEW.LeftMargin = *(args+11);

								prefsEW.patternNum = *(args+12);

								prefsEW.wdw_shadowDepth	= *(args+13);
								prefsEW.wdw_shadowDirection = *(args+14);
								prefsEW.wdw_shadowPen = *(args+15);
							}
							break;

						case PARSE_FORMAT:
							if (PR->numArgs==13)
							{
								GetNumericalArgs(PR, args);
								prefsEW.antiAliasLevel	= *(args+0);
								prefsEW.justification		= *(args+1);
								prefsEW.xSpacing				= *(args+2);
								prefsEW.ySpacing				= *(args+3);
								prefsEW.slantAmount			= *(args+4);
								prefsEW.slantValue			= *(args+5);
								prefsEW.underLineHeight	= *(args+6);
								prefsEW.underLineOffset	= *(args+7);
								prefsEW.shadowDepth			= *(args+8);
								prefsEW.shadow_Pen			= *(args+9);
								prefsEW.shadowType			= *(args+10);
								prefsEW.shadowDirection	= *(args+11);
							}
							break;

						case PARSE_STYLE:
							if (PR->numArgs==6)
							{
								GetNumericalArgs(PR, args);
								strcpy(fontname, PR->argString[1]);
								strcat(fontname, ".font");
								if ( OpenTypeFace(fontname, *(args+1), 0, FALSE) )	// FALSE means no messages if font can't be found
									prefsEW.charFont = textFont;
								else
									prefsEW.charFont = largeFont;
								prefsEW.charStyle = *(args+2);
								prefsEW.charColor = *(args+3);
								prefsEW.underlineColor = *(args+4);
							}
							break;

#endif

						case PARSE_GAMEPORT:
							if (PR->numArgs==13 || PR->numArgs==14)
							{										// GAMEPORT 1,2,3,4,5,6,7,8,9,10,11,microsecs
								GetNumericalArgs(PR, args);
								for(i=0; i<11; i++)
								{
									if ( strlen(PR->argString[i+1])==2 )	// GAMEPORT "","" etc.
										CPrefs.gameport[i] = 0;
									else
										CPrefs.gameport[i] = PR->argString[i+1][1];
								}
								//CPrefs.gameport_used=FALSE;
								//for(i=0; i<11; i++)
								//	if ( CPrefs.gameport[i] != 0 )
								//		CPrefs.gameport_used=TRUE;
								sscanf(PR->argString[12], "%d", &i);
								CPrefs.gameport_delay = i;
							}
#if 0
							if (PR->numArgs==14)
							{										// GAMEPORT 1,2,3,4,5,6,7,8,9,10,11,microsecs,"YES/NO"
								if (strcmpi(PR->argString[13],"ON")==0)
									CPrefs.gameport_used = TRUE;
								else if (strcmpi(PR->argString[13],"OFF")==0)
									CPrefs.gameport_used = FALSE;
							}
#endif
							break;

#ifndef USED_FOR_PLAYER

						case PARSE_FASTMENUS:
							if (PR->numArgs==2)
							{
								if (strcmpi(PR->argString[1],"YES")==0)
									CPrefs.fastMenus = TRUE;
								else if (strcmpi(PR->argString[1],"NO")==0)
									CPrefs.fastMenus = FALSE;
							}
							break;

#endif

						case PARSE_INPUT_DELAY:
							if (PR->numArgs==2)
							{
								sscanf(PR->argString[1], "%d", &i);
								CPrefs.input_delay = i;
							}
							break;

						case PARSE_FASTICONS:
							if (PR->numArgs==2)
							{
								if (strcmpi(PR->argString[1],"YES")==0)
									CPrefs.fastIcons = TRUE;
								else if (strcmpi(PR->argString[1],"NO")==0)
									CPrefs.fastIcons = FALSE;
							}
							break;
					}
				}
			}
		}
		CloseParseFile(PR);

		strcpy(path1, CPrefs.import_picture_Path);
		strcpy(path2, CPrefs.import_text_Path);
		strcpy(path3, CPrefs.document_Path);
		strcpy(path4, CPrefs.script_Path);
		strcpy(path5, CPrefs.anim_Path);
		strcpy(path6, CPrefs.music_Path);
		strcpy(path7, CPrefs.sample_Path);

		if ( !dongle )
			CPrefs.standBy=TRUE;

		return(TRUE);
	}

	return(FALSE);
}

/******** SetConfigDefaults() ********/

void SetConfigDefaults(void)
{
int i;

	prefsEW.TopMargin			 			= DEFAULT_TM;
	prefsEW.BottomMargin	 			= DEFAULT_BM;
	prefsEW.LeftMargin		 			= DEFAULT_LM;
	prefsEW.RightMargin		 			= DEFAULT_RM;
	prefsEW.Border				 			= DEFAULT_BORDER;
	prefsEW.BorderColor[0] 			= DEFAULT_BCOLOR;
	prefsEW.BorderColor[1] 			= DEFAULT_BCOLOR;
	prefsEW.BorderColor[2] 			= DEFAULT_BCOLOR;
	prefsEW.BorderColor[3] 			= DEFAULT_BCOLOR;
	prefsEW.BorderWidth		 			= DEFAULT_BWIDTH;
	prefsEW.BackFillType	 			= DEFAULT_BFTYPE;
	prefsEW.BackFillColor	 			= DEFAULT_BFCOLOR;
	prefsEW.flags					 			= 0;
	prefsEW.patternNum		 			= DEFAULT_PATTERN;
	prefsEW.wdw_shadowDepth 		= 4;
	prefsEW.wdw_shadowDirection	= 0;
	prefsEW.wdw_shadowPen				= 1;

	prefsEW.antiAliasLevel			= 0;
	prefsEW.justification				= 0;
	prefsEW.xSpacing						= 0;
	prefsEW.ySpacing						= 0;
	prefsEW.slantAmount					= 2;
	prefsEW.slantValue					= 1;
	prefsEW.underLineHeight			= 1;
	prefsEW.underLineOffset			= 0;
	prefsEW.shadowDepth					= 0;
	prefsEW.shadow_Pen					= 0;
	prefsEW.shadowType					= 0;
	prefsEW.shadowDirection			= 0;

	prefsEW.charFont						= largeFont;
	prefsEW.underlineColor			= 2;
	prefsEW.charStyle						= 0;
	prefsEW.charColor						= 2;

	/**** page resolution ****/

#ifndef USED_FOR_PLAYER
	for(i=0; i<32; i++)
		SetColorCM4(CPrefs.PageCM, allColors[i], i);
#endif

	/**** init misc ****/

	CPrefs.overScan	= 0;
	CPrefs.colorSet = 1;
	CPrefs.WorkBenchOn = TRUE;
	CPrefs.userLevel = 1;
	CPrefs.showDays = FALSE;
	stccpy(CPrefs.textEditor, TEXT_DEFAULT_ED, 128);
	CPrefs.thumbnailSize = SMALL_THUMBNAILS;

	CPrefs.printerDest				= PRINTER_DEST_PRINTER;
	CPrefs.printerTextOnly		= FALSE;
	CPrefs.printerCopies			= 1;
	CPrefs.printerScaleAndOri	= PRINTER_ORI_PORTRAIT;
	CPrefs.printerQuality			= PRINTER_QUALITY_LETTER;

	stccpy(CPrefs.F1_TIMECODE_STR, "00:00:00:00", 16);
	stccpy(CPrefs.F2_TIMECODE_STR, "00:00:01:00", 16);
	stccpy(CPrefs.F3_TIMECODE_STR, "00:00:02:00", 16);
	stccpy(CPrefs.F4_TIMECODE_STR, "00:00:05:00", 16);
	stccpy(CPrefs.F5_TIMECODE_STR, "00:00:10:00", 16);
	stccpy(CPrefs.F6_TIMECODE_STR, "00:00:20:00", 16);

	/**** init standard dirs ****/

	strcpy(CPrefs.import_picture_Path,"RAM:");
	UA_ValidatePath(CPrefs.import_picture_Path);
	stccpy(CPrefs.import_text_Path, CPrefs.import_picture_Path, SIZE_PATH);
	stccpy(CPrefs.document_Path, CPrefs.import_picture_Path, SIZE_PATH);
	stccpy(CPrefs.script_Path, CPrefs.import_picture_Path, SIZE_PATH);
	stccpy(CPrefs.anim_Path, CPrefs.import_picture_Path, SIZE_PATH);
	stccpy(CPrefs.music_Path, CPrefs.import_picture_Path, SIZE_PATH);
	stccpy(CPrefs.sample_Path, CPrefs.import_picture_Path, SIZE_PATH);

	/**** player device ****/

	if ( !GetPlayerDeviceInfo() )
	{
		CPrefs.PDevice.playerName[0] = '\0';
		stccpy(CPrefs.PDevice.deviceName, "serial.device", 40);
		CPrefs.PDevice.baudRate	= -1;	// -1 means no device selected
		CPrefs.PDevice.unit	= 1;
	}

	CPrefs.objectPreLoading = 30;	// b+1 scheme
	CPrefs.playOptions			= 3;	// 1=auto, 2=manual, 3=auto+manual
	CPrefs.scriptTiming			= 0;	// 0=normal, 1=precise
	CPrefs.bufferOptions		= 1;	// 0=keep, 1=flush

	//CPrefs.noGenLock = FALSE;

	CPrefs.lanCode = 1; // english
	MakeLanExt(CPrefs.lanCode,CPrefs.lanExtension);

	CPrefs.customTimeCode[0] = '\0';

	// 1 = cursor, 2=mouse, 4=c+m, 8=none, 16=ptr on, 32=ptr off, 64=ptr auto, 128=async

	CPrefs.mousePointer = 1 | 64;
	CPrefs.standBy = 0;
	CPrefs.playFrom = 0;

	// GamePort default

	for(i=0; i<11; i++)
		CPrefs.gameport[i] = 0;
	CPrefs.gameport_used = FALSE;
	CPrefs.gameport_delay = 250000L;

	// Fast menus

	CPrefs.fastMenus = FALSE;

	CPrefs.input_delay = 0L;	//1000000L;

	CPrefs.fastIcons = FALSE;
}

/******** MakeLanExt() ********/
/*
 * This code is also used in trans.c
 *
 */

void MakeLanExt(UBYTE lanCode, STRPTR lanExt)
{
	switch(lanCode)
	{
		case  1:	strcpy(lanExt, "English"); break;
		case  2:	strcpy(lanExt, "Nederlands"); break;
		case  3:	strcpy(lanExt, "Deutsch"); break;
		case  4:	strcpy(lanExt, "Français"); break;
		case  5:	strcpy(lanExt, "Español"); break;
		case  6:	strcpy(lanExt, "Italiano"); break;
		case  7:	strcpy(lanExt, "Português"); break;
		case  8:	strcpy(lanExt, "Dansk"); break;
		case  9:	strcpy(lanExt, "Svenska"); break;
		case 10:	strcpy(lanExt, "Norsk"); break;
		default:	strcpy(lanExt, "English"); break;
	}
}

#ifndef USED_FOR_PLAYER

/******** WriteConfigFile() ********/

void WriteConfigFile(void)
{
FILE *fp;
int i, p1, p2, p3, p4, bitValue, fontsize;
TEXT fontname[50];

	fp = fopen(CONFIG_TEXT, "w");
	if (fp==NULL)
	{
		UA_WarnUser(89);
		return;
	}

	// PAGE

	fprintf(fp, "%s %s\n", CAPSconfigList[PARSE_PAGE], CPrefs.pageMonName);
	
	// SCRIPT

	fprintf(fp, "%s %s %d %d\n", CAPSconfigList[PARSE_SCRIPT], CPrefs.scriptMonName,
						CPrefs.ScriptScreenWidth, CPrefs.ScriptScreenHeight);

	// PLAYER

	fprintf(fp, "%s %s\n", CAPSconfigList[PARSE_PLAYER], CPrefs.playerMonName);

	// COLORS

	fprintf(fp, "%s %d\n", CAPSconfigList[PARSE_COLORS], CPrefs.colorSet+1);

	// WORKBENCH

	if (CPrefs.WorkBenchOn)
		fprintf(fp, "%s ON\n", CAPSconfigList[PARSE_WORKBENCH]);
	else
		fprintf(fp, "%s OFF\n", CAPSconfigList[PARSE_WORKBENCH]);

	// PATHS

	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_IMPORT_PICTURE_PATH], CPrefs.import_picture_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_IMPORT_TEXT_PATH],		CPrefs.import_text_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_DOCUMENT_PATH],				CPrefs.document_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_SCRIPT_PATH],					CPrefs.script_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_ANIM_PATH],						CPrefs.anim_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_MUSIC_PATH],					CPrefs.music_Path);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_SAMPLE_PATH],					CPrefs.sample_Path);

	// USERLEVEL

	fprintf(fp, "%s %d\n", CAPSconfigList[PARSE_USERLEVEL], CPrefs.userLevel);

#if 0
	// SHOWPROG
	if (CPrefs.showDays)
		fprintf(fp, "%s ON\n", CAPSconfigList[PARSE_SHOWPROG]);
	else
		fprintf(fp, "%s OFF\n", CAPSconfigList[PARSE_SHOWPROG]);
#endif

	// TEXTEDITOR

	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_TEXTEDITOR],	CPrefs.textEditor);

	// THUMBNAILS

	if (CPrefs.thumbnailSize == SMALL_THUMBNAILS)
		fprintf(fp, "%s SMALL %d\n", CAPSconfigList[PARSE_THUMBNAILS],CPrefs.ThumbnailScreenDepth);
	else
		fprintf(fp, "%s LARGE %d\n", CAPSconfigList[PARSE_THUMBNAILS],CPrefs.ThumbnailScreenDepth);

	// Fx_TIMECODE

	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F1_TIMECODE], CPrefs.F1_TIMECODE_STR);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F2_TIMECODE], CPrefs.F2_TIMECODE_STR);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F3_TIMECODE], CPrefs.F3_TIMECODE_STR);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F4_TIMECODE], CPrefs.F4_TIMECODE_STR);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F5_TIMECODE], CPrefs.F5_TIMECODE_STR);
	fprintf(fp, "%s \"%s\"\n", CAPSconfigList[PARSE_F6_TIMECODE], CPrefs.F6_TIMECODE_STR);

	// PRINTERPREFS

	if (CPrefs.printerTextOnly)
		fprintf(fp, "%s %d, TRUE, %d, %d, %d\n", CAPSconfigList[PARSE_PRINTERPREFS],
						CPrefs.printerDest, CPrefs.printerCopies,
						CPrefs.printerScaleAndOri, CPrefs.printerQuality);
	else
		fprintf(fp, "%s %d, FALSE, %d, %d, %d\n", CAPSconfigList[PARSE_PRINTERPREFS],
						CPrefs.printerDest, CPrefs.printerCopies,
						CPrefs.printerScaleAndOri, CPrefs.printerQuality);

#if 0
	// NOGENLOCKING

	if ( CPrefs.noGenLock )
		fprintf(fp, "%s ON\n", CAPSconfigList[PARSE_NOGENLOCKING]);
	else
		fprintf(fp, "%s OFF\n", CAPSconfigList[PARSE_NOGENLOCKING]);
#endif

	// LANGUAGE

	fprintf(fp, "%s %d\n", CAPSconfigList[PARSE_LANGUAGE], CPrefs.lanCode);

#if 0
	// STANDBY
	if ( CPrefs.standBy )
		fprintf(fp, "%s ON\n", CAPSconfigList[PARSE_STANDBY]);
	else
		fprintf(fp, "%s OFF\n", CAPSconfigList[PARSE_STANDBY]);
#endif

#if 0
	// INPUT
	if ( CPrefs.mousePointer & 1 )						// cursor
		fprintf(fp, "%s CURSOR\n", CAPSconfigList[PARSE_INPUT]);
	else if ( CPrefs.mousePointer & 2 )				// mouse
		fprintf(fp, "%s MOUSE\n", CAPSconfigList[PARSE_INPUT]);
	else if ( CPrefs.mousePointer & 4 )				// cursor+mouse
		fprintf(fp, "%s CURSOR|MOUSE\n", CAPSconfigList[PARSE_INPUT]);
	else																			// none
		fprintf(fp, "%s NONE\n", CAPSconfigList[PARSE_INPUT]);
#endif

#if 0
	// POINTER
	if ( CPrefs.mousePointer & 16 )						// mouse on
		fprintf(fp, "%s ON\n", CAPSconfigList[PARSE_POINTER]);
	else if ( CPrefs.mousePointer & 32 )			// mouse off
		fprintf(fp, "%s OFF\n", CAPSconfigList[PARSE_POINTER]);
	else																			// auto
		fprintf(fp, "%s AUTO\n", CAPSconfigList[PARSE_POINTER]);
#endif

	// WINDOW

	p1=-1;
	p2=-1;
	p3=-1;
	p4=-1;
	bitValue = (ULONG)prefsEW.Border;
	if (TestBit(bitValue, BORDER_TOP))
		p1 = prefsEW.BorderColor[0];
	if (TestBit(bitValue, BORDER_RIGHT))
		p2 = prefsEW.BorderColor[1];
	if (TestBit(bitValue, BORDER_BOTTOM))
		p3 = prefsEW.BorderColor[2];
	if (TestBit(bitValue, BORDER_LEFT))
		p4 = prefsEW.BorderColor[3];
	fprintf(	fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						CAPSconfigList[PARSE_WINDOW],
						p1, p2, p3, p4, prefsEW.BorderWidth, prefsEW.BackFillColor,
						prefsEW.BackFillType+1,
						prefsEW.flags,
						prefsEW.TopMargin, prefsEW.RightMargin,
						prefsEW.BottomMargin, prefsEW.LeftMargin,
						prefsEW.patternNum,
						prefsEW.wdw_shadowDepth, prefsEW.wdw_shadowDirection, prefsEW.wdw_shadowPen );

	// FORMAT

	fprintf(	fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						CAPSconfigList[PARSE_FORMAT],
						prefsEW.antiAliasLevel,
						prefsEW.justification,
						prefsEW.xSpacing,
						prefsEW.ySpacing,
						prefsEW.slantAmount,
						prefsEW.slantValue,
						prefsEW.underLineHeight,
						prefsEW.underLineOffset,
						prefsEW.shadowDepth,
						prefsEW.shadow_Pen,
						prefsEW.shadowType,
						prefsEW.shadowDirection );

	// STYLE 
	if (!findFont(prefsEW.charFont, fontname, &fontsize))
	{
		strcpy(fontname, SHORTAPPFONT);
		fontsize=20;
	}
	fprintf(fp, "%s %s,%d,%d,%d,%d\n",
					CAPSconfigList[PARSE_STYLE],
					fontname,
					fontsize,
					prefsEW.charStyle,
					prefsEW.charColor,
					prefsEW.underlineColor);

	// HOMEDIR

	for(i=0; i<10; i++)
	{
		if ( *(homeDirs+i*SIZE_FILENAME) != '\0' )
		{
			fprintf(fp, "%s %d, \"%s\", \"%s\"\n",
							CAPSconfigList[PARSE_HOMEDIR], i+1,
							homeDirs + i*SIZE_FILENAME,
							homePaths + i*SIZE_FULLPATH );
		}
	}

	// APPLIC

	for(i=0; i<5; i++)
	{
		if (daUsedList[i])
			fprintf(fp, "%s \"%s\" \"%s\"\n", CAPSconfigList[PARSE_APPLIC],
							daPathList[i], daDescList[i]);
	}

	// GAMEPORT
	fprintf(fp, "%s ", CAPSconfigList[PARSE_GAMEPORT]);
	for(i=0; i<11; i++)
	{
		if (i>0)
			fprintf(fp, ",");
		if ( CPrefs.gameport[i] == 0 )
			fprintf(fp, "\"\"");
		else
			fprintf(fp, "\"%c\"", CPrefs.gameport[i]);
	}
	fprintf(fp, ",%d\n", CPrefs.gameport_delay);

	// FASTMENUS

	if ( CPrefs.fastMenus )
		fprintf(fp, "%s YES\n", CAPSconfigList[PARSE_FASTMENUS]);
	else
		fprintf(fp, "%s NO\n", CAPSconfigList[PARSE_FASTMENUS]);

	// INPUT_DELAY

	fprintf(fp, "%s %d\n", CAPSconfigList[PARSE_INPUT_DELAY], CPrefs.input_delay);

	// FASTICONS

	if ( CPrefs.fastIcons )
		fprintf(fp, "%s YES\n", CAPSconfigList[PARSE_FASTICONS]);
	else
		fprintf(fp, "%s NO\n", CAPSconfigList[PARSE_FASTICONS]);

	fclose(fp);
}

/******** updateCAPSconfig() ********/

void updateCAPSconfig(int which)
{
FILE *fp;
int i;
BOOL success;
TEXT temp[80], pattern[256];

	sprintf(temp, "%s_temp", CONFIG_TEXT);

	if (which==1)							// APPLIC
		stccpy(pattern, CAPSconfigList[PARSE_APPLIC], 256);
	else if (which==2)				// PRINTERPREFS
		stccpy(pattern, CAPSconfigList[PARSE_PRINTERPREFS], 256);
	else
		return;

	if (cleanUpFile(CONFIG_TEXT, pattern, CAPSconfigList))
	{
		fp = fopen(temp, "a");
		if (fp==NULL)
			return;

		if (which==1)						// APPLIC
		{
			for(i=0; i<5; i++)
			{
				if (daUsedList[i])
					fprintf(fp, "%s \"%s\", \"%s\"\n", CAPSconfigList[PARSE_APPLIC],
								daPathList[i], daDescList[i]);
			}
		}
		else if (which==2)			// PRINTERPREFS
		{
			if (CPrefs.printerTextOnly)
				fprintf(fp, "%s %d, TRUE, %d, %d, %d\n", CAPSconfigList[PARSE_PRINTERPREFS],
								CPrefs.printerDest, CPrefs.printerCopies,
								CPrefs.printerScaleAndOri, CPrefs.printerQuality);
			else
				fprintf(fp, "%s %d, FALSE, %d, %d, %d\n", CAPSconfigList[PARSE_PRINTERPREFS],
								CPrefs.printerDest, CPrefs.printerCopies,
								CPrefs.printerScaleAndOri, CPrefs.printerQuality);
		}

		fclose(fp);
		success=DeleteFile(CONFIG_TEXT);
		if (!success)
			UA_WarnUser(90);
		else
		{
			success=Rename(temp, CONFIG_TEXT);
			if (!success)
				UA_WarnUser(91);
		}
	}
}

/******** cleanUpFile() ********/
/*
 * Reads config file and writes out new one, except for lines
 * containing 'pattern' (this way e.g. the APPLIC lines are updated)
 *
 */

BOOL cleanUpFile(STRPTR path, STRPTR pattern, char **cmdList)
{
struct ParseRecord *PR;
TEXT buffer[MAXSCANDEPTH];
int instruc;
FILE *fp;
TEXT path2[256];

	sprintf(path2, "%s_temp", path);
	fp = fopen(path2, "w");
	if (fp==NULL)
		return(FALSE);

	/* open file */
	PR = (struct ParseRecord *)OpenParseFile(cmdList, path);
	if (PR!=NULL)
	{
		instruc = -1;
		while(instruc != PARSE_STOP)
		{
			instruc = GetParserLine((struct ParseRecord *)PR, buffer);
			if (instruc == PARSE_INTERPRET)
			{
				passOneParser((struct ParseRecord *)PR, buffer);
				if (passTwoParser((struct ParseRecord *)PR))
				{
					if (strcmp(PR->argString[0], pattern)!=0)
						fprintf(fp, "%s\n", buffer);
				}
			}
		}
		CloseParseFile(PR);
	}
	else
	{
		UA_WarnUser(92);
		fclose(fp);
		return(FALSE);
	}

	fclose(fp);
	return(TRUE);
}
#endif

/******** GetPlayerDeviceInfo() ********/

BOOL GetPlayerDeviceInfo(void)
{
FILE *fp;
int dummy;

	/* 0 Sony_1200_9600 9600 serial.device 3 */

	fp = fopen(PLAYER_UNITS, "r");
	if (fp==NULL)
		return(FALSE);
	fscanf(	fp, "%d %s %d %s %d\n",
					&dummy,
					CPrefs.PDevice.playerName,
					&(CPrefs.PDevice.baudRate),
					CPrefs.PDevice.deviceName,
					&(CPrefs.PDevice.unit) );
	fclose(fp);

	return(TRUE);
}

#ifndef USED_FOR_PLAYER

/******** InitMRO() ********/

void InitMRO(void)
{
int i;

	for(i=0; i<5; i++)
	{
		MRO_Script[ i*SIZE_FULLPATH ] = '\0';
		MRO_Page[   i*SIZE_FULLPATH ] = '\0';
	}

	LoadMRO();
}

/******** AddMRO() ********/

void AddMRO( STRPTR MRO, STRPTR path )
{
int i,hole;

	// Check if path should be remembered

	for(i=0; i<5; i++)
		if ( !strcmpi(&MRO[i*SIZE_FULLPATH],path) )
			return;	// path already remembered

	// OK, path should be remembered but is there an empty hole...

	hole=-1;
	for(i=0; i<5; i++)
	{
		if ( MRO[i*SIZE_FULLPATH] == '\0' )
		{
			hole=i;
			break;
		}
	}

	if ( hole!=-1 )
		stccpy(&MRO[hole*SIZE_FULLPATH],path,SIZE_FULLPATH);
	else
	{
		for(i=1; i<5; i++)
			stccpy(&MRO[(i-1)*SIZE_FULLPATH],&MRO[i*SIZE_FULLPATH],SIZE_FULLPATH);
		stccpy(&MRO[4*SIZE_FULLPATH],path,SIZE_FULLPATH);
	}

	SaveMRO();	// was originally only done at end of program but for safety reasons
							// (crashes on exiting) moved to here.
}

/******** SaveMRO() ********/

void SaveMRO(void)
{
FILE *fp;
TEXT path[SIZE_FULLPATH];
int i;

	UA_MakeFullPath(dir_system,"MRO_script",path);
	fp = fopen(path,"w");
	if (fp)
	{
		for(i=0; i<5; i++)
		{
			if ( MRO_Script[i*SIZE_FULLPATH] != '\0' )
			{
				if ( i>0 )
					fprintf(fp,"\n");
				fprintf(fp,"%s",&MRO_Script[i*SIZE_FULLPATH]);
			}
		}
		fclose(fp);
	}

	UA_MakeFullPath(dir_system,"MRO_page",path);
	fp = fopen(path,"w");
	if (fp)
	{
		for(i=0; i<5; i++)
		{
			if ( MRO_Page[i*SIZE_FULLPATH] != '\0' )
			{
				if ( i>0 )
					fprintf(fp,"\n");
				fprintf(fp,"%s",&MRO_Page[i*SIZE_FULLPATH]);
			}
		}
		fclose(fp);
	}
}

/******** LoadMRO() ********/

void LoadMRO(void)
{
FILE *fp;
TEXT path[SIZE_FULLPATH];
int i,j;
char ch;
struct FileLock *FL;

	UA_MakeFullPath(dir_system,"MRO_script",path);
	fp = fopen(path,"r");
	if (fp)
	{
		for(i=0; i<5; i++)
		{
			j=0;
			while( j<SIZE_FULLPATH )
			{
				ch = getc(fp);
				if ( feof(fp)!=0 || ch=='\0' || ch=='\n' )
					break;
				else
				{
					MRO_Script[i*SIZE_FULLPATH+j] = ch;
					MRO_Script[i*SIZE_FULLPATH+j+1]	= '\0';
					j++;
				}
			}
		}
		fclose(fp);
	}

	for(i=0; i<5; i++)
	{
		if ( MRO_Script[i*SIZE_FULLPATH] != '\0' )
		{
			FL = (struct FileLock *)Lock((STRPTR)&MRO_Script[i*SIZE_FULLPATH], (LONG)ACCESS_READ);
			if (!FL)
				MRO_Script[i*SIZE_FULLPATH] = '\0';
			else
				UnLock((BPTR)FL);
		}
	}

	UA_MakeFullPath(dir_system,"MRO_page",path);
	fp = fopen(path,"r");
	if (fp)
	{
		for(i=0; i<5; i++)
		{
			j=0;
			while( j<SIZE_FULLPATH )
			{
				ch = getc(fp);
				if ( feof(fp)!=0 || ch=='\0' || ch=='\n' )
					break;
				else
				{
					MRO_Page[i*SIZE_FULLPATH+j] = ch;
					MRO_Page[i*SIZE_FULLPATH+j+1]	= '\0';
					j++;
				}
			}
		}
		fclose(fp);
	}

	for(i=0; i<5; i++)
	{
		if ( MRO_Page[i*SIZE_FULLPATH] != '\0' )
		{
			FL = (struct FileLock *)Lock((STRPTR)&MRO_Page[i*SIZE_FULLPATH], (LONG)ACCESS_READ);
			if (!FL)
				MRO_Page[i*SIZE_FULLPATH] = '\0';
			else
				UnLock((BPTR)FL);
		}
	}
}

#endif

/******** E O F ********/
