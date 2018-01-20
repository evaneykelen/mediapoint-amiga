#define MODE_PLAY			0
#define MODE_RECORD		1
#define MODE_MISC			2

#define MISC_STOP			0
#define MISC_FADEIN		1
#define MISC_FADEOUT	2
#define MISC_SETVOL		3

struct Toccata_Record
{
	int mode;				// See MODE_XXX flags

	TEXT playPath[SIZE_FULLPATH];
	int playVolume;
	int playLoops;
	int playFadeIn;

	TEXT recordPath[SIZE_FULLPATH];
	int recordCompression;
	int stereo;
	int input;
	int gain;
	int freq;
	int level;

	int misc;				// See MISC_XXX flags
	int stopFadeOut;
	int fadeInSecs;
	int fadeOutSecs;
	int setVolFrom;
	int setVolTo;
	int setVolSecs;
};

/******** E O F *******/
