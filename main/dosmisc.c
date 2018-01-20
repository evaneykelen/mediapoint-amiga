/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** defines ****/

#define DEVICES 1
#define ASSIGNS 2
#define SCANSIZE 512L

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct FileInfoBlock *FIB;
extern struct Screen *scriptScreen;
extern struct FileListInfo FLI;
extern struct FileListInfo buffer_FLI;
extern UBYTE **msgs;
extern UBYTE *homeDirs;												
extern struct Library *medialinkLibBase;

/**** globals ****/

static char sortx[256], sortw[256];
static TEXT bufferPath[SIZE_FULLPATH] = { "\0" };
static int bufferOpts=-1;

/**** functions ****/

/******** OpenDir() ********/

BOOL OpenDir(STRPTR path, int opts)
{
struct FileLock	*fileLock;
int	numEntries,i,success;
TEXT fullPath[SIZE_FULLPATH];
ULONG type;

	/**** init vars ****/

	FLI.fileList			= NULL;
	FLI.numFiles			= 0;
	FLI.selectionList	= NULL;

	/**** allocate memory ****/

	FLI.fileList = (UBYTE *)AllocMem(FILELISTSIZE, MEMF_ANY | MEMF_CLEAR);
	if (FLI.fileList == NULL)
	{
		UA_WarnUser(74);
		return(FALSE);
	}

	FLI.selectionList = (UBYTE *)AllocMem(MAX_FILE_LIST_ENTRIES, MEMF_ANY | MEMF_CLEAR);
	if (FLI.selectionList == NULL)
	{
		UA_WarnUser(75);
		return(FALSE);
	}

	if ( buffer_FLI.fileList!=NULL && !strcmpi(bufferPath,path) && bufferOpts==opts )
	{
		CopyMem(buffer_FLI.fileList, FLI.fileList, FILELISTSIZE);
		FLI.numFiles = buffer_FLI.numFiles;
		return(TRUE);
	}

	/**** get hold of dir to scan ****/

	if (path[0]=='\0' || strlen(path)==0)
		strcpy(path,"RAM:");

	UA_ValidatePath(path);

	fileLock = (struct FileLock *)Lock((STRPTR)path, (LONG)ACCESS_READ);
	if (fileLock == NULL)
	{
		/**** go for a 2nd change ****/

		strcpy(path,"RAM:");

		fileLock = (struct FileLock *)Lock((STRPTR)path, (LONG)ACCESS_READ);
		if (fileLock == NULL)
		{
			UA_WarnUser(80);
			CloseDir();
			return(FALSE);
		}
	}

	if ((BOOL)Examine((BPTR)fileLock, (struct FileInfoBlock *)FIB) == FALSE)
	{
		UnLock((BPTR)fileLock);
		CloseDir();
		UA_WarnUser(81);
		return(FALSE);
	}

	/**** scan dir and throw out unwanted file type if so specified ****/

	numEntries=0;
	for (i=0; i<MAX_FILE_LIST_ENTRIES; i++)
	{
		success = ExNext((BPTR)fileLock, (struct FileInfoBlock *)FIB);
		if (!success)
			break;

		if ( (opts & DIR_OPT_NODIRS) && (FIB->fib_DirEntryType > 0) )	// dir
			;
		else if ( (opts&DIR_OPT_NOINFO) && (UA_FindString(FIB->fib_FileName,".info")!=-1) )
			;
		else if ( opts&DIR_OPT_ONLYINFO )
		{
			if ( (FIB->fib_DirEntryType < 0) && (UA_FindString(FIB->fib_FileName,".info")!=-1) )
			{
				if ( strlen(FIB->fib_FileName) > 5 )	// skip the infamous '.info' file
				{
					stccpy(FLI.fileList+numEntries*SIZE_FILENAME, FIB->fib_FileName, SIZE_FILENAME);
					*(FLI.fileList+numEntries*SIZE_FILENAME+strlen(FIB->fib_FileName)-5) = '\0';
					numEntries++;
				}
			}
		}
		else
		{
			stccpy(fullPath, path, SIZE_FULLPATH);	// path is validated
			strcat(fullPath, FIB->fib_FileName);

			if ( !(opts & DIR_OPT_ONLYDIRS) && (FIB->fib_DirEntryType < 0))	// file
			{
				if ( opts & DIR_OPT_ALL )
				{
					sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
					*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
					numEntries++;
				}
				else
				{
					type = checkFileType(fullPath, NULL);

					// Possibilities are: ILBM | PAGE
					//										ILBM | PAGE | ANIM
					//										ANIM

					if (	((opts & DIR_OPT_ILBM			) && type==ILBM							) ||
								((opts & DIR_OPT_ANIM			) && type==ANIM							) ||
								((opts & DIR_OPT_THUMBS		) && type==PAGETALK_ID_1		) ||
								((opts & DIR_OPT_SCRIPTS	) && type==SCRIPTTALK_ID_1	) ||
								((opts & DIR_OPT_MIDI			) && type==MThd							)	||
								((opts & DIR_OPT_MAUD			) && type==MAUD							)	)
					{
						sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
						*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
						numEntries++;
					}

					else if ( (opts & DIR_OPT_MUSIC) &&
										(	type!=ILBM && type!=ANIM && type!=PAGETALK_ID_1 &&
											type!=SCRIPTTALK_ID_1 && type!=MThd && type!=MAUD) )
					{
						sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
						*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
						numEntries++;
 					}
					else if ( (opts & DIR_OPT_SAMPLE) &&
										(	type!=ILBM && type!=ANIM && type!=PAGETALK_ID_1 &&
											type!=SCRIPTTALK_ID_1 && type!=MThd && type!=MAUD) )
					{
						sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
						*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
						numEntries++;
 					}
					else if (	(opts & DIR_OPT_ILBM) && (opts & DIR_OPT_THUMBS) &&
										(opts & DIR_OPT_ANIM) &&
										(type==ILBM || type==ANIM || type==PAGETALK_ID_1) )
					{
						sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
						*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
						numEntries++;
 					}
					else if (	(opts & DIR_OPT_ILBM) && (opts & DIR_OPT_THUMBS) &&
										(type==ILBM || type==PAGETALK_ID_1) )
					{
						sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)FILE_PRECODE, FIB->fib_FileName);
						*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
						numEntries++;
					}
				}
			}
			else if (FIB->fib_DirEntryType > 0)	// dir
			{
				sprintf(FLI.fileList+numEntries*SIZE_FILENAME, "%c%s", (UBYTE)DIR_PRECODE, FIB->fib_FileName);
				*(FLI.fileList+(numEntries*SIZE_FILENAME)+SIZE_FILENAME-1) = '\0';
				numEntries++;
			}
		}
	}

	/**** unlock scanned dir ****/

	UnLock((BPTR)fileLock);

	/**** alphabetically sort the dir list ****/

	if (numEntries > 1)
		QuickSortList(0, numEntries-1, FLI.fileList, (int)SIZE_FILENAME);

	FLI.numFiles = numEntries;

	/**** create buffer ****/

	if ( buffer_FLI.fileList != NULL )
	{
		/**** free old buffer ****/
		FreeMem(buffer_FLI.fileList, (buffer_FLI.numFiles+1)*SIZE_FILENAME);
		buffer_FLI.fileList=NULL;
		buffer_FLI.numFiles = 0;
		bufferPath[0]='\0';
		bufferOpts=-1;
	}

	/**** alloc new buffer ****/

	buffer_FLI.fileList = (UBYTE *)AllocMem((FLI.numFiles+1)*SIZE_FILENAME,
																					MEMF_ANY | MEMF_CLEAR);
	if ( buffer_FLI.fileList != NULL )
	{
		buffer_FLI.numFiles = FLI.numFiles;
		CopyMem(FLI.fileList, buffer_FLI.fileList, FLI.numFiles*SIZE_FILENAME);
		strcpy(bufferPath,path);
		bufferOpts = opts;
	}

	return(TRUE);
}

/******** CloseDir() ********/

void CloseDir(void)
{
	if (FLI.selectionList != NULL)
		FreeMem(FLI.selectionList, MAX_FILE_LIST_ENTRIES);

	if (FLI.fileList != NULL)
		FreeMem(FLI.fileList, FILELISTSIZE);

	FLI.selectionList=NULL;
	FLI.fileList=NULL;
	FLI.numFiles=0;
}

/******** QuickSortList() ********/

void QuickSortList(int l, int r, UBYTE *ptr, int lineSize)
{
int i=l, j=r;

	stccpy(sortx, ptr+((l+r)/2)*lineSize, 256);

	do
	{
		while(StringCmp(ptr+i*lineSize, sortx) < 0)
			i++;
		while(StringCmp(ptr+j*lineSize, sortx) > 0)
			j--;
		if (i > j)
			break;
		stccpy(sortw, ptr+i*lineSize, 256);
		stccpy(ptr+i*lineSize, ptr+j*lineSize, lineSize);
		stccpy(ptr+j*lineSize, sortw, lineSize);
	}
	while (++i <= --j);

	if (l<j)
		QuickSortList(l, j, ptr, lineSize);
	if (i<r)
		QuickSortList(i, r, ptr, lineSize);
}

/******** StringCmp() ********/

int StringCmp(STRPTR p, STRPTR q)
{
char x[SIZE_FILENAME],
		 w[SIZE_FILENAME];
int i;
char *xp, *wp;

	stccpy(x, p, SIZE_FILENAME);
	stccpy(w, q, SIZE_FILENAME);

	for(i=0; i<SIZE_FILENAME; i++)
	{
		*(x+i) = tolower(*(x+i));
		*(w+i) = tolower(*(w+i));
	}

	xp = x;
	wp = w;

	while( *xp != '\0' && *xp == *wp )
	{
		xp++;
		wp++;
	}

	return *xp-*wp;
}

#ifndef USED_FOR_PLAYER

/******** GetDevicesAndAssigns() ********/

BOOL GetDevicesAndAssigns(void)
{
//struct List disks;
//struct Node *disk;
int i,j;
UBYTE *tempbuff=NULL;
struct DosList *dlist, *newdlist;
char *str;

	if ( FLI.deviceList )
	{
		// Nested calling of this funcion.
		// Don't alloc twice
		// Leave the show

		return(TRUE);
	}

	/**** init vars ****/

	FLI.assignList	= NULL;
	FLI.numAssigns	= 0;
	FLI.deviceList	= NULL;
	FLI.numDevices	= 0;

	/**** allocate temporary buffer */

	tempbuff = (UBYTE *)AllocMem(SIZE_FILENAME*1024, MEMF_ANY | MEMF_CLEAR);
	if ( !tempbuff )
	{
		UA_WarnUser(76);
		return(FALSE);
	}

	/**** get devices ****/

	dlist = LockDosList(LDF_VOLUMES|LDF_READ);
	while( newdlist = NextDosEntry(dlist,LDF_VOLUMES|LDF_READ) )
	{
		str = BADDR(newdlist->dol_Name);
		str++;
		sprintf(tempbuff+FLI.numDevices*SIZE_FILENAME, "%s:", str);
		dlist = newdlist;
		FLI.numDevices += 1;
		if ( FLI.numDevices > 1023 )
			break;
	}
	if ( dlist )
		UnLockDosList(LDF_VOLUMES|LDF_READ);

	/**** allocate real memory and copy tempbuff into it ****/

	FLI.deviceList = (UBYTE *)AllocMem(FLI.numDevices*SIZE_FILENAME, MEMF_ANY|MEMF_CLEAR);
	if (FLI.deviceList==NULL)
	{
		UA_WarnUser(77);
		FreeMem(tempbuff, SIZE_FILENAME*1024);
		return(FALSE);	/* fail when no devices are available */
	}
	CopyMem(tempbuff, FLI.deviceList, FLI.numDevices*SIZE_FILENAME);

	/**** get assigns ****/

	dlist = LockDosList(LDF_ASSIGNS|LDF_READ);
	while( newdlist = NextDosEntry(dlist,LDF_ASSIGNS|LDF_READ) )
	{
		str = BADDR(newdlist->dol_Name);
		str++;
		sprintf(tempbuff+FLI.numAssigns*SIZE_FILENAME, "%s:", str);
		dlist = newdlist;
		FLI.numAssigns += 1;
		if ( FLI.numAssigns > 1023 )
			break;
	}
	if ( dlist )
		UnLockDosList(LDF_ASSIGNS|LDF_READ);

	/**** allocate real memory and copy tempbuff into it ****/

	FLI.assignList = (UBYTE *)AllocMem(FLI.numAssigns*SIZE_FILENAME, MEMF_ANY|MEMF_CLEAR);
	if (FLI.assignList==NULL)
	{
		UA_WarnUser(77);
		FreeMem(tempbuff, SIZE_FILENAME*1024);
		return(FALSE);	/* fail when no assigns are available */
	}
	CopyMem(tempbuff, FLI.assignList, FLI.numAssigns*SIZE_FILENAME);

	/**** free temp buff ****/

	FreeMem(tempbuff, SIZE_FILENAME*1024);

	/**** get homes ****/

	FLI.homeList = homeDirs;												
	j=0;
	for(i=0; i<10; i++)
	{
		if ( *(FLI.homeList+i*SIZE_FILENAME) != '\0' )
			j++;
	}
	FLI.numHomes = j;												

	/**** sort lists ****/

	if ( FLI.numDevices > 1 )
		QuickSortList(0, FLI.numDevices-1, FLI.deviceList, SIZE_FILENAME);

	if ( FLI.numAssigns > 1 )
		QuickSortList(0, FLI.numAssigns-1, FLI.assignList, SIZE_FILENAME);

	return(TRUE);
}

/******** FreeDevicesAndAssigns() ********/

void FreeDevicesAndAssigns(void)
{
	if ( FLI.deviceList )
	{
		FreeMem(FLI.deviceList, FLI.numDevices*SIZE_FILENAME);
		FreeMem(FLI.assignList, FLI.numAssigns*SIZE_FILENAME);
	}
	FLI.deviceList=NULL;
	FLI.assignList=NULL;
}

#endif

/******** checkFileType() ********/
/*
 * if fileName==NULL, path is already a fullPath.
 *
 */

ULONG checkFileType(STRPTR path, STRPTR fileName)
{
TEXT fullPath[SIZE_FULLPATH];
struct FileHandle *FH;
ULONG type[10];

	if (fileName!=NULL)
		UA_MakeFullPath(path, fileName, fullPath);
	else
		stccpy(fullPath, path, SIZE_FULLPATH);

	FH = (struct FileHandle *)Open((STRPTR)fullPath, (LONG)MODE_OLDFILE);
	if (FH!=NULL)
	{
		Read((BPTR)FH, type, 12);
		Close((BPTR)FH);
		if (      type[0]==FORM            && type[2]==ILBM )
			return(ILBM);
		else if ( type[0]==FORM            && type[2]==ANIM )
			return(ANIM);
		else if ( type[0]==PAGETALK_ID_1   && type[1]==PAGETALK_ID_2 )
			return(PAGETALK_ID_1);
		else if ( type[0]==SCRIPTTALK_ID_1 && type[1]==SCRIPTTALK_ID_2 )
			return(SCRIPTTALK_ID_1);
		else if ( type[0]==MThd )
			return(MThd);
		else if ( type[0]==FORM            && type[2]==MAUD )
			return(MAUD);
	}
	return(0L);
}

/******** GetParentOf() ********/

void GetParentOf(STRPTR path)
{
int xx[16];
int i, end;
TEXT tempPath[SIZE_FULLPATH];
char *p;

	if (path[strlen(path)-1]=='/')
		path[strlen(path)-1]='\0';	/* strip trailing slash */
	
	p = (char *)strchr(path, '/');
	if (p==NULL)
	{
		p = (char *)strchr(path, ':');
		if (p!=NULL)
			*(p+1)='\0';

		UA_ValidatePath(path);

		return;
	}

	stccpy(tempPath, path, SIZE_FULLPATH);
	if (stspfp(tempPath, xx)==0)
	{
		i=0;
		while(xx[i]!=-1 && i<16)
			i++;
		if (i>0)
		{
			end = xx[i-1]-1;
			*(path+end) = '\0';
		}
	}

	UA_ValidatePath(path);
}

#ifndef USED_FOR_PLAYER

/******** ExecuteTextEditor() ********/

BOOL ExecuteTextEditor(struct ScriptNodeRecord *this_node)
{
struct FileHandle	*DOSHandle;
BOOL retVal=FALSE;
TEXT buf[256];
TEXT fullPath[256];
TEXT fullPath2[256];

	UA_MakeFullPath(this_node->objectPath, this_node->objectName, fullPath);

	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if (DOSHandle != NULL)
	{
		/**** config has line like: TEXTEDITOR "c:ed %s -STICKY" ****/

		sprintf(fullPath2, "\"%s\"", fullPath);	// put double quotes around path

		sprintf(buf, CPrefs.textEditor, fullPath2);

		OpenWorkBench();

		if (WBenchToFront())
			retVal = (BOOL)Execute((UBYTE *)buf, (BPTR)NULL, (BPTR)DOSHandle);
		else
			UA_WarnUser(82);	// no wb

		Close((BPTR)DOSHandle);
	}

	ScreenToFront(scriptScreen);

	if (!retVal)
		Message(msgs[Msg_UnableToExecute-1], CPrefs.textEditor);

	return(retVal);
}

#endif

/******** GetInfoOnForm() ********/
/*
 * Scans 512 bytes deep in search for typeID
 */

BOOL GetInfoOnForm(	STRPTR path, ULONG typeID, UBYTE *storage, int storageSize,
										struct FileLock *lock )
{
struct FileHandle *FH;
UWORD *ptr;
UWORD i;
ULONG l, *lptr;
UWORD HW, LW;

	l = typeID & 0xffff0000L;
	l = l >> 16;
	HW = l;

	l = typeID & 0x0000ffffL;
	LW = (UWORD)l;

	if ( lock && GfxBase->LibNode.lib_Version >= 36 )
		FH = (struct FileHandle *)OpenFromLock((BPTR)lock);
	else
		FH = (struct FileHandle *)Open((STRPTR)path, (LONG)MODE_OLDFILE);

	if (FH!=NULL)
	{
		ptr = (UWORD *)AllocMem(SCANSIZE, MEMF_ANY | MEMF_CLEAR);
		if (ptr!=NULL)
		{
			Read((BPTR)FH, ptr, SCANSIZE);
			Close((BPTR)FH);

			lptr = (ULONG *)ptr;

			i=0;
			if (*lptr==FORM)
			{
				while(i<SCANSIZE)
				{
					if (*(ptr+i)==HW && *(ptr+i+1)==LW)
					{
						CopyMem(ptr+i+4, storage, storageSize);

						if (ptr!=NULL)
							FreeMem(ptr, SCANSIZE);

						return(TRUE);
					}
					i++;
				}
			}
	
			if (ptr!=NULL)
				FreeMem(ptr, SCANSIZE);
			ptr=NULL;

			return(FALSE);
		}
		else
		{
			Close((BPTR)FH);
			return(FALSE);
		}
	}
	else
		return(FALSE);
}

#if 0
/******** ExecuteMediaLinkStartUp() ********/

void ExecuteMediaLinkStartUp(void)
{
struct FileHandle	*DOSHandle;

	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if (DOSHandle != NULL)
	{
		OpenWorkBench();

		if (WBenchToFront())
			Execute((UBYTE *)EX_ML_STARTUP, (BPTR)NULL, (BPTR)DOSHandle);
		else
			UA_WarnUser(83);	/* no wb */

		Close((BPTR)DOSHandle);
	}
}
#endif

/******** UpdateDirCache() ********/
/*
 * After a page or script write to a cached dir, the next time the user
 * sees this dir, it should be reloaded. This functions does this.
 *
 */

void UpdateDirCache(STRPTR path)
{
	if ( buffer_FLI.fileList!=NULL && !strcmpi(bufferPath,path) )
	{
		FreeMem(buffer_FLI.fileList, (buffer_FLI.numFiles+1)*SIZE_FILENAME);
		buffer_FLI.fileList=NULL;
		buffer_FLI.numFiles = 0;
		bufferPath[0]='\0';
		bufferOpts=-1;
	}
}

/******** E O F ********/
