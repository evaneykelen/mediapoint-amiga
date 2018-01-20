#include "mllib_includes.h"

/**** functions ****/

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** AddStrGad() ********/

void AddStrGad(	struct Window *window, struct GadgetRecord *GR, UBYTE *txt,
								UBYTE *undo, int max, struct StringInfo *SI, struct Gadget *G,
								struct StringExtend *SE)
{
	SI->Buffer					= txt;
	SI->UndoBuffer			= undo;
	SI->BufferPos				= 0;
	SI->MaxChars				= max;

	SE->Font						= (struct TextFont *)window->RPort->Font;

	if ( window->RPort->BitMap->Depth==1 )
	{
		SE->Pens[0]				= 1;
		SE->Pens[1]				= 0;
		SE->ActivePens[0]	= 1;
		SE->ActivePens[1]	= 0;
	}
	else
	{
		SE->Pens[0]				= ChooseTextPen(window,GR);	// LO_PEN
		SE->Pens[1]				= ChoosePen(window,GR,AREA_PEN);
		SE->ActivePens[0]	= ChooseTextPen(window,GR);	// LO_PEN;
		SE->ActivePens[1]	= ChoosePen(window,GR,AREA_PEN);
	}

	SE->InitialModes		= SGM_EXITHELP;	// was 0L
	SE->EditHook				= NULL;
	SE->WorkBuffer			= NULL;

	SI->Extension = (struct StringExtend *)SE;

#if 0
	G->LeftEdge				= GR->x1 + 4;
	if ( LIBUA_IsWindowOnLacedScreen(window) )
		G->TopEdge			= GR->y1 + 3;
	else
		G->TopEdge			= GR->y1 + 2;
	G->Width					= GR->x2 - GR->x1 - 7;
	if ( LIBUA_IsWindowOnLacedScreen(window) )
		G->Height				= GR->y2 - GR->y1 - 4;
	else
		G->Height				= GR->y2 - GR->y1 - 3;
#endif

	G->LeftEdge				= GR->x1 + 4;
	G->TopEdge				= GR->y1 + 2;
	G->Width					= GR->x2 - GR->x1 - 7;
	G->Height					= GR->y2 - GR->y1 - 3;
#if 0
	if ( LIBUA_IsWindowOnLacedScreen(window) )
	{
		G->TopEdge			+= 2;
		G->Height				-= 4;
	}
#endif
	G->Flags					= GFLG_GADGHNONE | GFLG_STRINGEXTEND;
	G->Activation			= GACT_STRINGLEFT | GACT_RELVERIFY;
	G->GadgetType			= GTYP_STRGADGET;
	G->GadgetRender		= NULL;
	G->SelectRender		= NULL;
	G->GadgetText			= NULL;
	G->MutualExclude	= 0L;
	G->SpecialInfo		= SI;
	G->GadgetID				= 0;
	G->UserData				= NULL;

	AddGadget(window, G, -1L);
}

/******** RemoveStrGad() ********/

void RemoveStrGad(struct Window *window, struct Gadget *G)
{
	RemoveGList(window, G, 1);
}

/******** E O F ********/
