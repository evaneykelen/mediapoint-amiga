#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "msm:protos.h"
#include "msm:structs.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern UBYTE **msgs;

/**** globals ****/

char *session_commands[] = { "SCRIPT", "DEST", NULL }; 
struct SessionFuncs { BOOL (*func)(struct ParseRecord *, STRPTR, STRPTR, STRPTR, STRPTR, int *); };
struct SessionFuncs sessionfuncs[] = { { ScriptSessionFunc }, { DestSessionFunc }, };
enum { SESSION_SCRIPT, SESSION_DEST };

/**** functions ****/

/******** ParseSessionFile() ********/

BOOL ParseSessionFile(struct SessionRecord *session_rec)
{
struct ParseRecord *PR;
int instruc, line;
UBYTE *buffer;
TEXT whiteSpcs[512];
BOOL validScript=TRUE;
TEXT s1[SIZE_FULLPATH], s2[SIZE_FULLPATH], s3[SIZE_FULLPATH], s4[SIZE_FULLPATH];
int val;
TEXT sessionPath[SIZE_FULLPATH];
struct ScriptListNode *SLN;
struct DestListNode *DLN;

	NewList(&session_rec->scriptList);

	/**** open parser and alloc mem ****/

	if ( !session_rec->sessionPath[0] || !session_rec->sessionName[0] )
		return(FALSE);

	UA_MakeFullPath(session_rec->sessionPath, session_rec->sessionName, sessionPath);

	PR = (struct ParseRecord *)OpenParseFile(session_commands,sessionPath);
	if (!PR)
		return(FALSE);

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_CLEAR);
	if (!buffer)
	{
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** parse all lines ****/

	instruc = -1;
	line = 0;
	SLN = NULL;

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
					if ( PR->commandCode == SESSION_SCRIPT || PR->commandCode == SESSION_DEST )
					{
						if ( !SessionPerfFunc((struct GenericFuncs *)sessionfuncs,PR,s1,s2,s3,s4,&val) )
						{
							instruc=PARSE_STOP;
							validScript=FALSE;
						}
						else
						{
							if ( PR->commandCode == SESSION_SCRIPT )
							{
								SLN = (struct ScriptListNode *)AllocMem(sizeof(struct ScriptListNode),MEMF_CLEAR|MEMF_ANY);
								if (SLN)
								{
									AddTail(&session_rec->scriptList, (struct Node *)SLN);
									NewList(&SLN->destList);
									strcpy(SLN->scriptPath, s1);
									strcpy(SLN->scriptName, s2);
									SLN->swap = val;
								}
							}
							else if ( SLN && PR->commandCode == SESSION_DEST )
							{
								DLN = (struct DestListNode *)AllocMem(sizeof(struct DestListNode),MEMF_CLEAR|MEMF_ANY);
								if (DLN)
								{
									AddTail(&SLN->destList, (struct Node *)DLN);
									strcpy(DLN->ecpPath, s1);
									strcpy(DLN->ecpName, s2);
									strcpy(DLN->cdfPath, s3);
									strcpy(DLN->cdfName, s4);
								}
							}
						}
					}
					if (PR->commandCode == PRINTERROR_CODE)
					{
						validScript=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
		}
		line++;
	}

	/**** free memory ****/

	FreeMem(buffer,SCRIPT_LINESIZE);
	CloseParseFile(PR);

	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** FreeScriptAndDestLists() ********/

void FreeScriptAndDestLists(struct SessionRecord *session_rec)
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;

	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		// FREE DEST LIST

		dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;	// first node
		while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
		{
			FreeMem(dln_work_node,sizeof(struct DestListNode));
			dln_work_node = dln_next_node;
		}

		// FREE SCRIPT LIST

		FreeMem(sln_work_node,sizeof(struct ScriptListNode));
		sln_work_node = sln_next_node;
	}
}

/******** SessionPerfFunc() ********/

struct GenericFuncs { BOOL (*func)(APTR, APTR, APTR, APTR, APTR, APTR); };

BOOL SessionPerfFunc(	struct GenericFuncs *FuncList,
											struct ParseRecord *PR,
											STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int *val )
{
BOOL (*func)(APTR, APTR, APTR, APTR, APTR, APTR);
	func = FuncList[PR->commandCode].func;
	if (func!=NO_FUNCTION)
		return( (*(func))(PR,s1,s2,s3,s4,val) );
}

/******** ScriptSessionFunc() ********/

BOOL ScriptSessionFunc(	struct ParseRecord *PR, STRPTR s1, STRPTR s2, STRPTR s3, 
												STRPTR s4, int *val )
{
WORD args[MAX_PARSER_ARGS];
TEXT scrStr[512];

	if (PR->numArgs==4)
	{
		GetNumericalArgs(PR,args);

		ScriptToStr(PR->argString[1],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s1, scrStr);

		ScriptToStr(PR->argString[2],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s2, scrStr);

		*val = (int)*(args+2);
	}
	else
		return(FALSE);

	return(TRUE);
}

/******** DestSessionFunc() ********/

BOOL DestSessionFunc(	struct ParseRecord *PR, STRPTR s1, STRPTR s2, STRPTR s3, 
											STRPTR s4, int *val )
{
TEXT scrStr[512];

	if (PR->numArgs==5)
	{
		ScriptToStr(PR->argString[1],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s1, scrStr);

		ScriptToStr(PR->argString[2],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s2, scrStr);

		ScriptToStr(PR->argString[3],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s3, scrStr);

		ScriptToStr(PR->argString[4],scrStr);
		RemoveQuotes(scrStr);
		strcpy(s4, scrStr);
	}
	else
		return(FALSE);

	return(TRUE);
}

/******** SaveSession() ********/

BOOL SaveSession(struct SessionRecord *session_rec, STRPTR fullPath)
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
FILE *fp;

	fp = (FILE *)fopen(fullPath,"w");
	if ( !fp )
		return(FALSE);

	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		fprintf(fp, "%s \"%s\" \"%s\" %d\n", session_commands[SESSION_SCRIPT],
						sln_work_node->scriptPath, sln_work_node->scriptName,
						sln_work_node->swap);

		dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;	// first node
		while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
		{
			fprintf(fp, "%s \"%s\" \"%s\" \"%s\" \"%s\"\n", session_commands[SESSION_DEST],
							dln_work_node->ecpPath, dln_work_node->ecpName,
							dln_work_node->cdfPath, dln_work_node->cdfName);
			dln_work_node = dln_next_node;
		}

		fprintf(fp, "\n");

		sln_work_node = sln_next_node;
	}

	fclose(fp);

	SaveSessionIcon(fullPath);

	return(TRUE);
}

/******** E O F ********/
