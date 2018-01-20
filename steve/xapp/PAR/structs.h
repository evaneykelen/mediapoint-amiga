struct PAR_Record
{
	TEXT path[SIZE_FULLPATH];
	TEXT in[16];
	TEXT out[16];
	TEXT cue[16];
	int studio16Cue;
	int startFrame;
	int endFrame;

	int startPix;
	int endPix;
};

/******** E O F *******/
