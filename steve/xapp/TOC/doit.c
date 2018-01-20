#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Toccata_Record toc_rec;
struct ToccataBase *ToccataBase	= NULL;

	ToccataBase = ObtainToccata(NULL);
	if ( ToccataBase )
	{
		GetExtraData(ThisPI,&toc_rec);
		DoPreview(NULL,&toc_rec,NULL,NULL,TRUE,ToccataBase);
	}
	return(0);
}

/******** E O F ********/
