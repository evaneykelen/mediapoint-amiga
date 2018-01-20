//	File		:	loadsamp.h
//	Uses		:
//	Date		:	3 - july - 1993
//	Author	:	ing. C. Lieshout
//	Desc.		:	Prototypes for loadsamp.c
//

int get_chipmem( SoundInfo *sinfo );
void free_chipmem( SoundInfo *sinfo );
int loadsoundfile( SoundInfo *sinfo, char *filename, BOOL useQueue, ULONG signal );
void load_sound_frame( SoundInfo *sinfo );
void freesound( SoundInfo *sinfo );
void set_volume( SoundInfo *sinfo, int vol, int balance );
void reset_sound( SoundInfo *sinfo );
void fade_it_in( SoundInfo *sinfo, int seconds );
void fade_it_out( SoundInfo *sinfo, int seconds );
