#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

/**** functions ****/

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Sample_record *sample_rec, PROCESSINFO *ThisPI)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	/** 1 "file" loops vol freq fadeIn balance fromDisk track	filter **/
	/** 2 track																								       **/
	/** 3 secs track																					       **/ 
	/** 4 secs track																					       **/ 
	/** 5 vol track																						       **/ 

	sample_rec->filter = 0;	// ensure backwards compatibility - filter was always off

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		sample_rec->filename[0] = '\0';
		return;	// sorry Cees; no file means no playing either.
	}

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
		if ( numChars>=1 && numChars<SIZE_FULLPATH )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:	// 1st arg
					sscanf(tmp, "%d", &(sample_rec->action));
					break;

				case 1:	// 2nd arg
					switch(sample_rec->action)
					{
						case SAMPLE_PLAY:
							strcpy(sample_rec->filename, tmp);
							RemoveQuotes(sample_rec->filename);
							break;

						case SAMPLE_STOP:
							sscanf(tmp, "%d", &(sample_rec->trackStop));
							sample_rec->track = sample_rec->trackStop;
							break;

						case SAMPLE_FADEOUT:
							sscanf(tmp, "%d", &(sample_rec->fadeOut));
							break;

						case SAMPLE_FADEIN:
							sscanf(tmp, "%d", &(sample_rec->fadeIn));
							break;

						case SAMPLE_SETVOL:
							sscanf(tmp, "%d", &(sample_rec->setVolume));
							break;
					}
					break;

				case 2:	// 3rd arg
					switch(sample_rec->action)
					{
						case SAMPLE_PLAY:
							sscanf(tmp, "%d", &(sample_rec->loops));
							break;

						case SAMPLE_FADEOUT:
							sscanf(tmp, "%d", &(sample_rec->trackFadeOut));
							sample_rec->track = sample_rec->trackFadeOut;
							break;

						case SAMPLE_FADEIN:
							sscanf(tmp, "%d", &(sample_rec->trackFadeIn));
							sample_rec->track = sample_rec->trackFadeIn;
							break;

						case SAMPLE_SETVOL:
							sscanf(tmp, "%d", &(sample_rec->trackSetVol));
							sample_rec->track = sample_rec->trackSetVol;
							break;
					}
					break;

				case 3:	// 4th arg
					sscanf(tmp, "%d", &(sample_rec->volume));
					break;

				case 4:	// 5th arg
					sscanf(tmp, "%d", &(sample_rec->freq));
					break;

				case 5:	// 6th arg
					sscanf(tmp, "%d", &(sample_rec->playFadeIn));
					break;

				case 6:	// 7th arg
					sscanf(tmp, "%d", &(sample_rec->balance));
					break;

				case 7:	// 8th arg
					sscanf(tmp, "%d", &(sample_rec->playFromDisk));
					break;

				case 8:	// 9th arg
					sscanf(tmp, "%d", &(sample_rec->trackPlay));
					sample_rec->track = sample_rec->trackPlay;
					break;

				case 9:	// 10th arg
					sscanf(tmp, "%d", &(sample_rec->filter));
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

void PutVarsToPI(struct Sample_record *sample_rec, PROCESSINFO *ThisPI)
{
TEXT path[SIZE_FULLPATH];

	StrToScript(sample_rec->filename, path);

	switch(sample_rec->action)
	{
		case SAMPLE_PLAY:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d \\\"%s\\\" %d %d %d %d %d %d %d %d",
							sample_rec->action,
							path,
							sample_rec->loops,
							sample_rec->volume,
							sample_rec->freq,
							sample_rec->playFadeIn,
							sample_rec->balance,
							sample_rec->playFromDisk,
							sample_rec->trackPlay,
							sample_rec->filter );
			break;

		case SAMPLE_STOP:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d",
							sample_rec->action,
							sample_rec->trackStop );
			break;

		case SAMPLE_FADEOUT:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d",
							sample_rec->action,
							sample_rec->fadeOut,
							sample_rec->trackFadeOut );
			break;

		case SAMPLE_FADEIN:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d",
							sample_rec->action,
							sample_rec->fadeIn,
							sample_rec->trackFadeIn );
			break;

		case SAMPLE_SETVOL:
			sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
							"%d %d %d",
							sample_rec->action,
							sample_rec->setVolume,
							sample_rec->trackSetVol );
			break;
	}
}

/******** E O F ********/
