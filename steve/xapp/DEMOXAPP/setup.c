/********************************************************************************/
/********************************************************************************/
/*                                                                              */
/* Put the tab size to 2!                                                       */
/*                                                                              */
/********************************************************************************/
/********************************************************************************/

#include <exec/exec.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/serial.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <workbench/startup.h>
// CLIB
#include <clib/exec_protos.h>
// PRAGMAS
#include <pragmas/exec_pragmas.h>
// ROOT
#include <stdio.h>
#include <string.h>
// USER
#include "Gen/mp.h"
#include "Gen/init_protos.h"
#include "Gen/serial_protos.h"
#include "setup.h"
#include "protos.h"
#include "structs.h"

#define VERSI0N "\0$VER: 1.0"          
static UBYTE *vers = VERSI0N;

/**** globals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;

	if ( !OpenStuff(Demo_GR, &UAI) )
		return;

	MonitorUser(UAI.userWindow, ThisPI);

	CloseStuff(&UAI);
}

/******** MonitorUser() ********/

void MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal=FALSE;
struct EventData CED;
int ID;
UWORD *mypattern1;
struct DemoRecord demo_rec;
struct PopUpRecord PUR;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH], filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];

	/**** render gadgets and gadget text ****/

	UA_DrawGadgetList(window, Demo_GR);
	UA_DrawSpecialGadgetText(window, &Demo_GR[ 9], "Send text over serial port:", SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &Demo_GR[10], "Baud:", SPECIAL_TEXT_TOP);

	/**** alloc memory used by MediaPoint gadgets ****/

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return;
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** read .info -- watch out for correct xapp name!!! ****/

	GetInfoFile(ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath, "DemoXapp",
							demo_rec.devName, &(demo_rec.unit), &(demo_rec.baudrate));

	/**** open special libraries or devices ****/

	demo_rec.handshaking	= 0;	// 0=XON/XOFF  1=RTS/CTS  2=NONE
	demo_rec.parity				= 0;	// 0=NONE  1=EVEN  2=ODD  3=MARK  4=SPACE
	demo_rec.bits_char		= 1;	// 0=7 bits  1=8 bits
	demo_rec.stop_bits		= 1;	// 0=1 bit  1=2 bits 

	/**** get data from xapp if available ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		// Read data from xapp storage	

		GetVarsFromPI(&demo_rec, ThisPI);
	}
	else
	{
		// This happens when xapp is dragged in the script and nothing is selected yet.
		// No data in xapp storage -- initialize structure

		demo_rec.arexxCommand[0] = '\0';
		demo_rec.port[0] = '\0';
		demo_rec.serialText[0] = '\0';

		demo_rec.am_fm = 1;
		demo_rec.stereo = TRUE;
		demo_rec.dolby = FALSE;
	}

	/**** init pop-up requester ****/

	PUR.window	= NULL;
	PUR.GR			= PopUp_GR;
	PUR.ptr			= BaudRateList;	// see setup.h
	PUR.active	= 3;
	PUR.number	= 4;
	PUR.width		= 6;
	PUR.fit			= 0;
	PUR.top			= 0;

	/**** set buttons ****/

	UA_SetStringGadgetToString(window, &Demo_GR[ 6], demo_rec.arexxCommand);
	UA_SetStringGadgetToString(window, &Demo_GR[ 7], demo_rec.port);
	UA_SetStringGadgetToString(window, &Demo_GR[ 9], demo_rec.serialText);

	if ( demo_rec.baudrate==1200 )
		PUR.active = 0;
	else if ( demo_rec.baudrate==2400 )
		PUR.active = 1;
	else if ( demo_rec.baudrate==4800 )
		PUR.active = 2;
	else if ( demo_rec.baudrate==9600 )
		PUR.active = 3;
	UA_PrintPopUpChoice(window, &Demo_GR[10], &PUR);
	UA_SetCycleGadgetToVal(window, &Demo_GR[11], PUR.active);
	UA_SetStringGadgetToVal(window, &Demo_GR[12], demo_rec.baudrate);

	if ( demo_rec.am_fm == 1 )
	{
		UA_InvertButton(window, &Demo_GR[13]);
		UA_DrawSpecialGadgetText(window, &Demo_GR[15], "AM", SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &Demo_GR[16], "AM", SPECIAL_TEXT_CENTER);
	}
	else if ( demo_rec.am_fm == 2 )
	{
		UA_InvertButton(window, &Demo_GR[14]);
		UA_DrawSpecialGadgetText(window, &Demo_GR[15], "FM", SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &Demo_GR[16], "FM", SPECIAL_TEXT_CENTER);
	}

	if ( demo_rec.stereo )
		UA_InvertButton(window, &Demo_GR[17]);

	if ( demo_rec.dolby )
		UA_InvertButton(window, &Demo_GR[18]);

	/**** init file requester ****/

	strcpy(path, "RAM:");
	filename[0] = '\0';

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
	FRR.multiple	= FALSE;

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Demo_GR, &CED);
			switch(ID)
			{
				case 2:		// OK
do_ok:
					UA_HiliteButton(window, &Demo_GR[2]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 3:		// Preview
					UA_InvertButton(window, &Demo_GR[ID]);
					Preview(&demo_rec, ThisPI);
					UA_InvertButton(window, &Demo_GR[ID]);
					break;
					
				case 4:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Demo_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 6:		// ARexx command
					UA_ProcessStringGadget(window,Demo_GR,&Demo_GR[ID],&CED);
					UA_SetStringToGadgetString(&Demo_GR[ID],demo_rec.arexxCommand);
					break;

				case 7:		// ARexx port
					UA_ProcessStringGadget(window,Demo_GR,&Demo_GR[ID],&CED);
					UA_SetStringToGadgetString(&Demo_GR[ID],demo_rec.port);
					break;

				case 9:		// Serial text
					UA_ProcessStringGadget(window,Demo_GR,&Demo_GR[ID],&CED);
					UA_SetStringToGadgetString(&Demo_GR[ID],demo_rec.serialText);
					break;

				case 10:	// baud rate -- pop up variant
					UA_InvertButton(window, &Demo_GR[ID]);
					if ( UA_OpenPopUpWindow(window, &Demo_GR[ID], &PUR) )
					{
						UA_Monitor_PopUp(&PUR);
						UA_ClosePopUpWindow(&PUR);
						UA_SetStringGadgetToString(window, &Demo_GR[ID], PUR.ptr+PUR.active*PUR.width);
						sscanf(PUR.ptr+PUR.active*PUR.width,"%d",&demo_rec.baudrate);
					}
					UA_InvertButton(window, &Demo_GR[ID]);
					UA_PrintPopUpChoice(window, &Demo_GR[ID], &PUR);

					// Set other baud rate gadgets too
					UA_SetCycleGadgetToVal(window, &Demo_GR[11], PUR.active);
					UA_SetStringGadgetToVal(window, &Demo_GR[12], demo_rec.baudrate);
					break;

				case 11:	// baud rate -- cycle gadget variant
					UA_ProcessCycleGadget(window,&Demo_GR[ID],&CED);
					UA_SetValToCycleGadgetVal(&Demo_GR[ID],&PUR.active);
					// Set other baud rate gadgets too
					UA_PrintPopUpChoice(window, &Demo_GR[10], &PUR);
					if ( PUR.active==0 )
						demo_rec.baudrate = 1200;
					else if ( PUR.active==1 )
						demo_rec.baudrate = 2400;
					else if ( PUR.active==2 )
						demo_rec.baudrate = 4800;
					else if ( PUR.active==3 )
						demo_rec.baudrate = 9600;
					UA_SetStringGadgetToVal(window, &Demo_GR[12], demo_rec.baudrate);
					break;

				case 12:	// Baud rate -- integer gadget variant
					UA_ProcessStringGadget(window,Demo_GR,&Demo_GR[ID],&CED);
					UA_SetValToStringGadgetVal(&Demo_GR[ID],&demo_rec.baudrate);
					// Set other baud rate gadgets too
					if ( demo_rec.baudrate==1200 )
						PUR.active = 0;
					else if ( demo_rec.baudrate==2400 )
						PUR.active = 1;
					else if ( demo_rec.baudrate==4800 )
						PUR.active = 2;
					else if ( demo_rec.baudrate==9600 )
						PUR.active = 3;
					UA_PrintPopUpChoice(window, &Demo_GR[10], &PUR);
					UA_SetCycleGadgetToVal(window, &Demo_GR[11], PUR.active);
					break;

				case 13:
						if ( demo_rec.am_fm == 2 )
						{
							demo_rec.am_fm = 1;
							UA_InvertButton(window, &Demo_GR[13]);
							UA_InvertButton(window, &Demo_GR[14]);
							UA_ClearButton(window, &Demo_GR[15], AREA_PEN);
							UA_ClearButton(window, &Demo_GR[16], AREA_PEN);
							UA_DrawSpecialGadgetText(window, &Demo_GR[15], "AM", SPECIAL_TEXT_CENTER);
							UA_DrawSpecialGadgetText(window, &Demo_GR[16], "AM", SPECIAL_TEXT_CENTER);
						}
					break;

				case 14:
						if ( demo_rec.am_fm == 1 )
						{
							demo_rec.am_fm = 2;
							UA_InvertButton(window, &Demo_GR[13]);
							UA_InvertButton(window, &Demo_GR[14]);
							UA_ClearButton(window, &Demo_GR[15], AREA_PEN);
							UA_ClearButton(window, &Demo_GR[16], AREA_PEN);
							UA_DrawSpecialGadgetText(window, &Demo_GR[15], "FM", SPECIAL_TEXT_CENTER);
							UA_DrawSpecialGadgetText(window, &Demo_GR[16], "FM", SPECIAL_TEXT_CENTER);
						}
					break;

				case 17:
						if ( demo_rec.stereo )
							demo_rec.stereo = FALSE;
						else
							demo_rec.stereo = TRUE;
						UA_InvertButton(window, &Demo_GR[ID]);
					break;

				case 18:
						if ( demo_rec.dolby )
							demo_rec.dolby = FALSE;
						else
							demo_rec.dolby = TRUE;
						UA_InvertButton(window, &Demo_GR[ID]);
					break;

				case 19:
					UA_HiliteButton(window, &Demo_GR[ID]);
					FRR.title = "Select a file:";
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(FRR.path, FRR.fileName, fullPath);
						UA_ShortenString(window->RPort, fullPath, (Demo_GR[23].x2-Demo_GR[23].x1)-16);
						UA_ClearButton(window, &Demo_GR[23], AREA_PEN);
						UA_DrawSpecialGadgetText(window, &Demo_GR[23], fullPath, SPECIAL_TEXT_CENTER);
					}
					break;

				case 20:
					UA_HiliteButton(window, &Demo_GR[ID]);
					FRR.title = "Save this file as:";
					if ( UA_SaveAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(FRR.path, FRR.fileName, fullPath);
						UA_ShortenString(window->RPort, fullPath, (Demo_GR[23].x2-Demo_GR[23].x1)-16);
						UA_ClearButton(window, &Demo_GR[23], AREA_PEN);
						UA_DrawSpecialGadgetText(window, &Demo_GR[23], fullPath, SPECIAL_TEXT_CENTER);
						// The next call flushes the directory caching. The next time you use the
						// file requester to open a file, the file you just saved will be listed.
						UA_OpenAFile(NULL,&FRR,NULL);	// special trick to reset dir caching
					}
					break;

				case 21:
					UA_HiliteButton(window, &Demo_GR[ID]);
					GiveMessage(window, "This is a %s", "message.");
					break;

				case 22:
					UA_HiliteButton(window, &Demo_GR[ID]);
					if ( UA_OpenGenericWindow(window, TRUE, TRUE, "OK", "Cancel", QUESTION_ICON,
																		"Press OK or Cancel.", TRUE, NULL) )
					{
						UA_OpenGenericWindow(window, TRUE, FALSE, "OK", NULL, NO_ICON,
																		"You pressed OK.", TRUE, NULL);
					}
					else
					{
						UA_OpenGenericWindow(window, FALSE, TRUE, NULL, "Cancel", NO_ICON,
																		"You pressed Cancel.", TRUE, NULL);
					}
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	/**** free memory used by MediaPoint gadgets ****/

	FreeMem(mypattern1, 4L);

	/**** Store data in xapp storage (only if OK was clicked) ****/

	if ( retVal )
		PutVarsToPI(&demo_rec, ThisPI);

	return;
}

/******** E O F ********/
