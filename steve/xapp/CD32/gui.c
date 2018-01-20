#include "nb:pre.h"
#include "protos.h"
#include "mp_cdtv.h"
#include "all.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;

/******** OpenHostWindow() ********/

struct Window *OpenHostWindow(void)
{
struct NewWindow NewWindowStructure;

	NewWindowStructure.LeftEdge 		= 0;
	NewWindowStructure.TopEdge 			= 0;
	NewWindowStructure.Width 				= 320;
	NewWindowStructure.Height 			= 150;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= IDCMP_CLOSEWINDOW;
	NewWindowStructure.Flags 				= WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= NULL;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= WBENCHSCREEN;

	return( OpenWindow(&NewWindowStructure) );
}

/******** CloseHostWindow() ********/

void CloseHostWindow(struct Window *window)
{
	if ( window )
		CloseWindow(window);
}

/******** E O F ********/
