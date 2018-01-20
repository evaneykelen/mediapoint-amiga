// File		: iff_fsound.h
// Uses		:
//	Date		: ( 18 july 1992 ) 2 february 1993
// Author	: ing. C. Lieshout
// Desc.		: IFF id's and several IFF structures
//

#define Mid(a,b,c,d) ( (a)<<24 | (b)<<16 | (c) << 8 | (d) )

#define ID_FORM Mid('F','O','R','M')
#define ID_8SVX Mid('8','S','V','X')
#define ID_VHDR Mid('V','H','D','R')
#define ID_NAME Mid('N','A','M','E')
#define ID_ANNO Mid('A','N','N','O')
#define ID_COPY Mid('(','c',')',' ')
#define ID_AUTH Mid('A','U','T','H')
#define ID_ATAK Mid('A','T','A','K')
#define ID_RLSE Mid('R','L','S','E')
#define ID_CHAN Mid('C','H','A','N')
#define ID_SEQN Mid('S','E','Q','N')
#define ID_FADE Mid('F','A','D','E')
#define ID_BODY Mid('B','O','D','Y')

#define RIGHT 4L
#define LEFT 2L
#define STEREO 6L

struct Voice8Header
{
	ULONG	oneShotHiSample,
			repeatHiSample,
			samplesPerHiCycle;
	UWORD	samplesPerSec;
	UBYTE	ctOctave,
			sCompression;
	LONG	volume;
};

struct EGPoint		// chunk voor ATAK en RLSE ??
{
	UWORD duration;
	LONG	dest;
};

