// from file : ph:loadsegment.c
GLOBAL BPTR MLLoadSeg( struct List *, char *);
GLOBAL void MLUnLoadSeg( struct List *, BPTR );
GLOBAL struct List *LoadMLSegments( struct FileLock *);
GLOBAL void UnLoadMLSegments( struct List *);

// from file : geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

// from file : ph:functions.c
GLOBAL BOOL pc_SendMessage( char *, struct Message *);
GLOBAL void FreeTaskSignal( struct Task *, int );
GLOBAL int AllocTaskSignal( struct Task *);
GLOBAL BOOL CreateTaskPort( struct Task *, struct MsgPort *, char *);
GLOBAL void DeleteTaskPort( struct MsgPort *);

// from file : parguide.c
GLOBAL void FreeParGuide( struct MemVar *, int );
GLOBAL int InitParGuide( struct MemVar *);
GLOBAL BOOL FindChild( struct MemVar *, PROCESSINFO *);
GLOBAL PROCDIALOGUE *GetFreeChildDial( PROCESSINFO *);
GLOBAL PROCDIALOGUE *GetFreeSerDial( struct MemVar *);
GLOBAL BOOL FindPI( struct List *, PROCESSINFO *);
GLOBAL void TalkToPC( struct MemVar *, int );
GLOBAL void KillProcess( struct MemVar *);
GLOBAL void ChildTalk( struct MemVar *);
GLOBAL void CmdChildren( struct MemVar *, int );
GLOBAL void ParentTalk( struct MemVar *);
GLOBAL void GetReply( struct MemVar *);
GLOBAL int LoadObject( struct MemVar *, SNR *);
GLOBAL BOOL TermObject( struct MemVar *, SNR *, PROCDIALOGUE *);
GLOBAL void HoldObject( struct MemVar *, SNR *);
GLOBAL void RunObject( struct MemVar *, SNR *);
GLOBAL int PreloadObjects( struct MemVar *);
GLOBAL int PrepareChildren( struct MemVar *);
GLOBAL void main( int , char **);

// from file : ph:addworker.c
GLOBAL int AddWorker( struct Library *, MLSYSTEM *, struct List *, struct List *, SNR *, struct MsgPort *, struct MsgPort *, char *);

