// from file : serguide.c
GLOBAL void FreeSerGuide( struct MemVar *, int );
GLOBAL int InitSerGuide( struct MemVar *);
GLOBAL PROCDIALOGUE *GetFreeChildDial( PROCESSINFO *);
GLOBAL PROCDIALOGUE *GetFreeSerDial( struct MemVar *);
GLOBAL BOOL FindChild( struct MemVar *, PROCESSINFO *);
GLOBAL void KillProcess( struct MemVar *);
GLOBAL void TalkToPC( struct MemVar *, int );
GLOBAL void CleanUpMem( struct MemVar *);
GLOBAL void ChildTalk( struct MemVar *);
GLOBAL void CmdChildren( struct MemVar *, int );
GLOBAL void ResetPreloadChildren( struct MemVar *);
GLOBAL void PCTalk( struct MemVar *);
GLOBAL void GetReply( struct MemVar *);
GLOBAL int LoadObject( struct MemVar *, SNR *);
GLOBAL BOOL TermObject( struct MemVar *, SNR *, PROCDIALOGUE *);
GLOBAL void HoldObject( struct MemVar *, SNR *);
GLOBAL void PrepObject( struct MemVar *, SNR *);
GLOBAL void RunObject( struct MemVar *, SNR *);
GLOBAL BOOL TermParObject( struct MemVar *, SNR *, PROCDIALOGUE *);
GLOBAL void HoldParObject( struct MemVar *, SNR *);
GLOBAL void RunParObject( struct MemVar *, SNR *);
GLOBAL int PreloadObjects( struct MemVar *);
GLOBAL void main( int , char **);

