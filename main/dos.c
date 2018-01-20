/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct FileInfoBlock *FIB;
extern TEXT *dir_system;
extern struct RendezVousRecord rvrec;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** findFullPath() ********/

void findFullPath(struct FileLock *oriFL, STRPTR str)
{
struct FileLock *FL2;
TEXT revStr[100];
char *p;
int i;
struct FileLock *FL;

	FL = (struct FileLock *)DupLock((BPTR)oriFL);

	str[0] = '\0';
	do
	{
		Examine((BPTR)FL, (struct FileInfoBlock *)FIB);
		stccpy(revStr, FIB->fib_FileName, 100);
		p = strrev(revStr);
		strcat(str, p);
		strcat(str, "/");
		FL2 = (struct FileLock *)ParentDir((BPTR)FL);
		UnLock((BPTR)FL);
		FL = FL2;
	}
	while( FL );

	str[ strlen(str)-1 ] = '\0'; /* strip last slash */

	p = strrev(str);	/* reverse whole string */

	for(i=0; i<256; i++)	/* replace first slash by colon */
	{
		if ( str[i] == '/' || str[i] == '\0' )
		{
			str[i] = ':';
			break;
		}
	}

	UA_ValidatePath(str);
}

#ifndef USED_FOR_PLAYER

/******** LoadModule() ********/
/*
 * Loadseg()s modules like About
 *
 */

BOOL LoadModule(STRPTR moduleName, BOOL *returnCode)
{
BPTR MySegment;
struct Process *MyProc;
struct WBStartup MLStartup;
struct MsgPort *MyReplyPort;
ULONG ReplySignal;
struct WBStartup *ReplyMsg;
TEXT fullPath[SIZE_FULLPATH], portName[100];
struct WBArg MLArg;

	SetSpriteOfActWdw(SPRITE_BUSY);

	rvrec.returnCode = *returnCode;	// it is sometimes used to pass info

	rvrec.msgs = msgs;   

	sprintf(portName, "%s.port", moduleName);

	MyReplyPort = (struct MsgPort *)CreatePort(portName, 0);

	/**** create e.g. "work:medialink/xapps/system/moduleName" ****/

	sprintf(fullPath, "%s%s", dir_system, moduleName);

	MySegment = LoadSeg(fullPath);
	if (MySegment != NULL)
	{
		MyProc = (struct Process *)((ULONG)CreateProc(moduleName,5,
															MySegment, 20000)-(ULONG)sizeof(struct Task));
		if (MyProc != NULL)
		{
			MLStartup.sm_Process = &MyProc->pr_MsgPort;
			MLStartup.sm_Segment = MySegment;
			MLStartup.sm_NumArgs = 1;
			MLStartup.sm_ArgList = &MLArg;
			MLStartup.sm_ToolWindow = NULL;
			MLStartup.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
			MLStartup.sm_Message.mn_Length = sizeof(struct WBStartup);
			MLStartup.sm_Message.mn_ReplyPort = MyReplyPort;

			MLArg.wa_Lock = (BPTR)CPrefs.appdirLock;
			MLArg.wa_Name = portName;

			PutMsg(&MyProc->pr_MsgPort, (struct Message *)&MLStartup);

			ReplySignal = 1 << MyReplyPort->mp_SigBit;
			Wait(ReplySignal);
			ReplyMsg = (struct WBStartup *)GetMsg(MyReplyPort);

			*returnCode = rvrec.returnCode;

			UnLoadSeg(ReplyMsg->sm_Segment);
			DeletePort(MyReplyPort);
		}
		else
		{
			UA_WarnUser(84);
			SetSpriteOfActWdw(SPRITE_NORMAL);
			return(FALSE);
		}
	}
	else
	{
		UA_WarnUser(85);
		SetSpriteOfActWdw(SPRITE_NORMAL);
		return(FALSE);
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);

	return(TRUE);
}

#endif

/******** UA_MakeFullPath() ********/
/*
 * Makes from 'anim:' and 'ape' the string 'anim:ape' and
 * makes from 'anim:test/pics/hires' and 'ape' the
 * string 'anim:test/pics/hires/ape'. Even more, the string
 * 'anim:test/pics/hires/' (note the slash) and 'ape' is correctly
 * converted (no double slashes). This seems trivial but this routine
 * should be used instead of trying to connect them yourself.
 *
 */

void UA_MakeFullPath(STRPTR path, STRPTR name, STRPTR answer)
{
int len;

	len = strlen(path);
	if ( path[len-1]==':' || path[len-1]=='/' )
		sprintf(answer, "%s%s", path, name);
	else if (path!=NULL && path[0]!='\0')
		sprintf(answer, "%s/%s", path, name);
	else
		strcpy(answer, name);	// survived strcpy
}

/******** E O F ********/
