#include "nb:pre.h"
//#include "msm:protos.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "protos.h"
#include "structs.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;

/**** globals ****/

char *commands[] = {
/* ScriptTalk */
"ANIM", "AREXX", "DOS", "SOUND", "PAGE", "DATA", "MAIL", "XAPP",
/* PageTalk */
"CLIP", "CRAWL", "TEXT", "CLIPANIM",
/* Special Magic */
"FONT", "SYSTEM", "IFF", "SCRIPT",
NULL }; /* ALWAYS END WITH NULL! */

/**** functions ****/

/******** ProcessBigFile() ********/

BOOL ProcessBigFile(struct CDF_Record *CDF_rec)
{
struct ParseRecord *PR;
int instruc, line, numLines, pct;
BOOL validScript=TRUE;
UBYTE *buffer;
TEXT whiteSpcs[512];
TEXT reportStr[256];

	/**** open parser and alloc mem ****/

	PR = (struct ParseRecord *)OpenParseFile(commands,CDF_rec->BigFile_Path);
	if (PR==NULL)
	{
		//printser("Error during BigFile parsing\n");
		return(FALSE);
	}

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
						if ( !DoTransaction(CDF_rec, PR) )
							instruc=PARSE_STOP;
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
