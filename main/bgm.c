#include "nb:pre.h"

/**** externals ****/

extern struct RendezVousRecord rvrec;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern struct eventHandlerInfo EHI;

/**** functions ****/

/******** ShowBGM() ********/

void ShowBGM(void)
{
BOOL retval;
int i;
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
BOOL list[MAXEDITWINDOWS];

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		PaletteToBack();

	SetSpriteOfActWdw(SPRITE_BUSY);

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		rvrec.EW[i]	= EditWindowList[i];
		rvrec.ES[i]	= EditSupportList[i];
	}

	DrawAllHandles(LEAVE_ACTIVE);

	PrepareRedrawAll(prevX,prevY,prevW,prevH,newX,newY,newW,newH,list);

	retval = FALSE;
	if ( !LoadModule("BarGraphicsMaker", &retval) )
		UA_WarnUser(-1);

	if ( retval )
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditSupportList[i] && EditSupportList[i]->Active )
			{
				list[i] = TRUE;
				CorrectEW(EditWindowList[i]);
				newX[i] = EditWindowList[i]->X;
				newY[i] = EditWindowList[i]->Y;
				newW[i] = EditWindowList[i]->Width;
				newH[i] = EditWindowList[i]->Height;
				//ValidateBoundingBox(&newX[i],&newY[i],&newW[i],&newH[i]);
				//SetTheMargins();
			}
		}
		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);
	}

	DrawAllHandles(LEAVE_ACTIVE);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** E O F ********/
