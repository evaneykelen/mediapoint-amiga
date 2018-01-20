/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

void TranslateGR(struct GadgetRecord *GR, UBYTE **msgs);

/**** globals ****/

UBYTE **msgs;
ULONG msgsSize=0L;
ULONG *TextMem=NULL, TextMemSize=0L;
ULONG languagesAvailable=0L;

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct FileInfoBlock *FIB;
extern struct RendezVousRecord rvrec;
extern TEXT *dir_system;
extern struct Library *medialinkLibBase;

/**** gadgets ****/

extern struct GadgetRecord Entry_GR[];
extern struct GadgetRecord *kept_Script_GR;

/**** functions ****/

/******** TranslateApp() ********/

BOOL TranslateApp(BOOL firstTime)
{
	if ( !LoadTranslationFile(firstTime) )
		return(FALSE);

	UA_TranslateGR(Entry_GR, msgs);
	if ( !firstTime )
	{
		UA_TranslateGR(kept_Script_GR, msgs);
	}

	return(TRUE);
}

/******** LoadTranslationFile() ********/

BOOL LoadTranslationFile(BOOL firstTime)
{
struct FileHandle *FH;
ULONG *mem2;
int i;
struct FileInfoBlock *FIB;
struct FileLock *FL;
TEXT textsPath[SIZE_FULLPATH];

	/**** create "work:mediapoint/xapps/system/texts.english" ****/

	sprintf(textsPath, "%stexts.%s", dir_system, CPrefs.lanExtension);

	/**** alloc FIB mem ****/

	FIB = (struct FileInfoBlock *)AllocMem(
													(LONG)sizeof(struct FileInfoBlock), MEMF_ANY | MEMF_CLEAR);
	if (FIB == NULL)
		return(FALSE);

	/**** get file size ****/

	FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
	if (FL==NULL)
	{
		FreeMem(FIB, (LONG)sizeof(struct FileInfoBlock));
		return(FALSE);
	}

	if ( !Examine((BPTR)FL,(struct FileInfoBlock *)FIB) )
	{
		UnLock((BPTR)FL);
		FreeMem(FIB, (LONG)sizeof(struct FileInfoBlock));
		return(FALSE);
	}
	else
		UnLock((BPTR)FL);

	TextMemSize = FIB->fib_Size;
	TextMemSize += 20;	// some leeway

	FreeMem(FIB, (LONG)sizeof(struct FileInfoBlock));

	/**** allocate mem for language file ****/

	TextMem = (ULONG *)AllocMem(TextMemSize, MEMF_CLEAR);
	if ( TextMem==NULL )
		return(FALSE);
	mem2 = TextMem;	// store for later use

	/**** open language file ****/

	FH = (struct FileHandle *)Open((STRPTR)textsPath, (LONG)MODE_OLDFILE);
	if (FH==NULL)
	{
		FreeMem(TextMem, TextMemSize);
		return(FALSE);
	}

	/**** read and close language file ****/

	Read((BPTR)FH, TextMem, TextMemSize-10);
	Close((BPTR)FH);

	/**** count number of offsets ****/

	mem2 += (*TextMem/4);		// mem2 now points to 0x00000004 (first offset)
	msgsSize=0L;
	while ( *mem2++ != 0 )
		msgsSize++;

	msgsSize *= 4;
	msgsSize += 20;	// some leeway

	/**** allocate memory for offset table ****/

	msgs = (char **)AllocMem(msgsSize,MEMF_CLEAR);
	if (msgs==NULL)
	{
		FreeMem(TextMem, TextMemSize);
		return(FALSE);
	}

	/**** calculate offsets ****/

	mem2 = TextMem;
	mem2 += (*TextMem/4);		// mem2 now points to 0x00000004 (first offset)
	i=0;
	while ( *mem2 != 0 )
	{
		msgs[i] = (UBYTE *)TextMem+*mem2;
		mem2++;
		i++;
	}

	/**** find available languages ****/

	if ( firstTime )
	{
		languagesAvailable=0L;

		sprintf(textsPath, "%stexts.English", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_English;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Nederlands", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Nederlands;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Deutsch", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Deutsch;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Français", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Français;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Español", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Español;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Italiano", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Italiano;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Português", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Português;
			UnLock((BPTR)FL);
		}
	
		sprintf(textsPath, "%stexts.Dansk", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Dansk;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Svenska", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Svenska;
			UnLock((BPTR)FL);
		}

		sprintf(textsPath, "%stexts.Norsk", dir_system);
		FL = (struct FileLock *)Lock((STRPTR)textsPath,(LONG)ACCESS_READ);
		if (FL!=NULL)
		{
			languagesAvailable |= LAN_Norsk;
			UnLock((BPTR)FL);
		}
	}

	return(TRUE);
}

/******** UnLoadTranslationFile() ********/

void UnLoadTranslationFile(void)
{
	if (TextMemSize!=0L)
		FreeMem(TextMem, TextMemSize);
	if (msgsSize!=0L)
		FreeMem(msgs, msgsSize);
}

/******** E O F ********/
