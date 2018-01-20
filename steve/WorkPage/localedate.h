/**** defines ****/

#define LD_DATE_1 			1
#define LD_DATE_2 			2
#define LD_DATE_3 			3
#define LD_DATE_4 			4
#define LD_TIME_1 			5
#define LD_TIME_2 			6
#define LD_TIME_3 			7
#define LD_SECS					8
#define LD_DATE					9
#define LD_LONG_DAY			10
#define LD_SHORT_DAY		11
#define LD_LONG_MONTH		12
#define LD_SHORT_MONTH	13
#define LD_LONG_YEAR		14
#define LD_SHORT_YEAR		15

/**** functions ****/

void CreateLocaleTime(STRPTR str, int format);
int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer);

/******** E O F ********/
