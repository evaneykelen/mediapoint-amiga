#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "defs.h"
#include "structs.h"

/**** function declarations ****/

void GetVarsFromPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI);

/**** functions ****/

/******** XappDoIt() ********/

ULONG XappDoIt(PROCESSINFO *ThisPI)	//, BOOL *sigs, ULONG waitFlags)
{
struct Pdev_record Pdev_rec;
struct MsgPort *port;
ULONG signals=0L;

	Forbid();
	port = FindPort("PDEV_PORT");
	if (port!=NULL)
	{
		Permit();
		return(0);
	}
	port = CreatePort("PDEV_PORT",0L);
	if (port==NULL)
	{
		Permit();
		return(0);
	}
	Permit();

	Pdev_rec.window = NULL;	// make sure this one's zero

	GetVarsFromPI(&Pdev_rec, ThisPI);

	if ( !OpenPlayerDevice(&Pdev_rec) )
	{
		DeletePort(port);
		return(0);
	}

	PerformActions(&Pdev_rec,0L);	// sigs, &signals, 0L);	//waitFlags);
	ClosePlayerDevice(&Pdev_rec);

	Forbid();
	DeletePort(port);
	Permit();

	return(signals);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI)
{
TEXT cmd[10];
int start, end, video, audio1, audio2, index, blank, init;

	/*********************************************************************

			[cmd] (startframe) (endframe) [T/F] [T/F] [T/F] [T/F] [T/F] [T/F]

			[cmd] = 'E'  (Eject)\n");
							'F'  (Forward)\n");
							'FS' (Forward Slow)\n");
							'FF' (Forward Fast)\n");
							'I'  (Initialize)\n");
							'P'  (Play)\n");
							'R'  (Reverse)\n");
							'RS' (Reverse Slow)\n");
							'RF' (Reverse Fast)\n");
							'S'  (Search)\n");
							'SF' (Step Forward)\n");
							'SR' (Step Reverse)\n");
							'SB' (Standby)\n\n");
							'ST' (Still)\n\n");

			[T/F] = 'T' or 'F' (true or false) for respectively:

							Video, Audio1, Audio2, Index, Blank after showing and
							Initialize before start

			Example: P 2000 2200 T T T T F T --> play from frame 2000 to 2200

	*********************************************************************/

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%s %d %d %d %d %d %d %d %d",
					cmd, &start, &end, &video, &audio1, &audio2, &index, &blank, &init);

	if ( !strcmpi(cmd, "E") )
		Pdev_rec->action = pd_Eject;
	else if ( !strcmpi(cmd, "F") )
		Pdev_rec->action = pd_Forward;
	else if ( !strcmpi(cmd, "FS") )
		Pdev_rec->action = pd_ForwardSlow;
	else if ( !strcmpi(cmd, "FF") )
		Pdev_rec->action = pd_ForwardFast;
	else if ( !strcmpi(cmd, "I") )
		Pdev_rec->action = pd_Initialize;
	else if ( !strcmpi(cmd, "P") )
		Pdev_rec->action = pd_Play;
	else if ( !strcmpi(cmd, "R") )
		Pdev_rec->action = pd_Reverse;
	else if ( !strcmpi(cmd, "RS") )
		Pdev_rec->action = pd_ReverseSlow;
	else if ( !strcmpi(cmd, "RF") )
		Pdev_rec->action = pd_ReverseFast;
	else if ( !strcmpi(cmd, "S") )
		Pdev_rec->action = pd_Search;
	else if ( !strcmpi(cmd, "SF") )
		Pdev_rec->action = pd_StepForward;
	else if ( !strcmpi(cmd, "SR") )
		Pdev_rec->action = pd_StepReverse;
	else if ( !strcmpi(cmd, "SB") )
		Pdev_rec->action = pd_Standby;
	else if ( !strcmpi(cmd, "ST") )
		Pdev_rec->action = pd_Still;

	Pdev_rec->param1		= start;
	Pdev_rec->param2		= end;
	Pdev_rec->videoOn		= (BOOL)video;
	Pdev_rec->audio1On	= (BOOL)audio1;
	Pdev_rec->audio2On	= (BOOL)audio2;
	Pdev_rec->indexOn		= (BOOL)index;
	Pdev_rec->blank			= (BOOL)blank;
	Pdev_rec->init			= (BOOL)init;
}


/******** PutVarsToPI() ********/

void PutVarsToPI(struct Pdev_record *Pdev_rec, PROCESSINFO *ThisPI)
{
TEXT cmd[10];
int start, end, video, audio1, audio2, index, blank, init;

	switch ( Pdev_rec->action )
	{
		case pd_Eject:				strcpy(cmd, "E");		break;
		case pd_Forward:			strcpy(cmd, "F");		break;
		case pd_ForwardSlow:	strcpy(cmd, "FS");	break;
		case pd_ForwardFast:	strcpy(cmd, "FF");	break;
		case pd_Initialize:		strcpy(cmd, "I");		break;
		case pd_Play:					strcpy(cmd, "P");		break;
		case pd_Reverse:			strcpy(cmd, "R");		break;
		case pd_ReverseSlow:	strcpy(cmd, "RS");	break;
		case pd_ReverseFast:	strcpy(cmd, "RF");	break;
		case pd_Search:				strcpy(cmd, "S");		break;
		case pd_StepForward:	strcpy(cmd, "SF");	break;
		case pd_StepReverse:	strcpy(cmd, "SR");	break;
		case pd_Standby:			strcpy(cmd, "SB");	break;
		case pd_Still:				strcpy(cmd, "ST");	break;
	}

	start		= Pdev_rec->param1;
	end			=	Pdev_rec->param2;
	video		= Pdev_rec->videoOn;
	audio1	= Pdev_rec->audio1On;
	audio2	= Pdev_rec->audio2On;
	index		= Pdev_rec->indexOn;
	blank		= Pdev_rec->blank;
	init		= Pdev_rec->init;

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%s %d %d %d %d %d %d %d %d",
					cmd, start, end, video, audio1, audio2, index, blank, init);
}

/******** E O F ********/

#if 0

	if (SS==NULL)
	{
		SS = (struct SignalSemaphore *)AllocMem(sizeof(struct SignalSemaphore),
																						MEMF_PUBLIC | MEMF_CLEAR);
		InitSemaphore((struct SignalSemaphore *)SS);
	}

	if ( !AttemptSemaphore(SS) )
		return(0);

#endif
