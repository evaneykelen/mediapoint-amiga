#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"

/**** function declarations ****/

void GetVarsFromPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct CDTV_record CDTV_rec;

	CDTV_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&CDTV_rec, ThisPI);

	GetInfoFile("CDTV", ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath,
							CDTV_rec.devName, &(CDTV_rec.portNr), &(CDTV_rec.baudRate));

	if ( CDTV_rec.control==CONTROL_VIA_SERIAL )
	{
		if ( !Open_SerialPort(&CDTV_rec) )
			return(0);

		ControlCDTV(&CDTV_rec);

		Close_SerialPort(&CDTV_rec);
	}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI)
{
	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %s %s %d %d %s %d %d",
					&(CDTV_rec->action),
					&(CDTV_rec->command),
					&(CDTV_rec->song),
					&(CDTV_rec->from),
					&(CDTV_rec->to),
					CDTV_rec->start,
					CDTV_rec->end,
					&(CDTV_rec->fadeIn),
					&(CDTV_rec->control) );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %s %s %d %d",
					CDTV_rec->action,
					CDTV_rec->command,
					CDTV_rec->song,
					CDTV_rec->from,
					CDTV_rec->to,
					CDTV_rec->start,
					CDTV_rec->end,
					CDTV_rec->fadeIn,
					CDTV_rec->control );
}

/******** E O F ********/
