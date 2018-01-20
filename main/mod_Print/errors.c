#include "nb:pre.h"

/**** functions declarations ****/

void Formatter(char *str, char *s, va_list Arg);
char *ParseType(char *q);

/**** defines ****/

#define MAXARGSTRLEN 512	// see also globalallocs.c

/**** externals ****/

extern struct Library *medialinkLibBase;
extern UBYTE **msgs;

/**** static globals ****/

static va_list NewArgPtr;
static va_list OldArgPtr;
static char *Sptr, *Tptr;
static char *t = NULL;
static unsigned tSize;
static char *SavePtr;
static TEXT tStr[512];

/**** functions ****/

/******** printser() ********/

void printser(char *fmt, ...)
{
va_list Arg;
TEXT str[MAXARGSTRLEN];
int i;

	for(i=0; i<MAXARGSTRLEN; i++)
		str[i] = '\0';
	va_start(Arg, fmt);
	Formatter(str, fmt, NewArgPtr=Arg);
	free(t);
	//KPrintF(str);
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

	Forbid();
	wdw = IntuitionBase->ActiveWindow;
	Permit();
		
	if (wdw != NULL)
		UA_OpenGenericWindow(	wdw, TRUE, FALSE, msgs[Msg_OK-1], NULL, EXCLAMATION_ICON,
													str, TRUE, NULL);
}

/******** Formatter() ********/

void Formatter(char *str, char *s, va_list Arg)
{
char *p, *q;

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

/******** E O F ********/
