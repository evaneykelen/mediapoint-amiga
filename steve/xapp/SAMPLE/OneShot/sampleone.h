//	File		:	sampleone.h
//	Uses		:
//	Date		:	3 - july 1993, 19-jan-1994
//	Author	:	ing. C. Lieshout
//	Desc.		:	structs and defines used in de oneshot sample player
//

#define SEEK_END 2
#define SEEK_CUR 1
#define SEEK_SET 0

#define SI_STEREO		0x80

typedef struct
{
	UBYTE	*sounddata;
	LONG	memsize;
	LONG	soundlength;

	UWORD	period;
	UWORD channel;

	UBYTE	type;						// type sample
	UBYTE	end;

	LONG	audiosig;
	LONG	audionum;

	LONG	task;

	LONG dum;
	struct Interrupt int_audio;	// the audio interupt structure
	LONG dum2;

	long old_audio;
	struct Library *DOSBase;
} SoundInfoOne;
