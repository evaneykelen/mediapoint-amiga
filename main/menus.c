#include "nb:pre.h"

/**** externals ****/

extern UWORD chip gui_pattern[];
extern struct BitMap scratchBM;
extern struct RastPort scratchRP;
extern struct CapsPrefs CPrefs;
extern struct Library *medialinkLibBase;
extern struct MenuRecord **page_MR;

/**** functions ****/

/******** InitializeMenu() ********/

void InitializeMenu(struct MenuRecord *MR, struct TextFont *menuFont)
{
	InitBitMap(&MR->menuBM, 2, MR->width, MR->height);
	MR->menuBM.Planes[0] = (PLANEPTR)AllocRaster(MR->width, MR->height);
	MR->menuBM.Planes[1] = (PLANEPTR)AllocRaster(MR->width, MR->height);

	InitRastPort(&MR->menuRP); 
	MR->menuRP.BitMap = &MR->menuBM;

	SetFont(&MR->menuRP, menuFont);
}

/******** RenderMenuInterior() ********/

void RenderMenuInterior(struct MenuRecord *MR)
{
int i;

	if ( MR->height == 0 )
		return;

	SetAPen(&MR->menuRP, HI_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	RectFill(&MR->menuRP, 0L, 0L, (LONG)MR->width-1, (LONG)MR->height-1);

	SetAPen(&MR->menuRP, LO_PEN);
	Move(&MR->menuRP, 0L, 0L);
	Draw(&MR->menuRP, (LONG)MR->width-1, 0L);
	Draw(&MR->menuRP, (LONG)MR->width-1, (LONG)MR->height-1);
	Draw(&MR->menuRP, 0L, (LONG)MR->height-1);
	Draw(&MR->menuRP, 0L, 0L);

	Move(&MR->menuRP, (LONG)1L, 0L);
	Draw(&MR->menuRP, (LONG)1L, (LONG)MR->height-1);
	Move(&MR->menuRP, (LONG)MR->width-2, 0L);
	Draw(&MR->menuRP, (LONG)MR->width-2, (LONG)MR->height-1);

	for(i=0; i<16; i++)
		if (MR->title[i] != NULL)
			WriteIntoMenu(MR, i);

	WaitBlit();
}

/******** WriteIntoMenu() ********/

void WriteIntoMenu(struct MenuRecord *MR, int line)
{
TEXT buf[5];

	SetAPen(&MR->menuRP, LO_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	Move(&MR->menuRP, 3L, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
	Text(&MR->menuRP, MR->title[line], (LONG)strlen(MR->title[line]));
	if (MR->commandKey[line]!=NULL)
	{
		if ( !MR->shifted[line] )
		{
			Move(&MR->menuRP, MR->width-37, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
			buf[0] = 0x8c;	// amiga key
			buf[1] = MR->commandKey[line];
			buf[2] = '\0';
			Text(&MR->menuRP, buf, 2L);
		}
		else
		{
			Move(&MR->menuRP, MR->width-46, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
			buf[0] = 0x80;	// shift key
			buf[1] = 0x8c;	// amiga key
			buf[2] = MR->commandKey[line];
			buf[3] = '\0';
			Text(&MR->menuRP, buf, 3L);
		}
	}
}

/******** DrawMenuLine() ********/

void DrawMenuLine(struct MenuRecord *MR, int line)
{
int y;

	SetAPen(&MR->menuRP, LO_PEN);
	SetDrMd(&MR->menuRP, JAM1);

	y = MR->menuRP.TxBaseline + 5 + line*MHEIGHT;
	SetAfPt(&MR->menuRP, gui_pattern, 1);
	RectFill(&MR->menuRP, 0L, (LONG)y, (LONG)MR->width-1, (LONG)y);
	SetAfPt(&MR->menuRP, NULL, 0);
}

/******** RenderMenu() ********/

void RenderMenu(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY)
{
	if ( MR->height==0 )
		return;

	/**** save background ****/

	if ( MR->height > scratchBM.Rows )
		UA_WarnUser(-1);

	ClipBlit(window->RPort,offX,offY,&scratchRP,0,0,MR->width,MR->height,0xc0);
	WaitBlit();

	/**** clear background ****/ 

	SetAPen(window->RPort, BGND_PEN);
	SetDrMd(window->RPort, JAM1);
	RectFill(	window->RPort, offX, offY,
						offX+MR->width-1, offY+MR->height-1);
	WaitBlit();

	/**** show menu ****/

	WaitTOF();
	BltBitMapRastPort(&MR->menuBM,0,0,window->RPort,offX, offY,MR->width,MR->height,0xc0);
	WaitBlit();
}

/******** RemoveMenu() ********/

void RemoveMenu(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY)
{
	if ( MR->height==0 )
		return;

	/**** restore background ****/

	WaitTOF();
	ClipBlit(&scratchRP,0,0,window->RPort,offX,offY,MR->width,MR->height,0xc0);
	WaitBlit();
}

/******** FreeMenu() ********/

void FreeMenu(struct MenuRecord *MR)
{
	if ( MR!=NULL )
	{
		WaitBlit();
		if (MR->menuBM.Planes[0]!=NULL)
			FreeRaster(MR->menuBM.Planes[0], MR->width, MR->height);
		if (MR->menuBM.Planes[1]!=NULL)
			FreeRaster(MR->menuBM.Planes[1], MR->width, MR->height);
	}
}

/******** DisableMenu() ********/

void DisableMenu(struct MenuRecord *MR, int line)
{
	SetAPen(&MR->menuRP, HI_PEN);
	SetAfPt(&MR->menuRP, gui_pattern, 1);
	SetDrMd(&MR->menuRP, JAM1);
	RectFill(	&MR->menuRP, 2L, (LONG)line*MHEIGHT+2,
						(LONG)MR->width-4, (LONG)(line*MHEIGHT)+MHEIGHT-1);
	SetAfPt(&MR->menuRP, NULL, 0);
	MR->disabled[line] = TRUE;
}

/******** EnableMenu() ********/

void EnableMenu(struct MenuRecord *MR, int line)
{
	if ( CPrefs.userLevel==1 && MR==page_MR[PMISC_MENU] )
	{
		if ( line==PMISC_DEFINE )
			return;
	}		

	SetAPen(&MR->menuRP, HI_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	RectFill(	&MR->menuRP, 2L, (LONG)line*MHEIGHT+2,
						(LONG)MR->width-4, (LONG)(line*MHEIGHT)+MHEIGHT-1);
	WriteIntoMenu(MR, line);
	MR->disabled[line] = FALSE;
}

/******** ClearMenuLine() ********/

void ClearMenuLine(struct MenuRecord *MR, int line)
{
	SetAPen(&MR->menuRP, HI_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	RectFill(	&MR->menuRP, 2L, (LONG)line*MHEIGHT+2,
						(LONG)MR->width-4, (LONG)(line*MHEIGHT)+MHEIGHT-1);
}

/******** ToggleChooseMenuItem() ********/

void ToggleChooseMenuItem(struct MenuRecord *MR, int line)
{
	ClearMenuLine(MR, line);
	SetAPen(&MR->menuRP, LO_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	Move(&MR->menuRP, 3L, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
	if ( MR->title[line][0] == (UBYTE)'…' )	/* chosen */
		MR->title[line][0] = (UBYTE)'†';
	else
		MR->title[line][0] = (UBYTE)'…';
	Text(&MR->menuRP, MR->title[line], (LONG)strlen(MR->title[line]));
}

/******** SetChooseMenuItem() ********/

void SetChooseMenuItem(struct MenuRecord *MR, int line)
{
	ClearMenuLine(MR, line);
	SetAPen(&MR->menuRP, LO_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	Move(&MR->menuRP, 3L, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
	MR->title[line][0] = (UBYTE)'…';
	Text(&MR->menuRP, MR->title[line], (LONG)strlen(MR->title[line]));
}

/******** UnsetChooseMenuItem() ********/

void UnsetChooseMenuItem(struct MenuRecord *MR, int line)
{
	ClearMenuLine(MR, line);
	SetAPen(&MR->menuRP, LO_PEN);
	SetDrMd(&MR->menuRP, JAM1);
	Move(&MR->menuRP, 3L, (LONG)MR->menuRP.TxBaseline + line*MHEIGHT + 2);
	MR->title[line][0] = (UBYTE)'†';
	Text(&MR->menuRP, MR->title[line], (LONG)strlen(MR->title[line]));
}

/******** E O F ********/
