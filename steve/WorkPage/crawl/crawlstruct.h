#define CRAWLBLOCK_SIZE 1400

struct Crawl_Record
{
	char fontName[50];
	long fontSize;
	long speed;
	long col;
	long ypos;
	STRPTR txt;
	long zero_col;
	long window_height;
	long topmargin;
};

int crawl( char *crawldata, char *mem, struct Crawl_Record *rc );

/******** E O F ********/
