#define SCRIPT_LINESIZE 2048L

enum
{
/* ScriptTalk */
SM_ANIM, SM_AREXX, SM_DOS, SM_SOUND, SM_PAGE, SM_BINARY, SM_MAIL, SM_XAPP,
/* PageTalk */
SM_CLIP, SM_CRAWL, SM_TEXT, SM_CLIPANIM,
/* Special Magic */
 SM_FONT, SM_SYSTEM, SM_IFF, SM_SCRIPT
};

enum
{
MODE_CREATE_RUNTIME, MODE_FIND_MISSING, MODE_CALC_SIZE, MODE_UPLOAD,
};

#define MSM_ANIM_PATH		"Animations"
#define MSM_PAGE_PATH		"Pages"
#define MSM_IFF_PATH		"Pictures"
#define MSM_DOS_PATH		"DosScripts"
#define MSM_AREXX_PATH	"ArexxScripts"
#define MSM_SOUND_PATH	"Sounds"
#define MSM_XAPP_PATH		"XappData"
#define MSM_CLIP_PATH		"Pages/Clips"
#define MSM_CRAWL_PATH 	"CrawlData"
//#define MSM_TEXT_PATH		"TextData"
#define MSM_FONT_PATH		"Fonts"
#define MSM_SYSTEM_PATH	"System"
#define MSM_XAPPS_PATH	"Xapps"

#define SYSTEMFILES_YES	TRUE
#define SYSTEMFILES_NO	FALSE

#define TEMPSCRIPT			"RAM:MP_TempScript"
#define BIGFILE					"RAM:MP_BigFile"

/******** E O F ********/
