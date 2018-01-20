#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "structs.h"

/**** function declarations ****/

void GetVarsFromPI(struct Toaster_record *trec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Toaster_record *trec, PROCESSINFO *ThisPI);
extern BOOL PerformActions(struct Toaster_record *trec, STRPTR errorStr);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Toaster_record trec;

	GetVarsFromPI(&trec, ThisPI);
	PerformActions(&trec,NULL);	// NULL -> no error string

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Toaster_record *trec, PROCESSINFO *ThisPI)
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
					sscanf(tmp, "%d", &value); 
					trec->cmd = value;
					break;
				case 1:
					sscanf(tmp, "%d", &value); 
					trec->previewSource = value;
					break;
				case 2:
					sscanf(tmp, "%d", &value); 
					trec->transitionBank = value;
					break;
				case 3:
					sscanf(tmp, "%d", &value); 
					trec->transitionSpeed = value;
					break;
				case 4:
					sscanf(tmp, "%d", &value); 
					trec->transitionCol = value;
					break;
				case 5:
					sscanf(tmp, "%d", &value); 
					trec->transitionRow = value;
					break;
				case 6:
					sscanf(tmp, "%d", &value); 
					trec->transitionOwn = value;
					break;
				case 7:
					sscanf(tmp, "%d", &value); 
					trec->from = value;
					break;
				case 8:
					sscanf(tmp, "%d", &value); 
					trec->FS_Number = value;
					break;
				case 9:
					sscanf(tmp, "%d", &value); 
					trec->Into_Number = value;
					break;
				case 10:
					RemoveQuotes(tmp);
					strcpy(trec->loadFrameStore,tmp);
					break;
				case 11:
					RemoveQuotes(tmp);
					strcpy(trec->saveFrameStore,tmp);
					break;
				case 12:
					RemoveQuotes(tmp);
					strcpy(trec->frameStorePath,tmp);
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

void PutVarsToPI(struct Toaster_record *trec, PROCESSINFO *ThisPI)
{
UBYTE scrStr1[512];
UBYTE scrStr2[512];
UBYTE scrStr3[512];

	StrToScript(trec->loadFrameStore, scrStr1);
	StrToScript(trec->saveFrameStore, scrStr2);
	StrToScript(trec->frameStorePath, scrStr3);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %d %d %d %d %d \\\"%s\\\" \\\"%s\\\" \\\"%s\\\"",
					trec->cmd,
					trec->previewSource,
					trec->transitionBank,
					trec->transitionSpeed,
					trec->transitionCol,
					trec->transitionRow,
					trec->transitionOwn,
					trec->from,
					trec->FS_Number,
					trec->Into_Number,
					scrStr1,
					scrStr2,
					scrStr3 );
}

/******** E O F ********/
