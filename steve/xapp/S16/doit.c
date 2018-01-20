#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

extern struct Library *medialinkLibBase;

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Studio_Record studio_rec;

	GetExtraData(ThisPI,&studio_rec);

	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
	if ( medialinkLibBase )
	{
		PerformActions(&studio_rec);
		CloseLibrary((struct Library *)medialinkLibBase);
	}

	return(0);
}

/******** E O F ********/
