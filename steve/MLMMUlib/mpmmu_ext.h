/**********************************************************
*File : mpmmu_Ext.h
*/

extern struct SignalSemaphore MMUSemaphore;

extern struct List *MTList;

extern BOOL MemReside;
extern ULONG MemRestrict;
extern ULONG MemUsed;

GLOBAL void *__saveds __asm LIBMLMMU_AllocMem(register __d0 ULONG ByteSize,
																							register __d1 ULONG Attributes,
																							register __a1 char *Name );
GLOBAL void __saveds __asm LIBMLMMU_FreeMem(register __a1 void *MemoryBlock);
GLOBAL void *__saveds __asm LIBMLMMU_FindMemBlk(register __a1 char *Name);
GLOBAL MEMTAG *__saveds __asm LIBMLMMU_FindMem(register __a1 char *Name);
GLOBAL ULONG __saveds __asm LIBMLMMU_PleaseFree(register __d0 ULONG ReqSize);
GLOBAL ULONG __saveds __asm LIBMLMMU_AvailMem(register __d1 ULONG Attr);
GLOBAL struct List *__saveds __asm LIBMLMMU_GetMemList(void);
GLOBAL LONG __saveds __asm LIBMLMMU_GetMemStat(register __a1 void *MemoryBlock);
GLOBAL LONG __saveds __asm LIBMLMMU_SetMemStat(	register __d0 ULONG NewStatus,
																								register __a1 void *MemoryBlock);
GLOBAL void *__saveds __asm LIBMLMMU_OwnMemBlk( register __a1 void *MemoryBlock );
GLOBAL void __saveds __asm LIBMLMMU_DisOwnMemBlk( register __a1 void *MemoryBlock);
GLOBAL ULONG __saveds __asm LIBMLMMU_FlushMem(register __d1 ULONG Attributes);
