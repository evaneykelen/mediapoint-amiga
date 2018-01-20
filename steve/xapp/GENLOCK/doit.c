#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

/**** function declarations ****/

void GetVarsFromPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Genlock_record gl_rec;

	gl_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&gl_rec, ThisPI);

	PerformActions(&gl_rec,NULL,ThisPI);

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI)
{
	/**** action counterOn frame startFrame endFrame delay blank ****/

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d",
					&(gl_rec->mode),
					&(gl_rec->alwaysLaced) );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d",
					gl_rec->mode,
					gl_rec->alwaysLaced );
}

/******** E O F ********/
