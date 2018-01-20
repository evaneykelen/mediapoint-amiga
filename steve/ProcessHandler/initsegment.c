#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "minc:defs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "rexx_pragma.h"
#include "rexx_proto.h"
#include "rexx.h"
#include "external.h"

#define _PRINTF FALSE

BOOL NotAnIcon(STRPTR filename)
{
int len;
	len = strlen(filename);
	if ( len<5 || stricmp(&filename[len-5],".info") )
		return(TRUE);
	return(FALSE);
}

/***************************************************
*Func : Scan all tooltypes
*in   : FileName
*out  : TRUE -> File must be made resident
*		FALSE -> File must not be made resident
*		StackSize
*		OTT
*		Quiet
*/
BOOL ScanToolTypes( FileName, StackSize, OTT, Quiet)
char *FileName;
ULONG *StackSize;
ULONG *OTT;
BOOL *Quiet;
{
  struct DiskObject *dobj;
  char **ToolArray;
  char *ToolValue;
  BOOL B_Res;

	B_Res = FALSE;	

	*Quiet = FALSE;
	*StackSize = 4000;
	*OTT=OTT_PRELOAD;

	if( (*FileName) && (dobj = GetDiskObject(FileName)) )	 
	{
		ToolArray = (char **)dobj->do_ToolTypes;

		if( ToolValue = (char *)FindToolType(ToolArray,"TYPE"))
		{
			if(
				MatchToolValue(ToolValue,"CONTROLLER") ||
				MatchToolValue(ToolValue,"XAPP") ||
				MatchToolValue(ToolValue,"TIMER") ||
				MatchToolValue(ToolValue,"SYSTEM")
			  )
			{
				B_Res = TRUE;
				if( ToolValue = (char *)FindToolType(ToolArray,"RESIDENT"))
					*OTT = MatchToolValue(ToolValue,"YES") ? OTT_PRELOAD: OTT_AFTERLOAD;
			}

			if( ToolValue = (char *)FindToolType(ToolArray,"STOPQUIET"))
				*Quiet = (BOOL)MatchToolValue(ToolValue,"YES");

			*StackSize = dobj->do_StackSize;
			if(*StackSize < 4000)
				*StackSize = 4000;
		}
		FreeDiskObject(dobj);
	}

	return(B_Res);
}

/*******************************************************
*Func : Load all resident modules to be loaded at the 
*		startup of medialink
*in   : SegList
*		ToolsLock
*		FIB
*out  : TRUE -> error
*		FALSE -> ok
*/
int	GetPreResidentModules(SegList,ToolsLock,FIB)
struct List *SegList;
struct FileLock *ToolsLock;		// Lock on tools directory
struct FileInfoBlock *FIB;		// ptr to FileInfoBlock
{
  ULONG OTT;
  ULONG StackSize;
  BOOL Quiet;
  RESSEGMENT *NewResSeg;
  int Error;

	Error = FALSE;

	if(Examine((BPTR)ToolsLock,FIB))
	{
		while(ExNext((BPTR)ToolsLock,FIB))
		{
			//Check if the FIB is a file	
			if(FIB->fib_DirEntryType < 0 && NotAnIcon(FIB->fib_FileName) )
			{
				if(ScanToolTypes(FIB->fib_FileName,&StackSize,&OTT,&Quiet))
				{
					// Don't load what we already have
					if(FindName(SegList, strupr(FIB->fib_FileName)) == NULL)
					{
						if( (NewResSeg = (RESSEGMENT *)AllocMem(sizeof(RESSEGMENT), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
						{
							strcpy(NewResSeg->rs_Name,FIB->fib_FileName);
							NewResSeg->rs_Quiet = Quiet;

#if _PRINTF
							printf("aPP %s Quiet = %s",NewResSeg->rs_Node.ln_Name,Quiet ? "YES":"NO");
#endif

							NewResSeg->rs_StackSize = StackSize;
							if(OTT & OTT_PRELOAD)
								NewResSeg->rs_Node.ln_Type = NT_RESSEG_PRELOAD;
							if(OTT & OTT_AFTERLOAD)
								NewResSeg->rs_Node.ln_Type = NT_RESSEG_AFTERLOAD;

							NewResSeg->rs_Node.ln_Name = NewResSeg->rs_Name;
							NewResSeg->rs_Node.ln_Pri = 0;
							if(OTT & OTT_PRELOAD)
							{
#if _PRINTF
								printf("PC> Resident aPP found [%s], OTT = %x, Stack = %d\n",NewResSeg->rs_Node.ln_Name,OTT,StackSize);
#endif
								if( (MLMMU_AvailMem(MEMF_PUBLIC)<50000) )	
								{
									FreeMem(NewResSeg, sizeof(RESSEGMENT));
									return(TRUE);
								}

								if( (NewResSeg->rs_Segment = LoadSeg(FIB->fib_FileName)) == NULL)
								{
									FreeMem(NewResSeg, sizeof(RESSEGMENT));
									break;
								}
							}
							AddHead((struct List *)SegList, (struct Node *)NewResSeg);
						}
						else
						{
							Error = TRUE;
							break;
						}	
					}
				}
			}
		}
	}

	return(Error);
}

/*******************************************************
*Func : Load all resident modules to be loaded at the 
*		startup of medialink
*in   : SegList
*		ToolsLock
*		FIB
*out  : TRUE -> error
*		FALSE -> ok
*/
int	GetAfterResidentModules(SegList,ToolsLock,FIB)
struct List *SegList;
struct FileLock *ToolsLock;		// Lock on tools directory
struct FileInfoBlock *FIB;		// ptr to FileInfoBlock
{
  RESSEGMENT *CurSeg, *NextSeg;	
  int Error;

	Error = FALSE;

	CurSeg = (RESSEGMENT *)SegList->lh_Head;
	while( (NextSeg = (RESSEGMENT *)CurSeg->rs_Node.ln_Succ) != NULL)
	{
		if( 
			(CurSeg->rs_Node.ln_Type == NT_RESSEG_AFTERLOAD) && 
			(CurSeg->rs_Segment == NULL) && IsXappUsed(CurSeg->rs_Node.ln_Name)
		  )
		{
			if(MLMMU_AvailMem(MEMF_PUBLIC) < 50000)	
				return(TRUE);

			CurSeg->rs_Segment = LoadSeg(CurSeg->rs_Node.ln_Name);
#if _PRINTF
			printf("PC> Afterload aPP found [%s],Stack = %d,Loaded at %lx\n",CurSeg->rs_Node.ln_Name,CurSeg->rs_StackSize,CurSeg->rs_Segment);
#endif
		}
		CurSeg = NextSeg;
	}

	return(Error);
}

/************************************************************
*Func : Load all resident MediaLink segments in the SegList 
*		and store their Segment Ptrs.
*		All resident modules can be found in Tools
*		They should have there P-pure bit set
*
*in   : MainDirLock -> Lock to main directory
*		ObjectType -> if , load segments which have their
*					 LOADATSTART set to YES
*					 else load segments which have their
					 LOADATSTART set to NO.
*out  : NULL-> Error
*		else ptr to ResSegment list
*/
struct List *LoadMLSegments(SegList, MainDirLock, ObjectToolType)
struct List *SegList;
struct FileLock *MainDirLock;
ULONG  ObjectToolType;
{
 // RESSEGMENT *NewResSeg;
  struct FileLock *ToolsLock;		// Lock on tools directory
  struct FileLock *OldDirLock;		
  struct FileInfoBlock *FIB;		// ptr to FileInfoBlock
  int Error;
 
	Error = NO_ERROR;
	OldDirLock = (struct FileLock *)CurrentDir((BPTR)MainDirLock);

	if( (ToolsLock = (struct FileLock *)Lock("system",ACCESS_READ)) == NULL)	
	{
		CurrentDir((BPTR)OldDirLock);
		return(NULL);
	}
	CurrentDir((BPTR)ToolsLock);

	if( (FIB = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock),MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
	{	
		CurrentDir((BPTR)OldDirLock);
		UnLock((BPTR)ToolsLock);
		return(NULL);
	}

	if(SegList == NULL)
	{
		if( (SegList = (struct List *)AllocMem(sizeof(struct List),MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
		{
			FreeMem(FIB,sizeof(struct FileInfoBlock));
			CurrentDir((BPTR)OldDirLock);
			UnLock((BPTR)ToolsLock);
			return(NULL);
		}
		NewList(SegList);
	}

	if(ObjectToolType & OTT_PRELOAD)
		Error = GetPreResidentModules(SegList,ToolsLock,FIB);

	if(!Error)
		if(ObjectToolType & OTT_AFTERLOAD)
			Error = GetAfterResidentModules(SegList,ToolsLock,FIB);

	if(!Error)
	{
		UnLock((BPTR)ToolsLock);
		CurrentDir((BPTR)OldDirLock);
		if( (ToolsLock = (struct FileLock *)Lock("xapps",ACCESS_READ)) == NULL)	
		{
			CurrentDir((BPTR)OldDirLock);
			return(NULL);
		}
		CurrentDir((BPTR)ToolsLock);

		if(ObjectToolType & OTT_PRELOAD)
			Error = GetPreResidentModules(SegList,ToolsLock,FIB);

		if(!Error)
			if(ObjectToolType & OTT_AFTERLOAD)
				Error = GetAfterResidentModules(SegList,ToolsLock,FIB);
	}

	if(Error)
		UnLoadMLSegments(SegList,ObjectToolType);

	FreeMem(FIB,sizeof(struct FileInfoBlock));
	UnLock((BPTR)ToolsLock);
	CurrentDir((BPTR)OldDirLock);
	return(SegList);
}

/*************************************************
*Func : Unload all resident segments and free up
*		the Segment List
*in   : SegList -> Ptr to Segment List
*		ObjectType -> which type of segments should be unloaded
*out  : -
*/
void UnLoadMLSegments( SegList, ObjectToolType)
struct List *SegList;
ULONG ObjectToolType;
{
  RESSEGMENT *CurSeg, *NextSeg;	
  BOOL B_Unload, B_Remove;

	if(SegList == NULL)
		return;

	CurSeg = (RESSEGMENT *)SegList->lh_Head;
	while( (NextSeg = (RESSEGMENT *)CurSeg->rs_Node.ln_Succ) != NULL)
	{
		if(CurSeg->rs_Count == 0)
		{
			B_Unload = FALSE;
			B_Remove = FALSE;

			if( (CurSeg->rs_Node.ln_Type == NT_RESSEG_AFTERLOAD) && (ObjectToolType & OTT_AFTERLOAD))
				B_Unload = TRUE;

			if(ObjectToolType & OTT_PRELOAD)
				B_Unload = B_Remove = TRUE;

#if _PRINTF
			PrintSer("Resident segment [%s], Remove = %s, Unload = %s\n\r",CurSeg->rs_Node.ln_Name, B_Remove ? "YES":"NO",B_Unload ? "YES":"NO");
#endif
			if(B_Remove)	
				Remove((struct Node *)CurSeg);

			if(B_Unload && CurSeg->rs_Segment)
			{
				UnLoadSeg(CurSeg->rs_Segment);
				CurSeg->rs_Segment = NULL;
			}

			if(B_Remove)	
				FreeMem(CurSeg, sizeof(RESSEGMENT));
		}
#if _PRINTF
		else
			printf("PC> Couldn't unload resident segment [%s], still open %d\n\r",CurSeg->rs_Node.ln_Name,CurSeg->rs_Count); 
#endif
		CurSeg = NextSeg;
	}

	if(ObjectToolType & OTT_PRELOAD)	
		FreeMem(SegList, sizeof(struct List));
}
