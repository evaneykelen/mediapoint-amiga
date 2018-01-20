#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern TEXT reportStr[256];

/**** functions ****/

/******** ParsePageFile() ********/
/*
 * fpw1 = MP_BigFile
 * Be careful with destPath: it can be NULL!
 *
 */

BOOL ParsePageFile(	FILE *fpw1, STRPTR whiteSpcs, STRPTR buffer, BYTE mode,
										STRPTR destPath, STRPTR pagePath )
{
struct ParseRecord *PR;
int instruc, line;
BOOL validScript=TRUE;
//BOOL headerPassed=FALSE;
BOOL linePrinted;
FILE *fpw2;
TEXT tempPage[SIZE_FULLPATH];

	/**** init vars ****/

	instruc = -1;
	line = 0;

	/**** open parser ****/

	PR = (struct ParseRecord *)OpenParseFile(commands,pagePath);
	if (PR==NULL)
	{
		// "Warning: unable to find file %s"
		sprintf(reportStr,msgs[Msg_SM_32-1],pagePath);
		Report(reportStr);
		return(FALSE);
	}

	/**** open temporary pagefile ****/

	sprintf(tempPage,"%s.TMP",pagePath);
	fpw2 = fopen(tempPage,"w");
	if ( !fpw2 )
	{
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** parse all lines ****/

	while(instruc!=PARSE_STOP)
	{
		linePrinted=FALSE;
		instruc = Special_GetParserLine((struct ParseRecord *)PR, buffer, whiteSpcs);
		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buffer);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				if (line>=MAX_PARSER_NODES)
				{
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
				else
				{
					//headerPassed = TRUE;
					PR->sourceLine = line+1;
					if (	PR->commandCode == SM_CLIP ||
								PR->commandCode == SM_CRAWL ||
								PR->commandCode == SM_TEXT ||
								PR->commandCode == SM_CLIPANIM )
					{
						if ( !SMPerfFunc((struct GenericFuncs *)funcs,PR,fpw1,fpw2,whiteSpcs,buffer,mode,destPath) )
						{
							validScript=FALSE;	// ADDED Sunday 09-0ct-94 15:37:05
							instruc=PARSE_STOP;
						}
						linePrinted=TRUE;
					}
					if (PR->commandCode == PRINTERROR_CODE)
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
			if ( !linePrinted )
				fprintf(fpw2,"%s%s\n",whiteSpcs,buffer);
		}
		else if ( instruc == PARSE_NEXT )
			fprintf(fpw2,"\n");

		line++;
	}

	/**** close parser ****/

	fclose(fpw2);

	CloseParseFile(PR);

	//if (!headerPassed)
	//	validScript=FALSE;
	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** E O F ********/
