#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern TEXT reportStr[256];

/**** functions ****/

/******** CleanUpBigFile() ********/

BOOL CleanUpBigFile(STRPTR bigfilePath)
{
struct ParseRecord *PR;
int instruc, line, val, numLines, pct;
BOOL validScript=TRUE;
UBYTE *buffer;
TEXT whiteSpcs[512];
TEXT scrStr[512];
struct FileLock *FL;

	/**** open parser and alloc mem ****/

	PR = (struct ParseRecord *)OpenParseFile(commands,bigfilePath);
	if (PR==NULL)
		return(FALSE);

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_CLEAR);
	if ( buffer==NULL )
	{
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** count lines in script ****/

	numLines = 0L;
	instruc = -1;
	line = 0;
	while(instruc!=PARSE_STOP)
	{
		instruc = Special_GetParserLine((struct ParseRecord *)PR, buffer, whiteSpcs);
		if (instruc == PARSE_INTERPRET)
			numLines++;
	}
	fseek(PR->filePointer,0L,0);

	/**** parse all lines ****/

	instruc = -1;
	line = 0;
	while(instruc!=PARSE_STOP)
	{
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
					PR->sourceLine = line+1;
					if (PR->commandCode == PRINTERROR_CODE)
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
					else
					{
						// Delete .TMP file
						if ( PR->commandCode==SM_PAGE )
						{
							// Lock .TMP file
							ScriptToStr(PR->argString[1],scrStr);
							RemoveQuotes(scrStr);
							FL = (struct FileLock *)Lock((STRPTR)scrStr, (LONG)ACCESS_READ);
							if (FL)
							{
								UnLock((BPTR)FL);
								// Lock old file
								val = strlen(scrStr);
								if ( val>4 )
									scrStr[val-4]='\0';
								FL = (struct FileLock *)Lock((STRPTR)scrStr, (LONG)ACCESS_READ);
								if (FL)
								{
									UnLock((BPTR)FL);
									// Delete old file
									strcat(scrStr,".TMP");
									DeleteFile(scrStr);
								}
							}
						}
					}
				}
			}
		}
		line++;
		if ( numLines>0 )
		{
			pct = ((100*line)/numLines)/2;
			pct += 50;
			if ( pct<50 )
				pct=50;
			if ( pct>100 )
				pct=100;			
			sprintf(reportStr,"%d",pct);
			Report(reportStr);
		}
	}

	Report("100");

	/**** free memory ****/

	FreeMem(buffer,SCRIPT_LINESIZE);
	CloseParseFile(PR);

	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** E O F ********/
