/*********************************************
*File : mlmmu.h
*/

/***********************************************
*Desc : for each memoryblock allocated via
*		the mmu.library a memorytag is generated.
*		Other applications may try to allocate
*		that same memoryblock by passing a name
*		to the allocator
*/


/*************************************************
*Desc : Defs for mt_Status
*/



#define NT_MEMTAG	247				// Node type for MEMTAG 

#define MEMF_FREE_ALWAYS	(1L<<20)// Free also when count != 0

#define MEMF_USED	(1L<<17)		// Internal use only, if set at FlushMem
									// areas which are used will also be freed
#define MEMF_ALL	(1L<<16)		// Internal use only, if set at FlushMem
									// all areas will be freed except for when they 
									// are in use.

#define MEMF_STAY 	(1L<<15)		// mt_Attr, do not free this memory


#define MT_NOSTAT	-1				// returned by MLMMU_GetmemStat if memblk doesn't
									// exist.
	
#define MTB_INIT	0				// If set, this memory block has been initialised 
#define MTB_OWNED	1				// If set, this blk is currently owned by a process
									// Other apps may read from the area but may not
									// write into it
#define MTB_SETCLR	31				// if set, bits 0-30 will be set in mt_Status

#define MTF_INIT	(1<<MTB_INIT)	
#define MTF_OWNED	(1<<MTB_OWNED)
#define MTF_SETCLR	(1<<MTB_SETCLR) // else	bits 0-30 will be cleared in mt_Status

typedef struct
{
	struct Node mt_Node;		// linked list of memory blocks
								// nt_Node.ln_Name holds a pointer to a unique blkname
	UWORD		mt_Pad;			// Alignment word
	ULONG		mt_Attr;		// Memtypes
	int			mt_AppCnt;		// nr of applications using this memory blk
								// Set to 1 after first alloc
	ULONG		mt_Size;		// Total size of this memtag + ln_Name + BlkSize
	ULONG		mt_Status;		// Status of Memtag


	void		*mt_BlkBase;	// base address of allocated memory blk
								// This address is returned to the application
	ULONG		mt_BlkSize;		// Size of memory blk in bytes
} MEMTAG;

struct MsgNode
{
	struct Node node;
	UBYTE *txt;
	ULONG txtSize;
	ULONG flags;
};
