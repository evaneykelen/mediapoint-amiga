#include "nb:pre.h"
#include "protos.h"

#define SCRIPT_LINESIZE 2048L

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct UserApplicInfo UAI;
extern struct EventData CED;
extern struct FileListInfo FLI;
extern TEXT scanDevs[];
extern UWORD chip mypattern1[];
extern struct ScriptFuncs scriptFuncs[];

/**** functions ****/

/******** ParseAScriptFile() ********/

BOOL ParseAScriptFile(STRPTR path, STRPTR fileName, char **scriptCommands)
{
struct ParseRecord *PR;
TEXT fullPath[SIZE_FULLPATH];
int instruc, line;
BOOL validScript=TRUE;
BOOL headerPassed=FALSE;
UBYTE *buffer;
struct ScriptInfoRecord scriptSIR;
TEXT whiteSpcs[512];
BOOL linePrinted;

	UA_MakeFullPath(path, fileName, fullPath);

	/**** init vars ****/

	instruc = -1;
	line = 0;

	/**** alloc mem ****/

	scriptSIR.currentNode = (struct ScriptNodeRecord *)
																	AllocMem(sizeof(struct ScriptNodeRecord), MEMF_CLEAR);
	if (scriptSIR.currentNode==NULL)
		return(FALSE);

	PR = (struct ParseRecord *)OpenParseFile(scriptCommands, fullPath);
	if (PR==NULL)
	{
		FreeMem(scriptSIR.currentNode, sizeof(struct ScriptNodeRecord));
		//Message(msgs[Msg_UnableToReadScript], fileName);
		return(FALSE);
	}

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_CLEAR);
	if ( buffer==NULL )
	{
		CloseParseFile(PR);
		FreeMem(scriptSIR.currentNode, sizeof(struct ScriptNodeRecord));
		//Message(msgs[Msg_UnableToReadScript], fileName);
		return(FALSE);
	}

	/**** parse all lines ****/

	while(instruc!=PARSE_STOP)
	{
		linePrinted=FALSE;
		instruc = Special_GetParserLine((struct ParseRecord *)PR, buffer,whiteSpcs);
		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buffer);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				if (line>=MAX_PARSER_NODES)	// check for too large script
				{
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
				else if (line==0 && PR->commandCode!=TALK_SCRIPTTALK)	/* check for script validity */
				{
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
				else
				{
					if ( PR->commandCode == TALK_SCRIPTTALK )
						headerPassed = TRUE;
					PR->sourceLine = line+1;
					if (	PR->commandCode == TALK_ANIM ||
								PR->commandCode == TALK_AREXX ||
								PR->commandCode == TALK_USERAPPLIC ||
								PR->commandCode == TALK_DOS ||
								PR->commandCode == TALK_SOUND ||
								PR->commandCode == TALK_PAGE ||
								PR->commandCode == TALK_BINARY ||
								PR->commandCode == TALK_MAIL )
					{
						scriptSIR.currentNode->extraData = NULL;

						PerfFunc((struct GenericFuncs *)scriptFuncs, PR, &scriptSIR);

						CheckThisEntry(buffer,&scriptSIR);

						if ( scriptSIR.currentNode->extraData != NULL )
							FreeMem(scriptSIR.currentNode->extraData, scriptSIR.currentNode->extraDataSize);

						linePrinted=TRUE;
					}
					if (PR->commandCode == PRINTERROR_CODE)	/* an error was reported so stop parsing */
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
			else
			{
				/* catch weird command after line 0, line 0 is treated above */
				if (line>0 && PR->commandCode==-1)	/* command not valid */
				{
					validScript=FALSE;
					instruc=PARSE_STOP;
				}
			}
		}

		if ( !linePrinted )
				printf("[%s][%s]\n",whiteSpcs,buffer);

		line++;	/* keeps track of number of parsed lines */
	}

	/**** free memory ****/

	FreeMem(buffer,SCRIPT_LINESIZE);
	CloseParseFile(PR);
	FreeMem(scriptSIR.currentNode, sizeof(struct ScriptNodeRecord));

	if (!headerPassed)
		validScript=FALSE;

	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** Special_GetParserLine() ********/
/* input : pointer to allocated ParseRecord structure, ptr to string
 * output: PARSE_NEXT, PARSE_INTERPRET, PARSE_STOP
 */

int Special_GetParserLine(struct ParseRecord *PR, STRPTR buffer, STRPTR whiteSpcs)
{
int lenStr;
char *strPtr;

	if (feof(PR->filePointer)!=0)
		return(PARSE_STOP);
	buffer[0]='\0';
	fgets(buffer, MAXSCANDEPTH, PR->filePointer);

	whiteSpcs[0] = '\0';

	strPtr = stpblk(buffer);	/* skip leading white spaces */
	lenStr = strlen(strPtr);	/* length of trimmed string, with trailing cr */
//printf("%d %d\n",strlen(buffer),strlen(buffer)-strlen(strPtr));
	if (lenStr>1)							/* skip lines with only a cr */
	{
	stccpy(whiteSpcs,buffer,strlen(buffer)-lenStr+1);
		strcpy(buffer, strPtr);	// survived strcpy
		buffer[lenStr-1] = '\0';
	}

	if (buffer[0]==0x0a || buffer[0]==0x0d || buffer[0]==0x20 || buffer[0]==0x09)
		return(PARSE_NEXT);
	else if (buffer[0]!=0)
		return(PARSE_INTERPRET);

	return(PARSE_STOP);
}

/******** E O F ********/
