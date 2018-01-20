#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "protos.h"
#include "structs.h"
#define GUI_DEFS
#include "setup.h"

#define APPLICNAME "setup"

/**** function declarations ****/

void GetVarsFromPI(struct GVR_record *GVR_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct GVR_record *GVR_rec, PROCESSINFO *ThisPI);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName,
									int *portNr, int *baudRate );

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct GVR_record GVR_rec;

	GVR_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&GVR_rec, ThisPI);

	/**** open special libraries or devices ****/

	GetInfoFile(ThisPI, "GVR",
							GVR_rec.devName, &(GVR_rec.portNr), &(GVR_rec.baudRate) );

	if ( Open_SerialPort((struct standard_record *)&GVR_rec,8,8,2) )
	{
		PerformActions(&GVR_rec);
		Close_SerialPort((struct standard_record *)&GVR_rec);
	}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct GVR_record *GVR_rec, PROCESSINFO *ThisPI)
{
	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %s %s %d",
					&(GVR_rec->video),
					&(GVR_rec->audio),
					&(GVR_rec->blank),
					&(GVR_rec->init),
					&(GVR_rec->start),
					&(GVR_rec->end),
					&(GVR_rec->cmd) );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct GVR_record *GVR_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %s %s %d",
					GVR_rec->video,
					GVR_rec->audio,
					GVR_rec->blank,
					GVR_rec->init,
					GVR_rec->start,
					GVR_rec->end,
					GVR_rec->cmd );
}

/******** E O F ********/
