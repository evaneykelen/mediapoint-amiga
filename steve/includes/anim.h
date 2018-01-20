void do_anim( UBYTE *datablock );
int load_anim( char *filename,  UBYTE *datablock , WORD *argpointer );
void release_anim( UBYTE *datablock  );
void unload_anim( UBYTE *datablock  );
int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC );
