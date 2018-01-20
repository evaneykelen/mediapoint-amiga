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
extern FILE *logfile;
extern TEXT reportStr[256];

/**** functions ****/

/******** InterpretBigFile() ********/

BOOL InterpretBigFile(STRPTR bigfilePath, BYTE mode, int *systemSize, int *dataSize,
											int *largest, BOOL systemFiles)
{
struct ParseRecord *PR;
int instruc, line, val, numLines, pct;
BOOL validScript=TRUE;
//BOOL headerPassed=FALSE;
BOOL copyIt;
UBYTE *buffer;
TEXT whiteSpcs[512];
TEXT scrStr[512];
TEXT path[SIZE_FULLPATH];
struct FileLock *FL;

	if ( mode==MODE_CALC_SIZE )
	{
		*systemSize = 0;
		*dataSize = 0;
		*largest = 0;
	}

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
					//headerPassed = TRUE;
					PR->sourceLine = line+1;
					if (PR->commandCode == PRINTERROR_CODE)
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
					else
					{
						if ( mode==MODE_CREATE_RUNTIME )
						{
							copyIt = TRUE;
							if ( !systemFiles && (PR->commandCode==SM_SYSTEM || PR->commandCode==SM_FONT) )
								copyIt = FALSE;
							if ( copyIt )
								CopyFilesInBigFile(PR,systemFiles);
						}
						else if ( mode==MODE_FIND_MISSING )
						{
							if ( PR->commandCode==SM_PAGE )
							{
								// Besides every pagetalk document there is now also a
								// pagetalk.TMP file. If both exist then delete the old one
								// and rename the new one to the name of the old one.
								// The BigFile contains the .TMP file name

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
										if ( DeleteFile(scrStr) )
										{
											strcpy(path,scrStr);
											strcat(path,".TMP");	
											Rename(path,scrStr);	// oldname,newname
										}
									}
									else
									{
										if ( logfile )
										{
											//fprintf(logfile,"P3a ");
											fprintf(logfile,scrStr);
											fprintf(logfile,"\n");
										}
									}
								}
							}
						}
						else if ( mode==MODE_CALC_SIZE )
						{
							sscanf(PR->argString[3], "%d", &val);

							if ( val==0 )
							{
								if ( logfile )
								{
									ScriptToStr(PR->argString[1],scrStr);
									RemoveQuotes(scrStr);
									//fprintf(logfile,"P3b ");
									fprintf(logfile,scrStr);
									fprintf(logfile,"\n");
								}
							}

							if ( PR->commandCode==SM_SYSTEM || PR->commandCode==SM_FONT )
								*systemSize = *systemSize + val;
							else
								*dataSize = *dataSize + val;
							if ( val > *largest )
								*largest = val;

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

	//if (!headerPassed)
	//	validScript=FALSE;
	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** E O F ********/
