// from file : edit.c
void XappSetup(PROCESSINFO *PI);
APTR XappDoIt(PROCESSINFO *PI);
void StopPlaying(APTR mod);
void MakeFullPath(STRPTR path, STRPTR name, STRPTR answer);

// from file : mlp.a
GLOBAL int SetupPlayer( long vscan );
GLOBAL void KillPlayer(void);
GLOBAL APTR ReadModule(char *name);
GLOBAL void KillModule(APTR);
GLOBAL int PlayTune(APTR);
GLOBAL void StopTune(APTR);

// from file : geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

// from file : worker.c
GLOBAL void main( int , char **);

