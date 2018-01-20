#include "nb:pre.h"
#include "protos.h"

#define VERSION	"\0$VER: 1.0"

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs = NULL;
struct RendezVousRecord *rvrec = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *medialinkLibBase = NULL;
FILE *logfile;

/**** disable CTRL-C break ****/

int CXBRK(void) { return(0); }
void chkabort(void) { return; }

/**** functions ****/

/******** main() ********/

int main(int argc, char **argv)
{
struct MsgPort *port;
struct Node *node;
struct List *list;
STRPTR Type = NULL;
STRPTR LogName = NULL;

	if ( argc <  4 )
	{
		printf("error in arguments; use %s CDF_File TempScript BigFile [type] [logfile]\n", argv[0]);
		//printser("error in arguments; use %s CDF_File TempScript BigFile [type] [logfile]\n", argv[0]);
		return(0);
	}

	if( argc >= 4 )
		Type = argv[4];
	if( argc >= 5 )
		LogName = argv[5];

	/**** find the mother ship ****/

	if ( FindTask("RemoteAccess") )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port)
		{
			list = &(port->mp_MsgList);
			node = list->lh_Head;
			rvrec = (struct RendezVousRecord *)node->ln_Name;
			/**** drain it ****/
			IntuitionBase = (struct IntuitionBase *)rvrec->intuition;
			GfxBase = (struct GfxBase *)rvrec->graphics;
			medialinkLibBase = (struct Library *)rvrec->medialink;
			msgs = (UBYTE **)rvrec->msgs;
		}
	}

	/**** do your thing ***/

	Session(argv[1],argv[2],argv[3],Type,LogName );	// CDF, TempScript, BigFile

	/**** leave the show ****/

	return(0);
}

/******** Report() ********/

BOOL Report(STRPTR str)
{
BOOL (*func)(APTR);

	if ( rvrec )
	{
		func = rvrec->miscfunc;
		return( (*(func))(str) );
	}
	else if (str)		// str can be NULL, this clears the progress bar. This is only
	{								// used by func and has no purpose for (K)PrintF
		if ( !isdigit(str[0]) )	// don't print progress bar numbers!
			printf("REPORT: [%s]\n", str);
//			KPrintF("REPORT: [%s]\n", str);
	}

	return(TRUE);
}

/******** IsTheButtonHit() ********/

BOOL IsTheButtonHit(void)
{
BOOL (*func)(void);

	if ( rvrec )
	{
		func = rvrec->miscfunc2;
		return( (*(func))() );
	}
	return(FALSE);
}

/******** E O F ********/
