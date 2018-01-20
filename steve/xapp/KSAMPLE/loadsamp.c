// File		: loadsamp.c
// Uses		:
//	Date		: 2 february 1993 updated 3-7-1993
// Author	: ing. C. Lieshout
// Desc.		: Sample load and play routines
//

#define FREQ 3546895

#include <stdio.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include	<exec/libraries.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <mlmmu:mlmmu.h>
#include <mlmmu:mlmmu_pragma.h>
#include <mlmmu:mlmmu_proto.h>
#include <mlmmu:mpmmu_err.h>

#include "iff_fsound.h"
#include	"sample.h"

int load_sound_frame_disk( SoundInfo *sinfo );
int load_sound_frame_mem( SoundInfo *sinfo );
void load_sound_frame( SoundInfo *sinfo );
void freesound( SoundInfo *sinfo );
void change_sound( SoundInfo *sinfo );
void fade_in( SoundInfo *sinfo );

//===============================================
//	Name		: fade_it_in
//	Function	: calculate fade increments
//	Inputs	: pointer to soundinfo and no. seconds
//	Result	: sets the increments in soundinfo struct
//	Updated	: ( 1 - 7 - 1993 )
//
void fade_it_in( SoundInfo *sinfo, int seconds )
{
	WORD t;	

	seconds *= 50;
	t = sinfo->vol_right;
	t <<= 4;
	t = t /seconds;
	if( t <= 0 )
	{
//		KPrintF("Set increment at 1\n");
		t=2;
	}
	sinfo->inc_right = t;

	t = sinfo->vol_left;
	t <<= 4;
	t = t /seconds;
	if( t <= 0 )
	{
//		KPrintF("Set increment at 1\n");
		t=2;
	}

	sinfo->inc_left = t;

	sinfo->vol_temp_right = sinfo->vol_right;
	sinfo->vol_temp_left = sinfo->vol_left;
	sinfo->vol_right = 0;
	sinfo->vol_left = 0;
	fade_in( sinfo );
}

void fade_it_out( SoundInfo *sinfo, int seconds )
{
	WORD t;	

	seconds *= 50;
	t = sinfo->vol_right;
	t <<= 4;
	t = t /seconds;
	sinfo->inc_right = -t;

	t = sinfo->vol_left;
	t <<= 4;
	t = t /seconds;
	sinfo->inc_left = -t;
	sinfo->vol_temp_right = sinfo->vol_right << 4;
	sinfo->vol_temp_left = sinfo->vol_left << 4;
	sinfo->vol_right = 0;				// fade until you reach this value
	sinfo->vol_left = 0;
	fade_in( sinfo );
}

//===============================================
//	Name		: set_volume
//	Function	: set the channels with new volume and balance
//	Inputs	: pointer to soundinfo struct
//	Result	: none
//	Updated	: 14-07-1993
//
void set_volume( SoundInfo *sinfo, int vol, int balance )
{

	int vl,vr;

	vl = 64 * 100;

	vr = vl;

	if( balance != 0 )
		if( balance > 0 )	
			vl = ( vl / 5 ) * ( 5 - balance );
		else
			vr = ( vr / 5 ) * ( 5 + balance );

	sinfo->vol_right = ( vr * vol )/10000;
	sinfo->vol_left = ( vl * vol )/10000;

}

//===============================================
//	Name		: reset_sound
//	Function	: reset pointers and load first frame
//	Inputs	: pointer to soundinfo
//	Result	: sounds is ready to be started
//	Updated	: ( 15 - 7 - 1993 )
//
void reset_sound( SoundInfo *sinfo )
{

	struct Library *DOSBase;

	DOSBase = sinfo->DOSBase;
	sinfo->soundoffset = 0;
	sinfo->seq_offset = 0;
	sinfo->seq_play = 0;
	sinfo->loop = 0;
	sinfo->play_offset = sinfo->start_offset;
	if( sinfo->type & SI_DISK && sinfo->fp != NULL )
		Seek( sinfo->fp, sinfo->start_offset,OFFSET_BEGINNING );
	load_sound_frame( sinfo );
}

//===============================================
//	Name		: alloc_signals
//	Function	: allocate audio and fade interupts signals
//	Inputs	: pointer to soundinfo struct
//	Result	: a 1 if the allocate were both allocated
//				: Sets both signum and signal
//	Updated	: ( 1 - 7 - 1993 )
//
int alloc_signals( SoundInfo *sinfo, ULONG signal )
{
	int i;

	sinfo->fadenum = -1;
	sinfo->audionum = -1;

	// Create a signal mask different from capsport mask!
	// They might be the same because capsport comes from another planet...

	for(i=0; i<31; i++)
	{
		sinfo->fadenum = AllocSignal( i );	
		if ( sinfo->fadenum!=-1 )
		{
			sinfo->fadesig = 1L << sinfo->fadenum;
			if ( sinfo->fadesig==signal )
				FreeSignal( sinfo->fadenum );
			else
				break;
		}
	}

	for(i=0; i<31; i++)
	{
		sinfo->audionum = AllocSignal( i );	
		if ( sinfo->audionum!=-1 )
		{
			sinfo->audiosig = 1L << sinfo->audionum;
			if ( sinfo->audiosig==signal )
				FreeSignal( sinfo->audionum );
			else
				break;
		}
	}
/*
	sinfo->fadenum = AllocSignal( -1 );	
	if( sinfo->fadenum != -1 )
		sinfo->fadesig = 1L << sinfo->fadenum;

	sinfo->audionum = AllocSignal( -1 );	
	if( sinfo->audionum != -1 )
		sinfo->audiosig = 1L << sinfo->audionum;
*/


	sinfo->task = ( long )FindTask( 0 );
	if( sinfo->audionum == -1 || sinfo->fadenum == -1 )
		return 0;
	else
		return 1;
}

//===============================================
//	Name		:	get_chip_buffers
//	Function	:	allocate the two chip mem buffers
//	Inputs	:	pointer the sound info struct
//	Result	:	the chipdat struct is filled
//	Updated	:	3-7-1993
//
int get_chip_buffers( SoundInfo *sinfo )
{
	sinfo->chipdat.mem = ( char * )AllocMem( 2 * sinfo->chipsize, MEMF_CHIP | MEMF_CLEAR );
	if( sinfo->chipdat.mem != NULL )
	{
		sinfo->chipdat.chip1 = sinfo->chipdat.mem;
		sinfo->chipdat.chip2 = sinfo->chipdat.mem + sinfo->chipsize;
		return 1;
	}
	else
		return 0;
}

//===============================================
//	Name		:	get_chip_buffers_S
//	Function	:	allocate the two chip mem buffers for stereo play
//	Inputs	:	pointer the sound info struct
//	Result	:	the chipdat struct is filled
//	Updated	:	6-7-1993
//
int get_chip_buffers_S( SoundInfo *sinfo )
{
	sinfo->chipdat.memS = ( char * )AllocMem( 2 * sinfo->chipsize, MEMF_CHIP | MEMF_CLEAR );
	if( sinfo->chipdat.memS != NULL )
	{
		sinfo->chipdat.chipS1 = sinfo->chipdat.memS;
		sinfo->chipdat.chipS2 = sinfo->chipdat.memS + sinfo->chipsize;
		return 1;
	}
	else
		return 0;
}

//===============================================
//	Name		:	get_chipmem
//	Function	:	allocate chip mem buffers
//	Inputs	:	pointer the sound info struct
//	Result	:	the chipdat struct is filled
//	Updated	:	8-12-1993
//
int get_chipmem( SoundInfo *sinfo )
{
//	KPrintF("Allocate chipmem buffers ");
	if( sinfo->chipdat.mem == NULL ) 						// already assigned
	{
		if( !get_chip_buffers( sinfo ) )
		{
//			KPrintF("Fail\n");
			return 0;
		}
	}
//	else
//		KPrintF("Already done ");

	if( sinfo->type & SI_STEREO )
	{
//		KPrintF("Stereo ");
		if( sinfo->chipdat.memS == NULL )
			if( ! get_chip_buffers_S( sinfo ) )
			{
//				KPrintF("Fail\n");
				return 0;
			}
	}
//	KPrintF("Oke\n");
//	printf("mem is %lx\n",sinfo->chipdat.mem );
//	printf("memS is %lx\n",sinfo->chipdat.memS );
	load_sound_frame( sinfo );				// load the initial part
	return 1;
}

BYTE codeToDelta[16] = {-34, -21, -13,-8,-5,-3,-2,-1,0,1,2,3,5,8,13,21 };

void D1unpack( BYTE *source, LONG n, BYTE *dest, BYTE x )
{
	BYTE d;
	LONG i,lim;

	lim = n << 1;
	for( i = 0; i < lim; ++i )
	{
		d = source[ i >> 1 ];
		if( i & 1 )			// als i oneven dan de onderste nybble
			d &= 0x0f;
		else
			d >>= 4;			// i even de bovenste nybble
		d &= 0x0f;
		x += codeToDelta[d];
		dest[i] = x;
	}
}

//===============================================
//	Name		:	Fibo_unpack
//	Function	:	
//	Inputs	:	pointer to soundinfo struct, size off the body, dosfile pointer
//	Result	:	pointer to memory with uncompressed data, NULL when fail
//	Updated	:	22-05-93
//
BYTE *Fibo_unpack( SoundInfo *sinfo, LONG csize , BPTR file)
{
	struct Library *DOSBase;

	BYTE *tmem;
	BYTE *mem=NULL;
	LONG size;

	DOSBase = sinfo->DOSBase;

	tmem = ( BYTE *)AllocMem( csize, MEMF_FAST );
	if( tmem )
	{
		Read( file, tmem, csize );
		size = ( csize - 2 ) * 2;
		mem = ( BYTE *)AllocMem( size , MEMF_FAST );
		if( mem )
			D1unpack( tmem + 2, csize -2, mem, tmem[1] ); 
		FreeMem( tmem, csize );
	}
	return mem;
}

//===============================================
//	Name		:	loadsoundfile_disk
//	Function	:	load a sound file
//	Inputs	:	pointer to soundinfo struct
//	Result	:	non zero by succes
//				:	The file is kept open for loading when using diskplay
//	Updated	:	22-05-93
//
int loadsoundfile( SoundInfo *sinfo, char *filename, BOOL useQueue, ULONG signal )
{
	BPTR	file;
	LONG sizefile;
	LONG formsize;
	LONG formid;
	LONG chunkid;
	LONG chunksize;
	ULONG channel;
	UBYTE *mem;

	LONG size;
	int reterr = 1;
	int mpmmu_err = 0;

	struct Voice8Header vhdr;
	struct Library *DOSBase;
	struct Library *MLMMULibBase;
	struct FileInfoBlock *Finfo;
	BPTR	lock;

//	KPrintF("Load file %s\n",filename );

	sinfo->chipsize     = CHIP_SIZE;
	sinfo->end          = 0;
	sinfo->fp           = NULL;
	sinfo->seq_data     = NULL;
	sinfo->sounddata    = NULL;
	sinfo->chipdat.mem  = NULL;
	sinfo->chipdat.memS = NULL;
	sinfo->blockcount   = 0;
	sinfo->audionum     = -1;
	sinfo->fadenum      = -1;
	size = 0;

	sinfo->DOSBase  = OpenLibrary( "dos.library", 0 );
	DOSBase = sinfo->DOSBase;

	if( 1 )		//get_chip_buffers( sinfo ) ) get chip buffers later
	{
		size = 0;
		Finfo = ( struct FileInfoBlock * )AllocMem( sizeof( struct FileInfoBlock ), MEMF_PUBLIC );

		if( Finfo )
		{
			lock = Lock( filename, ACCESS_READ );
			if( lock != NULL )
			{
				Examine( lock, Finfo );
				size = Finfo->fib_Size;
				UnLock( lock );
			}
			FreeMem( (char *)Finfo, sizeof( struct FileInfoBlock ) );	
		}
		if( size != 0 )
		{
			file = Open((UBYTE *)filename,MODE_OLDFILE );
			if( file != 0 )
			{
				sinfo->fp = file;
//				Seek( file, 0L ,OFFSET_END );
//				size = Seek( file, 0, OFFSET_CURRENT);
//				Seek( file, 0L, OFFSET_BEGINNING );		// filepointer naar begin

				Read( file, (char *)&sizefile, 4 );

				if( sizefile != size - 6 )
				{
					Read(file, (char *)&formsize, 4);
					Read(file, (char *)&formid, 4);
					if( sizefile == ID_FORM )
					{
						if(  formid == ID_8SVX )
						{
//						Read(file, (char *)&formsize, 4);
//						Read(file, (char *)&formid, 4);
							formsize -= 4;	// de formnaam is al gelezen

							while( formsize > 0 )
							{
								Read(file, (char *)&chunkid, 4 );
								Read(file, (char *)&chunksize, 4 );

								if( chunksize & 1 )
									chunksize++;

								switch( chunkid )
								{

									case ID_VHDR :
										if( sizeof( struct Voice8Header )== chunksize )
											Read(file, (char*)&vhdr.oneShotHiSample, sizeof( struct Voice8Header ) );
										else
											Seek(file, chunksize, OFFSET_CURRENT );
										break;
									case ID_NAME :
									case ID_ANNO :
									case ID_COPY :
									case ID_AUTH :
									case ID_ATAK :
									case ID_RLSE :
											Seek(file, chunksize, OFFSET_CURRENT );
										break;
									case ID_CHAN :
											Read(file,(char *)&channel,sizeof(LONG) );
										break;								
									case ID_SEQN :
											if( sinfo->type & SI_INFO )
											{
												Seek(file, chunksize, OFFSET_CURRENT );
											}
											else
											{
												sinfo->seq_length = chunksize + 8;
												sinfo->seq_data = (ULONG *)AllocMem( chunksize + 8, MEMF_FAST );
												if( sinfo->seq_data )
												{
													Read(file,(char *)sinfo->seq_data,chunksize);
													sinfo->seq_data[chunksize/4] = 0;
													sinfo->seq_data[chunksize/4 + 1] = 0;
													sinfo->seq_offset = 0;
													sinfo->seq_play = 0;
													sinfo->type |= SI_SEQ;
//		KPrintF("AAI found a sequence\n");
												}
												else
													Seek(file, chunksize, OFFSET_CURRENT );
											}
										break;	

									case ID_FADE :
											Seek(file, chunksize, OFFSET_CURRENT );
										break;
									case ID_BODY :
											if( ( sinfo->type & SI_DISK ) || sinfo->type & SI_INFO )
											{
												sinfo->start_offset = Seek( file, 0, OFFSET_CURRENT);
												if( vhdr.sCompression == 0 )
													size = chunksize;
												else
												{
													size = ( chunksize - 2 ) * 2;
													sinfo->type |= SI_FIBO;
												}
											}
											else			// memory play
											{
												if( vhdr.sCompression == 0 )
												{
													size = chunksize;
													mem=  ( BYTE *)AllocMem( size, MEMF_FAST );
													if( mem != NULL )
													{
//					KPrintF("Actual read file\n");
														Read(file,mem,size );
													}
													else
													{
// Add here forced play from disk 30-11-1993
//					KPrintF("Out of memory for sample play\n");
//					KPrintF("Forcing diskplay\n");
														sinfo->type |= SI_DISK|SI_NORMAL;	// Set diskplay + normal bit on
														sinfo->start_offset = Seek( file, 0, OFFSET_CURRENT);

														if( vhdr.sCompression == 0 )
															size = chunksize;
														else
														{
															size = ( chunksize - 2 ) * 2;
															sinfo->type |= SI_FIBO;

														}
//														Seek(file, chunksize, OFFSET_CURRENT );
													}
												}
												else
												{
													size = ( chunksize - 2 ) * 2;
													mem = Fibo_unpack( sinfo, chunksize , file );
													sinfo->type |= SI_FIBO;
												}
											}
										break;
									default:
										Seek(file, chunksize, OFFSET_CURRENT );
								}
								formsize -= 8;
								formsize -= chunksize;
							}
							sinfo->vol_right = vhdr.volume >> 10;
							sinfo->period = (WORD)( 3579546 /  vhdr.samplesPerSec );
						}
						else
						{
							reterr = 0;								// there was no 8svx header in the iff file
							mpmmu_err = ERR_NO_8SVX;
						}
					}			// RAW data file
					else
					{
						sinfo->type |= SI_RAW;
						sinfo->vol_right = 64;
						sinfo->period = 128;
						sinfo->start_offset = Seek(file, 0L, OFFSET_BEGINNING );
						if( ( sinfo->type & SI_NORMAL ) && !( sinfo->type & SI_INFO ) )
						{
							mem=  ( BYTE *)AllocMem( size, MEMF_FAST );
							if( mem != NULL ) 
								Read(file,mem,size );
						}
					}
				}
				else
				{
					sinfo->type |= SI_RAW;
					sinfo->vol_right = 64;
					sinfo->period = 128;
					Seek(file, 2L, OFFSET_CURRENT );
					sinfo->start_offset = Seek(file, 0L, OFFSET_BEGINNING );
					size = sizefile;
					if( sinfo->type & SI_NORMAL && !( sinfo->type & SI_INFO ) )
					{
						mem=  ( BYTE *)AllocMem( size, MEMF_FAST );
						if( mem != NULL ) 
							Read(file,mem,size);
					}
				}

				if( channel == STEREO )
				{
					sinfo->type |= SI_STEREO;
//					KPrintF("stereo sample\n" );
					size >>= 1;										// size is size left and right 
//					if( !get_chip_buffers_S( sinfo ) )		// get chip buffers later
//						reterr = 0;
				}

				if( reterr == 1 )
					if( ( sinfo->type & SI_DISK ) && !( sinfo->type & SI_INFO )  )
					{	
						if( size )
						{
							sinfo->soundoffset 	= 0;			// start playing at 0
							sinfo->soundlength 	= size;	
							reterr = 1;
//							load_sound_frame( sinfo );			// do it later
						}
						else
						{
							reterr = 0;
							mpmmu_err = ERR_GENERAL;
						}
					}
					else
					{
						Close(file);
						sinfo->fp = NULL;
						if( mem && !( sinfo->type & SI_INFO ) )
						{
							sinfo->sounddata 	= mem;
							sinfo->soundoffset 	= 0;			// start playing at 0
							sinfo->soundlength 	= size;
//							load_sound_frame( sinfo );			// do it later
							reterr = 1;
						}
					}
			}
			else
			{
				reterr = 0;
				mpmmu_err = ERR_FILE_NOT_FOUND;
			}
		}
		else
		{
			reterr = 0;
			mpmmu_err = ERR_FILE_NOT_FOUND;
		}
	}

	sinfo->vol_left = sinfo->vol_right;

	if( reterr && !( sinfo->type & SI_INFO ) )
		reterr = alloc_signals( sinfo,signal );

	if( reterr == 0 )
	{
//		KPrintF("Freeing sound\n");
		freesound( sinfo );				// free all resources
	}
	if( useQueue && (mpmmu_err != 0) )
	{
		MLMMULibBase = OpenLibrary( "mpmmu.library", 0 );
		if ( MLMMULibBase )
		{
			char er[128];
			strcpy(er,"Sample: ");
			if( mpmmu_err == ERR_FILE_NOT_FOUND )
				sprintf(er,"Sample: file '%s' not found", filename);
			else if( mpmmu_err == ERR_NO_8SVX )
				strcpy(er,"Not an 8SVX IFF file");
			else if( mpmmu_err == ERR_GENERAL )
				strcpy(er,"General error");
			else
				strcpy(er,"Unknown error");	
			MLMMU_AddMsgToQueue(er, mpmmu_err );
			CloseLibrary( MLMMULibBase );
		}
	}

	return reterr;
}

//===============================================
//	Name		:	load_sound_frame_disk_fibo
//	Function	:	load the next piece of the sample with FIBO comp. from disk
//	Inputs	:	pointer to soundinfo
//	Result	:	the next frame is loaded and
//				:	if at the end a 1 is returned
//	Updated	:	7-10-1993
//
int load_sound_frame_disk_fibo( SoundInfo *sinfo )
{
	//char tt[250];
	int stop = 0;
	long t;
	long rs;
	long readt;
	long off;
	BYTE *mem = NULL;
	BYTE *memS;
	long csize;
	long fsize;

	struct Library *DOSBase;
	DOSBase = sinfo->DOSBase;

	csize = 2 + ( sinfo->chipsize / 2 );
	fsize = 2 + ( sinfo->soundlength / 2 );

	mem = AllocMem( sinfo->chipsize + 10, MEMF_PUBLIC );
	if( mem != NULL )
	{
		memS = mem + csize + 4;
		t = 0;
		rs = csize;
		off = 0;

		if( sinfo->end == 0 )
		{
			while( t < csize && rs > 0  && !stop )
			{
				t = rs;

				if( ( sinfo->soundoffset + rs ) > fsize )
					t = sinfo->soundlength - ( sinfo->soundoffset  );		// load rest

				readt = Read( sinfo->fp, mem + off, t );

				sinfo->soundoffset += readt;
				sinfo->play_offset += readt;

				if( sinfo->type & SI_STEREO )
				{
					Seek( sinfo->fp, fsize - readt , OFFSET_CURRENT );
					Read( sinfo->fp, memS + off, t );
					Seek( sinfo->fp, -fsize , OFFSET_CURRENT );
				}

				off += readt;
				rs -= readt;
				t = readt;
				sinfo->blockcount++;
				if( rs > 0 )				// Not everything is loaded
				{
					if( ( sinfo->type & SI_LOOPING ) && ( sinfo->loop < sinfo->loops || 
								sinfo->loops==0 ) )
					{
						sinfo->loop++;
						Seek( sinfo->fp, sinfo->start_offset,OFFSET_BEGINNING );
						sinfo->play_offset = sinfo->start_offset;
						sinfo->soundoffset = 0;
					}
					else		// unpack what you've got
					{
						D1unpack(mem+2,t-2,sinfo->chipdat.chip2,mem[1]);
						if( sinfo->type & SI_STEREO )
						{
							D1unpack(memS+2,t-2,sinfo->chipdat.chipS2,memS[1]);
							t = off;
							t *= 2;
							for( ; t < sinfo->chipsize; t++ )
								*(sinfo->chipdat.chipS2 + t) = 0;
						}
						t = off;
						t *= 2;
						for( ; t < sinfo->chipsize; t++ )
							*(sinfo->chipdat.chip2 + t) = 0;
						stop = 1;
						break;
					}
				}
				else
				{
					if( sinfo->type & SI_STEREO )
						D1unpack(memS+2,csize-2,sinfo->chipdat.chipS2,memS[1]);
					
					D1unpack(mem+2,csize-2,sinfo->chipdat.chip2,mem[1]);
				}
			}
		}
		else
		{
			for( t = 0; t < sinfo->chipsize; t++ )
				*(sinfo->chipdat.chip2 + t) = 0;
			stop = 1;
		}
		FreeMem( mem, sinfo->chipsize + 10 );
	}
	sinfo->end = stop;
	return stop;
}


//===============================================
//	Name		:	load_sound_frame_disk
//	Function	:	load the next piece of the sample from disk
//	Inputs	:	pointer to soundinfo
//	Result	:	the next frame is loaded and
//				:	if at the end a 1 is returned
//	Updated	:	3-7-1993
//
int load_sound_frame_disk( SoundInfo *sinfo )
{
	int stop = 0;
	long t;
	long rs;
	long readt;
	long off;

	struct Library *DOSBase;
	DOSBase = sinfo->DOSBase;

	t = 0;
	rs = sinfo->chipsize;
	off = 0;

//	KPrintF("Loading sound frame disk\n");
	if( sinfo->end == 0 )
	{
		while( t < sinfo->chipsize && rs > 0  && !stop )
		{
			t = rs;

			if( ( sinfo->soundoffset + rs ) > sinfo->soundlength )
				t = sinfo->soundlength - ( sinfo->soundoffset  );		// load rest

			readt = Read( sinfo->fp, sinfo->chipdat.chip2 + off, t );

// readt should be the same as t

			sinfo->soundoffset += readt;
			sinfo->play_offset += readt;

			if( sinfo->type & SI_STEREO )
			{
				Seek( sinfo->fp, sinfo->soundlength - readt , OFFSET_CURRENT );
				Read( sinfo->fp, sinfo->chipdat.chipS2 + off, t );
				Seek( sinfo->fp, -sinfo->soundlength , OFFSET_CURRENT );
			}

			off += readt;
			rs -= readt;
			t = readt;
			sinfo->blockcount++;
			if( rs > 0 )
			{
				if( ( sinfo->type & SI_LOOPING ) &&( sinfo->loop < sinfo->loops || sinfo->loops==0 ) )
				{
					sinfo->loop++;
					Seek( sinfo->fp, sinfo->start_offset,OFFSET_BEGINNING );
					sinfo->play_offset = sinfo->start_offset;
					sinfo->soundoffset = 0;
				}
				else
				{
					if( sinfo->type & SI_STEREO )
					{
						t = off;
						for( ; t < sinfo->chipsize; t++ )
							*(sinfo->chipdat.chipS2 + t) = 0;
					}
					t = off;
					for( ; t < sinfo->chipsize; t++ )
						*(sinfo->chipdat.chip2 + t ) = 0;
					stop = 1;
					break;
				}
			}
		}
	}
	else
	{
		for( t = 0; t < sinfo->chipsize; t++ )
			*(sinfo->chipdat.chip2 + t) = 0;
		stop = 1;
	}
	sinfo->end = stop;
	return stop;
}

//===============================================
//	Name		:	load_sound_frame_mem
//	Function	:	load the next piece of the sample from memory
//	Inputs	:	pointer to soundinfo
//	Result	:	the next frame is loaded and
//				:	if at the end a 1 is returned
//	Updated	:	3-7-1993
//
int load_sound_frame_mem( SoundInfo *sinfo )
{
//	char tt[200];
	int stop = 0;
	long t;
	long rs;
	long off;

	t = 0;
	rs = sinfo->chipsize;
	off = 0;

	if( sinfo->end == 0 )
	{
		while( t < sinfo->chipsize && rs > 0  && !stop )
		{
			t = rs;
			if( ( sinfo->soundoffset + rs ) > sinfo->soundlength )
				t = sinfo->soundlength - ( sinfo->soundoffset  );		// load rest
//			sprintf(tt,"Copy soffset %d, aantal %d, loff %d", sinfo->soundoffset, t, off );
//			KPrintF("%s\n",tt );

			CopyMem(	sinfo->sounddata+sinfo->soundoffset,
						sinfo->chipdat.chip2+off,t );

			if( sinfo->type & SI_STEREO )		// if stereo the left sample is 'size' further
				CopyMem(	sinfo->sounddata+sinfo->soundoffset+sinfo->soundlength,
							sinfo->chipdat.chipS2+off,t );

			sinfo->blockcount++;
			sinfo->soundoffset += t;
			rs -= t;
			off += t;

			if( rs > 0 )
			{
				if( ( sinfo->type & SI_LOOPING ) && ( sinfo->loop < sinfo->loops || 
							sinfo->loops==0 ) )
				{
					sinfo->loop++;
					sinfo->soundoffset = 0;
				}
				else
				{
					if( sinfo->type & SI_STEREO )
					{
						t = off;
						for( ; t < sinfo->chipsize; t++ )
							*(sinfo->chipdat.chipS2 + t) = 0;
					}
//					sprintf(tt,"Clearing from %d",t );
//					KPrintF("%s\n",tt );
					t = off;
					for( ; t < sinfo->chipsize; t++ )
						*(sinfo->chipdat.chip2 + t) = 0;
					stop = 1;
					break;
				}
			}
		}
	}
	else
	{
		for( t = 0; t < sinfo->chipsize; t++ )
			*(sinfo->chipdat.chip2 + t) = 0;
		stop = 1;
	}

	sinfo->end = stop;
	return stop;
}
#if 0
//===============================================
//	Name		:	load_sound_frame_mem_seq
//	Function	:	load the next piece of the sample from memory
//				:	with the use of the sequence data
//	Inputs	:	pointer to soundinfo
//	Result	:	the next frame is loaded and
//				:	if at the end a 1 is returned
//	Updated	:	13-10-1993
//
int load_sound_frame_mem_seq( SoundInfo *sinfo )
{
//	char tt[200];
	
	long s_start,s_stop;
	int stop = 0;
	long t;
	long rs;
	long off;

	t = 0;
	rs = sinfo->chipsize;
	off = 0;

	if( sinfo->end == 0 )
	{
		while( t < sinfo->chipsize && rs > 0  && !stop )
		{
			t = rs;

			s_start = sinfo->seq_data[ sinfo->seq_offset ];
			s_stop = sinfo->seq_data[ sinfo->seq_offset + 1 ];
			s_start += sinfo->seq_play;					// where in the seguence was I

			if( ( s_start + rs ) > s_stop )
			{
				t = s_start - s_stop;				// value that fits in the sequence
				sinfo->seq_offset++;
				sinf0->seq_play = 0;					// start with the begin of this sequence
				if( sinfo->seq_offset >= sinfo->seq_length )
				{
					sinfo->seq_offset = 0;
					stop = 1;				
				}
			}
			else			// whole the buffer can be filled with the part of this sequence
			{
				sinfo->seq_play += t;
			}

//	Does the whole sequence part fit in the block

			if( ( sinfo->soundoffset + rs ) > sinfo->soundlength )
				t = sinfo->soundlength - ( sinfo->soundoffset  );		// load rest

			CopyMem(	sinfo->sounddata+sinfo->soundoffset,
						sinfo->chipdat.chip2+off,t );

			if( sinfo->type & SI_STEREO )		// if stereo the left sample is 'size' further
				CopyMem(	sinfo->sounddata+sinfo->soundoffset+sinfo->soundlength,
							sinfo->chipdat.chipS2+off,t );

			sinfo->blockcount++;
			sinfo->soundoffset += t;
			rs -= t;
			off += t;

			if( rs > 0 )
			{
				if( ( sinfo->type & SI_LOOPING ) && ( sinfo->loop < sinfo->loops || 
							sinfo->loops==0 ) )
				{
//					printf("Looping\n");
					sinfo->loop++;
					sinfo->soundoffset = 0;
				}
				else
				{
					if( sinfo->type & SI_STEREO )
					{
						rs = t;
						for( ; t < sinfo->chipsize; t++ )
							*(sinfo->chipdat.chipS2 + t) = 0;
						t = rs;
					}
//					sprintf(tt,"Clearing from %d",t );
//					KPrintF("%s\n",tt );
					for( ; t < sinfo->chipsize; t++ )
						*(sinfo->chipdat.chip2 + t) = 0;
					stop = 1;
					break;
				}
			}
		}
	}
	else
	{
		for( t = 0; t < sinfo->chipsize; t++ )
			*(sinfo->chipdat.chip2 + t) = 0;
		stop = 1;
	}

	sinfo->end = stop;
	return stop;
}
#endif

//===============================================
//	Name		:	load_sound_frame
//	Function	:	load the next piece of the sample
//	Inputs	:	pointer to soundinfo
//	Result	:	the next frame is loaded and
//				:	if at the end a 1 is returned
//	Updated	:	3-7-1993
//
void load_sound_frame( SoundInfo *sinfo )
{
	if( sinfo->type & SI_DISK )
	{
		if( sinfo->type & SI_FIBO )
			load_sound_frame_disk_fibo( sinfo );
		else
			load_sound_frame_disk( sinfo );
	}
	else
		if( sinfo->type & SI_NORMAL )
			load_sound_frame_mem( sinfo );
}

//===============================================
//	Name		:	free_chipmem
//	Function	:	release chip memory
//	Inputs	:	pointer to soundinfo
//	Result	:	all chipmem allocated is freed
//	Updated	:	8-12-1993
//
void free_chipmem( SoundInfo *sinfo )
{
	struct Library *DOSBase;
	DOSBase = sinfo->DOSBase;

//	KPrintF("Freeing chipmem\n");
	if( sinfo->chipdat.mem != NULL )
		FreeMem( sinfo->chipdat.mem, sinfo->chipsize * 2 );
	if( sinfo->chipdat.memS != NULL )
		FreeMem( sinfo->chipdat.memS, sinfo->chipsize * 2 );
	sinfo->chipdat.mem = NULL;			// Don't clear me twice
	sinfo->chipdat.memS = NULL;
}

//===============================================
//	Name		:	freesound
//	Function	:	release all memory and close file
//	Inputs	:	pointer to soundinfo
//	Result	:	all data allocated is freed
//	Updated	:	3-7-1993
//
void freesound( SoundInfo *sinfo )
{

	struct Library *DOSBase;
	DOSBase = sinfo->DOSBase;

	if( sinfo->chipdat.mem != NULL )
		FreeMem( sinfo->chipdat.mem, sinfo->chipsize * 2 );
	if( sinfo->chipdat.memS != NULL )
		FreeMem( sinfo->chipdat.memS, sinfo->chipsize * 2 );
	if( sinfo->seq_data != NULL )
		FreeMem( sinfo->seq_data, sinfo->seq_length );
	if( sinfo->sounddata != NULL )
		if( sinfo->type & SI_STEREO )
			FreeMem( sinfo->sounddata, sinfo->soundlength*2 );
		else
			FreeMem( sinfo->sounddata, sinfo->soundlength );

	if( sinfo->fp != NULL )
		Close( sinfo->fp );
	if( sinfo->audionum != -1 )
		FreeSignal( sinfo->audionum );
	if( sinfo->fadenum != -1 )
		FreeSignal( sinfo->fadenum );
	if( DOSBase )
		CloseLibrary( DOSBase );

	sinfo->chipdat.mem = NULL;
	sinfo->chipdat.memS = NULL;
	sinfo->seq_data = NULL;
	sinfo->sounddata = NULL;
	sinfo->fp = NULL;
	sinfo->audionum = -1;
	sinfo->fadenum = -1;
	DOSBase = NULL;
}
