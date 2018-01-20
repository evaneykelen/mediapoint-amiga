#include <exec/types.h>
#include <exec/exec.h>
#include "devdisk/include/studio16.h"

struct StudioBase *StudioBase;

BOOL main(void)
{
struct ChanKey *key1, *key2;
struct ActiveSample *asList[3] = { NULL, NULL, NULL };
struct Region myregion1, myregion2;
int we_are_initing;

	/**** open library ****/

	StudioBase=(struct StudioBase *)OpenLibrary("studio.library",0L);
	if (StudioBase==NULL)
		return(FALSE);

	if (StudioBase->LibNode.lib_OpenCnt>=1)
		we_are_initing = TRUE; 
	else
		we_are_initing = FALSE;

	if ( !we_are_initing )
		InitStudio16UserEnv(ISUE_DO_ASSIGNS|ISUE_LOAD_STUDIO16BASE|ISUE_LOAD_DRIVERS,0,0);

	setmem(&myregion1, sizeof(struct Region), 0);		// clear memory
	strcpy(myregion1.samp_name,"work:untitled_l");	// copy sample name #1

	setmem(&myregion2, sizeof(struct Region), 0);		// clear memory
	strcpy(myregion2.samp_name,"work:untitled_r");	// copy sample name #2

/*
	myregion1.parms.volume	= 3200;
	myregion1.parms.rate		= 44100;
	myregion1.parms.pan			= 0;

	myregion2.parms.volume	= 3200;
	myregion2.parms.rate		= 44100;
	myregion2.parms.pan			= 200;
*/
	
	asList[0] = (struct ActiveSample *)ASOpen(&myregion1,
																						AS_PLAY | AS_AUTO_DATA | AS_AUTO_CLOSE |
																						AS_AUTO_FREECHAN);
	if (asList[0]==0)
	{
		if (we_are_initing)
			CloseAllModules(FALSE);
		CloseLibrary((struct Library *)StudioBase);
		return(FALSE);
	}

	asList[1] = (struct ActiveSample *)ASOpen(&myregion2,
																						AS_PLAY | AS_AUTO_DATA | AS_AUTO_CLOSE |
																						AS_AUTO_FREECHAN);
	if (asList[1]==0)
	{
		if (we_are_initing)
			CloseAllModules(FALSE);
		CloseLibrary((struct Library *)StudioBase);
		return(FALSE);
	}

	asList[0]->key = (struct ChanKey *)AllocChan(NULL,-1,CK_CHAN_PLAY);
	asList[1]->key = (struct ChanKey *)AllocChan(NULL,-1,CK_CHAN_PLAY);

	myregion1.parms.volume	= 3200;
	myregion1.parms.rate		= 44100;
	myregion1.parms.pan			= 0;

	myregion2.parms.volume	= 3200;
	myregion2.parms.rate		= 44100;
	myregion2.parms.pan			= 200;

	ASTriggerList(asList,0);	// flags=0 -- THIS IS NOT DOCUMENTED !?!

	if (we_are_initing)
		CloseAllModules(FALSE);
	CloseLibrary((struct Library *)StudioBase);

	return(TRUE);
}

/******* E O F ********/
