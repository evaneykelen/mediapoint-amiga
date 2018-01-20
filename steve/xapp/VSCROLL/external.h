// from file : mlp.a
GLOBAL int SetupPlayer(void);
GLOBAL void KillPlayer(void);
GLOBAL APTR ReadModule(char *name);
GLOBAL void KillModule(APTR);
GLOBAL int PlayTune(APTR);
GLOBAL void StopTune(APTR);

// from file : worker.c
GLOBAL void main( int , char **);

// from file easy_up.o

int get_varsize( void );
void do_easy_up( UBYTE *datablock );
int load_easy_up( char *filename,  UBYTE *datablock );
void release_easy_up( UBYTE *datablock  );
void unload_easy_up( UBYTE *datablock  );
int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC );

