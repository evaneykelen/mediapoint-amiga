#include "nb:pre.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern ULONG allocFlags;
extern struct Window *pageWindow;
extern UBYTE SW_pens[];
extern struct FER FontEntryRecord;
extern UWORD chip gui_pattern[];
extern struct TextFont *largeFont;
extern struct TextFont *smallFont;
extern struct TextFont *textFont;
extern struct List *fontList;

/**** gadgets ****/

extern struct GadgetRecord FontSelect_GR[];

/**** functions ****/

/******** Monitor_FontSelection() ********/

BOOL Monitor_FontSelection(struct EditWindow *ew)
{
struct Window *fontWdw;
struct FontEntry *FE;
struct TextFont *newFont=NULL;
TEXT fontName[55];
int size;

	if ( !TestBit(allocFlags,FONTS_SCANNED_FLAG) )
	{
		if ( !UA_ScanFontsDir(&FontEntryRecord) )
		{
			UA_WarnUser(218);
			return(FALSE);
		}
		SetBit(&allocFlags, FONTS_SCANNED_FLAG);
	}

	if ( UA_IsWindowOnLacedScreen(pageWindow) )
		SetFont(pageWindow->RPort, largeFont);

	FillInFER(ew->TEI->newText.charFont);

	fontWdw = UA_OpenFontListWindow(pageWindow, &FontEntryRecord,
																	FontSelect_GR, gui_pattern);
	if (fontWdw==NULL)
	{
		UA_WarnUser(219);
		return(FALSE);
	}

	SetFont(pageWindow->RPort, smallFont);

	newFont =  UA_Monitor_FontSelection(fontWdw, &FontEntryRecord,
																			FontSelect_GR, gui_pattern);
	if (newFont!=NULL)
	{
		textFont = newFont;	// set the global ptr textFont to the new font ptr
		if ( FontEntryRecord.selected1 != -1 )
		{
			/**** find the selected font ****/
			FE = (struct FontEntry *)FontEntryRecord.FEList[ FontEntryRecord.selected1 ];
			if ( FontEntryRecord.selected2 != -1 )
				size = FE->fontSize[ FontEntryRecord.selected2 ];
			else if ( FontEntryRecord.fontSize != -1 )
				size = FontEntryRecord.fontSize;
			if (size==-1)
				size=FE->fontSize[0];
			/**** open font and add to font list ****/
			sprintf(fontName, "%s.font", &FE->fontName[1]);
			if (!AddFontToList(fontName, size, newFont, fontList))
				UA_WarnUser(222);
		}
	}

	UA_CloseFontListWindow(fontWdw);

	if (newFont==NULL)
		return(FALSE);

	return(TRUE);
}

/******** FillInFER() ********/

void FillInFER(struct TextFont *font)
{
int i,j,fontsize;
struct FontEntry *FE;
TEXT fontname[50];

	FontEntryRecord.selected1 = -1;
	FontEntryRecord.selected2 = -1;
	FontEntryRecord.fontSize = -1;

	if ( !findFont(font, fontname, &fontsize) )
		return;	// jammer maar helaas...

	FE = (struct FontEntry *)FontEntryRecord.FEList[ 0 ];

	for(i=0; i<FontEntryRecord.numEntries1; i++)
	{
		FE = (struct FontEntry *)FontEntryRecord.FEList[ i ];
		if ( !strcmpi( &FE->fontName[1], fontname ) )	// bijna beet?
		{
			FontEntryRecord.selected1 = i;
			for(j=0; j<30; j++)
			{
				if ( FE->fontSize[j]==fontsize )
				{
					FontEntryRecord.selected2 = j;
					goto go_on;
				}
			}
		}
	}

	if ( FontEntryRecord.selected1 != -1 )
		FontEntryRecord.fontSize = fontsize;

go_on:

	if ( FontEntryRecord.selected1 != -1 && FontEntryRecord.selected2 != -1 )
	{
		FontEntryRecord.top1 = FontEntryRecord.selected1;
		if ( FontEntryRecord.top1+6 >= FontEntryRecord.numEntries1 )
			FontEntryRecord.top1 = FontEntryRecord.numEntries1-6;
		if ( FontEntryRecord.top1 < 0 )
			FontEntryRecord.top1 = 0;

		FontEntryRecord.top2 = FontEntryRecord.selected2;
		if ( FontEntryRecord.top2+4 >= FontEntryRecord.numEntries2 )
			FontEntryRecord.top2 = FontEntryRecord.numEntries2-4;
		if ( FontEntryRecord.top2 < 0 )
			FontEntryRecord.top2 = 0;
	}
}

/******** E O F ********/
