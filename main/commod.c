/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"
#include <libraries/commodities.h>
#include <clib/commodities_protos.h>

/**** EXTERNALS ****/

extern TEXT special_char[];
extern struct Screen *pageScreen;
extern struct Window *playWindow;

/**** GLOBALS ****/

struct MsgPort *broker_mp;
BOOL BlockAllInput=FALSE;

/**** STATICS ****/

STATIC struct NewBroker newbroker =
{
	NB_VERSION, "MP Magic", "MediaPoint® Magic Commodity",
	"© 92,93,94 by MediaPoint Int.", NBU_UNIQUE|NBU_NOTIFY, 0, 0, 0, 0
};

struct Library *CxBase = NULL;
STATIC CxObj *broker, *cocustom, *cosignal;
STATIC ULONG cxsigflag, signal, cxobjsignal;

/**** FUNCTIONS ****/

/******** OpenMagicBroker() ********/

BOOL OpenMagicBroker(void)
{
CxMsg *msg;
struct Task *task;

	CxBase = (struct Library *)OpenLibrary("commodities.library", 37L);
	if (CxBase==NULL)
		return(FALSE);

	broker_mp = (struct MsgPort *)CreateMsgPort();
	if (!broker_mp)
	{
		CloseLibrary((struct Library *)CxBase);
		return(FALSE);
	}

	newbroker.nb_Port = broker_mp;
	cxsigflag = 1L << broker_mp->mp_SigBit;
	newbroker.nb_Pri = 20;

	if (broker=CxBroker(&newbroker,NULL))
	{
		if ( cocustom = CxCustom(CxFunction,0L) )
		{
			AttachCxObj(broker, cocustom);
			if ( (signal=(ULONG)AllocSignal(-1L)) != -1 )
			{
				cxobjsignal = 1L << signal;
				cxsigflag |= cxobjsignal;
				task = FindTask(NULL);
				if (cosignal = CxSignal(task,signal))
				{
					AttachCxObj(cocustom, cosignal);
					ActivateCxObj(broker,1L);
					return(TRUE);	// all started OK
				}
				FreeSignal(signal);
			}
			DeleteCxObjAll(broker);
		}
	}

	while(msg=(CxMsg *)GetMsg(broker_mp))
		ReplyMsg((struct Message *)msg);

	DeletePort(broker_mp);

	CloseLibrary((struct Library *)CxBase);

	return(FALSE);	// everything went wrong
}

/******** CloseMagicBroker() ********/

void CloseMagicBroker(void)
{
CxMsg *msg;

	if (CxBase!=NULL)
	{
		FreeSignal(signal);
		DeleteCxObjAll(broker);
		while(msg=(CxMsg *)GetMsg(broker_mp))
			ReplyMsg((struct Message *)msg);
		DeletePort(broker_mp);
		CloseLibrary((struct Library *)CxBase);
	}
}

/******** CxFunction() ********/

void CxFunction(register CxMsg *cxm, CxObj *co)
{
register struct InputEvent *ie;
static USHORT Qualifier=0;

	ie = (struct InputEvent *)CxMsgData(cxm);

#ifndef USED_FOR_PLAYER

	if	(		!BlockAllInput &&
			 		(ie->ie_Class==IECLASS_RAWKEY) &&
					(ie->ie_Qualifier&IEQUALIFIER_LCOMMAND) &&
					(ie->ie_Code==0x36 || ie->ie_Code==0x37)
			)
		ScreenAtNormalPos();

#endif

	if ( BlockAllInput )
	{
		if( ie->ie_Class == IECLASS_RAWKEY )
			ie->ie_Qualifier &= ~IEQUALIFIER_LCOMMAND;
		else if( ie->ie_Class == IECLASS_RAWMOUSE )
			ActivateWindow(playWindow);
		return;
	}

	if ( IntuitionBase->ActiveScreen!=pageScreen )	// FirstScreen
		return;

	if( ie->ie_Class == IECLASS_RAWKEY )
	{
		if ( ie->ie_Code >= 0x60 )
		{
			switch( ie->ie_Code )
			{
				case 0x66:	// lcommand
					Qualifier = IEQUALIFIER_NUMERICPAD;
					break;
				case 0xe6:	// lcommand
					Qualifier = 0;
					break;
			}
		}
	}
	else if( ie->ie_Class == IECLASS_RAWMOUSE )
	{
		ie->ie_Qualifier |= Qualifier;
		ie->ie_Qualifier &= ~IEQUALIFIER_LCOMMAND;
	}
}

/******** E O F ********/
