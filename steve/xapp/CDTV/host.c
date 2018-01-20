#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"
#include "host.h"

#define VERSI0N "\0$VER: MediaPoint CDTV Host 1.1"          
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(struct Window *window, struct CDTV_record *CDTV_rec);
BOOL HostOpenLibs(void);
void HostCloseLibs(void);
void OpenUserApplicationFonts(struct UserApplicInfo *UAI, STRPTR fontName,
															int size1, int size2);
void CloseUserApplicationFonts(struct UserApplicInfo *UAI);

/**** external function declarations ****/

extern void OpenUserApplicationFonts(	struct UserApplicInfo *UAI, STRPTR fontName,
																			int size1, int size2);
extern void CloseUserApplicationFonts(struct UserApplicInfo *UAI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
UBYTE **msgs;	// needed by lowlevel.c but not used (I hope...)

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct UserApplicInfo UAI;
struct CDTV_record CDTV_rec;

	/**** open standard libraries ****/

	if ( !HostOpenLibs() )
		return;

	/**** parse .info file for special settings ****/

	GetInfoFile(argv[0], NULL, CDTV_rec.devName, &(CDTV_rec.portNr), &(CDTV_rec.baudRate));

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open and load the medialink font ***/

	OpenUserApplicationFonts(&UAI, "fonts:mediapoint.font", 10, 20);

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	UAI.windowModes  = 3;		/* open on the first (frontmost) screen */

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(Host_GR);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Host_GR[0].x2;
	UAI.windowHeight	= Host_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	CDTV_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Host_GR);
	UA_DisableButton(UAI.userWindow, &Host_GR[3], mypattern1); // Exit

	/**** welcome user ****/

	PrintString(UAI.userWindow, "Welcome to the MediaPoint CDTV host");
	PrintString(UAI.userWindow, "To work with it you'll have to:");
	PrintString(UAI.userWindow, "· Attach a disk drive to your CDTV,");
	PrintString(UAI.userWindow, "· Connect the CDTV to your Amiga with");
	PrintString(UAI.userWindow, "  a null-modem cable,");
	PrintString(UAI.userWindow, "· Insert an audio CD in the CDTV.");
	PrintString(UAI.userWindow, "   ");

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort(&CDTV_rec) )
	{
		PrintString(UAI.userWindow, "ERROR: devs:serial.device not present.");
		Delay(200L);
		UA_CloseWindow(&UAI);
		HostCloseLibs();
		return;
	}

	if ( !OpenCDTV(&CDTV_rec) )
	{
		PrintString(UAI.userWindow, "ERROR: devs:cdtv.device not present.");
		Delay(200L);
		UA_CloseWindow(&UAI);
		Close_SerialPort(&CDTV_rec);
		HostCloseLibs();
		return;
	}

	/**** monitor events ****/

	UA_EnableButton(UAI.userWindow, &Host_GR[3]); // Exit
	MonitorUser(UAI.userWindow, &CDTV_rec);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** close the medialink font ****/

	CloseUserApplicationFonts(&UAI);	/* function not in lib */

	/**** close specials libraries or devices ****/

	CloseCDTV(&CDTV_rec);

	Close_SerialPort(&CDTV_rec);

	/**** close all the libraries ****/

	HostCloseLibs();
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, struct CDTV_record *CDTV_rec)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;

	/**** init CDTV_rec ****/

	CDTV_rec->action			= 0;			// play song
	CDTV_rec->command			= 0;			// fade out slow
	CDTV_rec->song				= 1;
	CDTV_rec->from				= 1;
	CDTV_rec->to					= 2;
	strcpy(CDTV_rec->start, "00:05:00");
	strcpy(CDTV_rec->end, "00:10:00");
	CDTV_rec->fadeIn			= FALSE;
	CDTV_rec->control			= 1; // CONTROL_VIA_CDROM

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);

	while(loop)
	{
		if ( !Monitor_SerialPort(window, CDTV_rec) )
			return(FALSE);	// an error occurred

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
					ID = UA_CheckGadgetList(window, Host_GR, &CED);
					switch(ID)
					{
						case 3:	// OK
							UA_HiliteButton(window, &Host_GR[ID]);
							loop=FALSE;
							retVal=TRUE; 
							break;
					}
				}
			}
		}
	}

	return(retVal);
}

/******** HostOpenLibs() ********/

BOOL HostOpenLibs(void)
{
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
	if (IntuitionBase == NULL)
		return(FALSE);

	if (IntuitionBase->LibNode.lib_Version >= 36)
	{
		printf("Sorry: this program only runs on a CDTV\n");
		return(FALSE);
	}

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
	if (GfxBase == NULL)
		return(FALSE);

	DiskfontBase = (struct Library *)OpenLibrary("diskfont.library", 0L);
	if (DiskfontBase == NULL)
		return(FALSE);

	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library", 0L);
	if (medialinkLibBase == NULL)
		return(FALSE);
	if ( !UA_Open_ML_Lib() )
		return(FALSE);
	UA_PutCapsPort(NULL);	// we don't want to use the shared port paradigm

	return(TRUE);
}

/******** HostCloseLibs() ********/

void HostCloseLibs(void)
{
	if (IntuitionBase != NULL)
		CloseLibrary((struct Library *)IntuitionBase);

	if (GfxBase != NULL)
		CloseLibrary((struct Library *)GfxBase);

	if (DiskfontBase != NULL)
		CloseLibrary((struct Library *)DiskfontBase);

	UA_Close_ML_Lib();
	if (medialinkLibBase != NULL)
		CloseLibrary((struct Library *)medialinkLibBase);
}

/******** OpenUserApplicationFonts() ********/

void OpenUserApplicationFonts(struct UserApplicInfo *UAI, STRPTR fontName,
															int size1, int size2)
{
	UAI->small_TA.ta_Name	 = (UBYTE *)fontName;
	UAI->small_TA.ta_YSize = size1;
	UAI->small_TA.ta_Style = FS_NORMAL;
	UAI->small_TA.ta_Flags = FPF_DESIGNED;

	UAI->small_TF = (struct TextFont *)OpenDiskFont(&UAI->small_TA);
	if (UAI->small_TF==NULL)
	{
		UAI->small_TA.ta_Name	 = (UBYTE *)"topaz.font";
		UAI->small_TA.ta_YSize = 8;
		UAI->small_TA.ta_Style = FS_NORMAL;
		UAI->small_TA.ta_Flags = FPF_DESIGNED;
		UAI->small_TF = (struct TextFont *)OpenFont(&UAI->small_TA);
	}

	UAI->large_TA.ta_Name	 = (UBYTE *)fontName;
	UAI->large_TA.ta_YSize = size2;
	UAI->large_TA.ta_Style = FS_NORMAL;
	UAI->large_TA.ta_Flags = FPF_DESIGNED;

	UAI->large_TF = (struct TextFont *)OpenDiskFont(&UAI->large_TA);
	if (UAI->large_TF==NULL)
	{
		UAI->large_TA.ta_Name	 = (UBYTE *)"topaz.font";
		UAI->large_TA.ta_YSize = 8;
		UAI->large_TA.ta_Style = FS_NORMAL;
		UAI->large_TA.ta_Flags = FPF_DESIGNED;
		UAI->large_TF = (struct TextFont *)OpenFont(&UAI->large_TA);
	}
}

/******** CloseUserApplicationFonts() ********/

void CloseUserApplicationFonts(struct UserApplicInfo *UAI)
{
	if (UAI->small_TF != NULL)
		CloseFont((struct TextFont *)UAI->small_TF);
	if (UAI->large_TF != NULL)
		CloseFont((struct TextFont *)UAI->large_TF);
}

/******** E O F ********/
