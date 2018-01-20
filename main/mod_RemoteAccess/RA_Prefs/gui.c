#include "nb:pre.h"
#include "protos.h"

STATIC struct MsgPort *MyCreatePort(UBYTE *, LONG);
STATIC struct Task *oldTask;

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct Library *medialinkLibBase;
extern struct MsgPort *capsPort;

/******** OpenMPWindow() ********/

struct Window *OpenMPWindow(struct GadgetRecord *GR)
{
struct NewWindow NewWindowStructure;
WORD x,y;
struct Screen *screen;
struct Window *window;

	Forbid();
	screen = IntuitionBase->ActiveScreen;
	Permit();
	if ( screen )
	{
		x = ( screen->Width - GR[0].x2 ) / 2;
		y = ( screen->Height - GR[0].y2 ) / 2;
		if ( x<0 )
			x=0;
		if ( y<0 )
			y=0;
	}
	else
	{
		x=0;
		y=0;
	}

	NewWindowStructure.LeftEdge 		= x;
	NewWindowStructure.TopEdge 			= y;
	NewWindowStructure.Width 				= GR[0].x2;
	NewWindowStructure.Height 			= GR[0].y2;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= NULL;	//IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY;
	NewWindowStructure.Flags 				= WFLG_GIMMEZEROZERO | WFLG_ACTIVATE | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= NULL;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= WBENCHSCREEN;

	window = OpenWindow(&NewWindowStructure);
	if ( window )
	{
		window->UserPort = capsPort;
		ModifyIDCMP(window, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_GADGETDOWN | IDCMP_GADGETUP);
	}

	return(window);
}

/******** CloseMPWindow() ********/

void CloseMPWindow(struct Window *window)
{
	if ( window )
		UA_CloseWindowSafely(window);
}

/******** OpenInput() ********/

BOOL OpenInput(void)
{
	if ( !FindPort(ML_RENDEZ_VOUS) )
	{
		capsPort = (struct MsgPort *)MyCreatePort(MEDIALINKPORT, 0);
		if ( !capsPort )
			return(FALSE);

		UA_PutCapsPort(capsPort);
	}

	return(TRUE);
}

/******** CloseInput() ********/

void CloseInput(void)
{
	if ( !FindPort(ML_RENDEZ_VOUS) )
	{
		if ( capsPort )
			DeletePort((struct MsgPort *)capsPort);
	}
}

/******** MyCreatePort() ********/

STATIC struct MsgPort *MyCreatePort(UBYTE *name, LONG pri)
{
LONG sigBit;
struct MsgPort *mp;
int sig;

	sigBit = -1;
	for(sig=0; sig<32; sig++)
	{
		sigBit = AllocSignal( sig );
		if ( sigBit != -1 )
			break;
	}
	if ( sigBit==-1 )
		return(NULL);

	mp = (struct MsgPort *)AllocMem((ULONG)sizeof(struct MsgPort),(ULONG)MEMF_PUBLIC|MEMF_CLEAR);
	if ( !mp )
	{
		FreeSignal(sigBit);
		return(NULL);
	}

	mp->mp_Node.ln_Name		= name;
	mp->mp_Node.ln_Pri		= pri;
	mp->mp_Node.ln_Type		= NT_MSGPORT;
	mp->mp_Flags					= PA_SIGNAL;
	mp->mp_SigBit					= sigBit;
	mp->mp_SigTask				= (struct Task *)FindTask(0L);

	if (name)
		AddPort(mp);
	else
		NewList( &(mp->mp_MsgList) );

	return( mp );
}

/******** MPOpenLibs() ********/

BOOL MPOpenLibs(void)
{
TEXT path[SIZE_FULLPATH];
struct MsgPort *port;
struct Node *node;
struct List *list;
struct RendezVousRecord *rvrec;

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port)
	{
		list = &(port->mp_MsgList);
		node = list->lh_Head;
		rvrec = (struct RendezVousRecord *)node->ln_Name;
		IntuitionBase = (struct IntuitionBase *)rvrec->intuition;
		GfxBase = (struct GfxBase *)rvrec->graphics;
		medialinkLibBase = (struct Library *)rvrec->medialink;
		capsPort = (struct MsgPort *)FindPort(MEDIALINKPORT);
		oldTask = capsPort->mp_SigTask;
		capsPort->mp_SigTask = FindTask(NULL);
	}
	else
	{
		sprintf(path, "System/%s",ML_LIBRARY_1);
		medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
		if (!medialinkLibBase)
		{
			sprintf(path, "libs:%s", ML_LIBRARY_1);
			medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
			if (!medialinkLibBase)
			{
				sprintf(path, "%s", ML_LIBRARY_1);
				medialinkLibBase = (struct Library *)OpenLibrary(path, 0L);
				if (!medialinkLibBase)
					return(FALSE);
			}
		}
		if ( !UA_Open_ML_Lib() )
			return(FALSE);

		IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
		if (IntuitionBase == NULL)
			return(FALSE);

		GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
		if (GfxBase == NULL)
			return(FALSE);
	}

	return(TRUE);
}

/******** MPCloseLibs() ********/

void MPCloseLibs(void)
{
	if ( FindPort(ML_RENDEZ_VOUS) )
	{
		capsPort->mp_SigTask = oldTask;
	}
	else
	{
		if ( medialinkLibBase )
			CloseLibrary((struct Library *)medialinkLibBase);

		if ( IntuitionBase )
			CloseLibrary((struct Library *)IntuitionBase);

		if ( GfxBase )
			CloseLibrary((struct Library *)GfxBase);
	}
}

/******** E O F ********/
