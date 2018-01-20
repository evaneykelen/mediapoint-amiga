/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

/*********************************************************************************/
/**** remarks: because dir_xapps is not used when this module is used outside	****/
/**** MP, the userapplic info file can't be sniffed for max mem. It is put to	****/
/**** 8192 here. I hope this is enough, even for future (large) memory hungry	****/
/**** xapps.																																	****/
/*********************************************************************************/

#include "nb:pre.h"

#define USED_FOR_MP TRUE

/**** externals ****/

#if USED_FOR_MP
extern UBYTE *objectNameList[];
extern UBYTE **msgs;   
extern struct CapsPrefs CPrefs;
extern TEXT *dir_xapps;							// also used if this module is used separately
extern struct Library *medialinkLibBase;
#else
struct CapsPrefs CPrefs;	// dummy one
#endif
extern TEXT mainName[];

/**** GLOBALS ****/

	/**** SEE ALSO MPRI:PRINT_SEG.C !!!!!!!!! ****/

char *scriptCommands[] = {
"ANIM", "AREXX", "XAPP", "DOS", "STARTSER", "ENDSER", "STARTPAR", "ENDPAR",
"SOUND", "START", "END", "DURATION", "PROGRAM", "SCRIPTTALK", "PAGE", "COUNTER",
"GOTO", "DATA", "MAIL", "GLOBALEVENT", "TIMECODE", "LABEL", "NOP", "LOCALEVENT",
"VARIABLES", "INPUTSETTINGS",
NULL }; /* ALWAYS END WITH NULL! */

struct ScriptFuncs scriptFuncs[] =
{ 
	{ AnimFunc				},	{ ArexxDosFunc		}, 	{ UserApplicFunc	},
	{ ArexxDosFunc		}, 	{ StartSerFunc		}, 	{ EndSerFunc			},
	{ StartParFunc		}, 	{ EndParFunc			}, 	{ SoundFunc				},
	{ StartFunc				}, 	{ EndFunc					}, 	{ DurationFunc		},
	{ ProgramFunc			}, 	{ TalkFunc				}, 	{ PageFunc				},
	{ CounterFunc			}, 	{ GotoFunc				}, 	{ BinaryFunc			},
	{ MailFunc				}, 	{ GlobalEventFunc	}, 	{ TimeCodeFunc		},
	{ LabelFunc				}, 	{ NopFunc					}, 	{ LocalEventFunc	},
	{ VarsFunc				},  { InputSettingsFunc },
};

static TEXT scrStr[512];

/**** FUNCTIONS ****/

/******** AnimFunc() ********/

void AnimFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;
WORD args[MAX_PARSER_ARGS];

/* ANIM "path", cycle, rot, speed, diskanim, loopIt, wipe, speed, thick, vari */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs==11)
	{
		GetNumericalArgs(PR, args);

		node->nodeType = PR->commandCode;

		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		UA_SplitFullPath(scrStr, node->objectPath, node->objectName);

		/* scriptTiming = 0 for defer and 1 for continu */
		node->numericalArgs[0] = CPrefs.scriptTiming+1;

		node->numericalArgs[2]	= *(args+6);		// wipe
		node->numericalArgs[3]	= *(args+7);		// wipe
		node->numericalArgs[4]	= *(args+8);		// wipe
		node->numericalArgs[5]	= *(args+9);		// wipe

		node->numericalArgs[9]	= *(args+3);		// play speed  
		node->numericalArgs[10] = *(args+2);		// rot

		node->numericalArgs[11] = 0;

		if (strcmpi(&PR->argString[5][0], "ON")==0)	// disk anim 
			node->numericalArgs[11] = 2;

		if (strcmpi(&PR->argString[6][0], "ON")==0)	// loop it
			node->numericalArgs[11] |= 1;

		if (strcmpi(&PR->argString[2][0], "ON")==0)	// color cycle
			node->numericalArgs[12] = 1;
		else
			node->numericalArgs[12] = 0;
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** ArexxDosFunc() ********/

void ArexxDosFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;
WORD args[MAX_PARSER_ARGS];

	/*  AREXX "port name", 'commandstring', "comment", mode, type  or  */
	/*  AREXX "path", "comment", mode, type  */

	/*  DOS 'commandstring', "comment", mode, type, stack  or  */
	/*  DOS "path", "comment", mode, type, stack  */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs>=5 && PR->numArgs<=6)
	{
		GetNumericalArgs(PR, args);

		node->nodeType = PR->commandCode;

		if ( node->nodeType == TALK_DOS )
		{
			node->extraData = AllocMem(DOS_MEMSIZE, MEMF_ANY | MEMF_CLEAR);
#if USED_FOR_MP
			if (node->extraData==NULL)
				UA_WarnUser(192);
#endif

			node->extraDataSize = DOS_MEMSIZE;

			if ( strcmpi(&PR->argString[4][0], "SCRIPT")==0 )
			{
				if (strcmpi(&PR->argString[3][0], "DEFER")==0)
					node->numericalArgs[0] = ARGUMENT_DEFER;
				else if (strcmpi(&PR->argString[3][0], "CONTINUE")==0)
					node->numericalArgs[0] = ARGUMENT_CONTINUE;
				node->numericalArgs[1] = ARGUMENT_SCRIPT;
				ScriptToStr(PR->argString[1], scrStr);
				UA_SplitFullPath(scrStr, node->objectPath, node->objectName);
				ScriptToStr(PR->argString[2], scrStr);
				stccpy(node->extraData, scrStr, node->extraDataSize);	/* comment */
				RemoveQuotes(node->extraData);
			}
			else if ( strcmpi(&PR->argString[4][0], "COMMAND")==0 )
			{
				if (strcmpi(&PR->argString[3][0], "DEFER")==0)
					node->numericalArgs[0] = ARGUMENT_DEFER;
				else if (strcmpi(&PR->argString[3][0], "CONTINUE")==0)
					node->numericalArgs[0] = ARGUMENT_CONTINUE;
				node->numericalArgs[1] = ARGUMENT_COMMAND;
				ScriptToStr(PR->argString[1], scrStr);
				stccpy(node->objectPath, scrStr, MAX_PARSER_CHARS);	/* command */
				RemoveQuotes(node->objectPath);
				ScriptToStr(PR->argString[2], scrStr);
				stccpy(node->extraData, scrStr, node->extraDataSize);	/* comment */
				RemoveQuotes(node->extraData);
			}

			if ( PR->numArgs==6 )	// stack
				node->numericalArgs[2] = *(args+4);
		}
		else if ( node->nodeType == TALK_AREXX )
		{
			node->extraData = AllocMem(AREXX_MEMSIZE, MEMF_ANY | MEMF_CLEAR);
#if USED_FOR_MP
			if (node->extraData==NULL)
				UA_WarnUser(193);
#endif

			node->extraDataSize = AREXX_MEMSIZE;

			if ( strcmpi(&PR->argString[4][0], "SCRIPT")==0 )
			{
				if (strcmpi(&PR->argString[3][0], "DEFER")==0)
					node->numericalArgs[0] = ARGUMENT_DEFER;
				else if (strcmpi(&PR->argString[3][0], "CONTINUE")==0)
					node->numericalArgs[0] = ARGUMENT_CONTINUE;
				node->numericalArgs[1] = ARGUMENT_SCRIPT;
				ScriptToStr(PR->argString[1], scrStr);
				UA_SplitFullPath(scrStr, node->objectPath, node->objectName);
				ScriptToStr(PR->argString[2], scrStr);
				stccpy(node->extraData, scrStr, node->extraDataSize);	/* comment */
				RemoveQuotes(node->extraData);
			}
			else if ( strcmpi(&PR->argString[5][0], "COMMAND")==0 )
			{
				if (strcmpi(&PR->argString[4][0], "DEFER")==0)
					node->numericalArgs[0] = ARGUMENT_DEFER;
				else if (strcmpi(&PR->argString[4][0], "CONTINUE")==0)
					node->numericalArgs[0] = ARGUMENT_CONTINUE;
				node->numericalArgs[1] = ARGUMENT_COMMAND;
				ScriptToStr(PR->argString[1], scrStr);
				strcpy(&node->extraData[50], scrStr);	/* survived strcpy port name */
				RemoveQuotes(&node->extraData[50]);
				ScriptToStr(PR->argString[2], scrStr);
				stccpy(node->objectPath, scrStr, MAX_PARSER_CHARS);	/* command */
				RemoveQuotes(node->objectPath);
				ScriptToStr(PR->argString[3], scrStr);
				stccpy(node->extraData, scrStr, 50);	/* comment */
				RemoveQuotes(node->extraData);
			}
		}
#if USED_FOR_MP
		else
			printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** BinaryFunc() ********/

void BinaryFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;

	/*  BINARY "path"  */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs==2)
	{
		node->nodeType = PR->commandCode;
		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		UA_SplitFullPath(scrStr, node->objectPath, node->objectName);
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** UserApplicFunc() ********/

void UserApplicFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;
int i, memSize;
TEXT fullPath[SIZE_FULLPATH];
WORD args[MAX_PARSER_ARGS];

	/*  USERAPPLIC "tool name", "comment", ['argument string'], [defer/cont]  */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs>=3 || PR->numArgs<=5 )
	{
		GetNumericalArgs(PR, args);

		node->numericalArgs[0] = 1;	// DEFER

		node->nodeType = PR->commandCode;

		ScriptToStr(PR->argString[1], scrStr);
		stccpy(node->objectPath, scrStr, MAX_PARSER_CHARS);	/* tool name */

		// START NEW - SOME XAPP NAMES HAVE CHANGED

		if ( !strcmpi(node->objectPath,"\"genlock\"") )
			strcpy(node->objectPath,"\"G_Genlock\"");

		if ( !strcmpi(node->objectPath,"\"neptun\"") )
			strcpy(node->objectPath,"\"G_Neptun\"");

		if ( !strcmpi(node->objectPath,"\"studio16\"") )
			strcpy(node->objectPath,"\"S_Studio16\"");

		if ( !strcmpi(node->objectPath,"\"toccata\"") )
			strcpy(node->objectPath,"\"S_Toccata\"");

		// END NEW

		ScriptToStr(PR->argString[2], scrStr);
		stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);	/* comment */

		RemoveQuotes(node->objectPath);
		RemoveQuotes(node->objectName);

#if USED_FOR_MP
		UA_MakeFullPath(dir_xapps, node->objectPath, fullPath);
		GetMemSize(fullPath, &memSize);
#else
		memSize = 8192;
#endif

		node->extraData = AllocMem(memSize, MEMF_ANY | MEMF_CLEAR);
		node->extraDataSize = memSize;

#if USED_FOR_MP
		if (node->extraData==NULL)
				UA_WarnUser(194);
#endif

		if (PR->numArgs>=4)
		{
			strcpy(node->extraData, PR->argString[3]);	/* survived strcpy argument string */
			RemoveQuotes(node->extraData);
		}

		if (PR->numArgs==5)
		{
			node->numericalArgs[0] = *(args+3);
		}		

#if USED_FOR_MP
		/**** the list scroller needs to find the XaPP icons fast.			****/
		/**** In order to do so, I (mis)use a numericalArgs field to		****/
		/**** store a value which points to where the scroller can find ****/
		/**** the icon.																									****/

		i=0;
		while ( objectNameList[i] != NULL )
		{
			if (strcmpi(objectNameList[i],node->objectPath)==0)
			{
				node->numericalArgs[MAX_PARSER_ARGS-1] = i;
				break;
			}
			i++;
		}
#endif
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** LabelFunc() ********/

void LabelFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;

	/*  LABEL "string"  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	if (PR->numArgs==2)
	{
		node->nodeType = PR->commandCode;
		ScriptToStr(PR->argString[1], scrStr);
		stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
		RemoveQuotes(node->objectName);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** NopFunc() ********/

void NopFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;

	/*  NOP "string"  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	node->nodeType = PR->commandCode;

	if (PR->numArgs==2)
	{
		ScriptToStr(PR->argString[1], scrStr);
		stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
		RemoveQuotes(node->objectName);
	}
#endif
}

/******** MailFunc() ********/

void MailFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;

	/*  MAIL "path"  */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs==2)
	{
		node->nodeType = PR->commandCode;
		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		UA_SplitFullPath(scrStr, node->objectPath, node->objectName);
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** StartSerFunc() ********/
/* creates a new list, adds it to the array of lists, creates a new node
 * and stores the pointer to the list in this node.
 */

void StartSerFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
int i;
struct ScriptNodeRecord *node; 

	if (SIR->listType == TALK_STARTPAR)
	{
		if (PR->commandCode == TALK_STARTSER)
		{
			printError(PR, msgs[Msg_SerialWithinParallel-1]);
			return;
		}
	}
	else
	{
		if (SIR->currentList != NULL)
		{
			/* Add STARTSER to current list and make it the current node */
			/* but at the same time create a new list to which all new */
			/* nodes will be attached */

			node = (struct ScriptNodeRecord *)AddNode(SIR->currentList);
			if (node != NULL)
			{
				if ( CreateNewList(SIR) )
				{
					SIR->currentNode = node;
					SIR->listType = PR->commandCode;	/* STARTSER */
					node->nodeType = PR->commandCode;	/* STARTSER */
					node->list = SIR->currentList;		/* store pointer to sub list ! */

					if (PR->numArgs==2)
					{
						ScriptToStr(PR->argString[1], scrStr);
						stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
						RemoveQuotes(node->objectName);
					}

					for(i=0; i<CPrefs.MaxNumLists; i++)
					{
						if (SIR->lists[i] == NULL)
						{
							SIR->lists[i] = SIR->currentList;
							break;
						}
					}

					if ( mainName[0]=='\0' )
						strcpy(mainName,node->objectName);
				}
			}
		}
		else if ( CreateNewList(SIR) )	/* no list available yet */
		{
			for(i=0; i<CPrefs.MaxNumLists; i++)
			{
				if (SIR->lists[i] == NULL)
				{
					SIR->lists[i] = SIR->currentList;
					node = (struct ScriptNodeRecord *)AddNode(SIR->currentList);
					if (node != NULL)
					{
						node->nodeType = PR->commandCode;
						SIR->currentNode = node;
						SIR->listType = PR->commandCode;

						if (PR->numArgs==2)
						{
							ScriptToStr(PR->argString[1], scrStr);
							stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
							RemoveQuotes(node->objectName);
						}
					}
					break;
				}
			}
		}
	}
#endif
}

/******** EndSerFunc() ********/
/*
 * looks up the new 'currentList' and empties the 'currentNode'.
 *
 */

void EndSerFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
int i;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->lists[i] == SIR->currentList)
		{
			if (i>=1)
			{
				SIR->lists[i] = NULL;
				SIR->currentList = SIR->lists[i-1];
			}
			SIR->currentNode = NULL;
			break;
		}
	}
#endif
}

/******** StartParFunc() ********/
/*
 * creates a new list, adds it to the array of lists, creates a new node
 * and stores the pointer to the list in this node.
 *
 */

void StartParFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
int i;
struct ScriptNodeRecord *node; 

	if (SIR->listType == TALK_STARTPAR)
	{
		if (PR->commandCode == TALK_STARTPAR)
		{
			printError(PR, msgs[Msg_ParallelWithinParallel-1]);
			return;
		}
	}
	else
	{
		if (SIR->currentList != NULL)
		{
			/* Add STARTPAR to current list and make it the current node */
			/* but at the same time create a new list to which all new */
			/* nodes will be attached */

			node = (struct ScriptNodeRecord *)AddNode(SIR->currentList);
			if (node != NULL)
			{
				if ( CreateNewList(SIR) )
				{
					SIR->currentNode = node;
					SIR->listType = PR->commandCode;	/* STARTPAR */
					node->nodeType = PR->commandCode;	/* STARTPAR */
					node->list = SIR->currentList;		/* store pointer to sub list ! */

					if (PR->numArgs==2)
					{
						ScriptToStr(PR->argString[1], scrStr);
						stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
						RemoveQuotes(node->objectName);
					}

					for(i=0; i<CPrefs.MaxNumLists; i++)
					{
						if (SIR->lists[i] == NULL)
						{
							SIR->lists[i] = SIR->currentList;
							break;
						}
					}
				}
			}
		}
		else if ( CreateNewList(SIR) )	/* no list available yet */
		{
			for(i=0; i<CPrefs.MaxNumLists; i++)
			{
				if (SIR->lists[i] == NULL)
				{
					SIR->lists[i] = SIR->currentList;
					node = (struct ScriptNodeRecord *)AddNode(SIR->currentList);
					if (node != NULL)
					{
						node->nodeType = PR->commandCode;
						SIR->currentNode = node;
						SIR->listType = PR->commandCode;

						if (PR->numArgs==2)
						{
							ScriptToStr(PR->argString[1], scrStr);
							stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
							RemoveQuotes(node->objectName);
						}
					}
					break;
				}
			}
		}
	}
#endif
}

/******** EndParFunc() ********/
/* looks up the new 'currentList' and empties the 'currentNode'.
 */

void EndParFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
int i;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->lists[i] == SIR->currentList)
		{
			SIR->lists[i] = NULL;
			if (i>=1)
				SIR->currentList = SIR->lists[i-1];
			SIR->currentNode = NULL;
			SIR->listType = -1;
			break;
		}
	}
#endif
}

/******** SoundFunc() ********/

void SoundFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
WORD args[MAX_PARSER_ARGS];
struct ScriptNodeRecord *node;

	/*  SOUND "path", STOP/WAIT/LOOP  */

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs==3)
	{
		GetNumericalArgs(PR, args);

		node->nodeType = PR->commandCode;

		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		UA_SplitFullPath(scrStr, node->objectPath, node->objectName);

		if (strcmpi(&PR->argString[2][0], "STOP")==0)
		{
			node->numericalArgs[0] = 1;	// 2; ???????????
			node->numericalArgs[1] = 1;
		}		
		else if (strcmpi(&PR->argString[2][0], "WAIT")==0)
		{
			node->numericalArgs[0] = 1;
			node->numericalArgs[1] = 0;
		}		
		else if (strcmpi(&PR->argString[2][0], "LOOP")==0)
		{
			node->numericalArgs[0] = 2;
			node->numericalArgs[1] = 0;
		}		
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** StartFunc() ********/
/* adds a node to the 'currentList' and makes this node the 'currentNode'.
 */

void StartFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;

	if (SIR->currentList != NULL)
	{
		node = (struct ScriptNodeRecord *)AddNode(SIR->currentList);
		if (node != NULL)
			SIR->currentNode = node;
	}
#endif
}

/******** EndFunc() ********/
/* declares 'currentNode' closed by zeroing the currentNode pointer.
 */

void EndFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
	SIR->currentNode = NULL;
#endif
}

/******** DurationFunc() ********/

void DurationFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;

	/*  DURATION hh:mm:ss:t  */
	/*  DURATION hh:mm:ss:t hh:mm:ss:t  */	// parallel branch objects use this

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	if (PR->numArgs==2)
		DurationStringToSeconds(&PR->argString[1][0], &node->duration);
	else if (PR->numArgs==3)
	{
		DurationStringToSeconds(&PR->argString[1][0], &node->Start.ParHMSTOffset);
		DurationStringToSeconds(&PR->argString[2][0], &node->End.ParHMSTOffset);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);

#endif
}

/******** ProgramFunc() ********/

void ProgramFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
char *strPtr;
int numChars;
int bits[] = { 1,2,4,8,16,32,64 };
struct ScriptNodeRecord *node;
int a,b,c,d;

	/*  PROGRAM startTime,endTime  */

	/*  PROGRAM SU|MO|TU|WE|TH|FR|SA,startDay,endDay,startTime,endTime,mode  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	strPtr = PR->argString[1];
	node->dayBits = 0;

	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, "|");
		if (numChars==2)
		{
			if (strnicmp(strPtr, "SU", 2)==0)
				node->dayBits |= bits[0];
			else if (strnicmp(strPtr, "MO", 2)==0)
				node->dayBits |= bits[1];
			else if (strnicmp(strPtr, "TU", 2)==0)
				node->dayBits |= bits[2];
			else if (strnicmp(strPtr, "WE", 2)==0)
				node->dayBits |= bits[3];
			else if (strnicmp(strPtr, "TH", 2)==0)
				node->dayBits |= bits[4];
			else if (strnicmp(strPtr, "FR", 2)==0)
				node->dayBits |= bits[5];
			else if (strnicmp(strPtr, "SA", 2)==0)
				node->dayBits |= bits[6];
		}
		else
			break;
		strPtr += numChars+1;
	}

	if (PR->numArgs == 3)	/* HH:MM:SS:FF */
	{
		timeStringtoTimeCode(	PR->argString[1], &a, &b, &c, &d);
		node->Start.TimeCode.HH = (BYTE)a;
		node->Start.TimeCode.MM = (BYTE)b;
		node->Start.TimeCode.SS = (BYTE)c;
		node->Start.TimeCode.FF = (BYTE)d;
		timeStringtoTimeCode(PR->argString[2], &a, &b, &c, &d);
		node->End.TimeCode.HH = (BYTE)a;
		node->End.TimeCode.MM = (BYTE)b;
		node->End.TimeCode.SS = (BYTE)c;
		node->End.TimeCode.FF = (BYTE)d;
	}
	else if (PR->numArgs == 7)	/* HH:MM:SS:T */
	{
		dateStringtoDays(PR->argString[2], (int *)&node->Start.HHMMSS.Days);
		dateStringtoDays(PR->argString[3], (int *)&node->End.HHMMSS.Days);
		timeStringtoMinutesAndTicks(PR->argString[4], &a, &b);
		node->Start.HHMMSS.Minutes = (LONG)a;
		node->Start.HHMMSS.Ticks = (LONG)b;
		timeStringtoMinutesAndTicks(PR->argString[5], &a, &b);
		node->End.HHMMSS.Minutes = (LONG)a;
		node->End.HHMMSS.Ticks = (LONG)b;
		if (strcmpi(&PR->argString[6][0], "CYCLICAL")==0)
			node->startendMode = ARGUMENT_CYCLICAL;
		else if (strcmpi(&PR->argString[6][0], "CONTINUOUS")==0)
			node->startendMode = ARGUMENT_CONTINUOUS;
		else
			node->startendMode = ARGUMENT_CYCLICAL;	// Thanks to Chris - hopefully fixed elsewhere

	}
#endif
}

/******** TalkFunc() ********/

void TalkFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
WORD args[MAX_PARSER_ARGS];

	if (PR->numArgs==3)
	{
		GetNumericalArgs(PR, args);
		SIR->revision = (int)*(args+0);	// now used for user level
		SIR->version = (int)*(args+1);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** PageFunc() ********/

void PageFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *node;
WORD args[MAX_PARSER_ARGS];

	/*  PAGE "path", cycle, wipe, speed, thick, vari, type 							*/
	/*																								1 = pagetalk doc	*/
	/*																								2 = IFF ILBM 			*/

	node = SIR->currentNode;
	if (node==NULL)
	{
#if USED_FOR_MP
		printError(PR, msgs[Msg_MissesStart-1]);
#endif
		return;
	}

	if (PR->numArgs==8)
	{
		GetNumericalArgs(PR, args);

		node->nodeType = PR->commandCode;

		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		UA_SplitFullPath(scrStr, node->objectPath, node->objectName);

		/* scriptTiming = 0 for defer and 1 for continu */
		node->numericalArgs[0]	= CPrefs.scriptTiming+1;

		node->numericalArgs[2]	= *(args+2);		// wipe
		node->numericalArgs[3]	= *(args+3);		// wipe
		node->numericalArgs[4]	= *(args+4);		// wipe
		node->numericalArgs[5]	= *(args+5);		// wipe

		node->numericalArgs[15]	= *(args+6);		// type

		if (strcmpi(&PR->argString[2][0], "ON")==0)
			node->numericalArgs[12] = 1;
		else
			node->numericalArgs[12] = 0;
	}
#if USED_FOR_MP
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** GotoFunc() ********/

void GotoFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;

	/*  GOTO "string"  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	if (PR->numArgs==2)
	{
		node->nodeType = PR->commandCode;
		ScriptToStr(PR->argString[1], scrStr);
		stccpy(node->objectName, scrStr, MAX_OBJECTNAME_CHARS);
		RemoveQuotes(node->objectName);

		node->numericalArgs[0] = TG_GOTO;
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** GlobalEventFunc() ********/

void GlobalEventFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
int i;
struct ScriptEventRecord *SER;

	if (PR->numArgs==3)
	{
		i=0;
		while(i<MAX_GLOBAL_EVENTS)
		{
			if (SIR->globallocalEvents[i] == NULL)
			{
				SER = (struct ScriptEventRecord *)
					AllocMem(sizeof(struct ScriptEventRecord), MEMF_ANY | MEMF_CLEAR);
				if (SER!=NULL)
				{
					SIR->globallocalEvents[i] = (struct ScriptEventRecord *)SER;
					if(PR->argString[1][0]=='\"')	/* keycode */
					{
						SER->keyCode = (int)PR->argString[1][1];
						SER->rawkeyCode = -1;
					}
					else
					{
						SER->keyCode = -1;
						if (strcmpi(&PR->argString[1][0], TALK_HELP_KT)==0)
							SER->rawkeyCode = TALK_HELP_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_ESC_KT)==0)
							SER->rawkeyCode = TALK_ESC_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F1_KT)==0)
							SER->rawkeyCode = TALK_F1_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F2_KT)==0)
							SER->rawkeyCode = TALK_F2_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F3_KT)==0)
							SER->rawkeyCode = TALK_F3_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F4_KT)==0)
							SER->rawkeyCode = TALK_F4_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F5_KT)==0)
							SER->rawkeyCode = TALK_F5_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F6_KT)==0)
							SER->rawkeyCode = TALK_F6_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F7_KT)==0)
							SER->rawkeyCode = TALK_F7_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F8_KT)==0)
							SER->rawkeyCode = TALK_F8_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F9_KT)==0)
							SER->rawkeyCode = TALK_F9_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_F10_KT)==0)
							SER->rawkeyCode = TALK_F10_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_CURSORUP_KT)==0)
							SER->rawkeyCode = TALK_CURSORUP_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_CURSORDOWN_KT)==0)
							SER->rawkeyCode = TALK_CURSORDOWN_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_CURSORLEFT_KT)==0)
							SER->rawkeyCode = TALK_CURSORLEFT_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_CURSORRIGHT_KT)==0)
							SER->rawkeyCode = TALK_CURSORRIGHT_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_TAB_KT)==0)
							SER->rawkeyCode = TALK_TAB_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_DEL_KT)==0)
							SER->rawkeyCode = TALK_DEL_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_BACKSPACE_KT)==0)
							SER->rawkeyCode = TALK_BACKSPACE_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_RETURN_KT)==0)
							SER->rawkeyCode = TALK_RETURN_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_SPACE_KT)==0)
							SER->rawkeyCode = TALK_SPACE_KC;

						else if (strcmpi(&PR->argString[1][0], TALK_OPEN_BRACKET_KT)==0)
							SER->rawkeyCode = TALK_OPEN_BRACKET_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_CLOSE_BRACKET_KT)==0)
							SER->rawkeyCode = TALK_CLOSE_BRACKET_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_STAR_KT)==0)
							SER->rawkeyCode = TALK_STAR_KC;
						else if (strcmpi(&PR->argString[1][0], TALK_PLUS_KT)==0)
							SER->rawkeyCode = TALK_PLUS_KC;

						else
							SER->rawkeyCode = TALK_ESC_KC;
					}
					ScriptToStr(PR->argString[2], scrStr);
					stccpy(SER->labelName, scrStr, MAX_PARSER_CHARS);
					RemoveQuotes(SER->labelName);
					SER->labelSNR = (struct ScriptNodeRecord *)FindLabel(SIR, SER->labelName);
				}
				else
					UA_WarnUser(195);

				break;
			}
			i++;
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** TimeCodeFunc() ********/

void TimeCodeFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;
int a,b,c,d,offset;
WORD args[MAX_PARSER_ARGS];

	/*  TIMECODE INTERNAL, HHMMSS, timing, preload, playOpts, bufferOpts  */
	/*  TIMECODE INTERNAL, MIDI, FPS, sendOut, offset,
							 timing, preload, playOpts, bufferOpts, [custom driver]  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	if (PR->numArgs==6 || PR->numArgs==7 || PR->numArgs==9 || PR->numArgs==10 ||
			PR->numArgs==11 )
	{
		GetNumericalArgs(PR, args);

		node->nodeType = PR->commandCode;

		stccpy(node->objectName, msgs[Msg_TimeCode-1], MAX_OBJECTNAME_CHARS);

		if (strcmpi(&PR->argString[1][0], "EXTERNAL")==0)
			SIR->timeCodeSource = TIMESOURCE_EXTERNAL;
		else
			SIR->timeCodeSource = TIMESOURCE_INTERNAL;

		if (strcmpi(&PR->argString[2][0], "HHMMSS")==0)
			SIR->timeCodeFormat = TIMEFORMAT_HHMMSS;
		else if (strcmpi(&PR->argString[2][0], "MIDI")==0)
			SIR->timeCodeFormat = TIMEFORMAT_MIDI;
		else if (strcmpi(&PR->argString[2][0], "SMPTE")==0)
			SIR->timeCodeFormat = TIMEFORMAT_SMPTE;
		else if (strcmpi(&PR->argString[2][0], "MLTC")==0)
			SIR->timeCodeFormat = TIMEFORMAT_MLTC;
		else if (strcmpi(&PR->argString[2][0], "CUSTOM")==0)
			SIR->timeCodeFormat = TIMEFORMAT_CUSTOM;

		if (PR->numArgs==6 || PR->numArgs==7)
			offset=2;
		else
			offset=5;

		CPrefs.scriptTiming			= *(args+offset);
		CPrefs.scriptTiming--;
		CPrefs.objectPreLoading	= *(args+offset+1);

		// START PRELOAD

		if ( CPrefs.objectPreLoading == 1 )
			CPrefs.objectPreLoading = 10;	// 'none' equivalent
		else if ( CPrefs.objectPreLoading == 2 )
			CPrefs.objectPreLoading = 10;	// 'all after a label' equivalent
		else if ( CPrefs.objectPreLoading < 10 )
			CPrefs.objectPreLoading = 30;	// 'all' equivalent

		// END PRELOAD

		CPrefs.playOptions			= *(args+offset+2);
		CPrefs.bufferOptions		= *(args+offset+3);
		CPrefs.bufferOptions--;

		if ( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS )
		{
			if (strcmpi(&PR->argString[3][0], "25FPS")==0)
				SIR->timeCodeRate = TIMERATE_25FPS;
			else if (strcmpi(&PR->argString[3][0], "30FPS")==0)
				SIR->timeCodeRate = TIMERATE_30FPS;

			if (strcmpi(&PR->argString[4][0], "TRUE")==0)
				SIR->timeCodeOut = TRUE;
			else
				SIR->timeCodeOut = FALSE;

			timeStringtoTimeCode(	PR->argString[5], &a, &b, &c, &d);
			SIR->Offset.TimeCode.HH = (BYTE)a;
			SIR->Offset.TimeCode.MM = (BYTE)b;
			SIR->Offset.TimeCode.SS = (BYTE)c;
			SIR->Offset.TimeCode.FF = (BYTE)d;

			if ( PR->numArgs==11 && SIR->timeCodeFormat == TIMEFORMAT_CUSTOM )
				strcpy(CPrefs.customTimeCode, PR->argString[10]);
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** CounterFunc() ********/

void CounterFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;
int a,b,c,d;

	/*  COUNTER hh:mm:ss:ff, hh:mm:ss:ff  */

	node = SIR->currentNode;
	if (node==NULL)
	{
		printError(PR, msgs[Msg_MissesStart-1]);
		return;
	}

	if (PR->numArgs==3)
	{
		timeStringtoTimeCode(	PR->argString[1], &a, &b, &c, &d);
		node->Start.TimeCode.HH = (BYTE)a;
		node->Start.TimeCode.MM = (BYTE)b;
		node->Start.TimeCode.SS = (BYTE)c;
		node->Start.TimeCode.FF = (BYTE)d;

		timeStringtoTimeCode(	PR->argString[2], &a, &b, &c, &d);
		node->End.TimeCode.HH = (BYTE)a;
		node->End.TimeCode.MM = (BYTE)b;
		node->End.TimeCode.SS = (BYTE)c;
		node->End.TimeCode.FF = (BYTE)d;
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** LocalEventFunc() ********/

void LocalEventFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
struct ScriptNodeRecord *node;
struct PageEventRecord *per;
struct ScriptEventRecord *ser, **serlist;
int i;

	// LOCALEVENT "labelName", "buttonName"

	if ( PR->numArgs==2 || PR->numArgs==3 )
	{
		node = SIR->currentNode;

		if ( node->list == NULL )
		{
			/**** allocate list ****/

			node->list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
			if ( node->list != NULL )
			{
				/**** init list ****/

				NewList(node->list);

				/**** allocate node ****/
		
				per = (struct PageEventRecord *)AllocMem(	sizeof(struct PageEventRecord),
																									MEMF_ANY | MEMF_CLEAR);
				if (per != NULL)
				{
					/**** add node to list ****/

					AddTail((struct List *)node->list, (struct Node *)per);
					per->er_Header.ph_NodeType = TALK_LOCALEVENT;
					for(i=0; i<MAX_LOCAL_EVENTS; i++)
						per->er_LocalEvents[i] = NULL;

					/**** copy first line of LOCALEVENT info ****/

					serlist = (struct ScriptEventRecord **)per->er_LocalEvents;
					serlist[0] = (struct ScriptEventRecord *)AllocMem(
															sizeof(struct ScriptEventRecord), MEMF_ANY | MEMF_CLEAR);
					if ( serlist[0] != NULL )
					{
						ser = (struct ScriptEventRecord *)serlist[0];

						strcpy(ser->labelName, &PR->argString[1][0]);
						RemoveQuotes(ser->labelName);

						if ( PR->numArgs==3 )
						{
							strcpy(ser->buttonName, &PR->argString[2][0]);
							RemoveQuotes(ser->buttonName);
						}
						else
							strcpy(ser->buttonName,mainName);
					}
				}
			}
		}
		else	// later lines get an easier treatment
		{
			per = (struct PageEventRecord *)(node->list->lh_Head);
			if (per != NULL)
			{
				serlist = (struct ScriptEventRecord **)per->er_LocalEvents;
				i=0;
				while( serlist[i]!=NULL && i<MAXEDITWINDOWS )	// look-up empty hole
					i++;
				if ( i<MAXEDITWINDOWS )
				{
					serlist[i] = (struct ScriptEventRecord *)AllocMem(
															sizeof(struct ScriptEventRecord), MEMF_ANY | MEMF_CLEAR);
					if ( serlist[i] != NULL )
					{
						ser = (struct ScriptEventRecord *)serlist[i];

						strcpy(ser->labelName, &PR->argString[1][0]);
						RemoveQuotes(ser->labelName);

						strcpy(ser->buttonName, &PR->argString[2][0]);
						RemoveQuotes(ser->buttonName);
					}
				}
			}
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** VarsFunc() ********/

void VarsFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
VIR *vir;
VAR *var;
struct ScriptNodeRecord *node;
TEXT labelStr[75];

	// VARIABLES "declaration/expression"

	if (PR->numArgs==2)
	{
		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);

		if ( SIR->lists[0] == SIR->currentList )
		{
			vir = AllocVIR();
			if ( vir )
			{
				if ( DeclToVIR(SIR, scrStr, vir) )
					AddVIR(&(SIR->VIList),vir);
			}
		}
		else
		{
			node = SIR->currentNode;
			if (node==NULL)
			{
				printError(PR, msgs[Msg_MissesStart-1]);
				return;
			}

			node->nodeType = PR->commandCode;
			node->numericalArgs[0] = TG_CONDITIONJUMP;
			stccpy(node->objectName, msgs[Msg_Vars-1], MAX_OBJECTNAME_CHARS);

			var = AllocVAR();
			if ( var )
			{
				if ( !node->list )
				{
					node->list = (struct List *)AllocMem(sizeof(struct List),MEMF_CLEAR|MEMF_ANY);
					NewList( node->list );
				}

				if ( ExprToVAR(SIR, scrStr, var, labelStr) )
				{
					AddTail(node->list, (struct Node *)var);
					//strcpy(node->objectPath, labelStr);
				}
			}
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** InputSettingsFunc() ********/

void InputSettingsFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
#if USED_FOR_MP
WORD args[MAX_PARSER_ARGS];

	/*  INPUTSETTINGS MousePointer, ShowDays, GamePortUsed, StandBy  */

	if (PR->numArgs==5)
	{
		GetNumericalArgs(PR, args);
		CPrefs.mousePointer		= *(args+0);
		CPrefs.showDays				= *(args+1);
		CPrefs.gameport_used	= *(args+2);
		CPrefs.standBy				= *(args+3);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** E O F ********/
