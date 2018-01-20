// from file : workdos.c
GLOBAL void main( int , char **);

// from file playcdxl.o

int get_varsize( void );
void do_art( UBYTE *datablock );
int load_art( char *filename,  UBYTE *datablock , WORD *argpointer );
void release_art( UBYTE *datablock  );
void unload_art( UBYTE *datablock  );
int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC );
