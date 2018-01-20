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
extern int dateFormat[];						// times.c
extern struct RastPort gfxRP;
extern struct TextFont *smallFont;
extern struct Locale *Locale;
extern struct Library *LocaleBase;
extern TEXT *dir_xapps;
extern TEXT *dir_system;
extern TEXT *dir_scripts;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;

/**** gadgets ****/

extern struct GadgetRecord Prog_GR[];

/**** functions ****/

/******** DoReleaseTwoFirstPart() ********/

void DoReleaseTwoFirstPart(void)
{
	if (GfxBase->LibNode.lib_Version == 36)
		UA_WarnUser(56);

	/**** find out if kickstart 2.0, ECS, AA etc. are present ****/

	if (GfxBase->LibNode.lib_Version >= 37)
		CPrefs.SystemTwo = TRUE;
	else
		CPrefs.SystemTwo = FALSE;

	if (	CPrefs.SystemTwo &&
				((GfxBase->ChipRevBits0 & SETCHIPREV_ECS)==SETCHIPREV_ECS) )
		CPrefs.ECS_available = TRUE;	
	else
		CPrefs.ECS_available = FALSE;	

	if (	CPrefs.SystemTwo &&
				((GfxBase->ChipRevBits0 & SETCHIPREV_AA)==SETCHIPREV_AA) )
		CPrefs.AA_available = TRUE;	
	else
		CPrefs.AA_available = FALSE;
}

/******** DoReleaseTwo() ********/

BOOL DoReleaseTwo(void)
{
TEXT path[SIZE_PATH];

#if 0
	/**** determine whether we're running on a PAL or NTSC machine ****/

	if (CheckPAL("Workbench"))
		CPrefs.PalNtsc = PAL_MODE;
	else
		CPrefs.PalNtsc = NTSC_MODE;
#endif

	/**** Under 2.0, get start-up dir ****/

	if ( CPrefs.SystemTwo )
		CPrefs.appdirLock = (struct FileLock *)GetProgramDir();
	else
		CPrefs.appdirLock = (struct FileLock *)Lock("", (LONG)ACCESS_READ);
	if (CPrefs.appdirLock==NULL)
	{
		UA_WarnUser(57);
		return(FALSE);
	}

	/**** create e.g. "work:medialink/xapps/" ****/

	findFullPath(CPrefs.appdirLock, path);
	UA_MakeFullPath(path, "xapps", dir_xapps);
	UA_ValidatePath(dir_xapps);

	/**** create e.g. "work:medialink/system/" ****/

	UA_MakeFullPath(path, "system", dir_system);
	UA_ValidatePath(dir_system);

	/**** create e.g. "work:medialink/scripts/" ****/

	UA_MakeFullPath(path, "scripts", dir_scripts);
	UA_ValidatePath(dir_scripts);

	/*** try to open locale ****/

	CPrefs.locale = FALSE;
	if ( CPrefs.SystemTwo )
	{
		LocaleBase = (struct Library *)OpenLibrary("locale.library", 0);
		if (LocaleBase != NULL)
		{
			Locale = (struct Locale *)OpenLocale(NULL);
			if (Locale == NULL)
				UA_WarnUser(58);
			else
				CPrefs.locale = TRUE;
		}
	}

	return(TRUE);
}

/******** DoReleaseTwoSecondPart() ********/

void DoReleaseTwoSecondPart(void)
{
	FillLocaleStrings();	/* creates defaults on 1.3 machines */
}

/******** FillLocaleStrings() ********/
/*
 * gfxRP, gfxBM and smallFont MUST be initialized and opened!
 *
 */

void FillLocaleStrings(void)
{
TEXT str[256];
char *ptr, *MonthList, *DaysList;

	ptr = &str[0];
	MonthList = msgs[Msg_MonthNames-1];
	DaysList = msgs[Msg_LongDayNames-1];

	if ( CPrefs.locale )
	{
		/**** copy locale month names ****/

		ptr = GetLocaleStr(Locale, MON_1);
		stccpy( &MonthList[0], ptr, 15);
		
		ptr = GetLocaleStr(Locale, MON_2);
		stccpy( &MonthList[15], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_3);
		stccpy( &MonthList[30], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_4);
		stccpy( &MonthList[45], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_5);
		stccpy( &MonthList[60], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_6);
		stccpy( &MonthList[75], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_7);
		stccpy( &MonthList[90], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_8);
		stccpy( &MonthList[105], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_9);
		stccpy( &MonthList[120], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_10);
		stccpy( &MonthList[135], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_11);
		stccpy( &MonthList[150], ptr, 15);

		ptr = GetLocaleStr(Locale, MON_12);
		stccpy( &MonthList[165], ptr, 15);

		/**** copy locale day names ****/

		ptr = GetLocaleStr(Locale, DAY_1);
		stccpy( &DaysList[0], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_2);
		stccpy( &DaysList[15], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_3);
		stccpy( &DaysList[30], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_4);
		stccpy( &DaysList[45], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_5);
		stccpy( &DaysList[60], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_6);
		stccpy( &DaysList[75], ptr, 15);

		ptr = GetLocaleStr(Locale, DAY_7);
		stccpy( &DaysList[90], ptr, 15);

		/**** copy locale abbreviated day names ****/

		ptr = GetLocaleStr(Locale, ABDAY_1);
		stccpy(Prog_GR[15].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_2);
		stccpy(Prog_GR[16].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_3);
		stccpy(Prog_GR[17].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_4);
		stccpy(Prog_GR[18].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_5);
		stccpy(Prog_GR[19].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_6);
		stccpy(Prog_GR[20].txt, ptr, 3);

		ptr = GetLocaleStr(Locale, ABDAY_7);
		stccpy(Prog_GR[21].txt, ptr, 3);

		/**** see where where we're running ****/

		switch( Locale->loc_TelephoneCode )
		{
			case 61: CPrefs.countryCode = AUSTRALIA; break;

			case 32: CPrefs.countryCode = BELGIE; break;

			case  1: CPrefs.countryCode = USA; break;

			case 45: CPrefs.countryCode = DANMARK; break;

			case 49: CPrefs.countryCode = DEUTSCHLAND; break;

			case 33: CPrefs.countryCode = FRANCE; break;

			case 44: CPrefs.countryCode = GREAT_BRITTAIN; break;

			case 39: CPrefs.countryCode = ITALIA; break;

			case 31: CPrefs.countryCode = NEDERLAND; break;

			case 47: CPrefs.countryCode = NORGE; break;

			case 43: CPrefs.countryCode = OSTERREICH; break;

			case 351: CPrefs.countryCode = PORTUGAL; break;

			case 41: CPrefs.countryCode = SCHWEIZ; break;

			case 46: CPrefs.countryCode = SVERIGE; break;

			default: CPrefs.countryCode = USA; break;
		}
	}
	else	// no locale available
		CPrefs.countryCode = USA;

	WriteSmallLocaleStrings();
}

/******** WriteSmallLocaleStrings() ********/

void WriteSmallLocaleStrings(void)
{
int i,x;

	/**** write locale strings into gfx bitmap ****/

	SetFont(&gfxRP, smallFont);
	SetDrMd(&gfxRP, JAM2);
	SetAPen(&gfxRP, LO_PEN);
	SetBPen(&gfxRP, AREA_PEN);

	for(i=0; i<7; i++)
	{
		x = GetCenterX( &gfxRP, Prog_GR[15+i].txt, 1, 13);
		Move(&gfxRP, GFX_PROG_X+i*16+x, GFX_PROG_Y+7);
		Text(&gfxRP, Prog_GR[15+i].txt, 1L);
	}
}

/******** GetCenterX() ********/

int GetCenterX(struct RastPort *rp, STRPTR str, int numChars, int numPixels)
{
	return( (numPixels-TextLength(rp, str, numChars)) / 2 );
}

/******** E O F ********/
