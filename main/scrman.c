#include "nb:pre.h"

/**** externals ****/

extern struct RendezVousRecord rvrec;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Library *medialinkLibBase;
extern struct FileListInfo FLI;
extern char *scriptCommands[];
extern struct Document scriptDoc;
extern struct Window *scriptWindow;
extern UBYTE **msgs;   
extern struct ObjectInfo ObjectRecord;
extern TEXT *dir_scripts;

/**** functions ****/

/******** ShowSM() ********/

BOOL ShowSM(STRPTR scriptPath, STRPTR scriptName)
{
BOOL retval, loaded=FALSE;
TEXT fullPath[SIZE_FULLPATH], name[SIZE_FILENAME], path1[SIZE_FULLPATH], path2[SIZE_FULLPATH];
struct Document tempDoc; 
struct FileLock *FL;

	strcpy(name,scriptName);
	strcat(name,".MP");
	UA_MakeFullPath(scriptPath,name,fullPath);
	rvrec.aPtr = (APTR)fullPath;

	SetSpriteOfActWdw(SPRITE_BUSY);

	if (!WriteScript(scriptPath, name, &(ObjectRecord.scriptSIR), scriptCommands))
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		//UA_WarnUser(-1);
		return(FALSE);
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);

	if ( GetDevicesAndAssigns() )
		rvrec.aPtrTwo = (APTR)&FLI;
	else
		rvrec.aPtrTwo = NULL;

	retval = FALSE;
	if ( !LoadModule("ScriptManager", &retval) )
		UA_WarnUser(-1);

	if ( rvrec.aPtrTwo )
		FreeDevicesAndAssigns();

	if ( retval )
	{
		if (	UA_OpenGenericWindow(	scriptWindow, TRUE, TRUE,
																msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																QUESTION_ICON, msgs[Msg_SM_41-1], TRUE, NULL) )
		{
			SetSpriteOfActWdw(SPRITE_BUSY);

			strcpy(tempDoc.title, scriptDoc.title);	// keep name
			strcpy(name,scriptName);	// original script name

			// Close old doc
			scriptDoc.modified = FALSE;
			if ( scriptDoc.opened )
				do_Close(&scriptDoc,FALSE);

			// Read new doc
			strcpy(scriptDoc.path, "ram:");
			strcpy(scriptDoc.title, tempDoc.title);	// restore name
			strcat(scriptDoc.title, ".MP.fixed");

			// Try to minimize '.MP.MP.fixed' etc. problems by checking if
			// scriptname.fixed is not yet in ram. If not, rename it.

			UA_MakeFullPath("ram:",name,path1);
			strcat(path1,".fixed");
			FL = (struct FileLock *)Lock((STRPTR)path1, (LONG)ACCESS_READ);
			if (!FL)
			{
				UA_MakeFullPath(scriptDoc.path, scriptDoc.title, path2);
				Rename(path2, path1);	// oldname, newname
				strcpy(scriptDoc.path, "ram:");
				sprintf(scriptDoc.title, "%s.fixed", name);
				strcat(path1,".info");
				strcat(path2,".info");
				Rename(path2, path1);
			}
			else
				UnLock((BPTR)FL);

			// Open script

			OpenDocumentProc(&scriptDoc);
			if ( !ReadScript(scriptDoc.path, scriptDoc.title, scriptCommands) )
			{
				strcpy(scriptDoc.path,dir_scripts);
				strcpy(scriptDoc.title,msgs[Msg_Untitled-1]);
				ReadScript(scriptDoc.path, scriptDoc.title, scriptCommands);
			}
			SetOpenedStateScriptMenuItems();
			scriptDoc.opened = TRUE;
			loaded = TRUE;

			SetSpriteOfActWdw(SPRITE_NORMAL);
		}
	}

	DeleteFile(fullPath);
	strcat(fullPath,".info");
	DeleteFile(fullPath);

	return(loaded);
}

/******** E O F ********/
