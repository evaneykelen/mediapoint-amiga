#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"
#include "demo:gen/general_protos.h"

extern struct Library *medialinkLibBase;

/**** function declarations ****/

void GetVarsFromPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI);

extern BOOL PushButton(struct AirLink_record *);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct AirLink_record al_rec;
#if 0
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;
#endif

	//port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	//if (port)
	//{

		//list = &(port->mp_MsgList);
		//node = list->lh_Head;
		//rvrec = (struct RendezVousRecord *)node->ln_Name;
		//medialinkLibBase = (struct Library *)rvrec->medialink;

		if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		{
			al_rec.portName[0] = '\0';
			al_rec.buttonName[0] = '\0';

			GetVarsFromPI(&al_rec, ThisPI);

			medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
			if ( medialinkLibBase )
			{
				Forbid();
				if ( al_rec.portName[0] && al_rec.buttonName[0] )
					PushButton(&al_rec);
				Permit();
				CloseLibrary((struct Library *)medialinkLibBase);
			}
		}

	//}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI)
{
	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%s %s %d",
					al_rec->portName,
					al_rec->buttonName,
					&al_rec->active );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI)
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%s %s %d",
					al_rec->portName,
					al_rec->buttonName,
					al_rec->active );
}

/******** E O F ********/
