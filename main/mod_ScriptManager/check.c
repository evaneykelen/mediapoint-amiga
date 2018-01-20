#include "nb:pre.h"
#include "protos.h"

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

/******** CheckThisEntry() ********/

BOOL CheckThisEntry(STRPTR buffer, struct ScriptInfoRecord *SIR)
{
TEXT newPath[SIZE_FULLPATH];
BOOL storeNew=FALSE;

	switch( SIR->currentNode->nodeType )
	{
		case TALK_ANIM:
			storeNew = CheckThisPath(	SIR->currentNode->objectPath,
																SIR->currentNode->objectName,
																newPath);
			break;			
		case TALK_AREXX:
			break;			
		case TALK_USERAPPLIC:
			break;			
		case TALK_DOS:
			break;			
		case TALK_SOUND:
			break;			
		case TALK_PAGE:
			break;			
		case TALK_BINARY:
			break;			
		case TALK_MAIL:
			break;			
	}

	if ( storeNew )
	{
		strcpy(SIR->currentNode->objectPath, newPath);
	}

	return(TRUE);
}

/******** CheckThisPath ********/

BOOL CheckThisPath(STRPTR oldPath, STRPTR oldName, STRPTR newPath)
{
struct FileLock	*FL;
TEXT fullPath[SIZE_FULLPATH];
BOOL changed=FALSE;

	UA_MakeFullPath(oldPath, oldName, fullPath);

	FL = (struct FileLock *)Lock((STRPTR)fullPath, (LONG)ACCESS_READ);
	if (FL == NULL)
	{
		fullPath[0] = '\0';
		if ( FindNameInList(oldName,fullPath) )
			PrintString(fullPath);
		else
			PrintString("ANIM path doesn't exist");
	}
	else
		UnLock((BPTR)FL);

	return(changed);
}

/******** E O F ********/
