#include "nb:pre.h"
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <pragmas/locale_pragmas.h>
#include <time.h>

/**** externals ****/

extern struct Locale *Locale;
extern struct Library *LocaleBase;

/**** functions ****/

STATIC ULONG __saveds __asm HookFunc(	register __a0 struct Hook *hook,
																			register __a2 APTR				localePtr,
																			register __a1 char				ch);

/**** functions *****/

/******** GetDateString() ********/
/*
 * str must be >= 200 bytes!
 *
 */

void GetDateString(STRPTR str, BYTE format)
{
struct DateStamp ds;
struct Hook callback;
struct tm s;
time_t t;
UBYTE *formatStr;

	callback.h_Entry	= (void *)HookFunc;
	callback.h_Data		= str;

	if ( LocaleBase )
		DateStamp( &ds );
	else
	{
		time( &t );
		s = *localtime( &t );
	}

	str[0] = '\0';

	switch(format)
	{
		case LD_DATE_1:
			if ( LocaleBase )
				formatStr = "%A, %B %e, %Y";
			else
				formatStr = "%A, %B %d, %Y";
			break;
		case LD_DATE_2:
			if ( LocaleBase )
				formatStr = "%a, %b %e, %Y";
			else
				formatStr = "%a, %b %d, %Y";
			break;
		case LD_DATE_3:
			if ( LocaleBase )
				formatStr = "%B %e, %Y";
			else
				formatStr = "%B %d, %Y";
			break;
		case LD_DATE_4:
			if ( LocaleBase )
				formatStr = "%b %e, %Y";
			else
				formatStr = "%b %d, %Y";
			break;
		case LD_TIME_1:
			formatStr = "%I:%M";
			break;
		case LD_TIME_2:
			formatStr = "%Q:%M %p";
			break;
		case LD_TIME_3:
			formatStr = "%H:%M";
			break;
		case LD_SECS:
			formatStr = "%S";
			break;
		case LD_DATE:
			if ( LocaleBase )
				formatStr = "%e";
			else
				formatStr = "%d";
			break;
		case LD_LONG_DAY:
			formatStr = "%A";
			break;
		case LD_SHORT_DAY:
			formatStr = "%a";
			break;
		case LD_LONG_MONTH:
			formatStr = "%B";
			break;
		case LD_SHORT_MONTH:
			formatStr = "%b";
			break;
		case LD_LONG_YEAR:
			formatStr = "%Y";
			break;
		case LD_SHORT_YEAR:
			formatStr = "'%y";
			break;
		default:
			formatStr = "\0";
	}

	if ( formatStr[0] != '\0' )
	{
		if ( LocaleBase )
			FormatDate(Locale,formatStr,&ds,&callback);
		else
			strftime(str, 200, formatStr, &s );
	}
}

/******** HookFunc() ********/

STATIC ULONG __saveds __asm HookFunc(	register __a0 struct Hook *hook,
																			register __a2 APTR				localePtr,
																			register __a1 char				ch)
{
char str[2];

	str[0]=ch;
	str[1]='\0';
	strcat(hook->h_Data,str);

	return(0);
}

/******** ConvertDatePageToPlayer() ********/

void ConvertDatePageToPlayer(STRPTR str, int format)
{
	switch(format)
	{
		case LD_DATE_1:				strcpy(str, "@DATE1"); break;
		case LD_DATE_2:				strcpy(str, "@DATE2"); break;
		case LD_DATE_3:				strcpy(str, "@DATE3"); break; 			
		case LD_DATE_4:				strcpy(str, "@DATE4"); break;
		case LD_TIME_1:				strcpy(str, "@TIME1"); break;
		case LD_TIME_2:				strcpy(str, "@TIME2"); break;
		case LD_TIME_3:				strcpy(str, "@TIME3"); break;
		case LD_SECS:					strcpy(str, "@SECS"); break;
		case LD_DATE:					strcpy(str, "@DYNR"); break;
		case LD_LONG_DAY:			strcpy(str, "@LD"); break;
		case LD_SHORT_DAY:		strcpy(str, "@SD"); break;
		case LD_LONG_MONTH:		strcpy(str, "@LM"); break;
		case LD_SHORT_MONTH:	strcpy(str, "@SM"); break;
		case LD_LONG_YEAR:		strcpy(str, "@LY"); break;
		case LD_SHORT_YEAR:		strcpy(str, "@SY"); break;
		case LD_FILE:					strcpy(str, "@FILE="); break;
	}
}

#if 0
	             %a - abbreviated weekday name
	             %A - weekday name
	             %b - abbreviated month name
	             %B - month name
	             %c - same as "%a %b %d %H:%M:%S %Y"
	             %C - same as "%a %b %e %T %Z %Y"
	             %d - day number with leading 0s
	             %D - same as "%m/%d/%y"
	             %e - day number with leading spaces
	             %h - abbreviated month name
	             %H - hour using 24-hour style with leading 0s
	             %I - hour using 12-hour style with leading 0s
	             %j - julian date
	             %m - month number with leading 0s
	             %M - the number of minutes with leading 0s
	             %n - insert a linefeed
	             %p - AM or PM strings
	             %q - hour using 24-hour style
	             %Q - hour using 12-hour style
	             %r - same as "%I:%M:%S %p"
	             %R - same as "%H:%M"
	             %S - number of seconds with leadings 0s
	             %t - insert a tab character
	             %T - same as "%H:%M:%S"
	             %U - week number, taking Sunday as first day of week
	             %w - weekday number
	             %W - week number, taking Monday as first day of week
	             %x - same as "%m/%d/%y"
	             %X - same as "%H:%M:%S"
	             %y - year using two digits with leading 0s
	             %Y - year using four digits with leading 0s
#endif

/******** E O F ********/
