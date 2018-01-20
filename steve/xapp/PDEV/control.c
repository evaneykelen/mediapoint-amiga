#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "player.h"
#include "playercode.h"
#include "defs.h"
#include "demo:gen/wait50hz.h"

/**** externals ****/

#define PlayerBase Pdev_rec->PlayerBase

/**** functions ****/

/******** CommandPlayer() ********/

BOOL CommandPlayer(	struct Pdev_record *Pdev_rec, int acode, int x, int y,
										ULONG waitFlags )
{
ULONG mask, signal, hz_signal=0;
BOOL loop=TRUE,retval=TRUE;
struct wjif WJIF;

#if 0
	if( CheckIO((struct IORequest *)Pdev_rec->IOblock) )
	{
		AbortIO((struct IORequest *)Pdev_rec->IOblock);
		WaitIO((struct IORequest *)Pdev_rec->IOblock);
	}
#endif

	Pdev_rec->IOblock->io_Message.mn_Node.ln_Type = NT_MESSAGE;
	Pdev_rec->IOblock->io_Command	= acode;
	Pdev_rec->IOblock->iop_Format	= FORMAT_ARGS;
	Pdev_rec->IOblock->iop_Argx		= x;
	Pdev_rec->IOblock->iop_Argy		= y;
	Pdev_rec->IOblock->iop_Argin	= NULL;
	Pdev_rec->IOblock->iop_Macro	= 0;
	SendIO((struct IORequest *)Pdev_rec->IOblock);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);

	mask = (1L << Pdev_rec->port->mp_SigBit) | hz_signal;

/*
{
char strstr[50];
sprintf(strstr,"%x %x\n",(1L << Pdev_rec->port->mp_SigBit),hz_signal);
KPrintF(strstr);
}
*/

	while(loop)
	{
		signal = Wait(mask);

/*
{
char strstr[50];
sprintf(strstr,"signal = %x\n",signal);
KPrintF(strstr);
}
*/

		if ( signal & hz_signal )
		{
			retval=FALSE;
			loop=FALSE;
		}

		if( CheckIO((struct IORequest *)Pdev_rec->IOblock) )
		{
			WaitIO((struct IORequest *)Pdev_rec->IOblock);
			loop=FALSE;
		}
	}

//KPrintF("loop ended\n");

	if ( !retval )
	{
		if( CheckIO((struct IORequest *)Pdev_rec->IOblock) )
		{
			AbortIO((struct IORequest *)Pdev_rec->IOblock);
			WaitIO((struct IORequest *)Pdev_rec->IOblock);
		}
	}

//KPrintF("abort ended\n");

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** OpenPlayerDevice() ********/

BOOL OpenPlayerDevice(struct Pdev_record *Pdev_rec)
{
int error;

	if ( (Pdev_rec->port = CreatePort(0,0))==NULL )
		return(FALSE);

	if ((Pdev_rec->IOblock = (struct IOPlayer*)CreateExtIO(Pdev_rec->port,
																					sizeof(struct IOPlayer)))==NULL)
	{
		DeletePort(Pdev_rec->port);
		return(FALSE);
	}

	error = OpenDevice(PLAYER_DEVNAME, 0, (struct IORequest *)Pdev_rec->IOblock, 0);

	PlayerBase = (struct Library *)(Pdev_rec->IOblock->io_Device);
	if (error)
	{
		DeleteExtIO((struct IORequest *)Pdev_rec->IOblock);
		DeletePort(Pdev_rec->port);
		return(FALSE);
	}	

	if( error && Pdev_rec->IOblock->io_Device )
	{
		ClosePlayerDevice(Pdev_rec);
		return(FALSE);
	}
	else if (error)
	{
		DeleteExtIO((struct IORequest *)Pdev_rec->IOblock);
		DeletePort(Pdev_rec->port);
		return(FALSE);
	}

	return(TRUE);
}

/******** ClosePlayerDevice() ********/

void ClosePlayerDevice(struct Pdev_record *Pdev_rec)
{
	RemDevice(Pdev_rec->IOblock->io_Device);
	CloseDevice((struct IORequest *)Pdev_rec->IOblock);
	DeleteExtIO((struct IORequest *)Pdev_rec->IOblock);
	DeletePort(Pdev_rec->port);
}

/******** PerformActions() ********/

BOOL PerformActions( struct Pdev_record *Pdev_rec, ULONG waitFlags )	//BOOL *sigs, ULONG *signals,
{
int action;
BOOL retval=TRUE;

	if ( Pdev_rec->action == pd_Play && Pdev_rec->init )
	{
		if (!CommandPlayer(Pdev_rec, AC_AREADY, 0, 0, waitFlags))
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_DCLEAR, 0, 0, waitFlags))
			return(FALSE);
	}

	if ( Pdev_rec->indexOn )
	{
		if (!CommandPlayer(Pdev_rec, AC_AVPION, 0, 0, waitFlags))
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_AVCION, 0, 0, waitFlags))
			return(FALSE);
	}
	else
	{
		if (!CommandPlayer(Pdev_rec, AC_AVPIOF, 0, 0, waitFlags))
			return(FALSE);
		if (!CommandPlayer(Pdev_rec, AC_AVCIOF, 0, 0, waitFlags))
			return(FALSE);
	}

	if ( Pdev_rec->action!=pd_Play && Pdev_rec->videoOn )
	{
		if (!CommandPlayer(Pdev_rec, AC_AVVION, 0, 0, waitFlags))
			return(FALSE);
	}
	else
	{
		if (!CommandPlayer(Pdev_rec, AC_AVVIOF, 0, 0, waitFlags))
			return(FALSE);
	}

	if ( Pdev_rec->audio1On )
	{
		if (!CommandPlayer(Pdev_rec, AC_AA1ON, 0, 0, waitFlags))
			return(FALSE);
	}
	else
	{
		if (!CommandPlayer(Pdev_rec, AC_AA1OFF, 0, 0, waitFlags))
			return(FALSE);
	}

	if ( Pdev_rec->audio2On )
	{
		if (!CommandPlayer(Pdev_rec, AC_AA2ON, 0, 0, waitFlags))
			return(FALSE);
	}
	else
	{
		if (!CommandPlayer(Pdev_rec, AC_AA2OFF, 0, 0, waitFlags))
			return(FALSE);
	}

	switch( Pdev_rec->action )
	{
		case pd_Eject:
			retval=CommandPlayer(Pdev_rec, AC_AHJECT, 0, 0, waitFlags);
			break;

		case pd_Forward:
			retval=CommandPlayer(Pdev_rec, AC_AMPNF, 0, 0, waitFlags);
			break;
		case pd_ForwardSlow:
			retval=CommandPlayer(Pdev_rec, AC_AMPSF, 0, 0, waitFlags);
			break;
		case pd_ForwardFast:
			retval=CommandPlayer(Pdev_rec, AC_AMPFF, 0, 0, waitFlags);
			break;

		case pd_Initialize:
			if (!CommandPlayer(Pdev_rec, AC_AREADY, 0, 0, waitFlags))
				return(FALSE);
			if (!CommandPlayer(Pdev_rec, AC_DCLEAR, 0, 0, waitFlags))
				return(FALSE);
			break;

		case pd_Play:	// play n tru m
			if (Pdev_rec->param2 < Pdev_rec->param1)
				action = AC_AMPNR;
			else
				action = AC_AMPNF;
			if (Pdev_rec->param1 != Pdev_rec->param2)
			{
				// AC_AFGOST -> goto frame and still
				if (!CommandPlayer(Pdev_rec, AC_AFGOST, Pdev_rec->param1, 0, waitFlags))
					return(FALSE);

				// AC_AQENDM -> query end of search phase
				if (!CommandPlayer(Pdev_rec, AC_AQENDM, 0, 0, waitFlags))
					return(FALSE);

				if ( Pdev_rec->videoOn )
				{
					if (!CommandPlayer(Pdev_rec, AC_AVVION, 0, 0, waitFlags))
						return(FALSE);
				}
				else
				{
					if (!CommandPlayer(Pdev_rec, AC_AVVIOF, 0, 0, waitFlags))
						return(FALSE);
				}

				// action -> AC_AMPNR or AC_AMPNF
				// which means: Play reverse normal or Play forward normal

				if (!CommandPlayer(Pdev_rec, action, 0, 0, waitFlags))
					return(FALSE);

				if (Pdev_rec->param1 < Pdev_rec->param2)
					CommandPlayer(Pdev_rec, AC_AFSTOP, Pdev_rec->param2, 0, waitFlags);
				else
					CommandPlayer(Pdev_rec, AC_AFSTOP, Pdev_rec->param1, 0, waitFlags);

				CommandPlayer(Pdev_rec, AC_AFRETS, 0, 0, waitFlags);

#if 0
				if (action==AC_AMPNR)
				{
					if (!CommandPlayer(Pdev_rec, AC_AFINFO, Pdev_rec->param2+6, 0))
						return(FALSE);
				}
				else
				{
					if (!CommandPlayer(Pdev_rec, AC_AFINFO, Pdev_rec->param2-5, 0))
						return(FALSE);
				}

				if (!CommandPlayer(Pdev_rec, AC_AFRETI, 0, 0))
					return(FALSE);

				if (!CommandPlayer(Pdev_rec, AC_ASSTIL, 0, 0))
					return(FALSE);
#endif

				if ( Pdev_rec->blank )
				{
					if (!CommandPlayer(Pdev_rec, AC_AVVIOF, 0, 0, waitFlags))
						return(FALSE);
				}
			}
			break;

		case pd_Reverse:
			retval=CommandPlayer(Pdev_rec, AC_AMPNR, 0, 0, waitFlags);
			break;
		case pd_ReverseSlow:
			retval=CommandPlayer(Pdev_rec, AC_AMPSR, 0, 0, waitFlags);
			break;
		case pd_ReverseFast:
			retval=CommandPlayer(Pdev_rec, AC_AMPFR, 0, 0, waitFlags);
			break;

		case pd_Search:
			retval=CommandPlayer(Pdev_rec, AC_AFGOST, Pdev_rec->param1, 0, waitFlags);
			retval=CommandPlayer(Pdev_rec, AC_AQENDM, 0, 0, waitFlags);
			break;

		case pd_Standby:
			retval=CommandPlayer(Pdev_rec, AC_AROFF, 0, 0, waitFlags);
			break;

		case pd_StepForward:
			retval=CommandPlayer(Pdev_rec, AC_AMSTF, 0, 0, waitFlags);
			break;
		case pd_StepReverse:
			retval=CommandPlayer(Pdev_rec, AC_AMSTR, 0, 0, waitFlags);
			break;

		case pd_Still:
			retval=CommandPlayer(Pdev_rec, AC_ASSTIL, 0, 0, waitFlags);
			break;
	}

	return(retval);
}

/******** E O F ********/
