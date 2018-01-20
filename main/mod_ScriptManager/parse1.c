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
extern struct RendezVousRecord *rvrec;
extern TEXT reportStr[256];
extern struct UserApplicInfo UAI;
extern struct GadgetRecord DiskReq_GR[];
extern struct EventData CED;

/**** globals ****/

char *commands[] = {
/* ScriptTalk */
"ANIM", "AREXX", "DOS", "SOUND", "PAGE", "DATA", "MAIL", "XAPP",
/* PageTalk */
"CLIP", "CRAWL", "TEXT", "CLIPANIM",
/* Special Magic */
"FONT", "SYSTEM", "IFF", "SCRIPT",
NULL }; /* ALWAYS END WITH NULL! */

struct SMFuncs
{
	BOOL (*func)(struct ParseRecord *, FILE *, FILE *, STRPTR, STRPTR, BYTE, STRPTR);
};

struct SMFuncs funcs[] =
{ 
	{ AnimFuncSpec		 },
	{ ArexxDosFuncSpec },
	{ ArexxDosFuncSpec },
	{ SoundFuncSpec		 },
	{ PageFuncSpec		 },
	{ BinaryFuncSpec	 },
	{ MailFuncSpec		 },
	{ XappFuncSpec		 },
	{ ClipFuncSpec		 },
	{ CrawlFuncSpec		 },
	{ TextFuncSpec		 },
	{ ClipAnimFuncSpec },
	{ FontFuncSpec		 },
	{ SystemFuncSpec	 },
	{ IFFFuncSpec			 },
	{ ScriptFuncSpec	 },
};

struct List UsedFontsList;

/**** functions ****/

/******** CreateBigFile() ********/
/*
 * The script 'scriptPath will be parsed. The files 'bigFile' and 'tempScript'
 * will always be created. The 'mode' can be eg MODE_CREATE_RUNTIME. destPath can
 * point to a destination directory or be NULL. The file 'bigFile' can be used
 * later to take further steps.
 *
 */

BOOL CreateBigFile(	STRPTR scriptPath, STRPTR bigFile, STRPTR tempScript,
										BYTE mode, STRPTR destPath, BOOL systemFiles, STRPTR scriptName )
{
struct ParseRecord *PR;
int instruc, line, fileSize, i, numLines, pct;
BOOL validScript=TRUE;
//BOOL headerPassed=FALSE;
UBYTE *buffer;
TEXT whiteSpcs[512], path1[SIZE_FULLPATH], path2[SIZE_FULLPATH];
FILE *fpw1 = NULL;
FILE *fpw2 = NULL;
BOOL linePrinted;
TEXT destDevice[32];

	/**** open parser and alloc mem ****/

	// Get volume name of dest

	destDevice[0]='\0';
	for(i=0; i<32; i++)
	{
		if ( scriptPath[i]!=':' )
		{
			destDevice[i] = scriptPath[i];
			destDevice[i+1] = '\0';
		}
		else
			break;
	}
	strcat(destDevice,":");
	if ( !strnicmp(destDevice,"ram",3) )
		strcpy(destDevice,"RAM:");

	if ( !InsertRightDisk(scriptPath, destDevice, msgs[Msg_SM_38-1]) )
		return(FALSE);

	PR = (struct ParseRecord *)OpenParseFile(commands,scriptPath);
	if (PR==NULL)
	{
		// "Warning: unable to find file %s"
		sprintf(reportStr,msgs[Msg_SM_32-1],scriptPath);
		Report(reportStr);
		return(FALSE);
	}

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_CLEAR);
	if ( buffer==NULL )
	{
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** open write files ****/

	fpw1 = fopen(bigFile,"w");
	if ( !fpw1 )
	{
		FreeMem(buffer, SCRIPT_LINESIZE);
		CloseParseFile(PR);
		return(FALSE);
	}

	fpw2 = fopen(tempScript,"w");
	if ( !fpw2 )
	{
		fclose(fpw1);
		FreeMem(buffer, SCRIPT_LINESIZE);
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** put info on system files in bigfile ****/

	if ( systemFiles )
	{
		AddSystemFilesToBigFile(fpw1,destPath,MSM_SYSTEM_PATH,mode);
		AddSystemFilesToBigFile(fpw1,destPath,MSM_XAPPS_PATH,mode);
	}	

	/**** put player and script in bigfile ****/

	if ( (mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD) && systemFiles )
	{
		// Copy Player

		getpath((BPTR)rvrec->capsprefs->appdirLock,whiteSpcs);
		UA_MakeFullPath(whiteSpcs,"Player",path1);
		fileSize = GetFileSize(path1);
		UA_MakeFullPath(destPath,"Player",path2);

		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_SYSTEM],path1,path2,fileSize,GetDateStamp(path1));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_SYSTEM],path1,path2,fileSize,GetDateStamp(path1));
	}

	if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
	{
		// Copy "ram:MP_TempScript" to dest:oriname

		SplitFullPath(scriptPath,path2,path1);	// path1 is filename
		UA_MakeFullPath(destPath,path1,path2);

		// START -- Check if we copy this over a network --

		if ( scriptName )
			strcpy(path2,scriptName);

		// END -- Check if we copy this over a network --

		fileSize = GetFileSize(scriptPath);
		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_SCRIPT],TEMPSCRIPT,path2,fileSize,GetDateStamp(TEMPSCRIPT));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_SCRIPT],TEMPSCRIPT,path2,fileSize,GetDateStamp(TEMPSCRIPT));

		sprintf(path1,"%s.info",scriptPath);
		strcat(path2,".info");
		fileSize = GetFileSize(path1);

		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_SCRIPT],path1,path2,fileSize,GetDateStamp(path1));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_SCRIPT],path1,path2,fileSize,GetDateStamp(path1));
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

	/**** font list ****/

	InitFontsList();

	/**** parse all lines ****/

	instruc = -1;
	line = 0;
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
					if (	PR->commandCode == SM_ANIM ||
								PR->commandCode == SM_AREXX ||
								PR->commandCode == SM_DOS ||
								PR->commandCode == SM_SOUND ||
								PR->commandCode == SM_PAGE ||
								PR->commandCode == SM_BINARY ||
								PR->commandCode == SM_MAIL ||
								PR->commandCode == SM_XAPP )
					{
						if ( !SMPerfFunc((struct GenericFuncs *)funcs,PR,fpw1,fpw2,whiteSpcs,buffer,mode,destPath) )
						{
							instruc=PARSE_STOP;
							validScript=FALSE;
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

		if ( numLines>0 )
		{
			pct = ((100*line)/numLines)/2;
			if ( pct<0 )
				pct=0;
			if ( pct>50 )
				pct=50;			
			sprintf(reportStr,"%d",pct);
			Report(reportStr);
		}
	}

	Report("50");

	/**** free memory ****/

	FreeMem(buffer,SCRIPT_LINESIZE);
	CloseParseFile(PR);
	fclose(fpw1);
	fclose(fpw2);

	/**** font list ****/

	DeInitFontsList();

	if (!validScript)
		return(FALSE);

	return(TRUE);
}

/******** SMPerfFunc() ********/
/* input:    pointer to an array of pointers to functions, a pointer to
 *           a filled ParseRecord which holds the currently parsed command
 *           and a pointer to the current ScriptInfoRecord.
 * output:   -
 * function: jumps to a function, carrying along the PR and SIR pointers.
 */

struct GenericFuncs
{
	BOOL (*func)(APTR, APTR, APTR, APTR, APTR, BYTE, APTR);
};

BOOL SMPerfFunc(struct GenericFuncs *FuncList,
								struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
								STRPTR buffer, BYTE mode, STRPTR destPath)
{
BOOL (*func)(APTR, APTR, APTR, APTR, APTR, BYTE, APTR);

	func = FuncList[PR->commandCode].func;
	if (func!=NO_FUNCTION)
		return( (*(func))(PR,fpw1,fpw2,whiteSpcs,buffer,mode,destPath) );
}

/******** AddSystemFilesToBigFile() ********/

void AddSystemFilesToBigFile(FILE *fpw1, STRPTR destPath, STRPTR drawer, BYTE mode)
{
struct FileHandle *fileLock;
TEXT path1[SIZE_FULLPATH];
TEXT path2[SIZE_FULLPATH];
TEXT path3[SIZE_FULLPATH];
TEXT path4[SIZE_FULLPATH];
struct FileInfoBlock *FIB;
int i;

	FIB = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock),MEMF_ANY);
	if ( !FIB )
		return;

	path1[0]='\0';
	path2[0]='\0';
	path3[0]='\0';
	path4[0]='\0';
	getpath((BPTR)rvrec->capsprefs->appdirLock,path2);
	UA_MakeFullPath(path2,drawer,path1);

	fileLock = (struct FileHandle *)Lock((STRPTR)path1, (LONG)ACCESS_READ);
	if (fileLock)
	{
		if ((BOOL)Examine((BPTR)fileLock, (struct FileInfoBlock *)FIB) == FALSE)
		{
			UnLock((BPTR)fileLock);
			FreeMem(FIB,sizeof(struct FileInfoBlock));
			return;
		}

		UA_ValidatePath(path1);
		if ( destPath )
		{
			UA_MakeFullPath(destPath,drawer,path2);
			UA_ValidatePath(path2);
		}

		for (i=0; i<MAX_FILE_LIST_ENTRIES; i++)
		{
			if ( !ExNext((BPTR)fileLock, (struct FileInfoBlock *)FIB) )
				break;
			if ( FIB->fib_DirEntryType < 0 )
			{
				strcpy(path3,path1);	// original
				strcat(path3,FIB->fib_FileName);

				if ( destPath )
				{
					strcpy(path4,path2);	// original
					strcat(path4,FIB->fib_FileName);
				}

				MyTurnAssignIntoDir(path3);

				if ( mode==MODE_UPLOAD )
					RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_SYSTEM],path3,path4,FIB->fib_Size,GetDateStamp(path3));
				else
					fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_SYSTEM],path3,path4,FIB->fib_Size,GetDateStamp(path3));
			}
		}
		// free things
		UnLock((BPTR)fileLock);
		FreeMem(FIB,sizeof(struct FileInfoBlock));
	}
}

/******** GetDateStamp() ********/

int GetDateStamp(STRPTR file1)
{
BPTR lock1;
struct FileInfoBlock __aligned fib1;

	if( lock1=Lock(file1,SHARED_LOCK) )
	{
		if( Examine(lock1,&fib1) )
		{
			UnLock(lock1);
			return( (fib1.fib_Date.ds_Days*3600*24) + (fib1.fib_Date.ds_Minute*60) +
							(fib1.fib_Date.ds_Tick/TICKS_PER_SECOND) );
		}
		UnLock(lock1);
	}
	return(0);
}

/******** MyTurnAssignIntoDir() ********/

void MyTurnAssignIntoDir(STRPTR ass)
{
struct FileLock *fileLock;

	fileLock = (struct FileLock *)Lock((STRPTR)ass,(LONG)ACCESS_READ);
	if (fileLock != NULL)
	{
		if (my_getpath((BPTR)fileLock, ass)!=0)
			stccpy(ass, "SYS:", 10);
		UnLock((BPTR)fileLock);
	}
}

/******** my_getpath() ********/

int my_getpath(BPTR lock, char *path)
{
int error=-1;

	if ( NameFromLock(lock, path, SIZE_PATH) )
		error=0;
	return(error);
}

/******** InsertRightDisk() ********/

BOOL InsertRightDisk(STRPTR path, STRPTR destDevice, STRPTR msg)
{
struct FileLock *FL;

//KPrintF("trying to lock [%s]\n", path);

	FL = (struct FileLock *)Lock((STRPTR)path, (LONG)ACCESS_READ);
	while(!FL)
	{
		if ( !WaitForDisk(UAI.userWindow,destDevice,msg) )
			return( 0 );
		FL = (struct FileLock *)Lock((STRPTR)path, (LONG)ACCESS_READ);
	}
	if (FL)
		UnLock((BPTR)FL);

	return(TRUE);
}

/******** WaitForDisk() ********/

BOOL WaitForDisk(struct Window *window, STRPTR destDevice, STRPTR msg)
{
struct Window *reqwdw;
BOOL loop=TRUE, retval=FALSE;
TEXT str[512];
int ID;

	sprintf(str, msg, destDevice);

	reqwdw = UA_OpenMessagePanel( window, str );
	UA_TranslateGR(DiskReq_GR, msgs);
	UA_DrawGadgetList(reqwdw, DiskReq_GR);
	UA_SwitchFlagsOn(reqwdw,IDCMP_DISKINSERTED);

	while(loop)
	{
		UA_doStandardWait(reqwdw,&CED);
		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(reqwdw, DiskReq_GR, &CED);
					if ( ID==0 )
					{
						UA_HiliteButton(reqwdw, &DiskReq_GR[ID]);		
						retval=TRUE;
						loop=FALSE;
					}
					else
					{
						UA_HiliteButton(reqwdw, &DiskReq_GR[ID]);		
						retval=FALSE;
						loop=FALSE;
					}
				}
				break;

			case IDCMP_DISKINSERTED:
					retval=TRUE;
					loop=FALSE;
				break;
		}
	}

	UA_SwitchFlagsOff(reqwdw,IDCMP_DISKINSERTED);
	UA_CloseMessagePanel( reqwdw );

	return( retval );
}

/******** RA_BigFile() ********/

void RA_BigFile(FILE *fp, STRPTR argStr, STRPTR command, STRPTR path1, STRPTR path2,
								int fileSize, int dateStamp)
{
TEXT fileSize_Str[32];
TEXT dateStamp_Str[32];

	if ( fileSize==0 )
		return;	// don't store empty files

	fprintf(fp,argStr,command,path1,path2);

	sprintf(fileSize_Str,"%d",fileSize);
	sprintf(dateStamp_Str,"%d",dateStamp);

	fprintf(fp," \"%s\" \"%s\"\n",fileSize_Str,dateStamp_Str);
}

/******** InitFontsList() ********/

void InitFontsList(void)
{
	NewList(&UsedFontsList);
}

/******** AddToFontsList() ********/

void AddToFontsList(STRPTR fontPath)
{
struct Node *node;

	if ( !IsFontInList(fontPath) )
	{
		node = (struct Node *)AllocMem(sizeof(struct Node),MEMF_CLEAR|MEMF_ANY);
		if (node)
		{
			AddTail(&UsedFontsList, (struct Node *)node);
			node->ln_Name = (char *)AllocMem(strlen(fontPath)+1,MEMF_CLEAR|MEMF_ANY);
			if ( node->ln_Name )
				strcpy(node->ln_Name, fontPath);
		}
	}
}

/******** DeInitFontsList() ********/

void DeInitFontsList(void)
{
struct Node *work_node, *next_node;
ULONG size;

	work_node = UsedFontsList.lh_Head;	// first node
	while(next_node = (work_node->ln_Succ))
	{
		size = strlen(work_node->ln_Name) + 1;
		if ( work_node->ln_Name )
			FreeMem(work_node->ln_Name, size);
		FreeMem(work_node,sizeof(struct Node));
		work_node = next_node;
	}
}

/******** IsFontInList() ********/

BOOL IsFontInList(STRPTR fontPath)
{
struct Node *node;

	for(node=UsedFontsList.lh_Head; node->ln_Succ; node=node->ln_Succ)
		if ( !stricmp(node->ln_Name,fontPath) )
			return(TRUE);

	return(FALSE);
}

/******** E O F ********/
