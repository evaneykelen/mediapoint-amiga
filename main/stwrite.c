#include "nb:pre.h"

/**** DEFINES ****/

static TEXT START_TXT[] = "START\n";
static TEXT END_TXT[]		= "END\n";
static TEXT STARTSER[]	="STARTSER";
static TEXT ENDSER[]		="ENDSER";

/**** EXTERNALS ****/

extern struct CapsPrefs CPrefs;
extern TEXT *dir_scripts;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** STATIC GLOBALS ****/

int branchType=-1;

/**** FUNCTIONS ****/

/******** WriteScript() ********/

BOOL WriteScript(	STRPTR path, STRPTR filename, struct ScriptInfoRecord *SIR,
									char **scriptCommands)
{
struct ScriptEventRecord *SER;
int i;
FILE *fp;
TEXT buf[256], fullPath[SIZE_FULLPATH], scrStr_1[512];
VIR *this_vir;

	if ( !CheckWriteProtect(path,filename) )
	{
		Message(msgs[Msg_Overwriting-1],filename);
		return(FALSE);
	}

#ifdef USED_FOR_DEMO
	Message("Sorry, no saving in this version...");
	return(TRUE);
#else
	UA_MakeFullPath(path, filename, fullPath);

	UpdateDirCache(path);

	fp = (FILE *)fopen(fullPath, "w");
	if (fp == NULL)
	{
		Message(msgs[Msg_UnableToSaveScript-1], filename);
		return(FALSE);
	}

	// SCRIPTTALK 

	fprintf(fp, "SCRIPTTALK %d, %d\n\n", CPrefs.userLevel, SIR->version);

	// GLOBALEVENTS

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		SER = SIR->globallocalEvents[i];
		if (SER != NULL)
		{
			StrToScript(SER->labelName, scrStr_1);
			if (SER->keyCode != -1)
				fprintf(fp, "GLOBALEVENT \"%c\", \"%s\"\n", SER->keyCode, scrStr_1);
			else if (SER->rawkeyCode != -1)
			{
				KeyToKeyName(SER->keyCode,SER->rawkeyCode,buf);
				fprintf(fp, "GLOBALEVENT %s, \"%s\"\n", buf, scrStr_1);
			}
		}
	}

	// do an extra linefeed only when there are global events
	SER = SIR->globallocalEvents[0];
	if (SER != NULL)
		fprintf(fp, "\n");

	// VARIABLES

	for(this_vir = (VIR *)SIR->VIList.lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		VIRToDecl(this_vir, buf);
		StrToScript(buf, scrStr_1);
		fprintf(fp, "%s \"%s\"\n", scriptCommands[ TALK_VARS ], scrStr_1);
	}
	if ( SIR->VIList.lh_TailPred != (struct Node *)&SIR->VIList )
	{
		// do an extra linefeed only when there are variables
		fprintf(fp, "\n");
	}

	// INPUT SETTINGS

	if ( IsNodeTypePresent(SIR, TALK_INPUTSETTINGS) )
		fprintf(fp, "%s %d, %d, %d, %d\n\n", scriptCommands[ TALK_INPUTSETTINGS ],
						CPrefs.mousePointer, CPrefs.showDays, CPrefs.gameport_used, CPrefs.standBy);

	// REST OF SCRIPT

	FollowList(SIR, scriptCommands, fp);

	fclose(fp);

	SaveScriptIcon(fullPath);

	return(TRUE);
#endif
}

/******** FollowList() ********/

void FollowList(struct ScriptInfoRecord *SIR, char **scriptCommands,
								FILE *fp)
{
#ifndef USED_FOR_DEMO
struct List *list;
struct ScriptNodeRecord *nodeArray[128];
struct ScriptNodeRecord *currentNode;
int i, level;
BOOL loop, done;
int listtypeArray[128];
TEXT str1[64], str2[64], str3[64];
TEXT fullPath[256];
TEXT ON[]				="ON";
TEXT OFF[]			="OFF";
TEXT STARTPAR[]	="STARTPAR";
TEXT ENDPAR[]		="ENDPAR";
TEXT scrStr_1[512];
TEXT scrStr_2[512];
TEXT scrStr_3[512];

	for(i=0; i<128; i++)
	{
		nodeArray[i] = NULL;
		listtypeArray[i] = -1;
	}
	level=0;

	list = (struct List *)SIR->allLists[0];	/* root list */
	if (list==NULL)
		return;	/* no list available */

	currentNode = (struct ScriptNodeRecord *)SIR->allLists[0]->lh_Head;
	currentNode = (struct ScriptNodeRecord *)currentNode->node.ln_Succ;

	if (	(SIR->allLists[0]->lh_TailPred == (struct Node *)currentNode) &&
				(SIR->allLists[1]->lh_TailPred == (struct Node *)SIR->allLists[1]) )
	{
		WriteEmptyScript(SIR, scriptCommands, fp);
		return;
	}

	currentNode = (struct ScriptNodeRecord *)list->lh_Head;
	if (currentNode==NULL)
		return;	/* root list is empty */

	fprintf(fp, "%s\n", STARTSER);
	branchType = TALK_STARTSER;
	doProgram(SIR, currentNode, fp, level-1, TRUE);

	loop=TRUE;
	while(loop)
	{	
		switch(currentNode->nodeType)
		{
			case TALK_ANIM:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				UA_MakeFullPath(currentNode->objectPath, currentNode->objectName, fullPath);
				doIndent(fp, level+1);
				if ( currentNode->numericalArgs[12] )	// color cycle
					stccpy(str1, ON, 64);
				else
					stccpy(str1, OFF, 64);
				if ( currentNode->numericalArgs[11] & 2 )	// diskanim
					stccpy(str2, ON, 64);
				else
					stccpy(str2, OFF, 64);
				if ( currentNode->numericalArgs[11] & 1 )	// loopIt
					stccpy(str3, ON, 64);
				else
					stccpy(str3, OFF, 64);
				StrToScript(fullPath, scrStr_1);
				fprintf(fp, "%s \"%s\", %s, %d, %d, %s, %s, %d, %d, %d, %d\n",
								scriptCommands[currentNode->nodeType],
								scrStr_1,
								str1, (int)currentNode->numericalArgs[10],
								(int)currentNode->numericalArgs[9],
								str2, str3,
								(int)currentNode->numericalArgs[2],
								(int)currentNode->numericalArgs[3],
								(int)currentNode->numericalArgs[4],
								(int)currentNode->numericalArgs[5] );
				doProgram(SIR, currentNode, fp, level, FALSE);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_AREXX:
			case TALK_DOS:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				if (currentNode->numericalArgs[0] == ARGUMENT_DEFER)
					stccpy(str1, "DEFER", 64);
				else if (currentNode->numericalArgs[0] == ARGUMENT_CONTINUE)
					stccpy(str1, "CONTINUE", 64);
				if (currentNode->numericalArgs[1] == ARGUMENT_COMMAND)
					stccpy(str2, "COMMAND", 64);
				else if (currentNode->numericalArgs[1] == ARGUMENT_SCRIPT)
					stccpy(str2, "SCRIPT", 64);
				doIndent(fp, level+1);
				UA_MakeFullPath(currentNode->objectPath, currentNode->objectName, fullPath);
				if (currentNode->nodeType == TALK_AREXX)
				{
					if (currentNode->numericalArgs[1] == ARGUMENT_COMMAND)
					{
						StrToScript(&(currentNode->extraData[50]), scrStr_1);
						StrToScript(currentNode->objectPath, scrStr_2);
						StrToScript(currentNode->extraData, scrStr_3);
						fprintf(fp, "%s \"%s\", \"%s\", \"%s\", %s, %s\n",
										scriptCommands[currentNode->nodeType],
										scrStr_1, scrStr_2, scrStr_3,
										str1, str2);
					}
					else
					{
						StrToScript(fullPath, scrStr_1);
						StrToScript(currentNode->extraData, scrStr_2);
						fprintf(fp, "%s \"%s\", \"%s\", %s, %s\n",
										scriptCommands[currentNode->nodeType],
										scrStr_1, scrStr_2,
										str1, str2);
					}
				}
				else	/* DOS */
				{
					if (currentNode->numericalArgs[1] == ARGUMENT_COMMAND)
					{
						StrToScript(currentNode->objectPath, scrStr_1);
						StrToScript(currentNode->extraData, scrStr_2);
						fprintf(fp, "%s \"%s\", \"%s\", %s, %s, %d\n",
										scriptCommands[currentNode->nodeType],
										scrStr_1, scrStr_2,
										str1, str2, currentNode->numericalArgs[2]);
					}
					else
					{
						StrToScript(fullPath, scrStr_1);
						StrToScript(currentNode->extraData, scrStr_2);
						fprintf(fp, "%s \"%s\", \"%s\", %s, %s, %d\n",
										scriptCommands[currentNode->nodeType],
										scrStr_1, scrStr_2,
										str1, str2, currentNode->numericalArgs[2]);
					}
				}
				doProgram(SIR, currentNode, fp, level, FALSE);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_PAGE:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				UA_MakeFullPath(currentNode->objectPath, currentNode->objectName, fullPath);
				doIndent(fp, level+1);
				if ( currentNode->numericalArgs[12] )	// color cycle
					stccpy(str1, ON, 64);
				else
					stccpy(str1, OFF, 64);
				StrToScript(fullPath, scrStr_1);
				fprintf(fp, "%s \"%s\", %s, %d, %d, %d, %d, %d\n",
								scriptCommands[currentNode->nodeType],
								scrStr_1,
								str1,
								(int)currentNode->numericalArgs[2],
								(int)currentNode->numericalArgs[3],
								(int)currentNode->numericalArgs[4],
								(int)currentNode->numericalArgs[5],
								(int)currentNode->numericalArgs[15] );
				doProgram(SIR, currentNode, fp, level, FALSE);
				doLocalEvents(SIR, currentNode, fp, level, FALSE);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_VARS:
				if (level>0)
				{
					fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
					doVars(SIR, currentNode, fp, level, scriptCommands);
					doIndent(fp, level); fprintf(fp, END_TXT);
				}
				break;

			case TALK_BINARY:
			case TALK_GOTO:
			case TALK_LABEL:
			case TALK_NOP:
			case TALK_MAIL:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				doIndent(fp, level+1);
				UA_MakeFullPath(currentNode->objectPath, currentNode->objectName, fullPath);
				StrToScript(fullPath, scrStr_1);
				fprintf(fp, "%s \"%s\"\n",	scriptCommands[currentNode->nodeType], scrStr_1);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_TIMECODE:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				doIndent(fp, level+1);
				fprintf(fp, "%s ",	scriptCommands[currentNode->nodeType]);
				if ( SIR->timeCodeSource == TIMESOURCE_EXTERNAL )
					fprintf(fp, "EXTERNAL, ");
				else
					fprintf(fp, "INTERNAL, ");
				if ( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
					fprintf(fp, "HHMMSS");
				else if ( SIR->timeCodeFormat == TIMEFORMAT_MIDI)
					fprintf(fp, "MIDI");
				else if ( SIR->timeCodeFormat == TIMEFORMAT_SMPTE)
					fprintf(fp, "SMPTE");
				else if ( SIR->timeCodeFormat == TIMEFORMAT_MLTC)
					fprintf(fp, "MLTC");
				else if ( SIR->timeCodeFormat == TIMEFORMAT_CUSTOM)
					fprintf(fp, "CUSTOM");
				if ( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS)
				{
					if ( SIR->timeCodeRate == TIMERATE_24FPS)
						fprintf(fp, ", 24FPS");
					else if ( SIR->timeCodeRate == TIMERATE_25FPS)
						fprintf(fp, ", 25FPS");
					else if ( SIR->timeCodeRate == TIMERATE_30FPS_DF)
						fprintf(fp, ", 30FPS_DF");
					else if ( SIR->timeCodeRate == TIMERATE_30FPS)
						fprintf(fp, ", 30FPS");
					else if ( SIR->timeCodeRate == TIMERATE_MIDICLOCK)
						fprintf(fp, ", MIDICLOCK");

					if ( SIR->timeCodeOut )
						fprintf(fp, ", TRUE");
					else
						fprintf(fp, ", FALSE");

					fprintf(fp, ", %02d:%02d:%02d:%02d",
										SIR->Offset.TimeCode.HH,
										SIR->Offset.TimeCode.MM,
										SIR->Offset.TimeCode.SS,
										SIR->Offset.TimeCode.FF);
				}
				fprintf(fp, ", %d, %d, %d, %d", CPrefs.scriptTiming+1,
								CPrefs.objectPreLoading, CPrefs.playOptions, CPrefs.bufferOptions+1);
				if ( SIR->timeCodeFormat == TIMEFORMAT_CUSTOM )
					fprintf(fp, ", %s", CPrefs.customTimeCode);
				fprintf(fp, "\n");
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_SOUND:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				UA_MakeFullPath(currentNode->objectPath, currentNode->objectName, fullPath);
				doIndent(fp, level+1);
				if (currentNode->numericalArgs[1] == 1)		// stop
					strcpy(str1, "STOP");
				else
				{
					if (currentNode->numericalArgs[0] == 1)	// wait
						strcpy(str1, "WAIT");
					else
						strcpy(str1, "LOOP");
				}
				StrToScript(fullPath, scrStr_1);
				fprintf(fp, "%s \"%s\", %s\n", scriptCommands[currentNode->nodeType], scrStr_1, str1);
				doProgram(SIR, currentNode, fp, level, FALSE);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;

			case TALK_USERAPPLIC:
				fprintf(fp, "\n"); doIndent(fp, level); fprintf(fp, START_TXT);
				doIndent(fp, level+1);
				if (currentNode->extraData!=NULL &&	currentNode->extraData[0]!='\0' &&
						currentNode->extraDataSize>0  )
				{
					StrToScript(currentNode->objectPath, scrStr_1);
					StrToScript(currentNode->objectName, scrStr_2);
					fprintf(fp, "%s \"%s\", \"%s\", \"%s\", %d\n",
											scriptCommands[currentNode->nodeType],
											scrStr_1, scrStr_2, 
											currentNode->extraData,
											currentNode->numericalArgs[0]);
				}
				else
				{
					StrToScript(currentNode->objectPath, scrStr_1);
					StrToScript(currentNode->objectName, scrStr_2);
					fprintf(fp, "%s \"%s\", \"%s\", %d\n",
											scriptCommands[currentNode->nodeType],
											scrStr_1, scrStr_2,
											currentNode->numericalArgs[0]);
				}
				doProgram(SIR, currentNode, fp, level, FALSE);
				doIndent(fp, level); fprintf(fp, END_TXT);
				break;
		}

		/* check if node has a sub list */

		done=FALSE;

		if (currentNode->list != NULL &&
				(currentNode->nodeType==TALK_STARTSER || currentNode->nodeType==TALK_STARTPAR) &&
				currentNode->list->lh_TailPred == (struct Node *)currentNode->list)
		{
			fprintf(fp, "\n");
			doIndent(fp, level);
			if (currentNode->objectName[0] == '\0')
			{
				if (currentNode->nodeType==TALK_STARTSER)
				{
					branchType = TALK_STARTSER;
					fprintf(fp, "%s\n", STARTSER);
				}
				else
				{
					branchType = TALK_STARTPAR;
					fprintf(fp, "%s\n", STARTPAR);
				}
			}
			else
			{
				StrToScript(currentNode->objectName, scrStr_1);
				if (currentNode->nodeType==TALK_STARTSER)
				{
					branchType = TALK_STARTSER;
					fprintf(fp, "%s \"%s\"\n", STARTSER, scrStr_1);
				}
				else
				{
					branchType = TALK_STARTPAR;
					fprintf(fp, "%s \"%s\"\n", STARTPAR, scrStr_1);
				}
			}

			doProgram(SIR, currentNode, fp, level, FALSE);

			doIndent(fp, level);
			if (currentNode->nodeType==TALK_STARTSER)
				fprintf(fp, "%s\n", ENDSER);
			else
				fprintf(fp, "%s\n", ENDPAR);

			branchType = TALK_STARTSER;	// was -1

			done=TRUE;

			if (level==0)
			{
				loop=FALSE;
			}
		}

		if (!done && currentNode->list != NULL &&
				(currentNode->nodeType==TALK_STARTSER || currentNode->nodeType==TALK_STARTPAR)
				)	/* node has a sub list */
		{
			if ( SIR->allLists[1]->lh_TailPred == (struct Node *)SIR->allLists[1] )
			{
				branchType = TALK_STARTSER;

				fprintf(fp, "\n");
				doIndent(fp, level);

				StrToScript(currentNode->objectName, scrStr_1);
				if (currentNode->objectName[0] == '\0')
					fprintf(fp, "%s\n", STARTSER);
				else
					fprintf(fp, "%s \"%s\"\n", STARTSER, scrStr_1);

				doProgram(SIR, currentNode, fp, level, FALSE);

				doIndent(fp, level);

				fprintf(fp, "%s\n", ENDSER);

				branchType = TALK_STARTSER;	// was -1

				loop=FALSE;
				break;
			}
			else if (currentNode->nodeType == TALK_STARTSER)
			{
				branchType = TALK_STARTSER;

				fprintf(fp, "\n");
				doIndent(fp, level);

				StrToScript(currentNode->objectName, scrStr_1);
				if (currentNode->objectName[0] == '\0')
					fprintf(fp, "%s\n", STARTSER);
				else
					fprintf(fp, "%s \"%s\"\n", STARTSER, scrStr_1);

				listtypeArray[level] = TALK_STARTSER;
				doProgram(SIR, currentNode, fp, level, FALSE);	/* was TRUE */
				if (currentNode->list->lh_Head == NULL)
				{
					fprintf(fp, "empty\n");
					loop=FALSE;
					break;
				}
			}
			else if (currentNode->nodeType == TALK_STARTPAR)
			{
				branchType = TALK_STARTPAR;
	
				fprintf(fp, "\n");
				doIndent(fp, level);

				StrToScript(currentNode->objectName, scrStr_1);
				if (currentNode->objectName[0] == '\0')
					fprintf(fp, "%s\n", STARTPAR);
				else
					fprintf(fp, "%s \"%s\"\n", STARTPAR, scrStr_1);

				listtypeArray[level] = TALK_STARTPAR;
				doProgram(SIR, currentNode, fp, level, TRUE);
			}

			if (currentNode->node.ln_Succ->ln_Succ == NULL && level>=1)
				nodeArray[level] = NULL;
			else if (currentNode->node.ln_Succ->ln_Succ == NULL && level<1)
				nodeArray[level] = NULL;
			else
				nodeArray[level] = (struct ScriptNodeRecord *)currentNode->node.ln_Succ;	/* store next node */
			level++;
			currentNode = (struct ScriptNodeRecord *)currentNode->list->lh_Head;
		}
		else
		{
			currentNode = (struct ScriptNodeRecord *)currentNode->node.ln_Succ;
			if (currentNode->node.ln_Succ == NULL)
			{
				if (level>0 && listtypeArray[level-1]==TALK_STARTSER)
				{
					fprintf(fp, "\n");
					doIndent(fp, level-1);
					fprintf(fp, "%s\n", ENDSER);
					
					branchType = TALK_STARTSER;	// was -1
				}
				else if (level>0 && listtypeArray[level-1]==TALK_STARTPAR)
				{
					branchType = TALK_STARTSER;	// was PAR;

					fprintf(fp, "\n");
					doIndent(fp, level-1);
					fprintf(fp, "%s\n", ENDPAR);
				}
				level--;
				while( nodeArray[level]==NULL )
				{
					if (level>0 && listtypeArray[level-1]==TALK_STARTSER)
					{
						fprintf(fp, "\n");
						doIndent(fp, level-1);
						fprintf(fp, "%s\n", ENDSER);

						branchType = TALK_STARTSER;	// was -1
					}
					else if (level>0 && listtypeArray[level-1]==TALK_STARTPAR)
					{
						branchType = TALK_STARTSER;	// Was PAR;

						fprintf(fp, "\n");
						doIndent(fp, level-1);
						fprintf(fp, "%s\n", ENDPAR);
					}
					level--;
					if (level<0)
					{
						loop=FALSE;
						break;
					}
				}
				if (!loop)
					break;
				currentNode = nodeArray[level];
			}
		}
	}

	fprintf(fp, "\n%s\n", ENDSER);

#endif
}

/******** doProgram() ********/

void doProgram(struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node, FILE *fp, int level, BOOL extraLF)
{
#ifndef USED_FOR_DEMO
TEXT tempbuf[128];

	if (SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
	{
		if (branchType==TALK_STARTSER && node->duration != -1 &&
				node->nodeType!=TALK_STARTSER)
		{
			secondsToDuration(node->duration, tempbuf);
			if (extraLF)
			{
				fprintf(fp, "\n");
				extraLF=FALSE;
			}
			doIndent(fp, level+1);
			fprintf(fp, "DURATION %s\n", tempbuf);
		}
		else if (branchType==TALK_STARTPAR)
		{
			if ( node->nodeType==TALK_STARTPAR && node->duration != -1 )
			{
				secondsToDuration(node->duration, tempbuf);
				if (extraLF)
				{
					fprintf(fp, "\n");
					extraLF=FALSE;
				}
				doIndent(fp, level+1);
				fprintf(fp, "DURATION %s\n", tempbuf);
			}
			else
			{
				if ( node->Start.ParHMSTOffset!=-1 && node->End.ParHMSTOffset!=-1 )
				{
					secondsToDuration(node->Start.ParHMSTOffset, tempbuf);
					if (extraLF)
					{
						fprintf(fp, "\n");
						extraLF=FALSE;
					}
					doIndent(fp, level+1);
					fprintf(fp, "DURATION %s, ", tempbuf);
					secondsToDuration(node->End.ParHMSTOffset, tempbuf);
					fprintf(fp, "%s\n", tempbuf);
				}
				return;	// GET OUT HERE EARLY (SKIP DATE PROGRAMMING)
			}
		}

		if ( CPrefs.showDays && (node->dayBits != -1) )
		{
			if (extraLF)
				fprintf(fp, "\n");
			doIndent(fp, level+1);
			sprintf(tempbuf, "PROGRAM ");
			if (node->dayBits & 1)
				strcat(tempbuf, "SU|");
			if (node->dayBits & 2)
				strcat(tempbuf, "MO|");
			if (node->dayBits & 4)
				strcat(tempbuf, "TU|");
			if (node->dayBits & 8)
				strcat(tempbuf, "WE|");
			if (node->dayBits & 16)
				strcat(tempbuf, "TH|");
			if (node->dayBits & 32)
				strcat(tempbuf, "FR|");
			if (node->dayBits & 64)
				strcat(tempbuf, "SA");
			if ( tempbuf[strlen(tempbuf)-1] == '|' )
				tempbuf[strlen(tempbuf)-1] = '\0';
			if (node->Start.HHMMSS.Days == -1)
				strcat(tempbuf, "\n");
			fprintf(fp, tempbuf);
	
			if (node->Start.HHMMSS.Days != -1)
			{
				datestampToDate(node->Start.HHMMSS.Days, tempbuf);
				fprintf(fp, ", %s", tempbuf);
				datestampToDate(node->End.HHMMSS.Days, tempbuf);
				fprintf(fp, ", %s", tempbuf);
				datestampToTime(node->Start.HHMMSS.Minutes, node->Start.HHMMSS.Ticks, tempbuf);
				fprintf(fp, ", %s", tempbuf);
				datestampToTime(node->End.HHMMSS.Minutes, node->End.HHMMSS.Ticks, tempbuf);
				fprintf(fp, ", %s", tempbuf);
				if (node->startendMode==ARGUMENT_CYCLICAL)
					fprintf(fp, ", CYCLICAL\n");
				else if (node->startendMode==ARGUMENT_CONTINUOUS)
					fprintf(fp, ", CONTINUOUS\n");
				else
					fprintf(fp, ", CYCLICAL\n");	// Thanks to Chris - hopefully fixed elsewhere
			}
		}
	}
	else
	{
		if (node->Start.TimeCode.HH != -1)
		{
			if (extraLF)
				fprintf(fp, "\n");
			doIndent(fp, level+1);
			sprintf(tempbuf, "COUNTER %02d:%02d:%02d:%02d, %02d:%02d:%02d:%02d",
							node->Start.TimeCode.HH, node->Start.TimeCode.MM, node->Start.TimeCode.SS, node->Start.TimeCode.FF,
							node->End.TimeCode.HH, node->End.TimeCode.MM, node->End.TimeCode.SS, node->End.TimeCode.FF);
			fprintf(fp, "%s\n", tempbuf);
		}
	}
#endif
}

/******** WriteEmptyScript() ********/

void WriteEmptyScript(struct ScriptInfoRecord *SIR, char **scriptCommands,
								FILE *fp)
{
struct ScriptNodeRecord *currentNode;
TEXT scrStr_1[512];

	fprintf(fp, "%s\n", STARTSER);
	currentNode = (struct ScriptNodeRecord *)SIR->allLists[0]->lh_Head;

	doProgram(SIR, currentNode, fp, -1, TRUE);
	fprintf(fp, "\n");
	doIndent(fp, 0);

	currentNode = (struct ScriptNodeRecord *)currentNode->node.ln_Succ;

	StrToScript(currentNode->objectName, scrStr_1);
	if (currentNode->objectName[0] == '\0')
		fprintf(fp, "%s\n", STARTSER);
	else
		fprintf(fp, "%s \"%s\"\n", STARTSER, scrStr_1);

	doProgram(SIR, currentNode, fp, 0, FALSE);	/* was TRUE */
	fprintf(fp, "\n");
	doIndent(fp, 0);
	fprintf(fp, "%s\n", ENDSER);
	fprintf(fp, "\n");
	fprintf(fp, "%s\n", ENDSER);
}

/******** CreateEmptyScript() ********/

BOOL CreateEmptyScript(void)
{
FILE *fp;
TEXT path[SIZE_FULLPATH];

	UA_MakeFullPath(dir_scripts, "untitled", path);
	fp = (FILE *)fopen(path, "w");
	if (fp == NULL)
	{
		Message(msgs[Msg_UntitledMissesFatal-1]);
		return(FALSE);
	}
	else
		Message(msgs[Msg_UntitledMisses-1]);

	fprintf(fp, "SCRIPTTALK 1,0\n\n");
	fprintf(fp, "%s\n", STARTSER);
	doIndent(fp, 0);
	fprintf(fp, "%s\n", STARTSER);
	doIndent(fp, 0);
	fprintf(fp, "%s\n", ENDSER);
	fprintf(fp, "%s\n", ENDSER);

	fclose(fp);

	return(TRUE);
}

/******** doLocalEvents() ********/

void doLocalEvents(	struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node,
										FILE *fp, int level, BOOL extraLF)
{
#ifndef USED_FOR_DEMO
struct ScriptEventRecord **serlist, *ser;
struct PageEventRecord *per;
int i;
TEXT scrStr[80];
TEXT scrStr2[80];

	if ( node->nodeType==TALK_PAGE && node->numericalArgs[15]==1 && node->list!=NULL )
	{
		if (extraLF)
		{
			fprintf(fp, "\n");
			extraLF=FALSE;
		}

		per = (struct PageEventRecord *)(node->list->lh_Head);
		serlist = (struct ScriptEventRecord **)per->er_LocalEvents;

		for(i=0; i<MAX_LOCAL_EVENTS; i++)
		{
			if ( serlist[i] != NULL )
			{
				ser = serlist[i];
				doIndent(fp, level+1);
				StrToScript(ser->labelName, scrStr);
				StrToScript(ser->buttonName, scrStr2);
				fprintf(fp, "LOCALEVENT \"%s\", \"%s\"\n", scrStr, scrStr2);
			}
			else
				break;
		}
	}
#endif
}

/******** doVars() ********/

void doVars(	struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node,
							FILE *fp, int level, char **scriptCommands )
{
VAR *this_var;
TEXT buf[256], scrStr[512];

	if ( node->list )
	{
		for(this_var = (VAR *)node->list->lh_Head; 
				(VAR *)this_var->var_Node.ln_Succ;	
				this_var = (VAR *)this_var->var_Node.ln_Succ)
		{
			VARToExpr(SIR, buf, this_var);
			StrToScript(buf, scrStr);
			doIndent(fp, level+1);
			fprintf(fp, "%s \"%s\"\n", scriptCommands[ TALK_VARS ], scrStr);
		}
	}
}

/******** E O F ********/
