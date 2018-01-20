// from file: effects.o
GLOBAL int get_varsize( void );
GLOBAL void do_anim( UBYTE *datablock );
GLOBAL int load_anim( char *filename,  UBYTE *datablock , WORD *argpointer );
GLOBAL int unload_anim(UBYTE *datablock);
GLOBAL void release_anim( UBYTE *datablock);
GLOBAL int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC);

// from file : workanim.c
GLOBAL BOOL SendDialogue( PROCDIALOGUE *, PROCESSINFO *, int );
GLOBAL void main( int , char **);
