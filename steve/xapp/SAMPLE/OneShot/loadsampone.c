// File		: loadsampone.c
// Uses		:
//	Date		: 2 february 1993 updated 3-7-1993  20-1-1994
// Author	: ing. C. Lieshout
// Desc.		: Sample load and play routines for oneshot play
//

#define FREQ 3546895

//#include <stdio.h>
//#include <string.h>
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

#include "iff_fsound.h"
#include	"sampleone.h"

void freesound( SoundInfoOne *sinfo );

//===============================================
//	Name		: alloc_signals
//	Function	: allocate audio and fade interupts signals
//	Inputs	: pointer to soundinfo struct
//	Result	: a 1 if the allocate were both allocated
//				: Sets both signum and signal
//	Updated	: ( 1 - 7 - 1993 )
//
int alloc_signals( SoundInfoOne *sinfo )
{
	sinfo->audionum = -1;

	sinfo->audionum = AllocSignal( -1 );	
	if( sinfo->audionum != -1 )
		sinfo->audiosig = 1L << sinfo->audionum;

	sinfo->task = ( long )FindTask( 0 );
	if( sinfo->audionum == -1 )
		return 0;
	else
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
BYTE *Fibo_unpack( SoundInfoOne *sinfo, LONG csize , BPTR file)
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
		mem = ( BYTE *)AllocMem( size , MEMF_CHIP );
		if( mem )
			D1unpack( tmem + 2, csize -2, mem, tmem[1] ); 
		FreeMem( tmem, csize );
	}
	return mem;
}

//===============================================
//	Name		:	loadsoundfile
//	Function	:	load a sound file
//	Inputs	:	pointer to soundinfo struct
//	Result	:	non zero by succes
//	Updated	:	22-05-93
//
int loadsoundfile( SoundInfoOne *sinfo, char *filename )
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

	struct Voice8Header vhdr;
	struct Library *DOSBase;
	struct FileInfoBlock *Finfo;
	BPTR	lock;

//	KPrintF("Load file %s\n",filename );

	sinfo->sounddata    = NULL;
	sinfo->audionum     = -1;
	size = 0;

	sinfo->DOSBase  = OpenLibrary( "dos.library", 0 );
	DOSBase = sinfo->DOSBase;

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
				Read( file, (char *)&sizefile, 4 );

				if( sizefile != size - 6 )
				{
					Read(file, (char *)&formsize, 4);
					Read(file, (char *)&formid, 4);
					if( sizefile == ID_FORM )
					{
						if(  formid == ID_8SVX )
						{
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
										if( sizeof( struct Voice8Header ) == chunksize )
											Read(file, (char*)&vhdr.oneShotHiSample, sizeof( struct Voice8Header ) );
										else
											Seek(file, chunksize, OFFSET_CURRENT );
										break;
									case ID_CHAN :
											Read(file,(char *)&channel,sizeof(LONG) );
										break;
									case ID_BODY :
											{
												if( vhdr.sCompression == 0 )
												{
													size = chunksize;
													if( size < 128000 )
													{
														mem = ( BYTE *)AllocMem( size, MEMF_CHIP );
													}
													if( mem != NULL )
													{
//									KPrintF("Actual read file\n");
														Read(file,mem,size );
													}
													else
													{
//							KPrintF("Out of memory for sample play\n");
														reterr = 0;
													}
												}
												else
												{
													size = ( chunksize - 2 ) * 2;
													mem = Fibo_unpack( sinfo, chunksize , file );
												}
											}
										break;
									default:
										Seek(file, chunksize, OFFSET_CURRENT );
								}
								formsize -= 8;
								formsize -= chunksize;
							}
							sinfo->period = ( 3579546 /  (WORD)vhdr.samplesPerSec );
						}
						else
							reterr = 0;								// there was no 8svx header in the iff file
					}			// RAW data file
					else
					{
						sinfo->period = 128;
						if( size < 128000 )
						{
							mem = ( BYTE *)AllocMem( size, MEMF_CHIP );
							if( mem != NULL ) 
								Read(file,mem,size );
						}
						else
							reterr = 0;
					}
				}
				else
				{
					sinfo->period = 128;
					Seek(file, 2L, OFFSET_CURRENT );
					size = sizefile;
					if( size < 128000 )
					{
						mem=  ( BYTE *)AllocMem( size, MEMF_CHIP );
						if( mem != NULL ) 
							Read(file,mem,size);
					}
					else
						reterr = 0;
				}

				if( mem )
				{
					sinfo->sounddata = mem;
					sinfo->soundlength = size;
					sinfo->memsize = size;
				}

				if( channel == STEREO )
				{
					sinfo->type |= SI_STEREO;
					sinfo->soundlength = size >> 1;
//					KPrintF("stereo sample\n" );
				}
				Close( file );
			}
			else
				reterr = 0;
		}
		else
			reterr = 0;

	if( reterr )
		reterr = alloc_signals( sinfo );

	if( reterr == 0 )
		freesound( sinfo );				// free all resources

	return reterr;
}

//===============================================
//	Name		:	freesound
//	Function	:	release all memory and close file
//	Inputs	:	pointer to soundinfo
//	Result	:	all data allocated is freed
//	Updated	:	3-7-1993
//
void freesound( SoundInfoOne *sinfo )
{

	struct Library *DOSBase;
	DOSBase = sinfo->DOSBase;

	if( sinfo->sounddata != NULL )
		FreeMem( sinfo->sounddata, sinfo->memsize );
	if( sinfo->audionum != -1 )
		FreeSignal( sinfo->audionum );

	if( DOSBase )
		CloseLibrary( DOSBase );
	sinfo->sounddata = NULL;
	sinfo->audionum = -1;
	DOSBase = NULL;
}
