#include "nb:pre.h"
#include "protos.h"
#include "structs.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern UBYTE **msgs;
extern struct UserApplicInfo UAI;
extern FILE *logfile;
extern TEXT reportStr[256];

/**** functions ****/

/******** CopyFilesInBigFile() ********/

BOOL CopyFilesInBigFile(struct ParseRecord *PR, BOOL systemFiles)
{
int filesize, i;
LONG freesize;
TEXT source[SIZE_FULLPATH];
TEXT source2[SIZE_FULLPATH];
TEXT dest[SIZE_FULLPATH];
TEXT destDevice[32];
TEXT filename[SIZE_FILENAME];
TEXT path[SIZE_FULLPATH];
TEXT cmdStr[512];
BOOL ok_copy=TRUE;
struct FileLock *FL;
LONG vs;

	// Get source and dest paths

	ScriptToStr(PR->argString[1],source);
	RemoveQuotes(source);

	SplitFullPath(source,path,filename);
	if ( path[0]=='\0' || filename[0]=='\0' )
		return(FALSE);

	ScriptToStr(PR->argString[2],dest);
	RemoveQuotes(dest);

	SplitFullPath(dest,path,filename);
	if ( path[0]=='\0' || filename[0]=='\0' )
		return(FALSE);

	// Get volume name of dest

	destDevice[0]='\0';
	for(i=0; i<32; i++)
	{
		if ( dest[i]!=':' )
		{
			destDevice[i] = source[i];
			destDevice[i+1] = '\0';
		}
		else
			break;
	}
	//strcat(destDevice,":");
	if ( !strnicmp(destDevice,"ram",3) )
		strcpy(destDevice,"RAM:");

//KPrintF("[%s] [%s] [%s]\n",destDevice,dest,source);

	// Check if source is not available and dest is available.
	// This happens if a .TMP already has been copied and deleted
	// and in a later stage the file must be copied again.

	FL = (struct FileLock *)Lock((STRPTR)source, (LONG)ACCESS_READ);
	if (!FL)
	{
		FL = (struct FileLock *)Lock((STRPTR)dest, (LONG)ACCESS_READ);
		if (FL)
		{
			UnLock((BPTR)FL);
			return(TRUE);	// file's already there
		}
	}
	else
		UnLock((BPTR)FL);

	// Ask for the next disk (if necessary)

	if ( !InsertRightDisk(source,destDevice,msgs[Msg_SM_31-1]) )
		return(FALSE);

	// Get file size

	filesize = GetFileSize(source);

	if ( filesize==0 )
	{
		if ( logfile )
		{
			if ( PR->commandCode!=SM_PAGE )	// reported elsewhere
			{
				//fprintf(logfile,"CF ");
				fprintf(logfile,source);
				fprintf(logfile,"\n");
			}
		}
		return(FALSE);
	}

	// Get volume name of dest

	destDevice[0]='\0';
	for(i=0; i<32; i++)
	{
		if ( dest[i]!=':' )
		{
			destDevice[i] = dest[i];
			destDevice[i+1] = '\0';
		}
		else
			break;
	}
	strcat(destDevice,":");
	if ( !strnicmp(destDevice,"ram",3) )
		strcpy(destDevice,"RAM:");

	// Check if file is not larger than nominal volume size

	vs = GetVolumeSize(destDevice);
	if ( vs < filesize )
	{
		// "There's a file larger than the dest"
		sprintf(reportStr,"%s (%d/%d)", msgs[Msg_SM_36-1], vs, filesize);
		Report(reportStr);
		return(FALSE);
	}

	// Keep asking for empty disk until request satisfied or cancel chosen

	freesize = GetFreeSpace(destDevice);
	while( freesize < filesize )
	{
		if ( !WaitForDisk(UAI.userWindow,destDevice,msgs[Msg_SM_31-1]) )
			return(FALSE);
		freesize = GetFreeSpace(destDevice);
	}

	// Create destination path

	SplitFullPath(dest,path,filename);
	UA_ValidatePath(path);
	CreateLongDir(path);

	// Check if file needs to be copied

	FL = (struct FileLock *)Lock((STRPTR)dest, (LONG)ACCESS_READ);
	if (FL)
	{
		UnLock((BPTR)FL);

		// Check if date stamps are equeal. For pages check real file not .TMP

		strcpy(source2,source);
		if ( PR->commandCode==SM_PAGE )
		{	
			i = strlen(source);	
			if ( i>3 && !strnicmp( &source[i-4],".TMP",4 ) )
			{
				// Strip .TMP
				source2[i-4]='\0';
			}
		}

		if ( !DiffTime(source2,dest) )	// file exists AND no time difference
		{
			if ( PR->commandCode==SM_PAGE )
			{	
				i = strlen(source);	
				if ( i>3 && !strnicmp( &source[i-4],".TMP",4 ) )
				{
					// Delete unwanted and temporary .TMP file
					DeleteFile(source);
				}
			}
			return(TRUE);	// file already exists and of same vintage - why bother?
		}
	}

	// Notify user that we're copying

	sprintf(reportStr,msgs[Msg_SM_29-1],filename,path);
	Report(reportStr);

	// Copy file

	{ // START -- NEW COPY LOOP
		BOOL NotCopied=TRUE;
		int to=10;
		while( NotCopied && to>0 )
		{
			to--;
			sprintf(cmdStr, "copy \"%s\" to \"%s\"", source, dest);
			ok_copy = ExecuteCommand(cmdStr);
			if ( !ok_copy )
			{
				sprintf(reportStr,msgs[Msg_SM_28-1],path);
				Report(reportStr);
			}
			NotCopied = CheckSourceAndDest(source,dest);
		}
	} // END -- NEW COPY LOOP

	// Copy file.info

	strcat(source,".info");
	strcat(dest,".info");
	if ( !TheFileIsNotThere(source) )
	{
		{ // START -- NEW COPY LOOP
			BOOL NotCopied=TRUE;
			int to=10;
			while( NotCopied && to>0 )
			{
				to--;
				sprintf(cmdStr, "copy \"%s\" to \"%s\"", source, dest);
				ok_copy = ExecuteCommand(cmdStr);
				if ( !ok_copy )
				{
					sprintf(reportStr,msgs[Msg_SM_28-1],path);
					Report(reportStr);
				}
				NotCopied = CheckSourceAndDest(source,dest);
			}
		} // END -- NEW COPY LOOP
	}

	ScriptToStr(PR->argString[1],source);
	RemoveQuotes(source);

	ScriptToStr(PR->argString[2],dest);
	RemoveQuotes(dest);

	i = strlen(source);	
	if ( PR->commandCode==SM_PAGE && i>3 )
	{	
		if ( !strnicmp( &source[i-4],".TMP",4 ) )
		{
			// Delete unwanted and temporary .TMP file

			DeleteFile(source);

			// Strip .TMP and copy .info again

			source[i-4]='\0';

			// Copy file.info

			strcat(source,".info");
			strcat(dest,".info");
			if ( !TheFileIsNotThere(source) )
			{
				{ // START -- NEW COPY LOOP
					BOOL NotCopied=TRUE;
					int to=10;
					while( NotCopied && to>0 )
					{
						to--;
						sprintf(cmdStr, "copy \"%s\" to \"%s\"", source, dest);
						ok_copy = ExecuteCommand(cmdStr);
						if ( !ok_copy )
						{
							sprintf(reportStr,msgs[Msg_SM_28-1],path);
							Report(reportStr);
						}
						NotCopied = CheckSourceAndDest(source,dest);
					}
				} // END -- NEW COPY LOOP
			}
		}
	}

	return( ok_copy );
}

/******** GetFreeSpace() ********/

LONG GetFreeSpace(STRPTR dest)
{
int error;
struct InfoData __aligned info;

	if ( !strnicmp(dest,"ram",3) )	// this is RAM: or RAM DISK:
		return( (LONG)AvailMem(MEMF_PUBLIC) );
	else
	{
		error = getdfs(dest,&info);	
		if ( error!=0 )
			return(0);
		else
			return( (info.id_NumBlocks - info.id_NumBlocksUsed) * info.id_BytesPerBlock ); 
	}
}

/******** ExecuteCommand() ********/

BOOL ExecuteCommand(STRPTR cmd)
{
struct FileHandle *DOSHandle;
BOOL retVal=FALSE;

	//printser(cmd); printser("\n");

	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if ( DOSHandle )
	{
		retVal = (BOOL)Execute((UBYTE *)cmd, (BPTR)NULL, (BPTR)DOSHandle);
		Close((BPTR)DOSHandle);
	}
	return(retVal);
}

/******** CreateLongDir() ********/
 
void CreateLongDir(STRPTR newPath)
{
int xx[16],i,j,len;
char str1[256], str2[256];
struct FileLock *FL;

	FL = (struct FileLock *)Lock((STRPTR)newPath, (LONG)ACCESS_READ);
	if (FL)
	{
		UnLock((BPTR)FL);
		return;	// dir already exists - why bother?
	}

	strcpy(str1,newPath);
	len=strlen(str1);
	j=0;
	xx[j++]=0;
	xx[j]=-1;
	for(i=0; i<len; i++)
	{
		if ( str1[i]=='/' )
		{
			str1[i]='\0';
			xx[j++]=i+1;
			xx[j]=-1;
		}
	}
	if ( xx[1]==-1 )
		return;

	i=0;
	str2[0] = '\0';
	while( xx[i+1] != -1 )
	{
		if ( strlen(str2) > 0 )
			strcat(str2,"/");
		strcat(str2,&str1[ xx[i] ]);
		FL = (struct FileLock *)CreateDir( str2 );
		if (FL)
		{
			UnLock((BPTR)FL);

			// "Creating directory %s"
			sprintf(reportStr,msgs[Msg_SM_30-1],newPath);
			Report(reportStr);
		}
		i++;
	}
}

/******** GetVolumeSize() ********/

LONG GetVolumeSize(STRPTR path)
{
int error;
struct InfoData __aligned info;

	if ( !strnicmp(path,"ram",3) )	// this is RAM: or RAM DISK:
		return( (LONG)AvailMem(MEMF_PUBLIC) );
	else
	{
		error = getdfs(path,&info);	
		if ( error!=0 )
			return(0);
		else
			return( info.id_NumBlocks * info.id_BytesPerBlock ); 
	}
}

/******** DiffTime() ********/

BOOL DiffTime(STRPTR file1, STRPTR file2)
{
BPTR lock1;
BPTR lock2;
struct FileInfoBlock __aligned fib1;
struct FileInfoBlock __aligned fib2;
BOOL retval=TRUE;

	if( lock1=Lock(file1,SHARED_LOCK) )
	{
		if( lock2=Lock(file2,SHARED_LOCK) )
		{
			if( Examine(lock1,&fib1) && Examine(lock2,&fib2) )
			{
				if( 0 <= CompareDates(&fib1.fib_Date, &fib2.fib_Date) )
				{
					retval=FALSE;
				}
			}
			UnLock(lock2);
		}
		UnLock(lock1);
	}
	return( retval );
}

/******** CheckSourceAndDest() ********/
/*
 * Returns FALSE if copied OK (nice case of negativism!)
 *
 */

BOOL CheckSourceAndDest(STRPTR source, STRPTR dest)
{
BPTR lock1;
BPTR lock2;
struct FileInfoBlock __aligned fib1;
struct FileInfoBlock __aligned fib2;
BOOL retval=TRUE;

	if( lock1=Lock(source,SHARED_LOCK) )
	{
		if( lock2=Lock(dest,SHARED_LOCK) )
		{
			if( Examine(lock1,&fib1) && Examine(lock2,&fib2) )
			{
				if ( fib1.fib_Size != fib2.fib_Size )
				{
					sprintf(reportStr,"*** RESENDING %s ***",source);
					Report(reportStr);
				}
				else
					retval=FALSE;
			}
			UnLock(lock2);
		}
		UnLock(lock1);
	}

	if ( retval )
	{
		strcpy(reportStr,"Sizes were not equal.");
		Report(reportStr);
	}

	return( retval );
}
	
/******** E O F ********/
