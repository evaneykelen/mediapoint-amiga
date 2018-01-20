// from file : itc.c
GLOBAL void TransmitByte( TBEDATA *TD);
GLOBAL void __asm TBEIntHandler( register __a1 TBEDATA *TD);
GLOBAL int __asm ITCIntHandler(  register __a1 TBEDATA *SD);
GLOBAL int	InitITCInt( struct ScriptInfoRecord *, ULONG );
GLOBAL void FreeITCInt( struct ScriptInfoRecord *);

// from file : etc.c
GLOBAL void __asm ETCIntHandler(  register __a1 SYNCDATA *SD);
GLOBAL int	InitETCInt( struct ScriptInfoRecord *, ULONG );
GLOBAL void FreeETCInt( struct ScriptInfoRecord *);

