#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

/**** function declarations ****/

void GetVarsFromPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate, STRPTR unitStr);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Ion_record Ion_rec;

	Ion_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&Ion_rec, ThisPI);

	GetInfoFile(ThisPI, "ION", Ion_rec.devName, &(Ion_rec.portNr), &(Ion_rec.baudRate),
							Ion_rec.unitStr);

	/**** open special libraries or devices ****/

	if ( Open_SerialPort((struct standard_record *)&Ion_rec,8,8,2) )
	{
		PerformActions(&Ion_rec);
		Close_SerialPort((struct standard_record *)&Ion_rec);
	}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI)
{
	/**** action counterOn frame startFrame endFrame delay blank ****/

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %d %d",
					&(Ion_rec->action),
					&(Ion_rec->counterOn),
					&(Ion_rec->frame),
					&(Ion_rec->startFrame),
					&(Ion_rec->endFrame),
					&(Ion_rec->delay),
					&(Ion_rec->blank) );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct Ion_record *Ion_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %d %d",
					Ion_rec->action,
					Ion_rec->counterOn,
					Ion_rec->frame,
					Ion_rec->startFrame,
					Ion_rec->endFrame,
					Ion_rec->delay,
					Ion_rec->blank );
}

/******** E O F ********/
