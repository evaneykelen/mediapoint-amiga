#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:external.h"

#define _PRINTF FALSE

/************************************************************
*Func : Check if a module is already in memory
*		If not then loadSeg it 
*in   : SegList -> Ptr to public Resident Segment List
*					or NULL in case this is a module that needs to be loaded
*					from disk
*		Segname -> Name of segment to be loaded
*out  : BPTR to segment 
*		NULL -> Error
*		StackSize
*/
BPTR MLLoadSeg( SegList, SegName, StackSize)
struct List *SegList;
char *SegName;
ULONG *StackSize;
{
  RESSEGMENT *SegNode;
 // char *StrippedName;
  int i;

	strupr(SegName);
	
	i = strlen(SegName)-1;
	while(i > 0)
	{
		if(SegName[i-1] == '/' || SegName[i-1] == ':')
			break;
		i--;
	}
	
	if(SegList)
	{
		if( (SegNode = (RESSEGMENT *)FindName(SegList, &SegName[i])) != NULL)
		{
#if _PRINTF
			printf("-->loading Segment [%s] is resident\n\r",&SegName[i]);
#endif
			SegNode->rs_Count++;		// add 1 user
			*StackSize = SegNode->rs_StackSize;

			return(SegNode->rs_Segment);
		}
		else
		{
			// Load the segment
#if _PRINTF
			printf("Attempting to load disksegment %s\n",SegName);
#endif
			*StackSize = 4000;
			return(LoadSeg(SegName));
		}
	}
#if _PRINTF
	printf("Attempted load segment %s failed\n",SegName);
#endif

	return(NULL);
}

/************************************************************
*Func : Unload a segment from either the public resident
*		segment list or from the system segment list
*in   : SegList -> ptr to public resident segment list
*		Segment -> ptr to segment
*out  : -
*/
void MLUnLoadSeg( SegList, Segment)
struct List *SegList;
BPTR Segment;
{
  RESSEGMENT *CurSeg;

	for(CurSeg = (RESSEGMENT *)SegList->lh_Head;
		CurSeg->rs_Node.ln_Succ != NULL;
		CurSeg = (RESSEGMENT *)CurSeg->rs_Node.ln_Succ)
	{
		if(CurSeg->rs_Segment == Segment)
		{
			CurSeg->rs_Count--;
			return;
		}
	}
	// if not in public medialink list then remove it from the system list
#if _PRINTF
	PrintSer("Segment to be unloaded is not resident!\n");
#endif
	UnLoadSeg(Segment);
}

/************************************************************
*Func : Get the rs_Quiet value
*in   : SegList -> ptr to public resident segment list
*		Segment -> ptr to segment
*out  : value of rs_Quiet
*/
BOOL MLGetQuietSeg( SegList, Segment)
struct List *SegList;
BPTR Segment;
{
  RESSEGMENT *CurSeg;

	for(CurSeg = (RESSEGMENT *)SegList->lh_Head;
		CurSeg->rs_Node.ln_Succ != NULL;
		CurSeg = (RESSEGMENT *)CurSeg->rs_Node.ln_Succ)
	{
		if(CurSeg->rs_Segment == Segment)
			return(CurSeg->rs_Quiet);
	}
	// normally, this part will NEVER be reached
	return(FALSE);
}
