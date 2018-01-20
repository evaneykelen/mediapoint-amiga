/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct TextFont *systemFont;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern struct TextFont *tiny_smallFont;
extern struct TextFont *tiny_largeFont;
extern struct List *fontList;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** globals ****/

STATIC TEXT fontpath[SIZE_FULLPATH];

/**** functions ****/

/******** OpenAppFonts() ********/
/*
 * Opens topaz/8, medialink/10 and medialink/20 fonts.
 *
 */

BOOL OpenAppFonts(STRPTR path)
{
struct TextAttr textAttr;

	/**** open system font ****/

	textAttr.ta_Name = (UBYTE *)"topaz.font";
	textAttr.ta_YSize = 8;
	textAttr.ta_Style = FS_NORMAL;
	textAttr.ta_Flags = NULL;

	systemFont = OpenFont(&textAttr);
	if (systemFont==NULL)
	{
		UA_WarnUser(60);
		return(FALSE);
	}

	/**** open small font ****/

	sprintf(fontpath, "%s%s", path, APPFONT);

	textAttr.ta_Name = (UBYTE *)fontpath;
	textAttr.ta_YSize = 10;
	textAttr.ta_Style = FS_NORMAL;
	textAttr.ta_Flags = FPF_DESIGNED;

	smallFont = OpenDiskFont(&textAttr);
	if (smallFont==NULL)
	{
		textAttr.ta_Name = (UBYTE *)APPFONT;
		textAttr.ta_YSize = 10;
		textAttr.ta_Style = FS_NORMAL;
		textAttr.ta_Flags = FPF_DESIGNED;
		smallFont = OpenDiskFont(&textAttr);
		if (smallFont==NULL)
		{
			UA_WarnUser(61);
			smallFont = systemFont;
		}
	}

	/**** open large font ****/

	textAttr.ta_Name = (UBYTE *)fontpath;
	textAttr.ta_YSize = 20;
	textAttr.ta_Style = FS_NORMAL;
	textAttr.ta_Flags = FPF_DESIGNED;

	largeFont = OpenDiskFont(&textAttr);
	if (largeFont==NULL)
	{
		textAttr.ta_Name = (UBYTE *)APPFONT;
		textAttr.ta_YSize = 20;
		textAttr.ta_Style = FS_NORMAL;
		textAttr.ta_Flags = FPF_DESIGNED;
		largeFont = OpenDiskFont(&textAttr);
		if (largeFont==NULL)
		{
			UA_WarnUser(62);
			largeFont = systemFont;
		}
	}

	textFont = largeFont;	// make sample font same as large font

	/**** open tiny small font ****/

	sprintf(fontpath, "%s%s", path, APPFONT);

	textAttr.ta_Name = (UBYTE *)fontpath;
	textAttr.ta_YSize = 9;
	textAttr.ta_Style = FS_NORMAL;
	textAttr.ta_Flags = FPF_DESIGNED;

	tiny_smallFont = OpenDiskFont(&textAttr);
	if (tiny_smallFont==NULL)
		tiny_smallFont = smallFont;

	/**** open tiny large font ****/

	sprintf(fontpath, "%s%s", path, APPFONT);

	textAttr.ta_Name = (UBYTE *)fontpath;
	textAttr.ta_YSize = 18;
	textAttr.ta_Style = FS_NORMAL;
	textAttr.ta_Flags = FPF_DESIGNED;

	tiny_largeFont = OpenDiskFont(&textAttr);
	if (tiny_largeFont==NULL)
		tiny_largeFont = largeFont;

	return(TRUE);
}

#ifndef USED_FOR_PLAYER

/******** OpenTypeFace() ********/
/*
 * 'name' is e.g. MediaLink.font.
 * global 'textFont' is used to store ptr to font description.
 *
 */

BOOL OpenTypeFace(STRPTR name, int size, UBYTE flags, BOOL messageAllowed)
{
struct TextAttr textAttr;
struct FontListRecord *FLR;
TEXT fname[50];

	stccpy(fname, name, 50);
	strlwr(fname);

	for(FLR=(struct FontListRecord *)fontList->lh_Head;
			FLR->node.ln_Succ;
			FLR=(struct FontListRecord *)FLR->node.ln_Succ)
	{
		if ( !strcmp(fname, FLR->fontName) && FLR->fontSize==size ) // found
		{
			textFont = FLR->ptr;
			return(TRUE);
		}
	}

	/**** load new font ****/

	//ChangeSpriteImage(SPRITE_BUSY);

	textAttr.ta_Name	= (UBYTE *)fname;
	textAttr.ta_YSize	= size;
	textAttr.ta_Style	= FS_NORMAL;
	textAttr.ta_Flags	= NULL;

	if ( strcmp(fname, "topaz.font")==0 && (size==8 || size==9) )
		textAttr.ta_Flags	|= FPF_ROMFONT;

	textFont = (struct TextFont *)OpenDiskFont(&textAttr);
	if (textFont==NULL)
	{
		if ( messageAllowed )
			Message(msgs[Msg_FontNotFound-1], fname);
		textFont = largeFont; //systemFont;
	}

	/**** add font to font-list ****/

	if ( !AddFontToList(fname, size, textFont, fontList) )
		return(FALSE);

	//ChangeSpriteImage(SPRITE_NORMAL);

	return(TRUE);
}

/******** InitFontList() ********/

BOOL InitFontList(void)
{
	fontList = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
	if (fontList==NULL)
		return(FALSE);

	NewList(fontList);

	if ( !AddFontToList("topaz.font", 8, systemFont, fontList) )
		return(FALSE);

	if ( !AddFontToList(APPFONT, 10, smallFont, fontList) )
		return(FALSE);

	if ( !AddFontToList(APPFONT, 20, largeFont, fontList) )
		return(FALSE);

	if ( !AddFontToList(APPFONT, 9, tiny_smallFont, fontList) )
		return(FALSE);

	if ( !AddFontToList(APPFONT, 18, tiny_largeFont, fontList) )
		return(FALSE);

	return(TRUE);
}

/******** FreeFontList() ********/

void FreeFontList(void)
{
struct FontListRecord *work_FLR, *next_FLR;

	if ( fontList==NULL )	// not yet allocated
		return;

	work_FLR = (struct FontListRecord *)(fontList->lh_Head);	// first node
	while(next_FLR = (struct FontListRecord *)(work_FLR->node.ln_Succ))
	{
		if (work_FLR->ptr != NULL)
		{
			CloseFont(work_FLR->ptr);
		}
		FreeMem(work_FLR, sizeof(struct FontListRecord));
		work_FLR = next_FLR;
	}

	FreeMem(fontList, sizeof(struct List));
}

/******** AddFontToList() ********/

BOOL AddFontToList(	STRPTR fontName, int fontSize, struct TextFont *ptr,
										struct List *fontList)
{
struct FontListRecord *FLR;

	FLR = (struct FontListRecord *)AllocMem(sizeof(struct FontListRecord),
																					MEMF_ANY | MEMF_CLEAR);
	if (FLR==NULL)
		return(FALSE);

	stccpy(FLR->fontName, fontName, 50);
	FLR->fontSize = fontSize;
	FLR->ptr = ptr;
	AddTail(fontList, (struct Node *)FLR);

	return(TRUE);
}

#endif

/******** E O F ********/
