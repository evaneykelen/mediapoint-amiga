/**** mpmmu.c ****/

void *MLMMU_AllocMem(ULONG, ULONG, char *);
void MLMMU_FreeMem(void *);
void *MLMMU_FindMemBlk(char *);
MEMTAG *MLMMU_FindMem(char *);
ULONG MLMMU_PleaseFree(ULONG);
ULONG MLMMU_AvailMem(ULONG);
struct List *MLMMU_GetMemList(void);
LONG MLMMU_GetMemStat(void *);
LONG MLMMU_SetMemStat(ULONG, void *);
void *MLMMU_OwnMemBlk(void *);
void MLMMU_DisOwnMemBlk(void *);
ULONG MLMMU_FlushMem(ULONG);
ULONG MLMMU_ReallyFlushMem(ULONG);

/**** errors.c ****/

BOOL MLMMU_OpenMsgQueue(void);
void MLMMU_CloseMsgQueue(void);
void MLMMU_AddMsgToQueue(STRPTR, ULONG);
struct List *MLMMU_GetQueueList(void);

/******** E O F ********/
