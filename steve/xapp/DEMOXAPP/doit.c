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
#include <string.h>
#include <stdio.h>
// USER
#include "Gen/mp.h"
#include "Gen/serial_protos.h"
#include "Gen/support_protos.h"
#include "protos.h"
#include "structs.h"

extern struct Library *medialinkLibBase;

/**** functions ****/

/******** XappDoIt() ********/
/*
 * This function is called by the script player.
 *
 */

int XappDoIt(PROCESSINFO *ThisPI)
{
struct DemoRecord demo_rec;

	GetVarsFromPI(&demo_rec, ThisPI);
	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		Preview(&demo_rec, ThisPI);

	return(0);
}

/******** GetVarsFromPI() ********/
/*
 * Get data stored in the script. This data contains e.g. a file name etc. which
 * the user selected in the user interface.
 *
 * The data is used in the user interface part as well as the script player part.
 *
 */

void GetVarsFromPI(struct DemoRecord *demo_rec, PROCESSINFO *ThisPI)
{
int numChars, argNum, len, numSeen, value;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;
	ScriptToStr(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, scrStr);
	strPtr = scrStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if ( numChars>=1 && numChars<USERAPPLIC_MEMSIZE )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					RemoveQuotes(tmp);
					strcpy(demo_rec->arexxCommand,tmp);
					break;

				case 1:
					RemoveQuotes(tmp);
					strcpy(demo_rec->port,tmp);
					break;

				case 2:
					RemoveQuotes(tmp);
					strcpy(demo_rec->serialText,tmp);
					break;

				case 3:
					sscanf(tmp, "%d", &value);
				 	demo_rec->am_fm = value;
					break;

				case 4:
					sscanf(tmp, "%d", &value);
				 	demo_rec->stereo = value;
					break;

				case 5:
					sscanf(tmp, "%d", &value);
					demo_rec->dolby = value;
					break;

				case 6:
					sscanf(tmp, "%d", &value);
					demo_rec->baudrate = value;
					break;
			}
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PutVarsToPI() ********/
/*
 * Store the data into the script
 *
 */

void PutVarsToPI(struct DemoRecord *demo_rec, PROCESSINFO *ThisPI)
{
TEXT scrStr[USERAPPLIC_MEMSIZE];

	StrToScript(demo_rec->arexxCommand, scrStr);
	strcpy(demo_rec->arexxCommand, scrStr);

	StrToScript(demo_rec->port, scrStr);
	strcpy(demo_rec->port, scrStr);

	StrToScript(demo_rec->serialText, scrStr);
	strcpy(demo_rec->serialText, scrStr);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"\\\"%s\\\" \\\"%s\\\" \\\"%s\\\" %d %d %d %d",
					demo_rec->arexxCommand, demo_rec->port, demo_rec->serialText,
					demo_rec->am_fm, demo_rec->stereo, demo_rec->dolby, demo_rec->baudrate);
}

/******** Preview() ********/
/*
 * This function is used in the user interface part as well as the script player.
 *
 */

BOOL Preview(struct DemoRecord *demo_rec, PROCESSINFO *ThisPI)
{
TEXT resultStr[100];
ULONG RC;
BOOL libOpened = FALSE;
int dummy;

	if ( medialinkLibBase	== NULL )
	{
		medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
		if ( !medialinkLibBase )
			return(0);
		libOpened = TRUE;
	}

	// DO AREXX STUFF

	if ( demo_rec->arexxCommand[0] )
	{
		Forbid();
		if ( UA_IssueRexxCmd_V2("DEMO_XAPP", demo_rec->port, demo_rec->arexxCommand,
														resultStr, &RC) )
		{
			// resultStr now contains the result string ARexx returned
			// RC contains the result code
			// Note that not all arexx host return strings and/or codes.

			// Nothing is done here with the results.
		}
		Permit();
	}

	// DO SERIAL STUFF	

	// Read the .info file from this xapp. Obtain device name, unit number and
	// baudrate. The baudrate is put into 'dummy' here, because the user can choose
	// the baudrate in this example.

	GetInfoFile(ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath, "DemoXapp",
							demo_rec->devName, &demo_rec->unit, &dummy);

	demo_rec->handshaking = 2;	// none
	demo_rec->parity = 0;	// none
	demo_rec->bits_char = 1;	// 8 bits
	demo_rec->stop_bits = 0;	// 1 bit

	Forbid();
	if ( Open_SerialPort((struct SerRecord *)demo_rec) )
	{
		if ( SendStringViaSer((struct SerRecord *)demo_rec, demo_rec->serialText) )
		{
			// serial text sent correctly
		}
		Close_SerialPort((struct SerRecord *)demo_rec);
	}
	Permit();

	if ( libOpened )
		CloseLibrary((struct Library *)medialinkLibBase);

	return(TRUE);
}

/******** E O F ********/
