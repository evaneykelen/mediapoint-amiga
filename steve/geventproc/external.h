// from file : sg:geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

// from file : ph:functions.c
GLOBAL BOOL pc_SendMessage( char *, struct Message *);
GLOBAL void FreeTaskSignal( struct Task *, int );
GLOBAL int AllocTaskSignal( struct Task *);
GLOBAL BOOL CreateTaskPort( struct Task *, struct MsgPort *, char *);
GLOBAL void DeleteTaskPort( struct Msgport *);

// from file : globeproc.c
MOUSEJUMPREC *FindScreenButton(EVENTDATA *GED, BOOL ForFollowMouse, BOOL *immediate);
GLOBAL MOUSEJUMPREC *FindKeyButton(EVENTDATA *GED, int rawkeyCode);
GLOBAL int ASCIIToRawKey( EVENTDATA *, int );
GLOBAL BOOL MakeJumpList( EVENTDATA *, struct ScriptEventRecord **, EVENTJUMPREC *, MOUSEJUMPREC *);
GLOBAL void main( int , char **);
GLOBAL struct InputEvent *Int_GlobEProcHandler( struct InputEvent *, EVENTDATA * );
BOOL TestJoystick(EVENTDATA *GEventData, struct InputEvent *ie, UWORD *keys);
BOOL TestLightpen(EVENTDATA *GEventData, struct InputEvent *ie, UWORD *keys);
