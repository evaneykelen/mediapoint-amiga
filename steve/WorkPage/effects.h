PROCDIALOGUE *wipe_in( UBYTE *datablock );
void unload_file( UBYTE *datablock );
int load_wipe( char *filename,  UBYTE *datablock , WORD *argpointer );
void release_slide2( UBYTE *datablock  );
int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC, PROCESSINFO *ThisPi  );
UBYTE *get_name_pointer( void );
