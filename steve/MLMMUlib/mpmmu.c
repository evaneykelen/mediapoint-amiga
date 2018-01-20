#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/semaphores.h>
#include <proto/exec.h>
#include <string.h>

#include "mlmmu_pragma.h"

#include "mlmmu.h"
#include "mlmmu_proto.h"
#include "mpmmu_ext.h"
#include "mpmmu_app.h"

#define MLMMULibBase (getreg(REG_A6))
#define MINIMUM 250000

ULONG __saveds __asm LIBMLMMU_FlushMem(register __d1 ULONG Attributes);
ULONG __saveds __asm LIBMLMMU_AvailMem(register __d1 ULONG Attr);

/**********************************************************
*Func : Allocate a certain amount of memory from the
*				system memory pool.
*				First the MemTag list is searched for a memory
*				blk with the same name as passed.
*				If the memblk already exists its base is returned.
*				Else the memblk will be allocated and inserted into
*				the MemTag list.
*
*				If a memory blk could not be allocated this function
*				will scan the list for unused memblks. These blks
*				will be deleted until enough memory for the request
*				has become available
*
*in   : ByteSize 		-> Size of blk in bytes
*				Atributes 	-> MemType
*												note : A new memtype is used MEMF_STAY
												if a blk has to be freed and it has its
												mt_Attr field set to MEMF_STAY it will
												not be freed. The block will remain in
												memory until this function needs memory
												or when a expunge is performed.
*				Name 				-> Name of memoryblk to be allocated or NULL
*										   if this blk is private	
*out  : Ptr to blk area
*/

void *__saveds __asm LIBMLMMU_AllocMem(	register __d0 ULONG ByteSize,
																				register __d1 ULONG Attributes,
																				register __a1 char *Name )
{
MEMTAG *MT, *NextMT, *FitMT;
ULONG TmpAttr, TotByteSize, ClrSize, *ClrArea, memtype, minimum;

	if( MTList == NULL )
		return(NULL);

//{if(ByteSize<512){char str[256];sprintf(str,"A: %d %x [%s]\n",ByteSize,Attributes,Name);KPrintF(str);}}

	ObtainSemaphore(&MMUSemaphore);

	// What does caller want?

	if ( Attributes & MEMF_CHIP )
	{
		if ( AvailMem(MEMF_CHIP|MEMF_LARGEST) < ByteSize )
		{
			// Free CHIP mem is smaller than requested size.
			// Free some unused CHIP mem.

			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				memtype=TypeOfMem(MT);
				if ( (memtype & MEMF_CHIP) && (MT->mt_AppCnt == 0) )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_CHIP|MEMF_LARGEST) > ByteSize )
					break; 
			}
		}
	}	

	if ( Attributes & MEMF_FAST )
	{
		if ( AvailMem(MEMF_FAST|MEMF_LARGEST) < ByteSize )
		{
			// Free FAST mem is smaller than requested size.
			// Free some unused FAST mem.

			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				memtype=TypeOfMem(MT);
				if ( (memtype & MEMF_FAST) && (MT->mt_AppCnt == 0) )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_FAST|MEMF_LARGEST) > ByteSize )
					break; 
			}
		}
	}	

	if ( !(Attributes & MEMF_FAST) && !(Attributes & MEMF_CHIP) )
	{
		if ( AvailMem(MEMF_PUBLIC|MEMF_LARGEST) < ByteSize )
		{
			// Free ANY/PUBLIC mem is smaller than requested size.
			// Free some unused ANY/PUBLIC mem.

			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				if ( MT->mt_AppCnt == 0 )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_PUBLIC|MEMF_LARGEST) > ByteSize )
					break; 
			}
		}
	}	

	// Now we may assume that enough memory of requested type is available

	if( Name )	// Allocate memory with a name
	{
		// Step 1, try to find the requested memory area and
		// return its base if exists.

		if( (MT = (MEMTAG *)FindName(MTList, Name)) )
		{
			// memory area exists

			if(ByteSize <= MT->mt_BlkSize)
			{
				// step 1a, mem exists and new blk fits into the existing blk

				//if(TmpAttr & MEMF_CLEAR)
				if(Attributes & MEMF_CLEAR)
				{
					ClrArea = (ULONG *)MT->mt_BlkBase;
					for(ClrSize = (ByteSize>>2)-1; ClrSize != 0; ClrSize--)				
						ClrArea[ClrSize] = 0L;
				}

				if(MT->mt_AppCnt < 100000)
					MT->mt_AppCnt++;	// a new application will be using this mem block as well

				ReleaseSemaphore(&MMUSemaphore);

#if 0
{
char str1[200];
	sprintf(str1, "MPMMU 1 - C=%08ld  F=%08ld  LC=%08ld  LF=%08ld\n",
					AvailMem(MEMF_CHIP),
					AvailMem(MEMF_FAST),
					AvailMem(MEMF_CHIP|MEMF_LARGEST),
					AvailMem(MEMF_FAST|MEMF_LARGEST) );
KPrintF(str1);
}
#endif

				return(MT->mt_BlkBase);
			}
			else
			{
				// step 1b, requested size is more than the available amount
				// free old stuff, allocate new

				LIBMLMMU_FreeMem(MT);
			}
		}

		//                                  v => '/0' char includes
		TotByteSize = ByteSize + (sizeof(MEMTAG) + ((strlen(Name) + 8) & 0xfffffff8));
	}
	else
		TotByteSize = ByteSize + sizeof(MEMTAG);

	// Step 2, memory area doesn't exist, try to allocate it and link it onto
	// the MemTag list.

	TmpAttr = Attributes;
	TmpAttr &= ~((ULONG)MEMF_STAY|(ULONG)MEMF_FREE_ALWAYS);

	if ( MT = (MEMTAG *)AllocMem(TotByteSize,TmpAttr) )
	{
		// memory allocated
		MT->mt_Attr = Attributes;
		MT->mt_AppCnt = 1;
		MT->mt_Size = TotByteSize;
		if(Name)
		{
			//KPrintF("Alloc [%s],%ld\n",Name,TotByteSize );
			MT->mt_Node.ln_Name = (char *)((ULONG)MT + sizeof(MEMTAG));
			strcpy(MT->mt_Node.ln_Name,Name);
		}
		else
		{
			MT->mt_Node.ln_Name = NULL;
			//KPrintF("Alloc [NO],%ld\n",TotByteSize );
		}

		MT->mt_Node.ln_Type = NT_MEMTAG;
		MT->mt_Node.ln_Pri = 0;

		MT->mt_BlkBase = (VOID *)((ULONG)MT + TotByteSize - ByteSize);
		MT->mt_BlkSize = ByteSize;
		MT->mt_Status = 0L;			// new block has not been initialised yet

		AddHead((struct List *)MTList, (struct Node *)MT);

		FitMT = MT;

		/************************************/

		minimum = MINIMUM;

		if ( AvailMem(MEMF_PUBLIC|MEMF_LARGEST) < minimum )
		{
			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				if ( MT->mt_AppCnt == 0 )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_PUBLIC|MEMF_LARGEST) > minimum )
					break; 
			}
		}

		if ( AvailMem(MEMF_CHIP|MEMF_LARGEST) < minimum )
		{
			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				memtype=TypeOfMem(MT);
				if ( (memtype & MEMF_CHIP) && (MT->mt_AppCnt == 0) )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_CHIP|MEMF_LARGEST) > minimum )
					break; 
			}
		}

		// START NEW

		if ( AvailMem(MEMF_FAST|MEMF_LARGEST) < minimum )
		{
			MT = (MEMTAG *)MTList->lh_Head; 
			while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
			{		
				memtype=TypeOfMem(MT);
				if ( (memtype & MEMF_FAST) && (MT->mt_AppCnt == 0) )
				{
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
				MT = NextMT;
				if ( AvailMem(MEMF_FAST|MEMF_LARGEST) > minimum )
					break; 
			}
		}

		// END NEW

		/************************************/

		ReleaseSemaphore(&MMUSemaphore);

#if 0
{
char str1[200];
	sprintf(str1, "MPMMU 2 - C=%08ld  F=%08ld  LC=%08ld  LF=%08ld\n",
					AvailMem(MEMF_CHIP),
					AvailMem(MEMF_FAST),
					AvailMem(MEMF_CHIP|MEMF_LARGEST),
					AvailMem(MEMF_FAST|MEMF_LARGEST) );
KPrintF(str1);
}
#endif

		return(FitMT->mt_BlkBase);
	}

	// Step 4, Allocation failed again and there is nothing we can do.
	// Simply return NULL.

	ReleaseSemaphore(&MMUSemaphore);

	return(NULL);
}

/****************************************************
*Func : Free a memory block and remove it from the
*				memory list
*
*				If the mt_Attr field was set to MEMF_STAY
*				and mt_AppCnt becomes 0 then this block
*				will not be freed
*in   : MemoryBlock -> ptr to block
*out  : -
*/

void __saveds __asm LIBMLMMU_FreeMem(register __a1 void *MemoryBlock)
{
MEMTAG *MT;

	if(MTList == NULL)
		return;

	if(MemoryBlock == NULL)
		return;

	ObtainSemaphore(&MMUSemaphore);

	for(	MT = (MEMTAG *)MTList->lh_Head; 
				(MEMTAG *)MT->mt_Node.ln_Succ;	
				MT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{
//KPrintF("1 ");
		if(MT->mt_BlkBase == MemoryBlock)
		{
			if(MT->mt_AppCnt > 0)
				MT->mt_AppCnt--;

			if(MT->mt_AppCnt == 0)
			{	
//KPrintF("3 ");
 				if( !(MT->mt_Attr & MEMF_STAY) )
				{
//KPrintF("4 ");
/*
if ( MT->mt_Size>256 )
{
if (MT->mt_Node.ln_Name)
	KPrintF("freed [%s],%ld\n",MT->mt_Node.ln_Name,MT->mt_Size);
else
	KPrintF("freed [-],%ld\n",MT->mt_Size);
}
*/
					Remove((struct Node *)MT);
					FreeMem(MT,MT->mt_Size);
				}
			}

			break;
		}
	}

//KPrintF("\n");

	ReleaseSemaphore(&MMUSemaphore);
}

/*******************************************************
*Func : Find a memory blk 
*in	  : Name -> name of blk
*out  : NULL
*				Ptr to memblock
*/

void *__saveds __asm LIBMLMMU_FindMemBlk(register __a1 char *Name)
{
MEMTAG *MT;

	if ( MTList == NULL || Name==NULL )
		return(NULL);

	ObtainSemaphore(&MMUSemaphore);
	MT = (MEMTAG *)FindName(MTList, Name);
	ReleaseSemaphore(&MMUSemaphore);

	if(MT)
		return(MT->mt_BlkBase);
	else
		return(NULL);
}

/******************************************************
*Func : Search for a memblock with a given name
*in   : Name -> name of memblock
*out  : NULL -> blk not found
*				else ptr to memtag of memblock
*/

MEMTAG *__saveds __asm LIBMLMMU_FindMem(register __a1 char *Name)
{
MEMTAG *MT;

	if( MTList == NULL || Name == NULL )
		return(NULL);

	ObtainSemaphore(&MMUSemaphore);
	MT = (MEMTAG *)FindName(MTList, Name);
	ReleaseSemaphore(&MMUSemaphore);

	return(MT);
}

/**********************************************************
*Func : Try to free the amount of memory requested
*in   : MemSize -> least amount to be freed
*out  : Total amount freed
*/

ULONG __saveds __asm LIBMLMMU_PleaseFree(register __d0 ULONG ReqSize)
{
ULONG FreedMem;
MEMTAG *MT, *NextMT;

	FreedMem = 0;

	ObtainSemaphore(&MMUSemaphore);

	MT = (MEMTAG *)MTList->lh_Head; 
	while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{		
		if ( MT->mt_AppCnt == 0 )
		{
			Remove((struct Node *)MT);
			FreedMem += MT->mt_Size;
			FreeMem(MT,MT->mt_Size);
		}
		MT = NextMT;
		if ( AvailMem(MEMF_PUBLIC) > ReqSize )
			break; 
	}

	ReleaseSemaphore(&MMUSemaphore);

	return(FreedMem);
}

/**********************************************************
*Func : 
*in   :
*out  : bytes free
*/

ULONG __saveds __asm LIBMLMMU_AvailMem(register __d1 ULONG Attr)
{
ULONG SystemMem;

	SystemMem = AvailMem(Attr);

	if ( SystemMem < MINIMUM )
		return(0);
	else
		return(SystemMem);
}

/*************************************************
*Func : Simply return the Base of the Memtag list
*in   : -
*out  : Pointer to MEMTAG list
*/

struct List *__saveds __asm LIBMLMMU_GetMemList(void)
{
	return(MTList);
}

/*****************************************************
*Func : Return the status of a memory tag
*				Used to find out if a MemBlk has already been
*				initialized by an application
*in   : MemoryBlock -> Ptr to memmory area
*out  : Copy of mt_Status or
*		-1 if memblk was not found
*/

LONG __saveds __asm LIBMLMMU_GetMemStat(register __a1 void *MemoryBlock)
{
MEMTAG *MT;
LONG Status;

	if(MTList == NULL)
		return(MT_NOSTAT);

	ObtainSemaphore(&MMUSemaphore);

	Status = MT_NOSTAT;
	for(MT = (MEMTAG *)MTList->lh_Head; 
			(MEMTAG *)MT->mt_Node.ln_Succ;	
			MT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{
		if(MT->mt_BlkBase == MemoryBlock)
		{
			Status = MT->mt_Status;
			break;
		}
	}

	ReleaseSemaphore(&MMUSemaphore);
	return(Status);
}

/*****************************************************
*Func : Set the statusbits of a memory tag
*in   : NewStatus -> Bits to be set/cleared 
*				MemoryBlock -> Ptr to memmory area
*out  : Status of status could be set
*				-1 -> if memblk was not found
*			  or when somebody currently owns this blk
*/

LONG __saveds __asm LIBMLMMU_SetMemStat(register __d0 ULONG NewStatus,
																				register __a1 void *MemoryBlock)
{
MEMTAG *MT;
LONG Status;

	if(MTList == NULL)
		return(MT_NOSTAT);

	ObtainSemaphore(&MMUSemaphore);
	Status = MT_NOSTAT;

	for(MT = (MEMTAG *)MTList->lh_Head; 
			(MEMTAG *)MT->mt_Node.ln_Succ;	
			MT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{
		if(MT->mt_BlkBase == MemoryBlock)
		{
			if(MT->mt_Status & MTF_OWNED)
			{
				ReleaseSemaphore(&MMUSemaphore);
				return(MT_NOSTAT);
			}

 			Status = MT->mt_Status;

			if(NewStatus & MTF_SETCLR)
				Status |= NewStatus;
			else
				Status &= ~NewStatus;

			MT->mt_Status = Status;
			break;
		}
	}

	ReleaseSemaphore(&MMUSemaphore);
	return(Status);
}

/*******************************************************
*Func : Try to own a memory blk 
*in	  : MemoryBlock -> Ptr to memory area
*out  : NULL
*				Ptr to memblock -> Task is now woner
*/

void *__saveds __asm LIBMLMMU_OwnMemBlk( register __a1 void *MemoryBlock )
{
MEMTAG *MT;

	ObtainSemaphore(&MMUSemaphore);

	for(MT = (MEMTAG *)MTList->lh_Head; 
			(MEMTAG *)MT->mt_Node.ln_Succ;	
			MT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{
		if(MT->mt_BlkBase == MemoryBlock)
		{
			// If somebody owns this blk then return a NULL	
			if(MT->mt_Status & MTF_OWNED)
			{
				ReleaseSemaphore(&MMUSemaphore);
				return(NULL);
			}
			MT->mt_Status |= MTF_OWNED;
			break;
		}
	}
	ReleaseSemaphore(&MMUSemaphore);
	return(MT->mt_BlkBase);
}

/*******************************************************
*Func : Try to disown a memory blk 
*in	  : MemoryBlock -> Ptr to memory area
*out  : NULL
*				Ptr to memblock -> Task is now owner
*/

void __saveds __asm LIBMLMMU_DisOwnMemBlk( register __a1 void *MemoryBlock)
{
  MEMTAG *MT;

	ObtainSemaphore(&MMUSemaphore);

	for(MT = (MEMTAG *)MTList->lh_Head; 
			(MEMTAG *)MT->mt_Node.ln_Succ;
			MT = (MEMTAG *)MT->mt_Node.ln_Succ)
	{
		if(MT->mt_BlkBase == MemoryBlock)
		{
			// If somebody owns this blk then return a NULL	
			MT->mt_Status &= ~MTF_OWNED;
			break;
		}
	}
	ReleaseSemaphore(&MMUSemaphore);
}

/**********************************************************
*Func : Free memory areas
*in   : Attributes -> Which type of mem should be flushed
*					  The attribute bits form an AND mask
*					  to which to memblk should be fully equal
*					  MEMF_CHIP|MEMF_FAST doesn't make any
*					  sense since memory can't be of both types
*					  at the same time.
*					  MEMF_CHIP|MEMF_STAY will only free blks
*					  in chipmem with the stay bit set.
*					  MEMF_CHIP will free all chipmem blks
*					  regardless of their MEMF_STAY bit set
*out  : Nr of bytes freed
*/

ULONG __saveds __asm LIBMLMMU_FlushMem(register __d1 ULONG Attributes)
{
ULONG FreedMem;
BOOL RemoveMem;
MEMTAG *MT, *NextMT;

	ObtainSemaphore(&MMUSemaphore);

	FreedMem = 0;

	// free mem allocated via this library

	if(MTList != NULL)
	{
		MT = (MEMTAG *)MTList->lh_Head; 
		while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
		{	
			RemoveMem = FALSE;	

			if(Attributes & MEMF_USED)
				RemoveMem = TRUE;
			else
			{
				if((Attributes & MEMF_ALL) && (MT->mt_AppCnt == 0))
					RemoveMem = TRUE;
				else if((MT->mt_Attr & Attributes) == Attributes)
				{
					if(MT->mt_AppCnt == 0)
						RemoveMem = TRUE;	
				}
			}
						
			if(RemoveMem)
			{
				Remove((struct Node *)MT);
				FreedMem +=	MT->mt_Size;
				//MemUsed -= MT->mt_Size;
				FreeMem(MT,MT->mt_Size);
			}
			MT = NextMT;
		}
	}	

	ReleaseSemaphore(&MMUSemaphore);

	return(FreedMem);
}

ULONG __saveds __asm LIBMLMMU_ReallyFlushMem(register __d1 ULONG Attributes)
{
ULONG FreedMem;
BOOL RemoveMem;
MEMTAG *MT, *NextMT;

	ObtainSemaphore(&MMUSemaphore);

	FreedMem = 0;

	// free mem allocated via this library
/*
{
char str[100];
sprintf(str,"%lx %lx %lx\n",Attributes,MEMF_ALL,MEMF_USED);
KPrintF(str);
}
*/
	if(MTList != NULL)
	{
		MT = (MEMTAG *)MTList->lh_Head; 
		while( NextMT = (MEMTAG *)MT->mt_Node.ln_Succ)
		{	
			RemoveMem = FALSE;	

			if ( Attributes & MEMF_USED )	// used during library expunge
			{
	//KPrintF("MEMF_USED  ");
				RemoveMem = TRUE;
			}
			else if ( Attributes & MEMF_ALL )	// used when escape pressed and not in keep mode
			{
	//KPrintF("MEMF_ALL  ");
				if( MT->mt_Attr & MEMF_FREE_ALWAYS )
					RemoveMem = TRUE;
				else if ( MT->mt_AppCnt == 0 )
					RemoveMem = TRUE;	
			}
						
			if(RemoveMem)
			{
				Remove((struct Node *)MT);
				FreedMem +=	MT->mt_Size;
				//MemUsed -= MT->mt_Size;
/*
{
if (MT->mt_Node.ln_Name)
	KPrintF("really freed [%s],%ld\n",MT->mt_Node.ln_Name,MT->mt_Size);
else
	KPrintF("really freed [-],%ld\n",MT->mt_Size);
}
*/
				FreeMem(MT,MT->mt_Size);
			}
/*
			else
			{
{
if (MT->mt_Node.ln_Name)
	KPrintF("NOT freed [%s],%ld\n",MT->mt_Node.ln_Name,MT->mt_Size);
else
	KPrintF("NOT freed [-],%ld\n",MT->mt_Size);
}
			}
*/

			MT = NextMT;
		}
	}	

	ReleaseSemaphore(&MMUSemaphore);

	return(FreedMem);
}

/******** E O F ********/
