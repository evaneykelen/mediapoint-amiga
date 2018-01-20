#include "nb:pre.h"

/**************************************************************************/
/**************************************************************************/
/**** when doing menus, check page.c and pedit.c for dis/en able stuff ****/
/**************************************************************************/
/**************************************************************************/

STATIC TEXT NAME_SCRIPTMANAGER[] = { "ScriptManager" };
STATIC TEXT Icon_Project[] 	= { "" };
STATIC TEXT Icon_Edit[] 		= { "" };
STATIC TEXT Icon_Xfer[] 		= { "" };
STATIC TEXT Icon_Misc[] 		= { "" };
STATIC TEXT Icon_Text[] 		= { "" };

#define MENUWIDTH 180					// see also globalallocs.c	and  menubar.c
#define FAST_MENU_WIDTH		27	// see also initmenus.c
#define FAST_MENU_HEIGHT	74	// see also initmenus.c

/**** externals ****/

extern struct RastPort gfxRP;
extern struct TextFont *smallFont;
extern struct CapsPrefs CPrefs;
extern UBYTE *daPathList[];
extern UBYTE *daDescList[];
extern BOOL daUsedList[];
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern struct Screen **DA_Screens;
extern UBYTE **msgs;   
extern TEXT special_char[];
extern struct eventHandlerInfo EHI;
extern struct Library *medialinkLibBase;
extern struct MenuRecord fast_script_MR;
extern struct MenuRecord fast_page_MR;
extern TEXT MRO_Script[];
extern TEXT MRO_Page[];

/**** functions ****/

/******** OpenMenus() ********/

void OpenMenus(void)
{
int i,j;
TEXT buff[SIZE_FILENAME];

	SetFont(&gfxRP, smallFont);

	/**** init page menus ****/

	for (i=0; i<NUMMENUS; i++)
	{
		for(j=0; j<16; j++)
		{
			page_MR[i]->commandKey[j]	= NULL;
			page_MR[i]->disabled[j]		= FALSE;
			page_MR[i]->title[j] 			= NULL;
			page_MR[i]->shifted[j] 		= FALSE;
		}
	}

	for(j=0; j<16; j++)
	{
		fast_script_MR.commandKey[j]= NULL;
		fast_script_MR.disabled[j]	= FALSE;
		fast_script_MR.title[j] 		= NULL;
		fast_script_MR.shifted[j] 	= FALSE;

		fast_page_MR.commandKey[j]	= NULL;
		fast_page_MR.disabled[j]		= FALSE;
		fast_page_MR.title[j] 			= NULL;
		fast_page_MR.shifted[j] 		= FALSE;
	}

	/* DA */

	page_MR[DA_MENU]->width = MENUWIDTH;
	page_MR[DA_MENU]->height = (11*MHEIGHT)+2;

	page_MR[DA_MENU]->commandKey[0] = '?';

	for(i=0; i<5; i++)
	{
		if (daUsedList[i] && daPathList[i]!=NULL && daDescList[i]!=NULL)
		{
			stccpy(buff, daDescList[i], SIZE_FILENAME);
			UA_ShortenString(&gfxRP, buff, page_MR[DA_MENU]->width-24);
			stccpy(daDescList[i], buff, SIZE_FILENAME);
			page_MR[DA_MENU]->title[i+1] = daDescList[i];
		}
	}

	for(i=0; i<5; i++)
	{
		page_MR[DA_MENU]->commandKey[i+1] = 0x30+i;
		page_MR[DA_MENU]->shifted[i+1] = TRUE;
	}

	/* File */

	page_MR[FILE_MENU]->width = MENUWIDTH;
	page_MR[FILE_MENU]->height = (8*MHEIGHT)+2;

	page_MR[FILE_MENU]->commandKey[0] = 'N';
	page_MR[FILE_MENU]->commandKey[1] = 'O';
	page_MR[FILE_MENU]->commandKey[2] = 'W';
	page_MR[FILE_MENU]->commandKey[3] = 'S';
	page_MR[FILE_MENU]->commandKey[4] = 'S';
	page_MR[FILE_MENU]->commandKey[7] = 'Q';

	page_MR[FILE_MENU]->shifted[4] = TRUE;

	/* Edit */

	page_MR[EDIT_MENU]->width = MENUWIDTH;
	page_MR[EDIT_MENU]->height = (8*MHEIGHT)+2;

	page_MR[EDIT_MENU]->commandKey[0] = 'Z';
	page_MR[EDIT_MENU]->commandKey[1] = 'X';
	page_MR[EDIT_MENU]->commandKey[2] = 'C';
	page_MR[EDIT_MENU]->commandKey[3] = 'V';
	page_MR[EDIT_MENU]->commandKey[5] = 'A';

	/* Font */

	page_MR[FONT_MENU]->width = MENUWIDTH;
	page_MR[FONT_MENU]->height = (7*MHEIGHT)+2;

	page_MR[FONT_MENU]->commandKey[0] = 'F';
	page_MR[FONT_MENU]->commandKey[1] = 'Y';
	page_MR[FONT_MENU]->commandKey[2] = 'L';
	page_MR[FONT_MENU]->commandKey[3] = 'P';
	page_MR[FONT_MENU]->commandKey[4] = 'B';
	page_MR[FONT_MENU]->commandKey[5] = 'I';
	page_MR[FONT_MENU]->commandKey[6] = 'U';

	page_MR[FONT_MENU]->shifted[3] = TRUE;
	page_MR[FONT_MENU]->shifted[4] = TRUE;
	page_MR[FONT_MENU]->shifted[5] = TRUE;
	page_MR[FONT_MENU]->shifted[6] = TRUE;

	/* Misc */

	page_MR[PMISC_MENU]->width = MENUWIDTH;
	page_MR[PMISC_MENU]->height = (9*MHEIGHT)+2;

	page_MR[PMISC_MENU]->commandKey[0] = 'I';
	page_MR[PMISC_MENU]->commandKey[1] = 'D';
	page_MR[PMISC_MENU]->commandKey[2] = 'P';
	page_MR[PMISC_MENU]->commandKey[3] = 'M';
	page_MR[PMISC_MENU]->commandKey[4] = 'H';
	page_MR[PMISC_MENU]->commandKey[5] = 'R';
	page_MR[PMISC_MENU]->commandKey[6] = 'E';
	page_MR[PMISC_MENU]->commandKey[7] = 'T';
	page_MR[PMISC_MENU]->commandKey[8] = 'B';

	/* Screen */

	page_MR[SCREEN_MENU]->width	= MENUWIDTH;
	page_MR[SCREEN_MENU]->height = (11*MHEIGHT)+2;

	page_MR[SCREEN_MENU]->commandKey[0] = '1';
	page_MR[SCREEN_MENU]->commandKey[1] = '2';
	page_MR[SCREEN_MENU]->commandKey[2] = '0';

	/**** create page menus ****/

	InitializeMenu(page_MR[DA_MENU], smallFont);
	InitializeMenu(page_MR[FILE_MENU], smallFont);
	InitializeMenu(page_MR[EDIT_MENU], smallFont);
	InitializeMenu(page_MR[FONT_MENU], smallFont);
	InitializeMenu(page_MR[PMISC_MENU], smallFont);
	InitializeMenu(page_MR[SCREEN_MENU], smallFont);

	/**** init script menus ****/

	for (i=0; i<NUMMENUS; i++)
	{
		for(j=0; j<16; j++)
		{
			script_MR[i]->commandKey[j]	= NULL;
			script_MR[i]->disabled[j] = FALSE;
			script_MR[i]->title[j] = NULL;
			script_MR[i]->shifted[j] = FALSE;
		}
	}

	/**** fill script menus ****/

	/* DA */

	CopyMem(&page_MR[DA_MENU]->menuBM, &script_MR[DA_MENU]->menuBM, sizeof(struct BitMap));
	CopyMem(&page_MR[DA_MENU]->menuRP, &script_MR[DA_MENU]->menuRP, sizeof(struct RastPort));

	script_MR[DA_MENU]->width = page_MR[DA_MENU]->width;
	script_MR[DA_MENU]->height = page_MR[DA_MENU]->height;

	script_MR[DA_MENU]->title[0] = page_MR[DA_MENU]->title[0];

	script_MR[DA_MENU]->commandKey[0] = '?';

	for(i=0; i<10; i++)
	{
		script_MR[DA_MENU]->commandKey[i+1] = 0x30+i;
		script_MR[DA_MENU]->shifted[i+1] = TRUE;
	}

	/* File */

	script_MR[FILE_MENU]->width = MENUWIDTH;
	script_MR[FILE_MENU]->height = (8*MHEIGHT)+2;

	script_MR[FILE_MENU]->commandKey[0] = 'N';
	script_MR[FILE_MENU]->commandKey[1] = 'O';
	script_MR[FILE_MENU]->commandKey[2] = 'W';
	script_MR[FILE_MENU]->commandKey[3] = 'S';
	script_MR[FILE_MENU]->commandKey[4] = 'S';
	script_MR[FILE_MENU]->commandKey[7] = 'Q';

	script_MR[FILE_MENU]->shifted[4] = TRUE;

	script_MR[FILE_MENU]->title[0] = page_MR[FILE_MENU]->title[0];
	script_MR[FILE_MENU]->title[1] = page_MR[FILE_MENU]->title[1];
	script_MR[FILE_MENU]->title[2] = page_MR[FILE_MENU]->title[2];
	script_MR[FILE_MENU]->title[3] = page_MR[FILE_MENU]->title[3];
	script_MR[FILE_MENU]->title[4] = page_MR[FILE_MENU]->title[4];
	script_MR[FILE_MENU]->title[5] = page_MR[FILE_MENU]->title[5];
	script_MR[FILE_MENU]->title[6] = page_MR[FILE_MENU]->title[6];
	script_MR[FILE_MENU]->title[7] = page_MR[FILE_MENU]->title[7];

	/* Edit */

	script_MR[EDIT_MENU]->width	= MENUWIDTH;
	script_MR[EDIT_MENU]->height = (6*MHEIGHT)+2;

	script_MR[EDIT_MENU]->commandKey[0] = 'Z';
	script_MR[EDIT_MENU]->commandKey[1] = 'X';
	script_MR[EDIT_MENU]->commandKey[2] = 'C';
	script_MR[EDIT_MENU]->commandKey[3] = 'V';
	script_MR[EDIT_MENU]->commandKey[5] = 'A';

	script_MR[EDIT_MENU]->title[0] 	= page_MR[EDIT_MENU]->title[0];
	script_MR[EDIT_MENU]->title[1] 	= page_MR[EDIT_MENU]->title[1];
	script_MR[EDIT_MENU]->title[2] 	= page_MR[EDIT_MENU]->title[2];
	script_MR[EDIT_MENU]->title[3] 	= page_MR[EDIT_MENU]->title[3];
	script_MR[EDIT_MENU]->title[4]	= page_MR[EDIT_MENU]->title[4];
	script_MR[EDIT_MENU]->title[5] 	= page_MR[EDIT_MENU]->title[5];

	/* Xfer */

	script_MR[XFER_MENU]->width	= MENUWIDTH;
	script_MR[XFER_MENU]->height = (1*MHEIGHT)+2;

	script_MR[XFER_MENU]->commandKey[0] = 'R';

	/* Misc */

	script_MR[SMISC_MENU]->width = MENUWIDTH;
	script_MR[SMISC_MENU]->height = (5*MHEIGHT)+2;

	script_MR[SMISC_MENU]->commandKey[1] = 'B';
	script_MR[SMISC_MENU]->commandKey[3] = 'M';

	/* Screen */

	CopyMem(&page_MR[SCREEN_MENU]->menuBM, &script_MR[SCREEN_MENU]->menuBM, sizeof(struct BitMap));
	CopyMem(&page_MR[SCREEN_MENU]->menuRP, &script_MR[SCREEN_MENU]->menuRP, sizeof(struct RastPort));

	script_MR[SCREEN_MENU]->width = page_MR[SCREEN_MENU]->width;
	script_MR[SCREEN_MENU]->height = page_MR[SCREEN_MENU]->height;

	script_MR[SCREEN_MENU]->commandKey[0] = page_MR[SCREEN_MENU]->commandKey[0];
	script_MR[SCREEN_MENU]->commandKey[1] = page_MR[SCREEN_MENU]->commandKey[1];
	script_MR[SCREEN_MENU]->commandKey[2] = page_MR[SCREEN_MENU]->commandKey[2];

	script_MR[SCREEN_MENU]->title[0] = page_MR[SCREEN_MENU]->title[0];
	script_MR[SCREEN_MENU]->title[1] = page_MR[SCREEN_MENU]->title[1];
	script_MR[SCREEN_MENU]->title[2] = page_MR[SCREEN_MENU]->title[2];

	/**** create script menus ****/

	InitializeMenu(script_MR[FILE_MENU], smallFont);
	InitializeMenu(script_MR[EDIT_MENU], smallFont);
	InitializeMenu(script_MR[XFER_MENU], smallFont);
	InitializeMenu(script_MR[SMISC_MENU], smallFont);

	SetMenuTitles();

	/**** initialize toggle menus ****/
	/**** (leaves a space in front of menu items) ****/

	ToggleChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	ToggleChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);

	// START - NEW NEW NEW NEW

	fast_script_MR.width = FAST_MENU_WIDTH;
	fast_script_MR.height = FAST_MENU_HEIGHT;

	fast_page_MR.width = FAST_MENU_WIDTH;
	fast_page_MR.height = FAST_MENU_HEIGHT;

	InitializeMenu(&fast_script_MR, smallFont);
	InitializeMenu(&fast_page_MR, smallFont);

	// END - NEW NEW NEW NEW
}

/******** SetMenuTitles() ********/

void SetMenuTitles(void)
{
	page_MR[DA_MENU]->title[0]			= msgs[Msg_Menu_About-1];

	page_MR[FILE_MENU]->title[0]		= msgs[Msg_Menu_File_New-1];
	page_MR[FILE_MENU]->title[1]		= msgs[Msg_Menu_File_Open-1];
	page_MR[FILE_MENU]->title[2]		= msgs[Msg_Menu_File_Close-1];
	page_MR[FILE_MENU]->title[3]		= msgs[Msg_Menu_File_Save-1];
	page_MR[FILE_MENU]->title[4]		= msgs[Msg_Menu_File_SaveAs-1];
	page_MR[FILE_MENU]->title[5]		= msgs[Msg_Menu_File_PageSetUp-1];
	page_MR[FILE_MENU]->title[6]		= msgs[Msg_Menu_File_Print-1];
	page_MR[FILE_MENU]->title[7]		= msgs[Msg_Menu_File_Quit-1];

	page_MR[EDIT_MENU]->title[0]		= msgs[Msg_Menu_Edit_Undo-1];
	page_MR[EDIT_MENU]->title[1]		= msgs[Msg_Menu_Edit_Cut-1];
	page_MR[EDIT_MENU]->title[2]		= msgs[Msg_Menu_Edit_Copy-1];
	page_MR[EDIT_MENU]->title[3]		= msgs[Msg_Menu_Edit_Paste-1];
	page_MR[EDIT_MENU]->title[4]		= msgs[Msg_Menu_Edit_Clear-1];
	page_MR[EDIT_MENU]->title[5]		= msgs[Msg_Menu_Edit_SelectAll-1];
	page_MR[EDIT_MENU]->title[6]		= msgs[Msg_Menu_Edit_Distribute-1];
	page_MR[EDIT_MENU]->title[7]		= msgs[Msg_Menu_Edit_Duplicate-1];

	page_MR[FONT_MENU]->title[0]		= msgs[Msg_Menu_Font_Type-1];
	page_MR[FONT_MENU]->title[1]		= msgs[Msg_Menu_Font_Style-1];
	page_MR[FONT_MENU]->title[2]		= msgs[Msg_Menu_Font_Color-1];
	page_MR[FONT_MENU]->title[3]		= msgs[Msg_Plain-1];
	page_MR[FONT_MENU]->title[4]		= msgs[Msg_Bold-1];
	page_MR[FONT_MENU]->title[5]		= msgs[Msg_Italic-1];
	page_MR[FONT_MENU]->title[6]		= msgs[Msg_Underline-1];

	page_MR[PMISC_MENU]->title[0]		= msgs[Msg_Menu_PMisc_Import-1];
	page_MR[PMISC_MENU]->title[1]		= msgs[Msg_Menu_PMisc_Define-1];
	page_MR[PMISC_MENU]->title[2]		= msgs[Msg_Menu_PMisc_Palette-1];
	page_MR[PMISC_MENU]->title[3]		= msgs[Msg_Menu_PMisc_ScreenSize-1];
	page_MR[PMISC_MENU]->title[4]		= msgs[Msg_Menu_PMisc_Link-1];
	page_MR[PMISC_MENU]->title[5]		= msgs[Msg_Menu_PMisc_Remap-1];
	page_MR[PMISC_MENU]->title[6]		= msgs[Msg_Menu_PMisc_Specials-1];
	page_MR[PMISC_MENU]->title[7]		= msgs[Msg_Menu_PMisc_Transitions-1];
	page_MR[PMISC_MENU]->title[8]		= msgs[Msg_Menu_SMisc_LocalEvents-1];

	page_MR[SCREEN_MENU]->title[0] 	= msgs[Msg_Menu_Page-1];
	page_MR[SCREEN_MENU]->title[1] 	= msgs[Msg_Menu_Script-1];
	page_MR[SCREEN_MENU]->title[2] 	= msgs[Msg_Menu_Prefs-1];

	script_MR[DA_MENU]->title[0]		= page_MR[DA_MENU]->title[0];

	script_MR[FILE_MENU]->title[0] 	= page_MR[FILE_MENU]->title[0];
	script_MR[FILE_MENU]->title[1] 	= page_MR[FILE_MENU]->title[1];
	script_MR[FILE_MENU]->title[2] 	= page_MR[FILE_MENU]->title[2];
	script_MR[FILE_MENU]->title[3] 	= page_MR[FILE_MENU]->title[3];
	script_MR[FILE_MENU]->title[4] 	= page_MR[FILE_MENU]->title[4];
	script_MR[FILE_MENU]->title[5] 	= page_MR[FILE_MENU]->title[5];
	script_MR[FILE_MENU]->title[6] 	= page_MR[FILE_MENU]->title[6];
	script_MR[FILE_MENU]->title[7] 	= page_MR[FILE_MENU]->title[7];

	script_MR[EDIT_MENU]->title[0] 	= page_MR[EDIT_MENU]->title[0];
	script_MR[EDIT_MENU]->title[1] 	= page_MR[EDIT_MENU]->title[1];
	script_MR[EDIT_MENU]->title[2] 	= page_MR[EDIT_MENU]->title[2];
	script_MR[EDIT_MENU]->title[3] 	= page_MR[EDIT_MENU]->title[3];
	script_MR[EDIT_MENU]->title[4]	= page_MR[EDIT_MENU]->title[4];
	script_MR[EDIT_MENU]->title[5] 	= page_MR[EDIT_MENU]->title[5];

	script_MR[XFER_MENU]->title[0] 	= msgs[Msg_Menu_Xfer_Upload-1];
	//script_MR[XFER_MENU]->title[1] 	= msgs[Msg_Menu_Xfer_Download-1];

	script_MR[SMISC_MENU]->title[0]	= msgs[Msg_Menu_SMisc_ShowProg-1];
	script_MR[SMISC_MENU]->title[1]	= msgs[Msg_Menu_SMisc_LocalEvents-1];
	script_MR[SMISC_MENU]->title[2]	= msgs[Msg_TC_Title-1];
	script_MR[SMISC_MENU]->title[3]	= (UBYTE *)NAME_SCRIPTMANAGER;
	script_MR[SMISC_MENU]->title[4]	= msgs[Msg_Menu_SMisc_VarPath-1];

	script_MR[SCREEN_MENU]->title[0]= page_MR[SCREEN_MENU]->title[0];
	script_MR[SCREEN_MENU]->title[1]= page_MR[SCREEN_MENU]->title[1];
	script_MR[SCREEN_MENU]->title[2]= page_MR[SCREEN_MENU]->title[2];

	// START - NEW NEW NEW NEW

	fast_script_MR.title[DA_MENU]			= msgs[Msg_Menu_DA-1];
	fast_script_MR.title[FILE_MENU]		= Icon_Project;
	fast_script_MR.title[EDIT_MENU]		= Icon_Edit;
	fast_script_MR.title[XFER_MENU]		= Icon_Xfer;
	fast_script_MR.title[SMISC_MENU]	= Icon_Misc;
	fast_script_MR.title[SCREEN_MENU]	= msgs[Msg_Menu_Screen-1];

	fast_page_MR.title[DA_MENU]				= msgs[Msg_Menu_DA-1];
	fast_page_MR.title[FILE_MENU]			= Icon_Project;
	fast_page_MR.title[EDIT_MENU]			= Icon_Edit;
	fast_page_MR.title[FONT_MENU]			= Icon_Text;
	fast_page_MR.title[PMISC_MENU]		= Icon_Misc;
	fast_page_MR.title[SCREEN_MENU]		= msgs[Msg_Menu_Screen-1];

	// END - NEW NEW NEW NEW
}

/******** CalculateMenuParams() ********/

void CalculateMenuParams(void)
{
int interLen, i;
TEXT pmenu_spaces[10];
TEXT smenu_spaces[10];

	/**** create string for space between menu items ****/

	if ( CPrefs.PageScreenWidth < 640 )
		stccpy(pmenu_spaces, (STRPTR)LORES_INTERWIDTHTEXT, 10);
	else
		stccpy(pmenu_spaces, (STRPTR)HIRES_INTERWIDTHTEXT, 10);
	stccpy(smenu_spaces, (STRPTR)HIRES_INTERWIDTHTEXT, 10);

	/**** to calculate with the right font size, take a harmless little ****/
	/**** rastport like gfxRP, give it the small font and use that to		****/
	/**** calculate font widths etc.																		****/

	SetFont(&gfxRP, smallFont);

	/**** init page menus ****/

	interLen = TextLength(&gfxRP, pmenu_spaces, strlen(pmenu_spaces));

	/* DA */

	page_MR[DA_MENU]->titleX1		= 0;
	page_MR[DA_MENU]->titleX2		= page_MR[DA_MENU]->titleX1 +
																						TextLength(&gfxRP, msgs[Msg_Menu_DA-1], 1);
	page_MR[DA_MENU]->x					= page_MR[DA_MENU]->titleX1;
	page_MR[DA_MENU]->y 				= MHEIGHT;

	/* File */

	page_MR[FILE_MENU]->titleX1 	= page_MR[DA_MENU]->titleX2 + interLen;
	page_MR[FILE_MENU]->titleX2		= page_MR[FILE_MENU]->titleX1 +
							TextLength(&gfxRP, msgs[Msg_Menu_File-1], strlen(msgs[Msg_Menu_File-1]));
	page_MR[FILE_MENU]->x 				= page_MR[FILE_MENU]->titleX1;
	page_MR[FILE_MENU]->y 				= page_MR[DA_MENU]->y;

	/* Edit */

	page_MR[EDIT_MENU]->titleX1 	= page_MR[FILE_MENU]->titleX2 + interLen;
	page_MR[EDIT_MENU]->titleX2 	= page_MR[EDIT_MENU]->titleX1 +
								TextLength(&gfxRP, msgs[Msg_Menu_Edit-1], strlen(msgs[Msg_Menu_Edit-1]));
	page_MR[EDIT_MENU]->x 				= page_MR[EDIT_MENU]->titleX1;
	page_MR[EDIT_MENU]->y 				= page_MR[DA_MENU]->y;

	/* Font */

	page_MR[FONT_MENU]->titleX1 	= page_MR[EDIT_MENU]->titleX2 + interLen;
	page_MR[FONT_MENU]->titleX2 	= page_MR[FONT_MENU]->titleX1 +
													TextLength(&gfxRP, msgs[Msg_Menu_Font-1], strlen(msgs[Msg_Menu_Font-1]));
	page_MR[FONT_MENU]->x 				= page_MR[FONT_MENU]->titleX1;
	page_MR[FONT_MENU]->y 				= page_MR[DA_MENU]->y;

	/* Misc */

	page_MR[PMISC_MENU]->titleX1 	= page_MR[FONT_MENU]->titleX2 + interLen;
	page_MR[PMISC_MENU]->titleX2 	= page_MR[PMISC_MENU]->titleX1 +
													TextLength(&gfxRP, msgs[Msg_Menu_PMisc-1], strlen(msgs[Msg_Menu_PMisc-1]));
	page_MR[PMISC_MENU]->x 				= page_MR[PMISC_MENU]->titleX1;
	page_MR[PMISC_MENU]->y 				= page_MR[DA_MENU]->y;

	/* Screen */

	page_MR[SCREEN_MENU]->titleX1 	= page_MR[PMISC_MENU]->titleX2 + interLen;
	page_MR[SCREEN_MENU]->titleX2 	= page_MR[SCREEN_MENU]->titleX1 +
													TextLength(&gfxRP, msgs[Msg_Menu_Screen-1], strlen(msgs[Msg_Menu_Screen-1]));
	page_MR[SCREEN_MENU]->x 				= page_MR[SCREEN_MENU]->titleX1;
	page_MR[SCREEN_MENU]->y 				= page_MR[DA_MENU]->y;

	if ( CPrefs.PageScreenWidth < 640 )
	{
		page_MR[FONT_MENU]->x = CPrefs.PageScreenWidth - page_MR[FONT_MENU]->width - 1;
		page_MR[PMISC_MENU]->x = CPrefs.PageScreenWidth - page_MR[PMISC_MENU]->width - 1;
		page_MR[SCREEN_MENU]->x = CPrefs.PageScreenWidth - page_MR[SCREEN_MENU]->width - 1;
	}

	/**** init script menus ****/

	interLen = TextLength(&gfxRP, smenu_spaces, strlen(smenu_spaces));

	/* DA */

	script_MR[DA_MENU]->titleX1	= 0;
	script_MR[DA_MENU]->titleX2	= script_MR[DA_MENU]->titleX1 +
																					TextLength(&gfxRP, msgs[Msg_Menu_DA-1], 1);
	script_MR[DA_MENU]->x				= script_MR[DA_MENU]->titleX1;
	script_MR[DA_MENU]->y				= page_MR[DA_MENU]->y;

	/* File */

	script_MR[FILE_MENU]->titleX1	= script_MR[DA_MENU]->titleX2 + interLen;
	script_MR[FILE_MENU]->titleX2	= script_MR[FILE_MENU]->titleX1 +
								TextLength(&gfxRP, msgs[Msg_Menu_File-1], strlen(msgs[Msg_Menu_File-1]));
	script_MR[FILE_MENU]->x				= script_MR[FILE_MENU]->titleX1;
	script_MR[FILE_MENU]->y				= page_MR[FILE_MENU]->y;

	/* Edit */

	script_MR[EDIT_MENU]->titleX1	= script_MR[FILE_MENU]->titleX2 + interLen;
	script_MR[EDIT_MENU]->titleX2	= script_MR[EDIT_MENU]->titleX1 +
								TextLength(&gfxRP, msgs[Msg_Menu_Edit-1], strlen(msgs[Msg_Menu_Edit-1]));
	script_MR[EDIT_MENU]->x				= script_MR[EDIT_MENU]->titleX1;
	script_MR[EDIT_MENU]->y				= page_MR[EDIT_MENU]->y;

	/* Xfer */

	script_MR[XFER_MENU]->titleX1	= script_MR[EDIT_MENU]->titleX2 + interLen;
	script_MR[XFER_MENU]->titleX2	= script_MR[XFER_MENU]->titleX1 +
								TextLength(&gfxRP, msgs[Msg_Menu_Xfer-1], strlen(msgs[Msg_Menu_Xfer-1]));
	script_MR[XFER_MENU]->x				= script_MR[XFER_MENU]->titleX1;
	script_MR[XFER_MENU]->y				= script_MR[DA_MENU]->y;

	/* Misc */

	script_MR[SMISC_MENU]->titleX1	= script_MR[XFER_MENU]->titleX2 + interLen;
	script_MR[SMISC_MENU]->titleX2	= script_MR[SMISC_MENU]->titleX1 +
							TextLength(&gfxRP, msgs[Msg_Menu_SMisc-1], strlen(msgs[Msg_Menu_SMisc-1]));
	script_MR[SMISC_MENU]->x				= script_MR[SMISC_MENU]->titleX1;
	script_MR[SMISC_MENU]->y				= script_MR[DA_MENU]->y;

	/* Screen */

	script_MR[SCREEN_MENU]->titleX1	= script_MR[SMISC_MENU]->titleX2 + interLen;
	script_MR[SCREEN_MENU]->titleX2	= script_MR[SCREEN_MENU]->titleX1 +
						TextLength(&gfxRP, msgs[Msg_Menu_Screen-1], strlen(msgs[Msg_Menu_Screen-1]));
	script_MR[SCREEN_MENU]->x				= script_MR[SCREEN_MENU]->titleX1;
	script_MR[SCREEN_MENU]->y				= page_MR[SCREEN_MENU]->y;

	/**** modify title x1 and x2 coordinates ****/

	if ( CPrefs.PageScreenWidth < 640 )
	{
		for (i=0; i<NUMMENUS; i++)
		{
			page_MR[i]->x					+= 1;
			page_MR[i]->titleX1		+= 2;
			page_MR[i]->titleX2		+= 3;
		}
	}
	else
	{
		for (i=0; i<NUMMENUS; i++)
		{
			page_MR[i]->x					+= 3;
			page_MR[i]->titleX1		+= 4;
			page_MR[i]->titleX2		+= 7;
		}
	}

	for (i=0; i<NUMMENUS; i++)
	{
		script_MR[i]->x				+= 3;
		script_MR[i]->titleX1	+= 4;
		script_MR[i]->titleX2	+= 7;
	}
}

/******** CloseMenus() ********/

void CloseMenus(void)
{
int i;

	if ( page_MR[DA_MENU] == NULL )
		return;	// stuff not yet allocated!

	/**** reset menu heights as altered by prefs ****/

	page_MR[DA_MENU]->height 			= (11*MHEIGHT)+2;
	page_MR[FILE_MENU]->height 		= ( 8*MHEIGHT)+2;
	page_MR[EDIT_MENU]->height 		= ( 8*MHEIGHT)+2;
	page_MR[FONT_MENU]->height		= ( 7*MHEIGHT)+2;
	page_MR[PMISC_MENU]->height		= ( 9*MHEIGHT)+2;
	page_MR[SCREEN_MENU]->height	= (11*MHEIGHT)+2;

	script_MR[FILE_MENU]->height 	= ( 8*MHEIGHT)+2;
	script_MR[EDIT_MENU]->height 	= ( 6*MHEIGHT)+2;
	script_MR[XFER_MENU]->height	= ( 1*MHEIGHT)+2;
	script_MR[SMISC_MENU]->height	= ( 5*MHEIGHT)+2;

	for(i=0; i<NUMMENUS; i++)
		FreeMenu(page_MR[i]);

	FreeMenu(script_MR[FILE_MENU]);
	FreeMenu(script_MR[EDIT_MENU]);
	FreeMenu(script_MR[XFER_MENU]);
	FreeMenu(script_MR[SMISC_MENU]);

	// START - NEW NEW NEW NEW

	FreeMenu(&fast_script_MR);
	FreeMenu(&fast_page_MR);

	// END - NEW NEW NEW NEW
}

/******** Fill_DA_Menu() ********/

void Fill_DA_Menu(BOOL skipUntitled)
{
struct Screen *screen;
int i,j,left;
TEXT str[40];

	for(i=0; i<10; i++)
		DA_Screens[i] = NULL;

	for(i=0; i<NUMRUNNING; i++)
		page_MR[SCREEN_MENU]->title[i+3] = NULL;

	Forbid();

	screen = IntuitionBase->FirstScreen;
	i=0;
	j=0;
	left=NUMRUNNING;
	while(i<10)	/* look for max. of 10 screen */
	{
		if ( screen->Title!=NULL && screen->Height >= 150 &&
				 (!skipUntitled || strcmp((UBYTE *)special_char,(UBYTE *)screen->Title)) )
		{
			if (j<8)
			{
				stccpy(str, screen->Title, 28);
				UA_ShortenString(&page_MR[SCREEN_MENU]->menuRP, str, page_MR[SCREEN_MENU]->width-20);
				page_MR[SCREEN_MENU]->title[j+3] = &str[0];
				EnableMenu(page_MR[SCREEN_MENU], j+3);	/* clears line and displays text */
				left--;
			}

			DA_Screens[j] = screen;

			j++;
		}
		else if (	!screen->Title && screen->Height >= 150 )	// screen has no ptr to a title
		{
			if (j<8)
			{
				stccpy(str, msgs[Msg_Untitled-1], 40);
				page_MR[SCREEN_MENU]->title[j+3] = &str[0];
				EnableMenu(page_MR[SCREEN_MENU], j+3);	/* clears line and displays text */
				left--;
			}

			DA_Screens[j] = screen;

			j++;
		}

		screen = screen->NextScreen;
		if (screen==NULL)
			break;
		else
			i++;
	}

	Permit();

	for(i=11-left; i<3+NUMRUNNING; i++)
		ClearMenuLine(page_MR[SCREEN_MENU], i);
}

/******** Fill_MRO_Menu() ********/

void Fill_MRO_Menu(BYTE where)
{
int i;
TEXT str[SIZE_FULLPATH];
TEXT path[SIZE_FULLPATH];
TEXT filename[SIZE_FILENAME];

	for(i=0; i<5; i++)
		page_MR[DA_MENU]->title[i+6] = NULL;

	for(i=0; i<5; i++)
	{
		if ( where==STARTSCREEN_SCRIPT )
			stccpy(str, &MRO_Script[i*SIZE_FULLPATH], SIZE_FULLPATH-1);
		else
			stccpy(str, &MRO_Page[i*SIZE_FULLPATH], SIZE_FULLPATH-1);

		if ( str[0] )
		{
			UA_SplitFullPath(str, path, filename);
			UA_ShortenStringFront(&page_MR[DA_MENU]->menuRP, filename, page_MR[DA_MENU]->width-20);
			page_MR[DA_MENU]->title[i+6] = &filename[0];
			EnableMenu(page_MR[DA_MENU], i+6);	// clears line and displays text
		}
		else
			ClearMenuLine(page_MR[DA_MENU], i+6);
	}
}

/******** InitPageEditMenus() ********/

void InitPageEditMenus(void)
{
int i;

	DisableMenu(page_MR[FILE_MENU], FILE_CLOSE);
	DisableMenu(page_MR[FILE_MENU], FILE_SAVE);
	DisableMenu(page_MR[FILE_MENU], FILE_SAVEAS);
	DisableMenu(page_MR[FILE_MENU], FILE_PAGESETUP);
	DisableMenu(page_MR[FILE_MENU], FILE_PRINT);

	for(i=EDIT_UNDO; i<=EDIT_DUPLI; i++)
		DisableMenu(page_MR[EDIT_MENU],i);

	for(i=FONT_TYPE; i<=FONT_UNDERLINE; i++)
		DisableMenu(page_MR[FONT_MENU],i);

	for(i=PMISC_IMPORT; i<=PMISC_INTERACTIVE; i++)
		DisableMenu(page_MR[PMISC_MENU],i);
}

/******** InitScriptEditMenus() ********/

void InitScriptEditMenus(void)
{
int i;

	DisableMenu(script_MR[FILE_MENU], FILE_CLOSE);
	DisableMenu(script_MR[FILE_MENU], FILE_SAVE);
	DisableMenu(script_MR[FILE_MENU], FILE_SAVEAS);
	DisableMenu(script_MR[FILE_MENU], FILE_PAGESETUP);
	DisableMenu(script_MR[FILE_MENU], FILE_PRINT);

	for(i=EDIT_UNDO; i<=EDIT_SELECTALL; i++)
		DisableMenu(script_MR[EDIT_MENU],i);

	DisableMenu(script_MR[XFER_MENU], XFER_UPLOAD);
	//DisableMenu(script_MR[XFER_MENU], XFER_DOWNLOAD);

	DisableMenu(script_MR[SMISC_MENU], SMISC_SHOWPROG);
	DisableMenu(script_MR[SMISC_MENU], SMISC_LOCALEVENTS);
	DisableMenu(script_MR[SMISC_MENU], SMISC_VARPATH);
}

/******** ReRenderMenus() ********/

void ReRenderMenus(void)
{
int i;

	RenderMenuInterior(page_MR[DA_MENU]);
	RenderMenuInterior(page_MR[FILE_MENU]);
	RenderMenuInterior(page_MR[EDIT_MENU]);
	RenderMenuInterior(page_MR[FONT_MENU]);
	RenderMenuInterior(page_MR[PMISC_MENU]);
	RenderMenuInterior(page_MR[SCREEN_MENU]);

	RenderMenuInterior(script_MR[FILE_MENU]);
	RenderMenuInterior(script_MR[EDIT_MENU]);
	RenderMenuInterior(script_MR[XFER_MENU]);
	RenderMenuInterior(script_MR[SMISC_MENU]);

	for(i=0; i<11; i++) if ( page_MR[DA_MENU]->disabled[i] ) DisableMenu(page_MR[DA_MENU], i);
	for(i=0; i< 8; i++) if ( page_MR[FILE_MENU]->disabled[i] ) DisableMenu(page_MR[FILE_MENU], i);
	for(i=0; i< 8; i++) if ( page_MR[EDIT_MENU]->disabled[i] ) DisableMenu(page_MR[EDIT_MENU], i);
	for(i=0; i< 7; i++) if ( page_MR[FONT_MENU]->disabled[i] ) DisableMenu(page_MR[FONT_MENU], i);
	for(i=0; i< 9; i++) if ( page_MR[PMISC_MENU]->disabled[i] ) DisableMenu(page_MR[PMISC_MENU], i);

	for(i=0; i< 8; i++) if ( script_MR[FILE_MENU]->disabled[i] ) DisableMenu(script_MR[FILE_MENU], i);
	for(i=0; i< 6; i++) if ( script_MR[EDIT_MENU]->disabled[i] ) DisableMenu(script_MR[EDIT_MENU], i);
	for(i=0; i< 1; i++) if ( script_MR[XFER_MENU]->disabled[i] ) DisableMenu(script_MR[XFER_MENU], i);
	for(i=0; i< 5; i++) if ( script_MR[SMISC_MENU]->disabled[i] ) DisableMenu(script_MR[SMISC_MENU], i);

	if (EHI.activeScreen == STARTSCREEN_PAGE)
	{
		DisableMenu(page_MR[SCREEN_MENU], SCREEN_PAGE);
		EnableMenu(page_MR[SCREEN_MENU], SCREEN_SCRIPT);
	}
	else
	{
		EnableMenu(page_MR[SCREEN_MENU], SCREEN_PAGE);
		DisableMenu(page_MR[SCREEN_MENU], SCREEN_SCRIPT);
	}

	/**** page menu lines ****/

	if ( CPrefs.userLevel != 1 )
	{
		DrawMenuLine(page_MR[DA_MENU], 0);
		DrawMenuLine(page_MR[DA_MENU], 5);
	}

	DrawMenuLine(page_MR[FILE_MENU], 1);
	DrawMenuLine(page_MR[FILE_MENU], 4);
	DrawMenuLine(page_MR[FILE_MENU], 6);

	DrawMenuLine(page_MR[EDIT_MENU], 0);
	DrawMenuLine(page_MR[EDIT_MENU], 4);
	if ( CPrefs.userLevel != 1 )
		DrawMenuLine(page_MR[EDIT_MENU], 5);

	if ( CPrefs.userLevel != 1 )
		DrawMenuLine(page_MR[FONT_MENU], 2);

	DrawMenuLine(page_MR[SCREEN_MENU], 1);
	if ( CPrefs.userLevel != 1 )
		DrawMenuLine(page_MR[SCREEN_MENU], 2);

	/**** script menu lines ****/

	DrawMenuLine(script_MR[FILE_MENU], 1);
	DrawMenuLine(script_MR[FILE_MENU], 4);
	DrawMenuLine(script_MR[FILE_MENU], 6);

	DrawMenuLine(script_MR[EDIT_MENU], 0);
	DrawMenuLine(script_MR[EDIT_MENU], 4);

	DrawMenuLine(script_MR[SMISC_MENU], 1);

	// START - NEW NEW NEW NEW

	RenderMenuInterior(&fast_script_MR);
	RenderMenuInterior(&fast_page_MR);

	// END - NEW NEW NEW NEW
}

/******** E O F ********/
