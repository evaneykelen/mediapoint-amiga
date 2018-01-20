#include "nb:pre.h"
#include "protos.h"

/**** globals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct UserApplicInfo UAI;
extern struct EventData CED;
extern struct FileListInfo FLI;
extern TEXT scanDevs[];

/**** gadgets ****/

extern struct GadgetRecord SM_3_GR[];

/**** functions ****/

/******** DrawPage3Gadgets() ********/

void DrawPage3Gadgets(void)
{
	UA_DrawGadgetList(UAI.userWindow, SM_3_GR);
}

/******* CheckPage3() ********/

void CheckPage3(void)
{
int ID;

	ID = UA_CheckGadgetList(UAI.userWindow, SM_3_GR, &CED);
}

/******** E O F ********/
