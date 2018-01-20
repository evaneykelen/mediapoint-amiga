#include <exec/exec.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <libraries/locale.h>
#include <clib/locale_protos.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <time.h>
#include "nb:parser.h"

#include "localedate.h"

STATIC struct Library *OpenLocaleRecord(struct LocaleRecord *LR);
STATIC void CloseLocaleRecord(struct LocaleBase *LocaleBase, struct LocaleRecord *LR);
STATIC void GetDateString(struct LocaleBase *LocaleBase, struct LocaleRecord *LR,
													STRPTR str, BYTE format);
STATIC ULONG __saveds __asm HookFunc(	register __a0 struct Hook *hook,
																			register __a2 APTR				localePtr,
																			register __a1 char				ch);

/**** globals ****/

struct LocaleRecord
{
	struct Locale *Locale;
};

/**** functions *****/

#if 0
/******** main() ********/

void main(void)
{
int i;
TEXT str[200];

	for(i=1; i<=15; i++)
	{
		CreateLocaleTime(str, i);
		printf("%s\n",str);
	}
}
#endif

/******** CreateLocaleTime() ********/

void CreateLocaleTime(STRPTR str, int format)
{
struct LocaleRecord LR;
struct LocaleBase *LocaleBase;

	LocaleBase = (struct LocaleBase *)OpenLocaleRecord(&LR);
	GetDateString(LocaleBase, &LR, str, (BYTE)format);
	CloseLocaleRecord(LocaleBase, &LR);
}

/******** OpenLocaleRecord() ********/

STATIC struct Library *OpenLocaleRecord(struct LocaleRecord *LR)
{
struct LocaleBase *LocaleBase;

	LR->Locale = NULL;
	LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 0);
	if ( LocaleBase )
	{
		LR->Locale = (struct Locale *)OpenLocale(NULL);
		if ( LR->Locale )
			return((struct Library *)LocaleBase);
	}
	return(NULL);
}

/******** CloseLocaleRecord() ********/

STATIC void CloseLocaleRecord(struct LocaleBase *LocaleBase, struct LocaleRecord *LR)
{
	if ( LR->Locale )
		CloseLocale( LR->Locale );
	if ( LocaleBase )
		CloseLibrary( (struct Library *)LocaleBase );
}

/******** GetDateString() ********/

STATIC void GetDateString(struct LocaleBase *LocaleBase, struct LocaleRecord *LR,
													STRPTR str, BYTE format)
{
struct DateStamp ds;
struct Hook callback;
struct tm s;
time_t t;
UBYTE *formatStr;
struct Library *DOSBase;

	DOSBase = OpenLibrary("dos.library", 0 );

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
	}

	if ( LocaleBase )
		FormatDate(LR->Locale, formatStr, &ds, &callback);
	else
		strftime(str, 200, formatStr, &s );

	if( DOSBase )
		CloseLibrary( DOSBase );
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


/******** FindVarContents() ********/
/*
 * varName is e.g. '@my_var1 quick brown ... rest of string'
 * My assignment is to fill 'answer' with the current value of 'my_var1'.
 * If this var can't be found this function returns 0, indicating that
 * no characters were skipped in the process of recognizing the variable
 * name. If the var was recognized, it will return (in this case) 8.
 * Printing the rest of the string will then start with a space.
 *
 */

int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer)
{
VIR *this_vir;
TEXT local_varName[20];
int i=0;

	answer[0] = '\0';
	local_varName[0] = '\0';

	for(i=0; i<19; i++)
	{
		if ( varName[i+1] != ')' )	// +1 skips @
			local_varName[i] = varName[i+1];
		else
		{
			local_varName[i] = ')';
			break;
		}
	}

	if ( local_varName[i] != ')' )
		return(0);

	local_varName[i+1] = '\0';
	if ( local_varName[0] == '\0' )
		return(0);

	i = strlen(local_varName);

	if ( i<3 || local_varName[0] != '(' || local_varName[ i-1 ] != ')' )
		return(0);

#if 0
KPrintF("local var before shave [%s]\n", local_varName);
{
char strstr[10];
sprintf(strstr,"%d\n",i);
KPrintF(strstr);
}
#endif

	stccpy(local_varName, &local_varName[1], i-1);	// str=(str) -> copy from 'str'

//KPrintF("Looking for var [%s]\n", local_varName);

	for(this_vir = (VIR *)VIList->lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
//KPrintF("List var [%s]\n", this_vir->vir_Name);
		if ( !stricmp(local_varName, this_vir->vir_Name) )
		{
			if ( this_vir->vir_Type == VCT_STRING )
				stccpy(answer, this_vir->vir_String, 49);
			else
				sprintf(answer,"%d",this_vir->vir_Integer);
		}
	}

	if ( !answer[0] )
	{
		/* return(0); */
		// NEW PER Tuesday 12-Jul-94 16:54:48
		strcpy(answer," ");
	}

	return( (int)(strlen( local_varName ) + 3) );		// add @ + ( and )
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
	             %I - hour using 24-hour style with leading 0s
	             %H - hour using 12-hour style with leading 0s
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
