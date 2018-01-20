/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

/**** defines ****/

#define MAXARGSTRLEN 512	// see also globalallocs.c

/**** externals ****/

extern ULONG allocFlags;
extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct Window *thumbWindow;
extern UBYTE **msgs;
extern ULONG resourceFlags;

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *DiskfontBase;
extern struct Library *LocaleBase;
extern struct Library *medialinkLibBase;
extern struct Library *MLMMULibBase;
extern struct LayersBase *LayersBase;

/**** static globals ****/

static va_list NewArgPtr;
static va_list OldArgPtr;
static char *Sptr, *Tptr;
static char *t = NULL;
static unsigned tSize;
static char *SavePtr;
TEXT tStr[512];

static struct EasyStruct myES =
{
	sizeof(struct EasyStruct),	// es_StructSize
	0,													// es_Flags
	"Fatal Application Error",	// *es_Title
	"%s",												// *es_TextFormat
	"OK"												// *es_GadgetFormat
};	

extern int KPrintF(const char *, ...);

/**** functions ****/

/******** PrintSer() ********/

void PrintSer(char *fmt, ...)
{
va_list Arg;
TEXT str[MAXARGSTRLEN];
int i;
	for(i=0; i<MAXARGSTRLEN; i++)
		str[i] = '\0';
	va_start(Arg, fmt);
	Formatter(str, fmt, NewArgPtr=Arg);
	free(t);
	KPrintF(str);
	//printf(str);
}

/******** Early_WarnUser() ********/

void Early_WarnUser(STRPTR str)
{
	myES.es_TextFormat = (UBYTE *)str;
	if (GfxBase->LibNode.lib_Version >= 36)
		EasyRequest(NULL,&myES,NULL);
}

/******** Message() ********/

void Message(char *fmt, ...)
{
va_list Arg;
TEXT str[MAXARGSTRLEN];
int i;
struct Window *wdw=NULL;

	for(i=0; i<MAXARGSTRLEN; i++)
		str[i] = '\0';

	va_start(Arg, fmt);
	Formatter(str, fmt, NewArgPtr=Arg);
	free(t);

	if ( EHI.thumbsVisible )
		wdw = thumbWindow;
	else if ( EHI.activeScreen == STARTSCREEN_PAGE )
		wdw = pageWindow;
	else if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
		wdw = scriptWindow;
	else
	{
		Forbid();
		wdw = IntuitionBase->ActiveWindow;
		Permit();
	}

	if (wdw != NULL)
		UA_OpenGenericWindow(	wdw, TRUE, FALSE, msgs[Msg_OK-1], NULL,
													EXCLAMATION_ICON, str, TRUE, NULL);
}

/******** Formatter() ********/

void Formatter(char *str, char *s, va_list Arg)
{
char *p, *q;

	if ( !str || !s )
		return;

	p = q = str;
	Sptr = s;
	if ((t=(char *)malloc(tSize=strlen(s)))==NULL)
		return;	// Formatter error
	Tptr=t;
	OldArgPtr=Arg;
	while (*Sptr != '\0')
	{
		if ((*Tptr++ = *Sptr++) == '%')
		{
			SavePtr = Tptr-1;
			q=ParseType(q);
		}
		else
		{
			sprintf(q, "%c", *(Sptr-1));
			q++;
		}
	}
	va_end(OldArgPtr);
	*Tptr='\0';
	stccpy(str, p, MAXARGSTRLEN);
}

/******** ParseType() ********/

char *ParseType(char *q)
{
int val;

	switch(*Tptr++ = *Sptr++)
	{
		case 'd': val = (int)va_arg(OldArgPtr,char *);
							sprintf(tStr, "%d", val);
							strcpy(q, tStr);	// survived strcpy
							q+=strlen(tStr);
							break;

		case 's':	strcpy(tStr, (char *)va_arg(OldArgPtr,char *) );	// survived strcpy
							strcpy(q, tStr);	// survived strcpy
							q+=strlen(tStr);
							break;

		case 'x': val = (int)va_arg(OldArgPtr,char *);
							sprintf(tStr, "%x", val);
							strcpy(q, tStr);	// survived strcpy
							q+=strlen(tStr);
							break;
	}
	return(q);
}

/******** PostPlayMessages() ********/

BOOL PostPlayMessage(char *fmt, ...)
{
va_list Arg;
TEXT str[MAXARGSTRLEN];
int i;
struct Window *wdw;

	for(i=0; i<MAXARGSTRLEN; i++)
		str[i] = '\0';

	va_start(Arg, fmt);
	Formatter(str, fmt, NewArgPtr=Arg);
	free(t);

	Forbid();
	wdw = IntuitionBase->ActiveWindow;
	Permit();
	if (wdw)
		return(	UA_OpenGenericWindow(	wdw, TRUE, TRUE,
																	msgs[Msg_OK-1], msgs[Msg_Abort-1],
																	EXCLAMATION_ICON, str, TRUE, NULL) );
}

/******** ProcessMsgQueue() ********/

void ProcessMsgQueue(void)
{
struct List *errorList;
struct MsgNode *this_node;

	errorList = MLMMU_GetQueueList();
	if ( errorList )
	{
		for(this_node=(struct MsgNode *)(errorList->lh_Head);
				this_node->node.ln_Succ;
				this_node=(struct MsgNode *)this_node->node.ln_Succ)
		{
			if ( this_node->txt )
				if ( !PostPlayMessage(this_node->txt) )
					return;	// aborted by user
		}
	}
}

/******** E O F ********/
