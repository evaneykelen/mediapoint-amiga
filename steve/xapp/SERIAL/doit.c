#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"

extern BOOL DoSerial(struct SerRecord *ser_rec);

/**** function declarations ****/

void GetVarsFromPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct SerRecord ser_rec;

	GetVarsFromPI(&ser_rec, ThisPI);
	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		DoSerial(&ser_rec);

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI)
{
int numChars, argNum, len, numSeen, value;
char *strPtr;
char tmp[LARGE_USERAPPLIC_MEMSIZE], scrStr[LARGE_USERAPPLIC_MEMSIZE];

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
					strcpy(ser_rec->data,tmp);
					break;
				case 1:
					sscanf(tmp, "%d", &value);
				 	ser_rec->baudrate = value;
				case 2:
					sscanf(tmp, "%d", &value);
				 	ser_rec->handshaking = value;
				case 3:
					sscanf(tmp, "%d", &value);
				 	ser_rec->parity = value;
				case 4:
					sscanf(tmp, "%d", &value);
				 	ser_rec->bits_char = value;
				case 5:
					sscanf(tmp, "%d", &value);
				 	ser_rec->stop_bits = value;
				case 6:
					sscanf(tmp, "%d", &value);
				 	ser_rec->unit = value;
				case 7:
					sscanf(tmp, "%d", &value);
				 	ser_rec->pacing = value;
					break; 
				case 8:
					RemoveQuotes(tmp);
					strcpy(ser_rec->devName,tmp);
					break;
				default:
					return;	// get the hell out, s'thing is terribly wrong!
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

void PutVarsToPI(struct SerRecord *ser_rec, PROCESSINFO *ThisPI)
{
TEXT scrStr[270];

	StrToScript(ser_rec->data,scrStr);
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"\\\"%s\\\" %d %d %d %d %d %d %d \\\"%s\\\"",
					scrStr,
					ser_rec->baudrate,
					ser_rec->handshaking,
					ser_rec->parity,
					ser_rec->bits_char,
					ser_rec->stop_bits,
					ser_rec->unit,
					ser_rec->pacing,
					ser_rec->devName );
}

/******** E O F ********/
