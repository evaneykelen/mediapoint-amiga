//	File		:	sample.h
//	Uses		:
//	Date		:	3 - july 1993
//	Author	:	ing. C. Lieshout
//	Desc.		:	structs and defines used in de sample player
//

#define CHIP_SIZE 16000

#define SEEK_END 2
#define SEEK_CUR 1
#define SEEK_SET 0

#define SI_NORMAL		0x01
#define SI_INFO		0x02
#define SI_DISK		0x04
#define SI_SEQ			0x08
#define SI_FIBO		0x10
#define SI_RAW			0x20
#define SI_LOOPING	0x40
#define SI_STEREO		0x80

typedef struct
{
	char *chip1;
	char *chip2;
	char *chipS1;
	char *chipS2;
	char *mem;
	char *memS;
} ChipDat;

typedef struct
{
	UBYTE	*sounddata;
	LONG	soundlength;
	LONG	soundoffset;

	BPTR	fp;						// file-handle
	long	start_offset;			// points to sample start
	long	play_offset;			// points to sample playing point

	ULONG	*seq_data;				// pointer to seguence data
	long	seq_offset;				// points to playing seguence;
	long	seq_length;				// length of the sequence data
	long	seq_play;				// where in the seguence is the play pointer

	UWORD	loop;						// loops played

	UWORD	period;
	UWORD	loops;					// nr off loops
	UWORD channel;

	UBYTE	type;						// type sample
	UBYTE	end;
	WORD blockcount;
	WORD sigtest;					// sent out signal yes or no

	LONG	audiosig;
	LONG	audionum;

	LONG	fadesig;
	LONG	fadenum;
	LONG	task;

	long chipsize;
	ChipDat	chipdat;					// the chipmem buffers

	long dummy;
	struct Interrupt int_audio;	// the audio interupt structure
	struct Interrupt int_fade;		// the fade interupt structure
	long old_audio;
	struct Library *DOSBase;

	WORD filter;
	WORD vol_right;
	WORD vol_left;
	WORD vol_temp_right;
	WORD vol_temp_left;
	WORD inc_right;
	WORD inc_left;
} SoundInfo;
