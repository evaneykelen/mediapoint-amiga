#ifndef MINC_EXTERNAL_H
#define  MINC_EXTERNAL_H

// from file : sg:geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int, char **);

// from file : sync:SynchroIGMT.c
GLOBAL SYNCGUIDE *FindSyncGuide( struct List *, PROCESSINFO *);
GLOBAL void main( int , char **);

// from file : pg:parguide.c
GLOBAL void FreeParGuide( struct MemVar *, int );
GLOBAL int InitParGuide( struct MemVar *);

// from file : ph:playscript.c
GLOBAL void MakeBackList( struct ScriptInfoRecord *, struct List *, SNR *, int *, int *);

// from file : ph:procinit.c
GLOBAL int ProcessInitializer(struct ScriptInfoRecord *, BOOL);
GLOBAL void ProcessDeInitializer(void);
GLOBAL int TinyProcessInitializer(void);
GLOBAL void TinyProcessDeInitializer(void);

#endif

// from file : ph:addguide.c
GLOBAL int AddGuide( struct Library *, MLSYSTEM	*, struct List *, struct List *, struct ScriptInfoRecord *, int , SNR *, struct MsgPort *, struct MsgPort *, char *, int );

// from file : ph:addworker.c
GLOBAL int AddWorker( struct Library *, MLSYSTEM *, struct List *, struct List *, SNR *, struct MsgPort *, struct MsgPort *, char *);

// from file : ph:functions.c
GLOBAL BOOL pc_SendMessage( char *, struct Message *);
GLOBAL void FreeTaskSignal( struct Task *, int );
GLOBAL int AllocTaskSignal( struct Task *);
GLOBAL BOOL CreateTaskPort( struct Task *, struct MsgPort *, char *);
GLOBAL void DeleteTaskPort( struct MsgPort *);

// from file : ph:loadsegment.c
GLOBAL BPTR MLLoadSeg( struct List *, char *, ULONG *);
GLOBAL void MLUnLoadSeg( struct List *, BPTR );
GLOBAL BOOL MLGetQuietSeg( struct List *, BPTR );

