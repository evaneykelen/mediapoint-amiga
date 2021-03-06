/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Library *medialinkLibBase;
extern struct Screen *scriptScreen;

/**** globals ****/

static int dateconv_leapMonths[]	= { 31,29,31,30,31,30,31,31,30,31,30,31 };
static int dateconv_Months[]			= { 31,28,31,30,31,30,31,31,30,31,30,31 }; 

char *dateconv_monthNames[] = {	"Jan", "Feb", "Mar", "Apr",
																"May", "Jun", "Jul", "Aug",
																"Sep", "Oct", "Nov", "Dec"  };

/******

	1	=	dd/mm/yy
	2	=	dd-mm-yy
	3	=	dd.mm.yy
	4	=	dd mm yy
	5	=	dd.mm yy
	6	=	mm/dd/yy
	7	=	yy-mm-dd
	8	=	yy/mm/dd
	9	=	yymmdd

	1 = hh:mm:ss
	2 = hh.mm.ss

******/

/* first number is international access code, second is date type */

int dateFormat[] = {
														0,	/* undefined */
														1,	/* AUSTRALIA */
														1,	/* BELGIE */
														1,	/* BELGIQUE */
														1,	/* CANADA */
														7,	/* CANDADA_FR */
														8,	/* DANMARK */
														3,	/* DEUTSCHLAND */
														1,	/* FRANCE */
														1,	/* GREAT_BRITTAIN	*/
														4,	/* ITALIA */
														2,	/* NEDERLAND */
														3,	/* NORGE */
														9,	/* OSTERREICH */
														4,	/* PORTUGAL */
														5,	/* SCHWEIZ */
														5,	/* SUISSE */
														7,	/* SVERIGE */
														5,	/* SVIZZERA */
														1,	/* UNITED_KINGDOM	*/
														6,	/* USA */
};

/* first number is international access code, second is date type */

int timeFormat[] = {
														0,	/* undefined */
														1,	/* AUSTRALIA */
														1,	/* BELGIE */
														1,	/* BELGIQUE */
														1,	/* CANADA */
														1,	/* CANDADA_FR */
														1,	/* DANMARK */
														1,	/* DEUTSCHLAND */
														1,	/* FRANCE */
														1,	/* GREAT_BRITTAIN	*/
														1,	/* ITALIA */
														1,	/* NEDERLAND */
														2,	/* NORGE */
														1,	/* OSTERREICH */
														1,	/* PORTUGAL */
														1,	/* SCHWEIZ */
														1,	/* SUISSE */
														1,	/* SVERIGE */
														2,	/* SVIZZERA */
														1,	/* UNITED_KINGDOM	*/
														1,	/* USA */
};

/**** gadgets ****/

extern struct GadgetRecord Prog_GR[];

/******** dateStringtoDays() ********/
/*
 * converts a string like dd-mmm-yyyy to an int which represents the
 * number of days since Jan. 1, 1978. Returns -1 on error.
 *
 */

void dateStringtoDays(char *strPtr, int *totalDays)
{
int day, month, year;
char monthStr[4];
int nrLeaps;	/*, leap = FALSE;  */
int daysPassed;  
int *monthPtr;

	*totalDays=-1;

	if(sscanf(strPtr, "%2d-%3s-%4d",&day,monthStr,&year) == 3)
  {
		year -= ((year/100)*100);
		for(month=0; month<12; month++)
		{
			if(!strnicmp(monthStr, dateconv_monthNames[month],3))
				break;
		}
		if(!day || month == 12)
			return;
		day--;
		year=year+(year >= 78 ? 1900: 2000) - 1978;
		nrLeaps = (year+2)/4;

		if( !((year-2) % 4) )
		{
			nrLeaps--;
			monthPtr = dateconv_leapMonths;
		}
		else
			monthPtr = dateconv_Months;

		daysPassed = nrLeaps * 366 + (year-nrLeaps) * 365 + day;
		while(--month >= 0)
			daysPassed += monthPtr[month];
		*totalDays=daysPassed;
		return;
	}
}

/******** timeStringtoMinutesAndTicks() ********/
/*
 * converts a string like hh:mm:ss:t to an int which represents the
 * number of minutes past midnight and the number of ticks past minute
 * (50 ticks per second). 5 ticks are 1/10 of a second.
 *
 */

void timeStringtoMinutesAndTicks(char *strPtr, int *minutes, int *ticks)
{
int h,m,s,t;

	*minutes=-1;
	*ticks=-1;

	if(sscanf(strPtr,"%2d:%2d:%2d:%d",&h,&m,&s,&t) == 4)
	{
		*minutes = (h*60)+m;
		*ticks = s*50 + t*5;	/* no. of ticks */
	}
}

/******** timeStringtoDuration() ********/

void timeStringtoDuration(char *strPtr, int *hh, int *mm, int *ss, int *tt)
{
int h,m,s,t;

	if(sscanf(strPtr,"%2d:%2d:%2d:%d",&h,&m,&s,&t) == 4)
	{
		*hh = h;
		*mm = m;
		*ss = s;
		*tt = t;
	}
}

/******** timeStringtoTimeCode() ********/
/*
 * converts a string like hh:mm:ss:ff to ints
 *
 */

void timeStringtoTimeCode(char *strPtr, int *hh, int *mm, int *ss, int *ff)
{
int h,m,s,f;

	*hh=-1;
	*mm=-1;
	*ss=-1;
	*ff=-1;

	if(sscanf(strPtr,"%2d:%2d:%2d:%2d",&h,&m,&s,&f) == 4)
	{
		*hh=h;
		*mm=m;
		*ss=s;
		*ff=f;
	}
}

/******** DurationStringToSeconds() ********/

void DurationStringToSeconds(STRPTR str, ULONG *seconds)
{
ULONG h,m,s,t;

	h = (int)((str[0]-'0')*10 + (str[1]-'0'));
	h *= 36000;

	m = (int)((str[3]-'0')*10 + (str[4]-'0'));
	m *= 600;

	s = (int)((str[6]-'0')*10 + (str[7]-'0'));
	s *= 10;

	t = (int)(str[9]-'0');

	*seconds = h+m+s+t;
}

#ifndef USED_FOR_PLAYER

/******** secondsToDuration() ********/
/*
 * transforms e.g. 99*36000 + 59*600 + 59*10 + 9 to 99:59:59:9
 *
 */

void secondsToDuration(int seconds, STRPTR str)
{
int h,m,s,t;

	h = seconds/36000;
	seconds -= (h*36000);
	m = seconds/600;
	seconds -= (m*600);
	s = seconds/10;
	seconds -= (s*10);
	t = seconds;	
	sprintf(str, "%02d:%02d:%02d:%d", h,m,s,t);
}

/******** datestampToTime() ********/
/* - input:  minute, tick
 * - output: formatted string (e.g. 23:15:57:8)
 */

void datestampToTime(ULONG minute, ULONG tick, STRPTR str)
{
int h,m,s,t;

	h = (int)(minute/60L);
	m = (int)(minute%60L);
	s = (int)(tick/50L);
	tick = tick - (s*50);
	t = (int)(tick/5);
	sprintf(str, "%02d:%02d:%02d:%d", h,m,s,t);
}

/******** datestampToDate() ********/
/* - input:  days
 * - output: formatted string like dd-mmm-yy
 */

void datestampToDate(ULONG days, STRPTR str)
{
long n;
int m,d,y;

	n = days - 2251;
	y = (4 * n + 3) / 1461;
	n -= 1461 * y / 4;
	y += 1984;
	m = (5 * n + 2) / 153;
	d = n - (153 * m + 2) / 5 + 1;
	m += 3;
	if (m > 12)
	{
		y++;
		m -= 12;
	}
	sprintf(str, "%2d-%3s-%2d\0", d, dateconv_monthNames[m-1], y);
}

#endif

/******** SystemDate() ********/
/* - input: pointers to day, month, year 
 *
 */

void SystemDate(int *day, int *month, int *year)
{
struct DateStamp ds;
LONG n;
int m,d,y;

	DateStamp(&ds);
	n = ds.ds_Days - 2251;
	y = (4 * n + 3) / 1461;
	n -= 1461 * y / 4;
	y += 1984;
	m = (5 * n + 2) / 153;
	d = n - (153 * m + 2) / 5 + 1;
	m += 3;
	if (m > 12)
	{
		y++;
		m -= 12;
	}
	*month = m;
	*day = d;
	*year = y % 100;
}

#ifndef USED_FOR_PLAYER

/******** DayOfWeek() ********/
/* - input: maand (1..12) en jaar (00.99) alleen in 20ste eeuw! 
 * - result: 0..6 for zondag...zaterdag
 *
 * algorithm from Zeller's Congruence. Hello Mr Zeller.
 *
 * Calculates on which day the month starts!
 *
 */

int DayOfWeek(int month, int year)
{
float m, y, j;

	m = (float)month;
	y = (float)year;
	m = m - 2.0;
	if (m < 1.0)
	{
		m = m + 12.0;
		y = y - 1.0;
	}
	j = ((int)(2.6*m-0.19)) + 1.0 + y + ((int)(y/4.0)) + ((int)(19.0/4.0))
				- 2.0*19.0;
	j = j - ((int)(j/7.0)) * 7.0;
	return((int)j);
}

/******** MonthLength() ********/
/* - input: month (1..12) and year 
 * - result: days in month, takes leap years into account
 */

int MonthLength(int month, int year)
{
	month--;
	if (month == 1)	/* February */
	{
		/* leapyear? */
		if ((year % 4)==0)
			return(29);
		else
			return(28);
	}
	return( (int)(dateconv_Months[month]) );
}

/******** createStartEndDay() ********/
/*
 * startend == 0 for startDays and 1 for endDays
 *
 */

void createStartEndDay(LONG days, STRPTR str, int startend)
{
int day, month, year;

	if (days==-1)	/* no datestamp available */
		SystemDate(&day, &month, &year);
	else
		datestampToDMY(days, &day, &month, &year);
	if (days==-1 && startend==1)
		year++;
	DayMonthYearToString(str, day, month, year);
}

/******** createStartEndTime() ********/

void createStartEndTime(LONG minutes, LONG ticks, STRPTR str)
{
int hours, mins, secs, tenths;

	if (minutes==-1 || ticks==-1)	/* no datestamp available */
	{
		hours=0;
		mins=0;
		secs=0;
		tenths=0;
	}
	else
		datestampToHMST(minutes, ticks, &hours, &mins, &secs, &tenths);

	HoursMinsSecsTenthsToString(str, hours, mins, secs, tenths);
}

/******** datestampToDMY() ********/
/*
 * input: days datestamp
 * output: day, month (1..12), year (92)
 */

void datestampToDMY(ULONG days, int *day, int *month, int *year)
{
long n;
int m,d,y;

	n = days - 2251;
	y = (4 * n + 3) / 1461;
	n -= 1461 * y / 4;
	y += 1984;
	m = (5 * n + 2) / 153;
	d = n - (153 * m + 2) / 5 + 1;
	m += 3;
	if (m > 12)
	{
		y++;
		m -= 12;
	}
	*day = d;
	*month = m;
	*year = y % 100;
}

/******** datestampToHMST() ********/

void datestampToHMST(	ULONG minutes, ULONG ticks,
											int *hours, int *mins, int *secs, int *tenths)
{
	*hours = (int)(minutes/60L);
	*mins  = (int)(minutes%60L);
	*secs  = (int)(ticks/50L);
	ticks = ticks-(*secs*50);
	*tenths= (int)(ticks/5);
}

/******** DayMonthYearToString() ********/

void DayMonthYearToString(STRPTR str, int day, int month, int year)
{
	switch( dateFormat[CPrefs.countryCode] )
	{
		case 1: sprintf(str, "%02d/%02d/%02d", day, month, year); break;

		case 2: sprintf(str, "%02d-%02d-%02d", day, month, year); break;

		case 3: sprintf(str, "%02d.%02d.%02d", day, month, year); break;

		case 4: sprintf(str, "%02d %02d %02d", day, month, year); break;

		case 5: sprintf(str, "%02d.%02d %02d", day, month, year); break;

		case 6: sprintf(str, "%02d/%02d/%02d", month, day, year); break;

		case 7: sprintf(str, "%02d-%02d-%02d", year, month, day); break;

		case 8: sprintf(str, "%02d/%02d/%02d", year, month, day); break;

		case 9: sprintf(str, "%02d%02d%02d", year, month, day); break;

		default:sprintf(str, "%02d/%02d/%02d", month, day, year); break;
	}
}

/******** HoursMinsSecsTenthsToString() ********/

void HoursMinsSecsTenthsToString(STRPTR str, int hours, int mins, int secs, int tenths)
{
	switch( timeFormat[CPrefs.countryCode] )
	{
		case 1: SPrintf(str, "%02ld:%02ld:%02ld:%ld", hours, mins, secs, tenths); break;

		case 2: SPrintf(str, "%02ld.%02ld.%02ld.%ld", hours, mins, secs, tenths); break;

		default:SPrintf(str, "%02ld:%02ld:%02ld:%ld", hours, mins, secs, tenths); break;
	}
}

/******** HoursMinsSecsFramesToString() ********/

void HoursMinsSecsFramesToString(STRPTR str, int hours, int mins, int secs, int frames)
{
	SPrintf(str, "%02ld:%02ld:%02ld:%02ld", hours, mins, secs, frames);
}

/******** setStartEndCalender() ********/

void setStartEndCalender(struct Window *window, ULONG startDays, ULONG endDays)
{
int dayName, day, month, year;
struct CycleRecord *CR_ptr;

	/**** START ****/

	if (startDays==-1)	/* no datestamp available */
		SystemDate(&day, &month, &year);
	else
		datestampToDMY(startDays, &day, &month, &year);

	dayName = DayOfWeek(month, year);
	dayName = ((day-1)%7) + dayName;
	dayName = dayName % 7;

	UA_SetCycleGadgetToVal(	window, &Prog_GR[7], dayName);	/* sunday etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[8], day-1);	/* 1, 2, etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[9], month-1);	/* january etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[10], year % 90);	/* 1992 etc. */ 

	/**** set max month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[8].ptr;
	CR_ptr->number = MonthLength(month, year);

	/**** END ****/

	if (endDays==-1)	/* no datestamp available */
		SystemDate(&day, &month, &year);
	else
		datestampToDMY(endDays, &day, &month, &year);

	dayName = DayOfWeek(month, year);
	dayName = ((day-1)%7) + dayName;
	dayName = dayName % 7;

	UA_SetCycleGadgetToVal(	window, &Prog_GR[11], dayName);	/* sunday etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[12], day-1);	/* 1, 2, etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[13], month-1);	/* january etc. */ 
	UA_SetCycleGadgetToVal(	window, &Prog_GR[14], year % 90);	/* 1992 etc. */ 

	/**** set max month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[12].ptr;
	CR_ptr->number = MonthLength(month, year);
}												

/******** increaseDayName() ********/
/*
 * ID1 is DayName cycle and ID2 is dayNumber cycle gadget
 */

void increaseDayName(struct Window *window, int ID1, int ID2)
{
int day, month, year, monthLength;
struct CycleRecord *CR_ptr;
TEXT dateStr[16];

	/**** get current month and year ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+2].ptr;
	month = CR_ptr->active+1;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+3].ptr;
	year = CR_ptr->active+90;

	/**** get month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	monthLength = CR_ptr->number;

	if ( CR_ptr->active+1 == monthLength )	/* last day */
	{
		CR_ptr->active = 0;	/* start all over */

		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;

		CR_ptr->active = DayOfWeek(month, year);
	}
	else
		CR_ptr->active++;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+1].ptr;
	day = CR_ptr->active+1;

	DayMonthYearToString(dateStr, day, month, year);
	if (ID1==7)
		ID1=2;
	else
		ID1=3;
	UA_SetStringGadgetToString(window, &Prog_GR[ID1], dateStr);
}												

/******** decreaseDayName() ********/
/*
 * ID1 is DayName cycle and ID2 is dayNumber cycle gadget
 */

void decreaseDayName(struct Window *window, int ID1, int ID2)
{
int day, month, year, monthLength, dayName;
struct CycleRecord *CR_ptr;
TEXT dateStr[16];

	/**** get current month and year ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+2].ptr;
	month = CR_ptr->active+1;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+3].ptr;
	year = CR_ptr->active+90;

	/**** get month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	monthLength = CR_ptr->number;

	if ( CR_ptr->active == 0 )	/* first day */
	{
		CR_ptr->active = monthLength-1;	/* jump to last day */

		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;

		dayName = DayOfWeek(month, year);
		dayName = ((monthLength-1)%7) + dayName;
		dayName = dayName % 7;

		CR_ptr->active = dayName;
	}
	else
		CR_ptr->active--;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+1].ptr;
	day = CR_ptr->active+1;

	DayMonthYearToString(dateStr, day, month, year);
	if (ID1==7)
		ID1=2;
	else
		ID1=3;
	UA_SetStringGadgetToString(window, &Prog_GR[ID1], dateStr);
}												

/******** increaseDay() ********/
/*
 * ID1 is DayName cycle and ID2 is dayNumber cycle gadget
 */

void increaseDay(struct Window *window, int ID1, int ID2)
{
int day, month, year, monthLength;
struct CycleRecord *CR_ptr;
TEXT dateStr[16];

	/**** get current month and year ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+2].ptr;
	month = CR_ptr->active+1;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+3].ptr;
	year = CR_ptr->active+90;

	/**** get month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	monthLength = CR_ptr->number;

	if ( CR_ptr->active == 0 )	/* past last day */
	{
		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;
		CR_ptr->active = DayOfWeek(month, year);
	}
	else
	{
		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;
		CR_ptr->active++;
		CR_ptr->active = CR_ptr->active % 7;
	}

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	day = CR_ptr->active+1;

	DayMonthYearToString(dateStr, day, month, year);
	if (ID1==7)
		ID1=2;
	else
		ID1=3;
	UA_SetStringGadgetToString(window, &Prog_GR[ID1], dateStr);
}												

/******** decreaseDay() ********/
/*
 * ID1 is DayName cycle and ID2 is dayNumber cycle gadget
 */

void decreaseDay(struct Window *window, int ID1, int ID2)
{
int day, month, year, monthLength, dayName;
struct CycleRecord *CR_ptr;
TEXT dateStr[16];

	/**** get current month and year ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+2].ptr;
	month = CR_ptr->active+1;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+3].ptr;
	year = CR_ptr->active+90;

	/**** get month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	monthLength = CR_ptr->number;

	if ( CR_ptr->active == monthLength-1 )	/* past first day */
	{
		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;

		dayName = DayOfWeek(month, year);
		dayName = ((monthLength-1)%7) + dayName;
		dayName = dayName % 7;

		CR_ptr->active = dayName;
	}
	else
	{
		CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;
		CR_ptr->active--;
		if (CR_ptr->active < 0)
			CR_ptr->active += 7;
		else
			CR_ptr->active = CR_ptr->active % 7;
	}

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	day = CR_ptr->active+1;

	DayMonthYearToString(dateStr, day, month, year);
	if (ID1==7)
		ID1=2;
	else
		ID1=3;
	UA_SetStringGadgetToString(window, &Prog_GR[ID1], dateStr);
}												

/******** changeMonth() ********/
/*
 * ID1 is DayName cycle and ID2 is dayNumber cycle gadget
 */

void changeMonth(struct Window *window, int ID1, int ID2)
{
int day, month, year, monthLength, dayName;
struct CycleRecord *CR_ptr;
TEXT dateStr[16];

	/**** get current month and year ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+2].ptr;
	month = CR_ptr->active+1;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1+3].ptr;
	year = CR_ptr->active+90;

	/**** get month length ****/

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	monthLength = MonthLength(month, year);
	CR_ptr->number = monthLength;

	if ( CR_ptr->active+1 > monthLength )
		CR_ptr->active = monthLength - 1;

	dayName = DayOfWeek(month, year);
	dayName = (CR_ptr->active%7) + dayName;
	dayName = dayName % 7;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID1].ptr;
	CR_ptr->active = dayName;

	CR_ptr = (struct CycleRecord *)Prog_GR[ID2].ptr;
	day = CR_ptr->active+1;

	DayMonthYearToString(dateStr, day, month, year);
	if (ID1==7)
		ID1=2;
	else
		ID1=3;
	UA_SetStringGadgetToString(window, &Prog_GR[ID1], dateStr);
}												

/******** CheckEnteredDate() ********/

void CheckEnteredDate(struct Window *window, struct GadgetRecord *GR, int ID)
{
struct StringRecord *SR_ptr;
int day, month, year, i, monthLength, dayName;
TEXT tmp[32];

	day = 0;
	month = 0;
	year = 0;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr == NULL)
		return;

	if (strlen(SR_ptr->buffer) <= 4)
	{
		DisplayBeep(scriptScreen);
		return;
	}

	if ( dateFormat[CPrefs.countryCode]==9 )
	{
		if (strlen(SR_ptr->buffer) != 6)
		{
			DisplayBeep(scriptScreen);
			return;
		}
		tmp[0] = SR_ptr->buffer[0];
		tmp[1] = SR_ptr->buffer[1];
		tmp[2] = ' ';
		tmp[3] = SR_ptr->buffer[2];
		tmp[4] = SR_ptr->buffer[3];
		tmp[5] = ' ';
		tmp[6] = SR_ptr->buffer[4];
		tmp[7] = SR_ptr->buffer[5];
	}
	else
	{
		stccpy(tmp, SR_ptr->buffer, 10);

		/**** remove - . etc ****/	
		for(i=0; i<strlen(tmp); i++)
			if ( isdigit(tmp[i]) == 0 )
				tmp[i]=' ';
	}

	switch( dateFormat[CPrefs.countryCode] )
	{
		case 1: sscanf(tmp, "%d %d %d", &day, &month, &year); break;
		case 2: sscanf(tmp, "%d %d %d", &day, &month, &year); break;
		case 3: sscanf(tmp, "%d %d %d", &day, &month, &year); break;
		case 4: sscanf(tmp, "%d %d %d", &day, &month, &year); break;
		case 5: sscanf(tmp, "%d %d %d", &day, &month, &year); break;
		case 6: sscanf(tmp, "%d %d %d", &month, &day, &year); break;
		case 7: sscanf(tmp, "%d %d %d", &year, &month, &day); break;
		case 8: sscanf(tmp, "%d %d %d", &year, &month, &day); break;
		case 9: sscanf(tmp, "%d %d %d", &year, &month, &day); break;
		default:sscanf(tmp, "%d %d %d", &month, &day, &year); break;
	}

	if (year<90)
		year = 90;
	else if (year>98)
		year=98;

	if (month<1)
		month = 1;
	else if (month>12)
		month=12;

	monthLength = MonthLength(month, year);

	if (day<1)
		day = 1;
	else if (day>monthLength)
		day = monthLength;

	DayMonthYearToString(SR_ptr->buffer, day, month, year);

	/**** set cycle gadgets accordingly ****/

	if ( ID != -1 )
	{
		dayName = DayOfWeek(month, year);
		dayName = ((day-1)%7) + dayName;
		dayName = dayName % 7;

		UA_SetCycleGadgetToVal(	window, &Prog_GR[ID], dayName);	/* sunday etc. */ 
		UA_SetCycleGadgetToVal(	window, &Prog_GR[ID+1], day-1);	/* 1, 2, etc. */ 
		UA_SetCycleGadgetToVal(	window, &Prog_GR[ID+2], month-1);	/* january etc. */ 
		UA_SetCycleGadgetToVal(	window, &Prog_GR[ID+3], year % 90);	/* 1992 etc. */ 
	}
}

/******** CheckEnteredTime() ********/

void CheckEnteredTime(struct GadgetRecord *GR)
{
struct StringRecord *SR_ptr;
int hours, mins, secs, tenths, i;
TEXT tmp[32];

	hours = 0;
	mins = 0;
	secs = 0;
	tenths = 0;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr == NULL)
		return;

	stccpy(tmp, SR_ptr->buffer, 30);

	/**** remove - . etc ****/	
	for(i=0; i<strlen(tmp); i++)
		if ( isdigit(tmp[i]) == 0 )
			tmp[i]=' ';

	sscanf(tmp, "%d %d %d %d", &hours, &mins, &secs, &tenths);

	CheckHMST(&hours, &mins, &secs, &tenths);

	HoursMinsSecsTenthsToString(SR_ptr->buffer, hours, mins, secs, tenths);
}

/******** SystemTime() ********/
/* - input: pointers to hours, mins, secs 
 *
 */

void SystemTime(int *hours, int *mins, int *secs)
{
struct DateStamp ds;

	DateStamp(&ds);
	*hours = (int)(ds.ds_Minute/60L);
	*mins = (int)(ds.ds_Minute%60L);
	*secs = (int)(ds.ds_Tick/50L);
}

/******** CheckEnteredTimeCode() ********/

void CheckEnteredTimeCode(int rate, struct GadgetRecord *GR)
{
struct StringRecord *SR_ptr;
int hours, mins, secs, frames, i;
TEXT tmp[16];

	hours		= 0;
	mins		= 0;
	secs		= 0;
	frames	= 0;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr == NULL)
		return;

	stccpy(tmp, SR_ptr->buffer, 16);

	/**** remove - . etc ****/	

	for(i=0; i<strlen(tmp); i++)
		if ( isdigit(tmp[i]) == 0 )
			tmp[i]=' ';

	sscanf(tmp, "%d %d %d %d", &hours, &mins, &secs, &frames);

	CheckHMSF(&hours, &mins, &secs, &frames, rate);

	sprintf(SR_ptr->buffer, "%02d:%02d:%02d:%02d", hours, mins, secs, frames);
}

/******** GetDayNumber() ********/

int GetDayNumber(void)
{
int dayNum,d,m,y;
	SystemDate(&d, &m, &y);
	dayNum = DayOfWeek(m, y);
	dayNum = ((d-1)%7) + dayNum;
	dayNum = dayNum % 7;
	return(dayNum);
}

/******** CheckHMST() ********/

void CheckHMST(int *h, int *m, int *s, int *t)
{
	if ( *h < 0 )
		*h = 0;
	if ( *h > 23 )
		*h = 23;

	if ( *m < 0 )
		*m = 0;
	if ( *m > 59 )
		*m = 59;

	if ( *s < 0 )
		*s = 0;
	if ( *s > 59 )
		*s = 59;

	if ( *t < 0 )
		*t = 0;
	if ( *t > 9 )
		*t = 9;
}

/******** CheckHMSF() ********/

void CheckHMSF(int *h, int *m, int *s, int *f, int rate)
{
int maxFF;

	if ( rate==TIMERATE_24FPS )
		maxFF=23;
	else if ( rate==TIMERATE_25FPS )
		maxFF=24;
	else if ( rate==TIMERATE_30FPS_DF )
		maxFF=29;
	else if ( rate==TIMERATE_30FPS )
		maxFF=29;
	else if ( rate==TIMERATE_MIDICLOCK )
		maxFF=29;

	if ( *h < 0 )
		*h = 0;
	if ( *h > 99 )
		*h = 99;

	if ( *m < 0 )
		*m = 0;
	if ( *m > 59 )
		*m = 59;

	if ( *s < 0 )
		*s = 0;
	if ( *s > 59 )
		*s = 59;

	if ( *f < 0 )
		*f = 0;
	if ( *f > maxFF )
		*f = maxFF;
}

/******** CheckHMST_Cycle() ********/

void CheckHMST_Cycle(int *h, int *m, int *s, int *t)
{
	if ( *h < 0 )
		*h = 23;
	if ( *h > 23 )
		*h = 0;

	if ( *m < 0 )
		*m = 59;
	if ( *m > 59 )
		*m = 0;

	if ( *s < 0 )
		*s = 59;
	if ( *s > 59 )
		*s = 0;

	if ( *t < 0 )
		*t = 9;
	if ( *t > 9 )
		*t = 0;
}

/******** CheckHMSF_Cycle() ********/

void CheckHMSF_Cycle(int *h, int *m, int *s, int *f, int rate)
{
int maxFF;

	if ( rate==TIMERATE_24FPS )
		maxFF=23;
	else if ( rate==TIMERATE_25FPS )
		maxFF=24;
	else if ( rate==TIMERATE_30FPS_DF )
		maxFF=29;
	else if ( rate==TIMERATE_30FPS )
		maxFF=29;
	else if ( rate==TIMERATE_MIDICLOCK )
		maxFF=29;

	if ( *h < 0 )
		*h = 99;
	if ( *h > 99 )
		*h = 0;

	if ( *m < 0 )
		*m = 59;
	if ( *m > 59 )
		*m = 0;

	if ( *s < 0 )
		*s = 59;
	if ( *s > 59 )
		*s = 0;

	if ( *f < 0 )
		*f = maxFF;
	if ( *f > maxFF )
		*f = 0;
}

#endif

/******** PutDateIntoIt() ********/

void PutDateIntoIt(struct ScriptNodeRecord *this_node)
{
int a,b,c,val1;
TEXT str[20];

	if (!CPrefs.showDays)
		return;

	if ( this_node->Start.HHMMSS.Days!=-1 && this_node->End.HHMMSS.Days!=-1 )
		return;

#if 0
	SystemTime(&a,&b,&c);																							// hh mm ss

	sprintf(str, "%2d:%2d:%2d:0", a, b, c);														// hh:mm:ss:t
	timeStringtoMinutesAndTicks(str, &val1, &val2);

	this_node->Start.HHMMSS.Minutes = (LONG)val1;
	this_node->Start.HHMMSS.Ticks = (LONG)val2;

	this_node->End.HHMMSS.Minutes = (LONG)val1;
	this_node->End.HHMMSS.Ticks = (LONG)val2;
#endif

	SystemDate(&a,&b,&c);																							// dd mm yy

	sprintf(str, "%d-%s-19%d", a, dateconv_monthNames[b-1], (int)c);		// dd-mmm-yyyy
	dateStringtoDays(str, &val1);
	this_node->Start.HHMMSS.Days = (LONG)val1;

	sprintf(str, "%d-%s-19%d", a, dateconv_monthNames[b-1], (int)c+1);	// dd-mmm-yyyy
	dateStringtoDays(str, &val1);
	this_node->End.HHMMSS.Days = (LONG)val1;
}

/******** E O F ********/
