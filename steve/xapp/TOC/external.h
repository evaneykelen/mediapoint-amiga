// from file : workdos.c
GLOBAL void main( int , char **);

// from file playcdxl.o

int get_varsize( void );
void do_cdxl( UBYTE *datablock );
int load_cdxl( char *filename,  UBYTE *datablock , WORD *argpointer );
void release_cdxl( UBYTE *datablock  );
void unload_cdxl( UBYTE *datablock  );
int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC );

