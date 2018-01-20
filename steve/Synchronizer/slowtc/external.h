// from file : itc.c
GLOBAL int __asm ITCIntHandler(  register __a1 SYNCDATA *SD);
GLOBAL int	InitITCInt( struct ScriptInfoRecord *, ULONG );
GLOBAL void FreeITCInt( struct ScriptInfoRecord *);
GLOBAL void PulseFrameCntr( void);

// from file : etc.c
GLOBAL void __asm ETCIntHandler(  register __a1 SYNCDATA *SD);
GLOBAL int	InitETCInt( struct ScriptInfoRecord *, ULONG );
GLOBAL void FreeETCInt( struct ScriptInfoRecord *);

// from file : tempoeditor.c
GLOBAL void ReturnPunch( struct List *, TIMEREQUEST *, struct MsgPort *, struct MsgPort *);
GLOBAL void TempoEditor( void);

