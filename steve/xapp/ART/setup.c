#include "nb:pre.h"
#include "samp_gadgets.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

/**** internal function declarations ****/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);

/**** external function declarations ****/

extern BOOL OpenAllLibs(void);
extern void CloseAllLibs(void);
extern void OpenUserApplicationFonts(	struct UserApplicInfo *UAI, STRPTR fontName,
																			int size1, int size2);
extern void CloseUserApplicationFonts(struct UserApplicInfo *UAI);
extern void AdjustGadgetCoords(struct GadgetRecord *GR, int xoffset, int yoffset);
extern BOOL IsScreenLaced(struct Screen *myScreen);
void XappSetup(PROCESSINFO *ThisPI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;

	/**** open standard libraries ****/

	if ( !OpenAllLibs() )
		return;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open and load the medialink font ***/

	OpenUserApplicationFonts(&UAI, "fonts:mediapoint.font", 10, 20);

	/**** open a window ****/

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(CDXL_GR);

	UAI.windowX			 = -1;	/* -1 means center on screen */
	UAI.windowY			 = -1;	/* -1 means center on screen */
	UAI.windowWidth	 = CDXL_GR[0].x2;
	UAI.windowHeight = CDXL_GR[0].y2;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, CDXL_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** close the medialink font ****/

	CloseUserApplicationFonts(&UAI);	/* function not in lib */

	/**** close all the libraries ****/

	CloseAllLibs();
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
struct FileReqRecord FRR;
struct FileListInfo FLI;
TEXT path[SIZE_PATH];
TEXT filename[SIZE_FILENAME];
TEXT fullpath[SIZE_FULLPATH];

#if 0
	/**** parse string ****/

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %d %d %d %d %d", &values[0], &values[1],
					&values[2], &values[3], &values[4], &values[5], &values[6],
					&values[7], &values[8], &values[9] );
#endif

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2] = 46;		// transition
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[6] = 0;			// x offset (relative to centered position)
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[7] = 0;			// y offset (relative to centered position)
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = 12;		// FPS
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = 1;		// Rotations
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = 0;		// CBITS
	}

	/**** set up FRR ****/

	strcpy(path, "SYS:");
	filename[0] = '\0';

	FRR.path							= path;
	FRR.fileName					= filename;
	FRR.title							= "Choose a CDXL file";
	FRR.opts							= DIR_OPT_ALL; // DIR_OPT_ONLYDIRS
	FRR.multiple					= FALSE;
	FRR.FileRequester_GR	= (struct GadgetRecord *)FileRequester_GR;
	FRR.SaveFile_GR				= NULL;
	FRR.FLI								= &FLI;
	FRR.FIB								= (struct FileInfoBlock *)AllocMem(
															(LONG)sizeof(struct FileInfoBlock),
															MEMF_ANY | MEMF_CLEAR);
	if (FRR.FIB == NULL)
		return(FALSE);

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		strcpy(fullpath, ThisPI->pi_Arguments.ar_Worker.aw_ExtraData);
		UA_ShortenString(	window->RPort, fullpath,
											(CDXL_GR[2].x2-CDXL_GR[2].x1)-16);
		UA_DrawText(window, &CDXL_GR[2], fullpath);
		UA_EnableButton(window, &CDXL_GR[5]);	// preview
	}
	else
	{
		UA_DrawText(window, &CDXL_GR[2], "No file selected" );
		UA_DisableButton(window, &CDXL_GR[5], mypattern1);	// preview
	}

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);

	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			while( message = (struct IntuiMessage *)GetMsg(window->UserPort) )
			{
				CED.Class			= message->Class;
				CED.Code			= message->Code;
				CED.Qualifier	= message->Qualifier;
				CED.MouseX 		= message->MouseX;
				CED.MouseY 		= message->MouseY;
				ReplyMsg((struct Message *)message);

				if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, CDXL_GR, &CED);
					switch(ID)
					{
						case 2:
							if ( UA_OpenAFile(window, &FRR, mypattern1) )
							{
								UA_MakeFullPath(path, filename, fullpath);
								UA_ShortenString(	window->RPort, fullpath,
																	(CDXL_GR[2].x2-CDXL_GR[2].x1)-16);
								UA_ClearButton(window, &CDXL_GR[2], AREA_PEN);
								UA_DrawText(window, &CDXL_GR[2], fullpath);
								UA_EnableButton(window, &CDXL_GR[5]);	// preview
							}
							break;

						case 3:	// OK
							UA_HiliteButton(window, &CDXL_GR[ID]);
							loop=FALSE;
							retVal=TRUE; 
							break;

						case 4:	// Cancel
							UA_HiliteButton(window, &CDXL_GR[ID]);
							loop=FALSE;
							retVal=FALSE; 
							break;

						case 5: // Preview
							UA_InvertButton(window, &CDXL_GR[ID]);
							Delay(100L);
							UA_InvertButton(window, &CDXL_GR[ID]);
							break;
					}
				}
			}
		}
	}

	FreeMem(FRR.FIB, (LONG)sizeof(struct FileInfoBlock));

	if ( retVal )
	{
		UA_MakeFullPath(path, filename, fullpath);
		strcpy(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, fullpath);
	}

	return(retVal);
}

/******** E O F ********/
