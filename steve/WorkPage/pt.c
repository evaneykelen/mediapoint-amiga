#include <exec/exec.h>
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <libraries/diskfont.h>
#include <string.h>
#include <stdio.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/graphics.h>

#include "nb:capsdefines.h"
#include "nb:newdefines.h"

#include "pascal:include/toolslib.h"
#include "pascal:include/txed.h"
#include "pascal:include/txedstyles.h"
#include "pascal:include/txedtools.h"
#include "pascal:include/colsearch.h"
#include "pascal:include/scarem.h"
#include "nb:parser.h"
#include "nb:capsstructs.h"

/**** defines ****/

#define RAWBUFSIZE 2000

/**** function declarations ****/

struct List *WP_InitFontList(void);
void WP_FreeFontList(struct List *list);
struct TextFont *WP_OpenTypeFace(struct List *list, STRPTR name, int size);
BOOL WP_AddFontToList(STRPTR fontName, int fontSize, struct TextFont *ptr,
											struct List *list);
BOOL WP_ParseText(struct List *list, UBYTE *buffer, struct EditWindow *ew);
void WP_FetchString(STRPTR buffer, STRPTR dest, int max, int *count);
void WP_FetchInteger(STRPTR buffer, int *dest, int *count);

/******** WP_InitFontList() ********/
/* 
 * Allocates memory for list header, returns pointer to it or NULL on error.
 *
 */

struct List *WP_InitFontList(void)
{
struct List *list;
struct TextFont *textFont;

	list = (struct List *)AllocMem(sizeof(struct List),MEMF_CLEAR);
	if ( list!=NULL )
	{
		NewList(list);
		/**** by default add topaz to the list for fall back purposes ****/
		textFont = WP_OpenTypeFace(list, "topaz.font", 8);
		if ( textFont!=NULL )
			WP_AddFontToList("topaz.font", 8, textFont, list);
	}

	return(list);
}

/******** WP_FreeFontList() ********/
/*
 * Frees all fonts, list nodes and list header
 *
 */

void WP_FreeFontList(struct List *list)
{
struct FontListRecord *work_FLR, *next_FLR;

	if ( list==NULL )	// you don't fool me...
		return;

	work_FLR = (struct FontListRecord *)(list->lh_Head);	// first node
	while( next_FLR = (struct FontListRecord *)(work_FLR->node.ln_Succ) )
	{
		if ( work_FLR->ptr != NULL )
			CloseFont(work_FLR->ptr);
		FreeMem(work_FLR, sizeof(struct FontListRecord));
		work_FLR = next_FLR;
	}

	FreeMem(list, sizeof(struct List));
}

/******** WP_OpenTypeFace() ********/
/*
 * 'name' is e.g. MediaLink.font.
 *
 */

struct TextFont *WP_OpenTypeFace(struct List *list, STRPTR name, int size)
{
struct TextAttr textAttr;
struct FontListRecord *FLR;
TEXT fname[50];
struct TextFont *textFont;

	stccpy(fname, name, 50);
	strlwr(fname);	// convert string to lowercase

	/**** first try to find font in our font list ****/

	for(FLR=(struct FontListRecord *)list->lh_Head;
			FLR->node.ln_Succ;
			FLR=(struct FontListRecord *)FLR->node.ln_Succ)
	{
		if ( !strcmp(fname, FLR->fontName) && FLR->fontSize==size ) // found
			return(FLR->ptr);
	}

	/**** not in list, open new font ****/

	textAttr.ta_Name	= (UBYTE *)fname;
	textAttr.ta_YSize	= size;
	textAttr.ta_Style	= FS_NORMAL;
	textAttr.ta_Flags	= NULL;

	if ( strcmp(fname, "topaz.font")==0 && (size==8 || size==9) )
		textAttr.ta_Flags	|= FPF_ROMFONT;

	textFont = (struct TextFont *)OpenDiskFont(&textAttr);
	if (textFont==NULL)
	{
		FLR=(struct FontListRecord *)list->lh_Head;	// first list entry contains topaz
		return( FLR->ptr );
	}

	/**** add font to font list ****/

	if ( !WP_AddFontToList(fname, size, textFont, list) )
		return(NULL);

	return(textFont);
}

/******** WP_AddFontToList() ********/
/*
 * Allocate node for font list and copy font into in it
 *
 */

BOOL WP_AddFontToList(STRPTR fontName, int fontSize, struct TextFont *ptr,
											struct List *list)
{
struct FontListRecord *FLR;

	FLR = (struct FontListRecord *)AllocMem(sizeof(struct FontListRecord),MEMF_CLEAR);
	if (FLR==NULL)
		return(FALSE);

	stccpy(FLR->fontName, fontName, 50);
	FLR->fontSize = fontSize;
	FLR->ptr = ptr;
	AddTail(list, (struct Node *)FLR);

	return(TRUE);
}

/******** WP_ParseText() ********/

BOOL WP_ParseText(struct List *list, UBYTE *buffer, struct EditWindow *ew)
{
struct FontListRecord *FLR;
int i, fontsize, add, numChars, n;
LONG actualLen;
TEXT fontname[50];
struct TextFont *charFont;
UWORD	charStyle;
UBYTE	charColor;
UBYTE underlineColor;

	/**** set default values ****/

	FLR=(struct FontListRecord *)list->lh_Head;	// first list entry contains topaz

	charFont	= FLR->ptr;	// topaz
	charStyle = 0;
	charColor = 2;
	underlineColor = 2;

	/**** parse the raw buffer and create the text buffer ****/

	numChars=0;	// number of characters actually stored in text buffer
	actualLen = strlen(buffer);

	for(i=0; i<actualLen; i++)
	{
		if ( buffer[i]=='^' )						// start of command sequence
		{
			i++;
			if ( buffer[i]=='f' )					// font name follows
			{
				i++;
				WP_FetchString(&buffer[i], fontname, 50, &i);
				strcat(fontname, ".font");
			}
			else if ( buffer[i]=='s' )		// size follows
			{
				i++;
				WP_FetchInteger(&buffer[i], &fontsize, &i);
				charFont = WP_OpenTypeFace(list, fontname, fontsize);
				if ( charFont == NULL )
					charFont = FLR->ptr;
			}
			else if ( buffer[i]=='c' )		// color follows
			{
				i++;
				WP_FetchInteger(&buffer[i], &n, &i);
				charColor = n;
			}
			else if ( buffer[i]=='l' )		// f from lf follows
			{
				ew->TEI->text[numChars].charFont				= charFont;
				ew->TEI->text[numChars].charStyle				= charStyle;
				ew->TEI->text[numChars].charColor				= charColor;
				ew->TEI->text[numChars].underlineColor	= underlineColor;
				ew->TEI->text[numChars].charCode				= 0x0a;
				numChars++;

				i=i+2;
			}
			else if ( buffer[i]=='a' )		// attribute follows
			{
				i++;
				WP_FetchInteger(&buffer[i], &n, &i);
				charStyle = n;
			}
			else if ( buffer[i]=='u' )		// attribute follows
			{
				i++;
				WP_FetchInteger(&buffer[i], &n, &i);
				underlineColor = n;
			}
			else if ( buffer[i]==0x5c )		// backslash, linefeed
				i++;
		}
		else
		{
			if (	(buffer[i]>=0x20 && buffer[i]!=127) ||
						buffer[i]==0x0d || buffer[i]==0x08 ||
						buffer[i]==0x0c || buffer[i]==0x0a )
			{
				add=0;
				if( buffer[i]==0x0d )
				{
					if( buffer[ i + 1 ] != 0x0a )
						buffer[i]=0x0a;
					else
						i++;
				}
				else if ( buffer[i]=='\\' && buffer[i+1]=='\"' )
				{
					buffer[i]='\"';
					add=1;
				}
				ew->TEI->text[numChars].charFont				= charFont;
				ew->TEI->text[numChars].charStyle				= charStyle;
				ew->TEI->text[numChars].charColor				= charColor;
				ew->TEI->text[numChars].underlineColor	= underlineColor;
				ew->TEI->text[numChars].charCode				= buffer[i];
				numChars++;
				i=i+add;
			}
		}
	}

	ew->TEI->text[numChars].charCode = 0;

#if 0
	{
	char str[100];
	for(i=0; i<numChars; i++)
	{
		sprintf(str, "%x %d %d %x\n",
				ew->TEI->text[i].charFont,
				ew->TEI->text[i].charStyle,
				ew->TEI->text[i].charColor,
				ew->TEI->text[i].charCode );
		KPrintF(str);
	}
	}
#endif

	return(TRUE);
}

/******** WP_FetchString() ********/
/*
 * buffer points to start of parameter string
 * e.g. <fGaramond, buffer points to the G
 *
 */

void WP_FetchString(STRPTR buffer, STRPTR dest, int max, int *count)
{
int i;

	for(i=0; i<max; i++)
	{
		if ( buffer[i]=='^' )
		{
			dest[i]=0;
			return;
		}
		dest[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** WP_FetchInteger() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <s64, buffer points to the 6
 *
 */

void WP_FetchInteger(STRPTR buffer, int *dest, int *count)
{
int i;
TEXT str[10];

	for(i=0; i<10; i++)
	{
		if ( buffer[i]=='^' )
		{
			str[i]=0;
			sscanf(str, "%d", dest);
			return;
		}
		str[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** E O F ********/
