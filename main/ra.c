#include "nb:pre.h"

/**** externals ****/

extern struct RendezVousRecord rvrec;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Library *medialinkLibBase;
extern char *scriptCommands[];
extern struct Document scriptDoc;
extern struct Window *scriptWindow;
extern UBYTE **msgs;   
extern struct ObjectInfo ObjectRecord;
extern TEXT *dir_system;
extern struct FileListInfo FLI;

/**** functions ****/

/******** ShowRA() ********/

void ShowRA(STRPTR scriptPath, STRPTR scriptName, int mode)
{
BOOL retval;
TEXT fullPath[SIZE_FULLPATH], name[SIZE_FILENAME];

#if 0
	strcpy(name,scriptName);
	strcat(name,".MP");
	UA_MakeFullPath(scriptPath,name,fullPath);
	rvrec.aPtr = (APTR)fullPath;

	SetSpriteOfActWdw(SPRITE_BUSY);

	if (!WriteScript(scriptPath, name, &(ObjectRecord.scriptSIR), scriptCommands))
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(-1);
		return;
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);

	if ( GetDevicesAndAssigns() )
		rvrec.aPtrTwo = (APTR)&FLI;
	else
		rvrec.aPtrTwo = NULL;
#endif

	rvrec.aPtrThree = dir_system;
	rvrec.aLong = mode;

	retval = FALSE;
	if ( !LoadModule("RemoteAccess", &retval) )
		UA_WarnUser(-1);

#if 0
	if ( rvrec.aPtrTwo )
		FreeDevicesAndAssigns();

	DeleteFile(fullPath);
	strcat(fullPath,".info");
	DeleteFile(fullPath);
#endif
}

/******** E O F ********/
