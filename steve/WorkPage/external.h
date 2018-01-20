// from file : geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

// from file : effects.o
GLOBAL get_varsize( void );
GLOBAL void wipe_in( UBYTE *datablock );
GLOBAL void unload_file( UBYTE *datablock );
GLOBAL int load_wipe( char *filename,  UBYTE *datablock , WORD *argpointer,long print_err );
GLOBAL int load_mem_wipe( char *formptr,  UBYTE *datablock , WORD *argpointer );
GLOBAL void release_slide2( UBYTE *datablock  );
GLOBAL int pass_mlsystem( MLSYSTEM * , UBYTE *datablock, struct MsgPort *Pi_PtoC, WORD *effnums, int SigNum_NextObject , UWORD *patterns);
GLOBAL UBYTE *get_name_pointer( void );
GLOBAL int create_screen( long *data, UBYTE *datablock, WORD *argpointer );
GLOBAL int create_window( long *data, UBYTE *datablock );
GLOBAL int create_clip( long *data, UBYTE *datablock );
GLOBAL int create_animclip( long *data, UBYTE *datablock );
GLOBAL int create_text( long *data, UBYTE *datablock, struct EditWindow *ew);
GLOBAL void set_colors( long *data, UBYTE *datablock );
GLOBAL void finish_doc( UBYTE *datablock );
GLOBAL void update_screen( UBYTE *datablock );


// from file : workpage.c
GLOBAL void main( int , char **);

