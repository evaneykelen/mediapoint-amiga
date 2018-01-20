#include "nb:pre.h"
#include "protos.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct UserApplicInfo UAI;
extern struct EventData CED;
extern struct FileListInfo FLI;
extern TEXT scanDevs[];
extern UWORD chip mypattern1[];
extern char *scriptCommands[];

/**** globals ****/

BOOL checked_volumes[10] = { 0,0,0,0,0, 0,0,0,0,0 };

/**** gadgets ****/

extern struct GadgetRecord SM_2_GR[];

/**** functions ****/

/******** DrawPage2Gadgets() ********/

void DrawPage2Gadgets(void)
{
int i;
struct GadgetRecord GR;

	UA_DrawGadgetList(UAI.userWindow, SM_2_GR);

	/**** render volume names ****/

	for(i=0; i<8; i++)
	{
		if ( scanDevs[i*SIZE_FILENAME] != '\0' )
		{
			CopyMem(&SM_2_GR[i+1], &GR, sizeof(struct GadgetRecord));
			GR.txt = &scanDevs[i*SIZE_FILENAME];
			UA_DrawGadget(UAI.userWindow,&GR);
			//UA_InvertButton(UAI.userWindow, &SM_2_GR[i+1]);
			UA_DisableButton(UAI.userWindow, &SM_2_GR[10], mypattern1);	// scan
		}
		else
			UA_DisableButton(UAI.userWindow, &SM_2_GR[i+1], mypattern1);
	}

	PrintString("Select volumes to scan.");
}

/******* CheckPage2() ********/

void CheckPage2(void)
{
int ID,i,atLeastOne;

	ID = UA_CheckGadgetList(UAI.userWindow, SM_2_GR, &CED);

	/**** check 'check' buttons ****/

	if ( ID>=1 && ID<=8 )
	{
		UA_InvertButton(UAI.userWindow, &SM_2_GR[ID]);

		if ( checked_volumes[ID-1] == TRUE )
			checked_volumes[ID-1] = FALSE;
		else
			checked_volumes[ID-1] = TRUE;

		/**** dis/enable 'Scan' button ****/

		atLeastOne=FALSE;

		for(i=0; i<8; i++)
			if ( checked_volumes[i] )
				atLeastOne=TRUE;

		if ( atLeastOne && UA_IsGadgetDisabled(&SM_2_GR[10]) )
			UA_EnableButton(UAI.userWindow, &SM_2_GR[10]);	// scan

		if ( !atLeastOne && !UA_IsGadgetDisabled(&SM_2_GR[10]) )
			UA_DisableButton(UAI.userWindow, &SM_2_GR[10], mypattern1);	// scan
	}

	if ( ID==10 )	// scan
	{
		UA_HiliteButton(UAI.userWindow, &SM_2_GR[ID]);
		FixScript();
	}
}

/******** FixScript() ********/

void FixScript(void)
{
int i;

	PrintString("Scanning started.");

	OpenDiskList();

	if ( OpenScanMem() )
	{
		for(i=0; i<8; i++)
			if ( checked_volumes[i] && scanDevs[i*SIZE_FILENAME] != '\0' )
				PlaceInList(&scanDevs[i*SIZE_FILENAME]);

		if ( !ParseAScriptFile("msm:","test_script",scriptCommands))
			printf("script parse failed\n");
		else
			printf("script parse succesful\n");

		CloseScanMem();
	}

	CloseDiskList();

	/**** disable all gadgets so user can only choose exit ****/

	for(i=0; i<8; i++)
		UA_DisableButton(UAI.userWindow, &SM_2_GR[i+1], mypattern1);
	UA_DisableButton(UAI.userWindow, &SM_2_GR[10], mypattern1);
}

/******** PrintString() ********/

void PrintString(STRPTR str)
{
	SetAPen(UAI.userWindow->RPort, LO_PEN);
	SetBPen(UAI.userWindow->RPort, AREA_PEN);

	ScrollRaster(	UAI.userWindow->RPort, 0, UAI.userWindow->RPort->TxBaseline+5,
								SM_2_GR[11].x1+2, SM_2_GR[11].y1+2,
								SM_2_GR[11].x2-2, SM_2_GR[11].y2-2); 

	Move(	UAI.userWindow->RPort,
				SM_2_GR[11].x1+5,
				SM_2_GR[11].y2-UAI.userWindow->RPort->TxBaseline);
	Text(UAI.userWindow->RPort, str, strlen(str));
}

/******** E O F ********/
