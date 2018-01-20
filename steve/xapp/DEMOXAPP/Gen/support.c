#include <exec/exec.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/serial.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <workbench/startup.h>
// CLIB
#include <clib/exec_protos.h>
// PRAGMAS
#include <pragmas/exec_pragmas.h>
// ROOT
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// USER
#include "demo:Gen/mp.h"

/**** defines ****/

#define MAXARGSTRLEN 512

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/**** static globals ****/

static va_list NewArgPtr;
static va_list OldArgPtr;
static char *Sptr, *Tptr;
static char *t = NULL;
static unsigned tSize;
static char *SavePtr;
TEXT tStr[512];

STATIC void Gen_Formatter(char *str, char *s, va_list Arg);
STATIC char *Gen_ParseType(char *q);

/**** functions ****/

/******** GiveMessage() ********/

void GiveMessage(struct Window *window, char *fmt, ...)
{
va_list Arg;
TEXT str[MAXARGSTRLEN];
int i;

	for(i=0; i<MAXARGSTRLEN; i++)
		str[i] = '\0';

	va_start(Arg, fmt);
	Gen_Formatter(str, fmt, NewArgPtr=Arg);
	free(t);

	if (window != NULL)
		UA_OpenGenericWindow(	window, TRUE, FALSE, "OK", NULL, EXCLAMATION_ICON,
													str, TRUE, NULL);
}

/******** Formatter() ********/

STATIC void Gen_Formatter(char *str, char *s, va_list Arg)
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
			q=Gen_ParseType(q);
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

/******** Gen_ParseType() ********/

STATIC char *Gen_ParseType(char *q)
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

/******** StrToScript() ********/
/*
 * Converts "foo "bar" we'll"" to "foo \"bar"\ we'll\""
 *
 */

void StrToScript(char *oldStr, char *newStr)
{
int i,j,len;

	*newStr='\0';
	len = strlen(oldStr);
	if ( len==0 )
		return;	
	i=0;
	j=0;
	while( i<len && *(oldStr+i)!='\0' )
	{
		if ( *(oldStr+i) == '\"' )
		{
			*(newStr+j) 	= '\\';
			*(newStr+j+1) = '\"';
			j=j+2;
		}
		else
		{
			*(newStr+j) = *(oldStr+i);
			j++;
		}
		i++;
	}
	*(newStr+j) = '\0';
}

/******** ScriptToStr() ********/
/*
 * Converts "foo \"bar"\ we'll\"" to "foo "bar" we'll""
 *
 */

void ScriptToStr(char *oldStr, char *newStr)
{
int i,j,len;

	*newStr='\0';
	len = strlen(oldStr);
	if ( len==0 )
		return;	
	i=0;
	j=0;
	while( i<len && *(oldStr+i)!='\0' )
	{
		if ( *(oldStr+i) == '\\' && *(oldStr+i+1) == '\"' )
		{
			*(newStr+j) = '\"';
			i=i+2;
			j++;
		}
		else
		{
			*(newStr+j) = *(oldStr+i);
			i++;
			j++;
		}
	}
	*(newStr+j) = '\0';
}

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** E O F ********/
