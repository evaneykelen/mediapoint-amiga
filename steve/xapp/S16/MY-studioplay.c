#include <exec/types.h>
#include <exec/exec.h>
//#include "structs.h"
#include "include/studio16.h"
//#include "studiopragmas.h"
#include <pragmas/exec_pragmas.h>

/**** externals ****/

//extern struct StudioBase *StudioBase;

BOOL PlaySample(STRPTR file1, STRPTR file2);

struct StudioBase *StudioBase;

/**** functions ****/

void main(int argc, char **argv)
{
	if (argc==3)
		PlaySample(argv[1],argv[2]);
printf("out\n");
}

/******** PlaySample() ********/

BOOL PlaySample(STRPTR file1, STRPTR file2)
{
struct ChanKey *key1, *key2;
struct ActiveSample *as1, *as2;
struct Region myregion1, myregion2;
BOOL we_are_initing;
int rate;

	StudioBase = (struct StudioBase *)OpenLibrary("studio.library",0);
	if (StudioBase==NULL)
		return(FALSE);

	/** If this is the first time studio.library is opened, we	**/
	/** need to tell studio.library to go find all the working	**/
	/** samples in the audio: directory and assign studio16:		**/

	if (StudioBase->LibNode.lib_OpenCnt==1)
		we_are_initing=TRUE; 
	else
		we_are_initing=FALSE;

	InitStudio16UserEnv(ISUE_DO_ASSIGNS | ISUE_LOAD_STUDIO16BASE |
											ISUE_LOAD_DRIVERS, 0, 0); //, StudioBase);

	/** ASOpen() etc. use the Region struct to define what part of the			**/
	/** sample we should play.  Define a region covering the entire sample	**/
	/** can be done by setting it to zero.  ASOpen() will fill end,etc.			**/

	setmem(&myregion1, sizeof(struct Region), 0);
	strcpy(myregion1.samp_name, file1);

	setmem(&myregion2, sizeof(struct Region), 0);
	strcpy(myregion2.samp_name, file2);
	
	/**** Activate the sample for playback & preload the data						****/
	/**** AS_AUTO_DATA means that DiskIO will handle ASRead() requests	****/

	as1 = (struct ActiveSample *)ASOpen(&myregion1,
																			AS_PLAY | AS_AUTO_DATA );
/*																			AS_AUTO_CLOSE | AS_AUTO_FREECHAN ); */
//																			StudioBase );
	if (as1==0)
	{
		if (we_are_initing)
			CloseAllModules(FALSE); //, StudioBase);
		return(FALSE);
	}

	as2 = (struct ActiveSample *)ASOpen(&myregion2,
																			AS_PLAY | AS_AUTO_DATA );
/*																			AS_AUTO_CLOSE | AS_AUTO_FREECHAN );*/
//																			StudioBase);
	if (as2==0)
	{
		if (we_are_initing)
			CloseAllModules(FALSE); //, StudioBase);
		return(FALSE);
	}

printf("AFTER OPEN\n");

	/**** Allocate any free channel for playback on ****/

	as1->key = (struct ChanKey *)AllocChan(NULL, -1, CK_CHAN_PLAY); //, StudioBase);
	key1 = as1->key;

	as2->key = (struct ChanKey *)AllocChan(NULL, -1, CK_CHAN_PLAY); //, StudioBase);
	key2 = as2->key;

printf("AFTER ALLOC\n");

	if (key1)
		ASTrigger(as1); //, StudioBase);	// start playback

	if (key2)
		ASTrigger(as2); //, StudioBase);	// start playback

printf("AFTER TRIGGER\n");



rate=as1->region.parms.rate;
ASClose(as1);		/** wait for it to finish & close it **/
ASClose(as2);		/** wait for it to finish & close it **/
//Delay((50*6144)/rate);	/** AD1012Handler bug (wait for card buffers to empty) **/
FreeChan(key1);
FreeChan(key2);
if (we_are_initing)
	CloseAllModules(FALSE);
CloseLibrary(StudioBase);


#if 0
	CloseLibrary((struct Library *)StudioBase);

printf("AFTER CLOSE\n");
#endif

	return(TRUE);
}

/******** E O F ********/
