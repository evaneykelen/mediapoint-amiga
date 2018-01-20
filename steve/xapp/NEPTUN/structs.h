
struct Neptun_record
{
	int page;

	// page 1

	int computer1;
	int genlock_amiga;

	int video1;
	int normal_invert;

	int overlay1;
	int normal_alpha;

	// page 2	

	int computer2;
	int fadein_fadeout1;

	int video2;
	int fadein_fadeout2;

	// page 3

	int computer3;
	TEXT duration1[10];
	int from1;
	int pct1f;
	int to1;
	int pct1t;

	int video3;
	TEXT duration2[10];
	int from2;
	int pct2f;
	int to2;
	int pct2t;

	// version

	int version;	// 1 is old, 2 is new
};

/******** E O F ********/
