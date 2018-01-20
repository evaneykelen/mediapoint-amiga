/**** structs.h ****/

#define SAMPLE_PLAY			1
#define SAMPLE_STOP			2
#define SAMPLE_FADEOUT	3
#define SAMPLE_FADEIN		4
#define SAMPLE_SETVOL		5

#define	DEFAULT_VOL				100
#define DEFAULT_FREQ			28867
#define DEFAULT_FADE			0
#define DEFAULT_LOOPS			2
#define DEFAULT_BALANCE		0	// ranges from -5...+5 (``left to right'')
#define DEFAULT_FADEINOUT	5
#define DEFAULT_SETVOL		100

struct Sample_record
{
	int action;	// SAMPLE_PLAY etc.

	TEXT filename[SIZE_FULLPATH];
	int loops;
	int volume;
	int freq;
	int playFadeIn;
	int balance;

	int fadeOut;

	int fadeIn;

	int setVolume;

	int trackPlay, trackStop, trackFadeIn, trackFadeOut, trackSetVol;	// 0 or 1
	int track;

	int playFromDisk;
	int filter;
	int soundBox;
};

/******** E O F ********/
