/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/***** externals ****/

extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** OpenParseFile() ********/
/* input : pointer to list of command, filename
 * output: pointer to allocated ParseRecord structure, NULL on error
 */

struct ParseRecord *OpenParseFile(char **cmdList, STRPTR path)
{
FILE *fp;
struct ParseRecord *PR;

	fp = (FILE *)fopen(path, "r");
	if (fp == NULL)
		return(NULL);

	PR = (struct ParseRecord *)OpenParser();
	if (PR == NULL)
	{
		fclose(fp);
		return(NULL);
	}

	PR->commandList = (char **)cmdList;
	PR->filePointer = (FILE *)fp;

	return((struct ParseRecord *)PR);
}

/******** CloseParseFile() ********/
/* input : pointer to allocated ParseRecord structure
 * output: -
 */

void CloseParseFile(struct ParseRecord *PR)
{
	fclose((FILE *)PR->filePointer);
	CloseParser((struct ParseRecord *)PR);
}	

/******** OpenParser() ********/
/* input : -
 * output: pointer to allocated ParseRecord structure, NULL on error
 */

struct ParseRecord *OpenParser(void)
{
int i;
struct ParseRecord *PR;

	PR = (struct ParseRecord *)AllocMem((LONG)sizeof(struct ParseRecord),
																			MEMF_ANY | MEMF_CLEAR);
	if (PR==NULL)
	{
		UA_WarnUser(86);
		return(NULL);
	}

	for(i=0; i<MAX_PARSER_ARGS; i++)
	{
		PR->argString[i] = (char *)AllocMem((LONG)MAXSCANDEPTH, MEMF_ANY | MEMF_CLEAR);
		if (PR->argString[i] == NULL)
		{
			UA_WarnUser(87);
			for(i=0; i<MAX_PARSER_ARGS; i++)
			{
				if (PR->argString[i] != NULL)
					FreeMem(PR->argString[i], (LONG)MAXSCANDEPTH);
				PR->argString[i] = NULL;
			}
			return(NULL);
		}
	}

	return((struct ParseRecord *)PR);
}

/******** CloseParser() ********/
/* input : pointer to allocated ParseRecord structure
 * output: -
 */

void CloseParser(struct ParseRecord *PR)
{
int i;

	if (PR==NULL)
		return;	/* They don't fool me */

	for(i=0; i<MAX_PARSER_ARGS; i++)
	{
		if (PR->argString[i] != NULL)
			FreeMem(PR->argString[i], (LONG)MAXSCANDEPTH);
		PR->argString[i] = NULL;
	}

	FreeMem(PR, (LONG)sizeof(struct ParseRecord));
}

/******** GetParserLine() ********/
/* input : pointer to allocated ParseRecord structure, ptr to string
 * output: PARSE_NEXT, PARSE_INTERPRET, PARSE_STOP
 */

int GetParserLine(struct ParseRecord *PR, STRPTR buffer)
{
int lenStr;
char *strPtr;

	if (feof(PR->filePointer)!=0)
		return(PARSE_STOP);
	buffer[0]='\0';
	fgets(buffer, MAXSCANDEPTH, PR->filePointer);

	strPtr = stpblk(buffer);	/* skip leading white spaces */
	lenStr = strlen(strPtr);	/* length of trimmed string, with trailing cr */
	if (lenStr>1)							/* skip lines with only a cr */
	{
		strcpy(buffer, strPtr);	// survived strcpy
		buffer[lenStr-1] = '\0';
	}

	if (buffer[0]==0x0a || buffer[0]==0x0d || buffer[0]==0x20 || buffer[0]==0x09)
		return(PARSE_NEXT);
	else if (buffer[0]!=0)
		return(PARSE_INTERPRET);

	return(PARSE_STOP);
}

/******** passOneParser() ********/
/* input : ptr to allocated Parser structure, ptr to one line of source
 * output: -
 */

void passOneParser(struct ParseRecord *PR, char *buffer)
{
int numChars, argNum, len;
char *strPtr;

	strPtr = buffer;	/* save pointer */
	len = strlen(buffer);

	argNum=0;
	while(argNum < MAX_PARSER_ARGS)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	/* space, tab, comma */
		if (numChars>=1)
		{
			stccpy(PR->argString[argNum], strPtr, numChars+1);
			argNum++;
		}
		strPtr += numChars+1;
	}
	PR->numArgs = argNum;
}

/******** passTwoParser() ********/
/* input:  pointer to allocated Parser structure
 * output: TRUE or FALSE depending if first word of source was understood
 */

BOOL passTwoParser(struct ParseRecord *PR)
{
	PR->commandCode = GetCommandID((struct ParseRecord *)PR);
	if (PR->commandCode==-1)
		return(FALSE);
	else
		return(TRUE);
}

/******** GetCommandID() ********/
/* input:  pointer to allocated Parser structure
 * output: parsed keyword ID, -1 on not found
 */

int GetCommandID(struct ParseRecord *PR)
{
int i=0;
	
	while(PR->commandList[i]!=NULL)	/* commandList guaranteed to end with NULL */
	{
		if (stricmp(PR->argString[0], PR->commandList[i])==0)
			return(i);	/* command set starts with command ID 1 */
		i++;
	}
	return(-1);
}

/******** GetNumericalArgs() ********/
/*
 * input:  pointer to allocated Parser structure, pointer to array of WORDs
 * output: -
 * function: converts all arguments to WORDs
 */

void GetNumericalArgs(struct ParseRecord *PR, WORD *args)
{
int i,val;

	for(i=0; i<PR->numArgs; i++)
	{
		sscanf(PR->argString[i+1], "%d", &val);
		args[i] = (WORD)val;
	}
}

/******** printError() ********/
/* input:    pointer to a ParseRecord structure, pointer to additional
 *           text.
 * output:   -
 */

void printError(struct ParseRecord *PR, STRPTR str)
{
	if (PR!=NULL && PR->commandCode!=-1)
		Message(msgs[Msg_SyntaxErrorInLine-1], PR->sourceLine, PR->commandList[PR->commandCode], str);
	else
		Message(msgs[Msg_SyntaxErrorInScript-1], str);

	if (PR!=NULL)
		PR->commandCode = PRINTERROR_CODE;
}

/******** StrToScript() ********/
/*
 * Converts "foo "bar" we'll"" to "foo \"bar"\ we'll\""
 *
 */

void StrToScript(char *oldStr, char *newStr)
{
int i,j,len;

	*newStr='\0';
	len = strlen(oldStr);
	if ( len==0 )
		return;	
	i=0;
	j=0;
	while( i<len && *(oldStr+i)!='\0' )
	{
		if ( *(oldStr+i) == '\"' )
		{
			*(newStr+j) 	= '\\';
			*(newStr+j+1) = '\"';
			j=j+2;
		}
		else
		{
			*(newStr+j) = *(oldStr+i);
			j++;
		}
		i++;
	}
	*(newStr+j) = '\0';
}

/******** ScriptToStr() ********/
/*
 * Converts "foo \"bar"\ we'll\"" to "foo "bar" we'll""
 *
 */

void ScriptToStr(char *oldStr, char *newStr)
{
int i,j,len;

	*newStr='\0';
	len = strlen(oldStr);
	if ( len==0 )
		return;	
	i=0;
	j=0;
	while( i<len && *(oldStr+i)!='\0' )
	{
		if ( *(oldStr+i) == '\\' && *(oldStr+i+1) == '\"' )
		{
			*(newStr+j) = '\"';
			i=i+2;
			j++;
		}
		else
		{
			*(newStr+j) = *(oldStr+i);
			i++;
			j++;
		}
	}
	*(newStr+j) = '\0';
}

/******** Special_GetParserLine() ********/
/*
 * input : pointer to allocated ParseRecord structure, ptr to string
 * output: PARSE_NEXT, PARSE_INTERPRET, PARSE_STOP
 *
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

	if (lenStr>1)							/* skip lines with only a cr */
	{
		stccpy(whiteSpcs,buffer,strlen(buffer)-lenStr+1);
		strcpy(buffer, strPtr);
		buffer[lenStr-1] = '\0';
	}

	if (buffer[0]==0x0a || buffer[0]==0x0d || buffer[0]==0x20 || buffer[0]==0x09)
		return(PARSE_NEXT);
	else if (buffer[0]!=0)
		return(PARSE_INTERPRET);

	return(PARSE_STOP);
}

/******** E O F ********/
