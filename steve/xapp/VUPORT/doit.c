#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

/**** function declarations ****/

void GetVarsFromPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct VuPort_record vp_rec;

	vp_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&vp_rec, ThisPI);

	GetInfoFile(ThisPI, "VuPort", vp_rec.devName, &(vp_rec.portNr), &(vp_rec.baudRate));

	/**** open special libraries or devices ****/

	if ( Open_SerialPort((struct standard_record *)&vp_rec,8,8,1) )
	{
		PerformActions(&vp_rec);
		Close_SerialPort((struct standard_record *)&vp_rec);
	}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI)
{
	/**** action counterOn frame startFrame endFrame delay blank ****/

	vp_rec->arg1 = 0;
	vp_rec->arg2 = 0;

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d",
					&(vp_rec->command),
					&(vp_rec->unit),
					&(vp_rec->arg1),
					&(vp_rec->arg2) );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d",
					vp_rec->command,
					vp_rec->unit,
					vp_rec->arg1,
					vp_rec->arg2 );
}

/******** E O F ********/
