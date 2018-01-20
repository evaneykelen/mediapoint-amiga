#include "nb:pre.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <stat.h>	// used for stat() to get file size

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;
extern struct UserApplicInfo UAI;
extern TEXT reportStr[256];

/**** static globals ****/

STATIC TEXT scrStr[512], fullPath[SIZE_FULLPATH], oriPath[SIZE_FULLPATH];
STATIC TEXT objectPath[SIZE_FULLPATH], objectName[SIZE_FULLPATH];
STATIC TEXT tempStr[512];
STATIC WORD args[MAX_PARSER_ARGS];
STATIC TEXT argstr[MAXSCANDEPTH];
STATIC TEXT str1[MAXSCANDEPTH];
STATIC TEXT str2[MAXSCANDEPTH];

/**** functions ****/

/***************************************************************************/
/************************** S C R I P T T A L K ****************************/
/***************************************************************************/

/******** AnimFuncSpec() ********/
/*
 * fpw1 = MP_BigFile   fpw2 = MP_TempScript
 * Be careful with destPath: it can be NULL!
 *
 */

BOOL AnimFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*  ANIM "path", cycle, rot, speed, diskanim, loopIt, wipe, speed, thick, vari  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==11)
	{
		// READ
		GetNumericalArgs(PR,args);
		ScriptToStr(PR->argString[1],scrStr);

//KPrintF("ANIM [%s]\n",scrStr);
		if ( !strncmp(scrStr,"\"@(",3) )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		RemoveQuotes(scrStr);
		strcpy(oriPath,scrStr);
		SplitFullPath(oriPath,objectPath,objectName);
		if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		// CHECK
		if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
		{
			UA_MakeFullPath(destPath,MSM_ANIM_PATH,tempStr);
			UA_MakeFullPath(tempStr,objectName,fullPath);
			StrToScript(fullPath,scrStr);
			if ( TheFileIsNotThere(oriPath) )
				return(FALSE);
			if ( mode==MODE_UPLOAD )
				fileSize = GetFileSize(oriPath);
		}
		else if ( mode==MODE_FIND_MISSING )
		{
			if ( FindNameInList(oriPath,objectName,SM_ANIM) )
			{
				StrToScript(oriPath,scrStr);
				fileSize = GetFileSize(oriPath);
			}
		}
		else if ( mode==MODE_CALC_SIZE )
			fileSize = GetFileSize(oriPath);

		// WRITE
		fprintf(fpw2, "%s", whiteSpcs);
		fprintf(fpw2, "%s \"%s\", %s, %d, %d, %s, %s, %d, %d, %d, %d\n",
						commands[PR->commandCode],
						scrStr,
						PR->argString[2],
						*(args+2), *(args+3),
						PR->argString[5], PR->argString[6],
						*(args+6), *(args+7), *(args+8), *(args+9) );

		// MP_BigFile

		MyTurnAssignIntoDir(oriPath);
		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
	}

	return(TRUE);
}

/******** ArexxDosFuncSpec() ********/

BOOL ArexxDosFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
											STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*  AREXX "port name", 'commandstring', "comment", mode, type  or  */
	/*  AREXX "path", "comment", mode, type  */

	/*  DOS 'commandstring', "comment", mode, type, stack  or  */
	/*  DOS "path", "comment", mode, type, stack  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==5 || PR->numArgs==6)
	{
		if (	(PR->commandCode == SM_AREXX && !strcmpi(PR->argString[5],"COMMAND") ) ||
					(PR->commandCode == SM_DOS && !strcmpi(PR->argString[4],"COMMAND") ) )
		{
			if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
				return(FALSE);
		}
		else if ( (PR->commandCode == SM_AREXX && PR->numArgs==5 &&
							 !strcmpi(PR->argString[4],"SCRIPT")) ||
							(PR->commandCode == SM_DOS && !strcmpi(PR->argString[4],"SCRIPT")) )
		{
			// READ
			GetNumericalArgs(PR,args);
			ScriptToStr(PR->argString[1],scrStr);

			if ( !strncmp(scrStr,"\"@(",3) )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			RemoveQuotes(scrStr);
			strcpy(oriPath,scrStr);
			SplitFullPath(oriPath,objectPath,objectName);
			if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			// CHECK
			if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
			{
				if ( PR->commandCode == SM_AREXX )
					UA_MakeFullPath(destPath,MSM_AREXX_PATH,tempStr);
				else if ( PR->commandCode == SM_DOS )
					UA_MakeFullPath(destPath,MSM_DOS_PATH,tempStr);
				UA_MakeFullPath(tempStr,objectName,fullPath);
				StrToScript(fullPath,scrStr);
				if ( TheFileIsNotThere(oriPath) )
					return(FALSE);
				if ( mode==MODE_UPLOAD )
					fileSize = GetFileSize(oriPath);
			}
			else if ( mode==MODE_FIND_MISSING )
			{
				if ( FindNameInList(oriPath,objectName,PR->commandCode) )
				{
					StrToScript(oriPath, scrStr);
					fileSize = GetFileSize(oriPath);
				}
			}
			else if ( mode==MODE_CALC_SIZE )
				fileSize = GetFileSize(oriPath);

			// WRITE
			fprintf(fpw2, "%s", whiteSpcs);
			if ( PR->commandCode == SM_DOS )
				fprintf(fpw2, "%s \"%s\", %s, %s, %s, %s\n",
								commands[PR->commandCode],
								scrStr,
								PR->argString[2], PR->argString[3], PR->argString[4], PR->argString[5]);
			else
				fprintf(fpw2, "%s \"%s\", %s, %s, %s\n",
								commands[PR->commandCode],
								scrStr,
								PR->argString[2], PR->argString[3], PR->argString[4]);

			// MP_BigFile

			MyTurnAssignIntoDir(oriPath);
			if ( mode==MODE_UPLOAD )
				RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			else
				fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		}
	}

	return(TRUE);
}

/******** SoundFuncSpec() ********/

BOOL SoundFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*  SOUND "path", STOP/WAIT/LOOP  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==3)
	{
		if ( !strcmpi(PR->argString[2],"STOP") )
		{
			if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
				return(FALSE);
		}
		else
		{
			// READ
			GetNumericalArgs(PR,args);
			ScriptToStr(PR->argString[1],scrStr);

//KPrintF("SOUND [%s]\n",scrStr);
			if ( !strncmp(scrStr,"\"@(",3) )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			RemoveQuotes(scrStr);
			strcpy(oriPath,scrStr);
			SplitFullPath(oriPath,objectPath,objectName);
			if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			// CHECK
			if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
			{
				UA_MakeFullPath(destPath,MSM_SOUND_PATH,tempStr);
				UA_MakeFullPath(tempStr,objectName,fullPath);
				StrToScript(fullPath,scrStr);
				if ( TheFileIsNotThere(oriPath) )
					return(FALSE);
				if ( mode==MODE_UPLOAD )
					fileSize = GetFileSize(oriPath);
			}
			else if ( mode==MODE_FIND_MISSING )
			{
				if ( FindNameInList(oriPath,objectName,SM_SOUND) )
				{
					StrToScript(oriPath,scrStr);
					fileSize = GetFileSize(oriPath);
				}
			}
			else if ( mode==MODE_CALC_SIZE )
				fileSize = GetFileSize(oriPath);

			// WRITE
			fprintf(fpw2, "%s", whiteSpcs);
			fprintf(fpw2, "%s \"%s\", %s\n",
							commands[PR->commandCode],
							scrStr,
							PR->argString[2]);

			// MP_BigFile
	
			MyTurnAssignIntoDir(oriPath);
			if ( mode==MODE_UPLOAD )
				RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			else
				fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		}
	}

	return(TRUE);
}

/******** PageFuncSpec() ********/

BOOL PageFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize, type;

	/*  PAGE "path", cycle, wipe, speed, thick, vari, type 							*/
	/*																								1 = pagetalk doc	*/
	/*																								2 = IFF ILBM 			*/

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==8)
	{
		// READ
		GetNumericalArgs(PR,args);
		ScriptToStr(PR->argString[1],scrStr);

//KPrintF("PAGE [%s]\n",scrStr);
		if ( !strncmp(scrStr,"\"@(",3) )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		RemoveQuotes(scrStr);
		strcpy(oriPath,scrStr);
		SplitFullPath(oriPath,objectPath,objectName);
		if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		// CHECK
		if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
		{
			if ( *(args+6) == 1 )
				UA_MakeFullPath(destPath,MSM_PAGE_PATH,tempStr);
			else
				UA_MakeFullPath(destPath,MSM_IFF_PATH,tempStr);
			UA_MakeFullPath(tempStr,objectName,fullPath);
			StrToScript(fullPath,scrStr);
			if ( TheFileIsNotThere(oriPath) )
				return(FALSE);
			if ( mode==MODE_UPLOAD )
				fileSize = GetFileSize(oriPath);
		}
		else if ( mode==MODE_FIND_MISSING )
		{
			if ( *(args+6) == 1 )
				type = SM_PAGE;
			else
				type = SM_IFF;
			if ( FindNameInList(oriPath,objectName,type) )
			{
				StrToScript(oriPath,scrStr);
				fileSize = GetFileSize(oriPath);
			}
		}
		else if ( mode==MODE_CALC_SIZE )
			fileSize = GetFileSize(oriPath);

		// WRITE
		fprintf(fpw2, "%s", whiteSpcs);
		fprintf(fpw2, "%s \"%s\", %s, %d, %d, %d, %d, %d\n",
						commands[PR->commandCode],
						scrStr,
						PR->argString[2],
						*(args+2), *(args+3), *(args+4), *(args+5), *(args+6) );

		if ( *(args+6) == 1 )	// PAGE
		{
			// MP_BigFile -> notice .TMP -> This is created by ParsePageFile
			if ( mode==MODE_UPLOAD )
				RA_BigFile(fpw1,"%s \"%s.TMP\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			else
				fprintf(fpw1,"%s \"%s.TMP\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));

			// DO SAME KIND OF CHECKS ON FILES EMBEDDED IN PAGETALK FILE
			if ( !ParsePageFile(fpw1,whiteSpcs,buffer,mode,destPath,oriPath) )
			{
				if ( logfile )
				{
					//fprintf(logfile,"P2 ");
					fprintf(logfile,oriPath);
					fprintf(logfile,"\n");
				}
				return(FALSE);
			}
		}
		else	// IFF
		{
			// MP_BigFile
			MyTurnAssignIntoDir(oriPath);
			if ( mode==MODE_UPLOAD )
				RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_IFF],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			else
				fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_IFF],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		}
	}

	return(TRUE);
}

/******** BinaryFuncSpec() ********/

BOOL BinaryFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath)
{
	if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
		return(FALSE);
	return(TRUE);
}

/******** MailFuncSpec() ********/

BOOL MailFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
	if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
		return(FALSE);
	return(TRUE);
}

/******** XappFuncSpec() ********/

BOOL XappFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize, fileSize2;
int arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8;
int arg9,arg10,arg11,arg12,arg13,arg14,arg15,arg16;
BOOL parseStr1=FALSE, parseStr2=FALSE;
TEXT fullPath2[SIZE_FULLPATH], oriPath2[SIZE_FULLPATH];

	/*  XAPP "tool name", "comment", ['argument string'], [defer/cont]  */

	fullPath[0] = '\0';
	fullPath2[0] = '\0';
	oriPath[0] = '\0';
	oriPath2[0] = '\0';
	fileSize = 0;
	fileSize2 = 0;

	if (PR->numArgs>=3 && PR->numArgs<=5)
	{
		// READ
		GetNumericalArgs(PR,args);

		if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"CDXL\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s %d %d %d", str1, &arg1, &arg2, &arg3);
			parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"IV-24\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%d %s %d", &arg1, str1, &arg2);
			if ( arg1==3 )
				parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"MIDI\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s %d", str1, &arg1);
			parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"SAMPLE\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%d %s %d %d %d %d %d %d %d", &arg1, str1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8);
			if ( arg1==1 )
				parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"STUDIO16\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s %s %d %d %d %d", str1, str2, &arg1, &arg2, &arg3, &arg4);
			parseStr1 = TRUE;
			parseStr2 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"TOCCATA\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%d %s %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d",
							&arg1, str1, &arg2, &arg3, &arg4, str2, &arg5, &arg6, &arg7, 
							&arg8, &arg9, &arg10, &arg11, &arg12, &arg13, &arg14, &arg15, &arg16);
			parseStr1 = TRUE;
			parseStr2 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"CREDITROLL\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s", str1);
			parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"RESOURCE\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s %d", str1, &arg1);
			parseStr1 = TRUE;
		}
		else if ( PR->numArgs>3 && !strcmpi(PR->argString[1],"\"NEWSCRIPT\"") )
		{
			strcpy(argstr,PR->argString[3]);
			RemoveQuotes(argstr);
			sscanf(argstr,"%s", str1);
			parseStr1 = TRUE;
		}

		if ( parseStr1 )
		{
			ScriptToStr(str1,scrStr);

//KPrintF("XAPP 1 [%s]\n",scrStr);
		if ( !strncmp(scrStr,"@(",2) )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			RemoveQuotes(scrStr);
			strcpy(oriPath,scrStr);
			SplitFullPath(oriPath,objectPath,objectName);
			if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			// CHECK
			if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
			{
				UA_MakeFullPath(destPath,MSM_XAPP_PATH,tempStr);
				UA_MakeFullPath(tempStr,objectName,fullPath);
				StrToScript(fullPath,scrStr);
				if ( TheFileIsNotThere(oriPath) )
					return(FALSE);
				if ( mode==MODE_UPLOAD )
					fileSize = GetFileSize(oriPath);
			}
			else if ( mode==MODE_FIND_MISSING )
			{
				if ( FindNameInList(oriPath,objectName,SM_XAPP) )
				{
					StrToScript(oriPath,scrStr);
					fileSize = GetFileSize(oriPath);
				}
			}
			else if ( mode==MODE_CALC_SIZE )
				fileSize = GetFileSize(oriPath);

			strcpy(str1,scrStr);
		}

		if ( parseStr2 )
		{
			ScriptToStr(str2,scrStr);

//KPrintF("XAPP 2 [%s]\n",scrStr);
		if ( !strncmp(scrStr,"@(",2) )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			RemoveQuotes(scrStr);
			strcpy(oriPath2,scrStr);
			SplitFullPath(oriPath2,objectPath,objectName);
			if ( objectPath[0]=='\0' || objectName[0]=='\0' || objectName[0]=='@' )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			// CHECK
			if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
			{
				UA_MakeFullPath(destPath,MSM_XAPP_PATH,tempStr);
				UA_MakeFullPath(tempStr,objectName,fullPath2);
				StrToScript(fullPath2,scrStr);
				if ( TheFileIsNotThere(oriPath2) )
					return(FALSE);
				if ( mode==MODE_UPLOAD )
					fileSize2 = GetFileSize(oriPath2);
			}
			else if ( mode==MODE_FIND_MISSING )
			{
				if ( FindNameInList(oriPath2,objectName,SM_XAPP) )
				{
					StrToScript(oriPath2,scrStr);
					fileSize2 = GetFileSize(oriPath2);
				}
			}
			else if ( mode==MODE_CALC_SIZE )
				fileSize2 = GetFileSize(oriPath2);

			strcpy(str2,scrStr);
		}

		if ( !parseStr1 && !parseStr2 )
		{
			if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
				return(FALSE);
		}
		else
		{	
			// WRITE
			fprintf(fpw2, "%s", whiteSpcs);

			if ( !strcmpi(PR->argString[1],"\"CDXL\"") )
				sprintf(argstr,"\"\\\"%s\\\" %d %d %d\"", str1, arg1, arg2, arg3);
			else if ( !strcmpi(PR->argString[1],"\"IV-24\"") )
				sprintf(argstr,"\"%d \\\"%s\\\" %d\"", arg1, str1, arg2);
			else if ( !strcmpi(PR->argString[1],"\"MIDI\"") )
				sprintf(argstr,"\"\\\"%s\\\" %d\"", str1, arg1);
			else if ( !strcmpi(PR->argString[1],"\"SAMPLE\"") )
				sprintf(argstr,"\"%d \\\"%s\\\" %d %d %d %d %d %d %d\"", arg1, str1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
			else if ( !strcmpi(PR->argString[1],"\"STUDIO16\"") )
				sprintf(argstr,"\"\\\"%s\\\" \\\"%s\\\" %d %d %d %d\"", str1, str2, arg1, arg2, arg3, arg4);
			else if ( !strcmpi(PR->argString[1],"\"TOCCATA\"") )
				sprintf(argstr,"\"%d \\\"%s\\\" %d %d %d \\\"%s\\\" %d %d %d %d %d %d %d %d %d %d %d %d\"",
								arg1, str1, arg2, arg3, arg4, str2, arg5, arg6, arg7, 
								arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16);
			else if ( !strcmpi(PR->argString[1],"\"CREDITROLL\"") )
				sprintf(argstr,"\"\\\"%s\\\"\"", str1);
			else if ( !strcmpi(PR->argString[1],"\"RESOURCE\"") )
				sprintf(argstr,"\"\\\"%s\\\" %d\"", str1, arg1);
			else if ( !strcmpi(PR->argString[1],"\"NEWSCRIPT\"") )
				sprintf(argstr,"\"\\\"%s\\\"\"", str1);

			if ( PR->numArgs==4 )
				fprintf(fpw2, "%s %s, %s, %s\n",	commands[PR->commandCode], PR->argString[1], PR->argString[2], argstr );
			else
				fprintf(fpw2, "%s %s, %s, %s, %s\n",	commands[PR->commandCode], PR->argString[1], PR->argString[2], argstr, PR->argString[4] );

			// MP_BigFile

			if ( parseStr1 )
			{
				MyTurnAssignIntoDir(oriPath);
				if ( mode==MODE_UPLOAD )
					RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
				else
					fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			}

			if ( parseStr2 )
			{
				MyTurnAssignIntoDir(oriPath2);
				if ( mode==MODE_UPLOAD )
					RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath2,fullPath2,fileSize2,GetDateStamp(oriPath2));
				else
					fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath2,fullPath2,fileSize2,GetDateStamp(oriPath2));
			}
		}
	}

	return(TRUE);
}

/***************************************************************************/
/***************************** P A G E T A L K *****************************/
/***************************************************************************/

/******** ClipFuncSpec() ********/

BOOL ClipFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*  CLIP "path", x, y, w, h, scale  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==7)
	{
		// READ
		GetNumericalArgs(PR,args);
		ScriptToStr(PR->argString[1],scrStr);
		RemoveQuotes(scrStr);
		strcpy(oriPath,scrStr);
		SplitFullPath(oriPath,objectPath,objectName);
		if ( objectPath[0]=='\0' || objectName[0]=='\0' )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		// CHECK
		if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
		{
			UA_MakeFullPath(destPath,MSM_CLIP_PATH,tempStr);
			UA_MakeFullPath(tempStr,objectName,fullPath);
			StrToScript(fullPath,scrStr);
			if ( TheFileIsNotThere(oriPath) )
				return(FALSE);
			if ( mode==MODE_UPLOAD )
				fileSize = GetFileSize(oriPath);
		}
		else if ( mode==MODE_FIND_MISSING )
		{
			if ( FindNameInList(oriPath,objectName,SM_CLIP) )
			{
				StrToScript(oriPath,scrStr);
				fileSize = GetFileSize(oriPath);
			}
		}
		else if ( mode==MODE_CALC_SIZE )
			fileSize = GetFileSize(oriPath);

		// WRITE
		fprintf(fpw2, "%s", whiteSpcs);
		fprintf(fpw2, "%s \"%s\",%d,%d,%d,%d,%s\n",
						commands[PR->commandCode],
						scrStr,
						*(args+1), *(args+2), *(args+3), *(args+4),
						PR->argString[6] );

		// MP_BigFile

		MyTurnAssignIntoDir(oriPath);
		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
	}

	return(TRUE);
}

/******** CrawlFuncSpec() ********/

BOOL CrawlFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*	CRAWL "fontname", size, speed, color              */
	/*	CRAWL "fontname", size, speed, color, "filename"  */
	/*				(no .font)																	*/

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==5 || PR->numArgs==6)
	{
		// READ
		GetNumericalArgs(PR,args);
		if ( PR->numArgs==6 )
		{
			ScriptToStr(PR->argString[5],scrStr);
			RemoveQuotes(scrStr);
			strcpy(oriPath,scrStr);
			SplitFullPath(oriPath,objectPath,objectName);
			if ( objectPath[0]=='\0' || objectName[0]=='\0' )
				return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

			// CHECK
			if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
			{
				UA_MakeFullPath(destPath,MSM_CRAWL_PATH,tempStr);
				UA_MakeFullPath(tempStr,objectName,fullPath);
				StrToScript(fullPath,scrStr);
				if ( TheFileIsNotThere(oriPath) )
					return(FALSE);
				if ( mode==MODE_UPLOAD )
					fileSize = GetFileSize(oriPath);
			}
			else if ( mode==MODE_FIND_MISSING )
			{
				if ( FindNameInList(oriPath,objectName,SM_CRAWL) )
				{
					StrToScript(oriPath,scrStr);
					fileSize = GetFileSize(oriPath);
				}
			}
			else if ( mode==MODE_CALC_SIZE )
				fileSize = GetFileSize(oriPath);
		}

		// WRITE
		fprintf(fpw2, "%s", whiteSpcs);
		if ( PR->numArgs==6 )
			fprintf(fpw2, "%s %s, %d, %d, %d, \"%s\"\n",
							commands[PR->commandCode],
							PR->argString[1],
							*(args+1), *(args+2), *(args+3),
							scrStr );
		else
			fprintf(fpw2, "%s %s, %d, %d, %d\n",
							commands[PR->commandCode],
							PR->argString[1],
							*(args+1), *(args+2), *(args+3) );

		// MP_BigFile

		if ( PR->numArgs==6 )
		{
			MyTurnAssignIntoDir(oriPath);
			if ( mode==MODE_UPLOAD )
				RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
			else
				fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		}

		// NOW TAKE CARE OF THE FONTS -> CREATE A STRING LIKE THE 'TEXT' COMMAND 

		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);
		sprintf(str1, "^f%s^^s%d^", scrStr, *(args+1));
		if ( !CheckThisFont(PR,fpw1,fpw2,whiteSpcs,buffer,mode,destPath,str1) )
			return(FALSE);
	}
	else
	{
		if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
			return(FALSE);
	}

	return(TRUE);
}

/******** TextFuncSpec() ********/

BOOL TextFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*	TEXT "string"  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==2)
	{
		// READ
		ScriptToStr(PR->argString[1], scrStr);
		RemoveQuotes(scrStr);

		strcpy(str1,scrStr);	// added str1
		if ( !CheckThisFont(PR,fpw1,fpw2,whiteSpcs,buffer,mode,destPath,str1) )	// added str1
			return(FALSE);

		// WRITE
		if ( !JustWriteIt(PR,fpw2,whiteSpcs,buffer) )
			return(FALSE);
	}

	return(TRUE);
}

/******** ClipAnimFuncSpec() ********/

BOOL ClipAnimFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
											STRPTR buffer, BYTE mode, STRPTR destPath)
{
int fileSize;

	/*  CLIPANIM "path", x, y, w, h, fps, loops, disk  */

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;

	if (PR->numArgs==9)
	{
		// READ
		GetNumericalArgs(PR,args);
		ScriptToStr(PR->argString[1],scrStr);
		RemoveQuotes(scrStr);
		strcpy(oriPath,scrStr);
		SplitFullPath(oriPath,objectPath,objectName);
		if ( objectPath[0]=='\0' || objectName[0]=='\0' )
			return( JustWriteIt(PR,fpw2,whiteSpcs,buffer) );

		// CHECK
		if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
		{
			UA_MakeFullPath(destPath,MSM_ANIM_PATH,tempStr);	// CLIPANIMS TO ANIM DRAWER
			UA_MakeFullPath(tempStr,objectName,fullPath);
			StrToScript(fullPath,scrStr);
			if ( TheFileIsNotThere(oriPath) )
				return(FALSE);
			if ( mode==MODE_UPLOAD )
				fileSize = GetFileSize(oriPath);
		}
		else if ( mode==MODE_FIND_MISSING )
		{
			if ( FindNameInList(oriPath,objectName,SM_CLIPANIM) )
			{
				StrToScript(oriPath,scrStr);
				fileSize = GetFileSize(oriPath);
			}
		}
		else if ( mode==MODE_CALC_SIZE )
			fileSize = GetFileSize(oriPath);

		// WRITE
		fprintf(fpw2, "%s", whiteSpcs);
		fprintf(fpw2, "%s \"%s\",%d,%d,%d,%d,%d,%d,%d\n",
						commands[PR->commandCode],
						scrStr,
						*(args+1), *(args+2), *(args+3), *(args+4), *(args+5), *(args+6), *(args+7)
						);

		// MP_BigFile

		MyTurnAssignIntoDir(oriPath);
		if ( mode==MODE_UPLOAD )
			RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
		else
			fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[PR->commandCode],oriPath,fullPath,fileSize,GetDateStamp(oriPath));
	}

	return(TRUE);
}

/******** FontFuncSpec() ********/

BOOL FontFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
	return(TRUE);
}

/******** SystemFuncSpec() ********/

BOOL SystemFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath)
{
	return(TRUE);
}

/******** IFFFuncSpec() ********/

BOOL IFFFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath)
{
	return(TRUE);
}

/******** ScriptFuncSpec() ********/

BOOL ScriptFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath)
{
	return(TRUE);
}

/******** JustWriteIt() ********/

BOOL JustWriteIt(struct ParseRecord *PR, FILE *fpw, STRPTR whiteSpcs, STRPTR buffer)
{
	if (fpw)
		fprintf(fpw,"%s%s\n",whiteSpcs,buffer);
	return(TRUE);
}

/******** SplitFullPath() ********/
/*
 * If fullPath doesn't contain a ':'	-> path becomes current path ( getcd() )
 *																		-> filename becomes fullPath
 *
 * If fullPath is shorter than 3 characters:
 *																		-> path and filename become '\0'
 *
 * If fullPath is enclosed by '"':		-> double quotes are removed before parsing
 *
 * After stripping, path will get the UA_ValidatePath treatment
 *
 */

void SplitFullPath(STRPTR fullPath, STRPTR path, STRPTR filename)
{
int i,j,pos,len;

	path[0] = '\0';
	filename[0] = '\0';

	if ( !fullPath )
	{
		return;
	}

	// check if a volume name is present

	if ( UA_FindString(fullPath, ":")==-1 )
	{
		getcd(0, path);
		stccpy(filename, fullPath, SIZE_FILENAME);
		return;
	}

	// check minimal length

	if (strlen(fullPath)<3)
	{
		return;
	}

	// remove trailing and ending double quotes

	if ( fullPath[0] == '\"' )
	{
		len = strlen(fullPath)-2;
		for(i=0; i<len; i++)
			fullPath[i] = fullPath[i+1];
		fullPath[len] = '\0';
	}

	// split path and filename

	pos=-1;
	len=strlen(fullPath);
	for(j=len-1; j>0; j--)
	{
		if ( fullPath[j] == ':' || fullPath[j] == '/' )
		{
			pos=j;
			break;
		}
	}

	if ( pos != -1 )
	{
		stccpy(path, fullPath, pos+2);
		stccpy(filename, &fullPath[j+1], len-pos+1);

		UA_ValidatePath(path);
	}
}

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** GetFileSize() ********/

int GetFileSize(STRPTR path)
{
int rc;
struct stat st;

	rc = stat(path,&st);
	if (rc==0)
		return( st.st_size );
	else
	{
		// "Warning: unable to find file %s"
		sprintf(reportStr,msgs[Msg_SM_32-1],path);
		Report(reportStr);
		return( 0 );
	}
}

/******** CheckThisFont() ********/

BOOL CheckThisFont(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath, STRPTR fontStr )
{
int i, actualLen, fontsize, fileSize;
TEXT fontname[60];

	if ( mode==MODE_FIND_MISSING )
		return(TRUE);	// leave right away - no business here

	fullPath[0] = '\0';
	oriPath[0] = '\0';
	fileSize = 0;
	scrStr[0] = '\0';

	actualLen = strlen(fontStr);
	if ( actualLen<2 )
		return(TRUE);

	fontname[0] = '\0';
	for(i=0; i<actualLen; i++)
	{
		if ( fontStr[i]=='^' )						// start of command sequence
		{
			i++;
			if ( fontStr[i]=='f' )					// font name follows
			{
				i++;
				FetchString(&fontStr[i], fontname, 50, &i);
			}
			else if ( fontStr[i]=='s' )		// size follows
			{
				i++;
				FetchInteger(&fontStr[i], &fontsize, &i);
				if ( fontname[0] != '\0' )
				{
					// Create eg "fonts:mediapoint.font"

					UA_MakeFullPath("fonts:",fontname,fullPath);
					strcat(fullPath,".font");
					fileSize = GetFileSize(fullPath);
					StrToScript(fullPath,oriPath);

					// Create eg "dest:Fonts/mediapoint.font"

					if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
					{
						UA_MakeFullPath(destPath,MSM_FONT_PATH,tempStr);
						UA_MakeFullPath(tempStr,fontname,fullPath);
						strcat(fullPath,".font");
						StrToScript(fullPath,scrStr);
					}

					MyTurnAssignIntoDir(oriPath);
					if ( !IsFontInList(oriPath) )
					{
						AddToFontsList(oriPath);
						if ( mode==MODE_UPLOAD )
							RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_FONT],oriPath,scrStr,fileSize,GetDateStamp(oriPath));
						else
							fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_FONT],oriPath,scrStr,fileSize,GetDateStamp(oriPath));
					}

					// Create eg "fonts:mediapoint/10"

					sprintf(fullPath, "fonts:%s/%d", fontname, fontsize);
					fileSize = GetFileSize(fullPath);
					StrToScript(fullPath,oriPath);

					// Create eg "dest:Fonts/mediapoint/10"

					if ( mode==MODE_CREATE_RUNTIME || mode==MODE_UPLOAD )
					{
						sprintf(fullPath, "%s/%s/%d", MSM_FONT_PATH, fontname, fontsize);
						UA_MakeFullPath(destPath,fullPath,tempStr);
						StrToScript(tempStr,scrStr);
					}

					MyTurnAssignIntoDir(oriPath);
					if ( !IsFontInList(oriPath) )
					{
						AddToFontsList(oriPath);
						if ( mode==MODE_UPLOAD )
							RA_BigFile(fpw1,"%s \"%s\" \"%s\"",commands[SM_FONT],oriPath,scrStr,fileSize,GetDateStamp(oriPath));
						else
							fprintf(fpw1,"%s \"%s\" \"%s\" %d %d\n",commands[SM_FONT],oriPath,scrStr,fileSize,GetDateStamp(oriPath));
					}
				}
			}
		}
	}

	return(TRUE);
}

/******** FetchString() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <fGaramond, buffer points to the G
 *
 */

void FetchString(STRPTR buffer, STRPTR dest, int max, int *count)
{
int i;

	for(i=0; i<max; i++)
	{
		if ( buffer[i]=='^' )
		{
			dest[i]=0;
			return;
		}
		dest[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** FetchInteger() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <s64, buffer points to the 6
 *
 */

void FetchInteger(STRPTR buffer, int *dest, int *count)
{
int i;
TEXT str[10];

	for(i=0; i<10; i++)
	{
		if ( buffer[i]=='^' )
		{
			str[i]=0;
			sscanf(str, "%d", dest);
			return;
		}
		str[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** TheFileIsNotThere() ********/

BOOL TheFileIsNotThere(STRPTR path)
{
struct FileHandle *FL;

	FL = (struct FileHandle *)Lock((STRPTR)path, (LONG)ACCESS_READ);
	if (FL)
	{
		UnLock((BPTR)FL);
		return(FALSE);
	}
	return(TRUE);
}

/******** E O F ********/
