#include "devdisk/include/studio16.h"
#include "nb:pre.h"
//#include "exec/types.h"
//#include "exec/exec.h"
//#include <pragmas/exec_pragmas.h>
//#include <pragmas/dos_pragmas.h>
//#include "devdisk/include/studio16.h"
#include "structs.h"

BOOL PlaySample(struct Studio_record *Studio_rec, struct ActiveSample *asList[]);
void StopSample(struct Studio_record *Studio_rec, struct ActiveSample *asList[]);

#if 0

void main(void)
{
struct Studio_record studio_rec;

	strcpy(studio_rec.file1,"aso:studio16/samples/sample_R");
	strcpy(studio_rec.file2,"aso:studio16/samples/sample_L");

	if ( PlaySample(&studio_rec,NULL) )
		printf("OK\n");
	else
		printf("ERROR\n");

	exit(0);
}

#endif

/******** PlaySample() ********/

BOOL PlaySample(struct Studio_record *Studio_rec,
								struct ActiveSample *asList[])
{
struct ChanKey *key1, *key2;
struct ActiveSample *as1, *as2;
struct StudioRegion myregion1, myregion2;
int rate,we_are_initing;
struct StudioBase *StudioBase;
struct DOSBase *DOSBase;                                   

	StudioBase=(struct StudioBase *)OpenLibrary("studio.library",0);
	if (StudioBase==NULL)
	{
		printf("Can't open studio.library\n");
		return(FALSE);
	}

KPrintF("Studio lib opened\n");

	DOSBase = (struct DOSBase *)OpenLibrary("dos.library",0);  

KPrintF("DOS lib opened\n");

	/** If this is the first time studio.library is opened, we **/
	/** need to tell studio.library to go find all the working **/
	/** samples in the audio: directory and assign studio16: **/
	
	if (StudioBase->LibNode.lib_OpenCnt==1)
		we_are_initing=TRUE; 
	else
		we_are_initing=FALSE;
	
	InitStudio16UserEnv(ISUE_DO_ASSIGNS|
											ISUE_LOAD_STUDIO16BASE|
											ISUE_LOAD_DRIVERS, 0, 0, StudioBase);

KPrintF("InitStudio16UserEnv opened\n");
#if 0	
	/** ASOpen() etc. use the Region struct to define what part of the **/
	/** sample we should play.  Define a region covering the entire sample **/
	/** can be done by setting it to zero.  ASOpen() will fill end,etc. **/
	
	setmem(&myregion1, sizeof(struct StudioRegion), 0);
	strcpy(myregion1.samp_name,Studio_rec->file1);

	setmem(&myregion2, sizeof(struct StudioRegion), 0);
	strcpy(myregion2.samp_name,Studio_rec->file2);
		
	/** Activate the sample for playback & preload the data **/
	/** AS_AUTO_DATA means that DiskIO will handle ASRead() requests **/
	
	as1=(struct ActiveSample *)ASOpen(&myregion1, AS_PLAY|AS_AUTO_DATA, StudioBase);
	if (as1==0)
	{
		printf("Error opening sample for playback!\n");
		if (we_are_initing)
			CloseAllModules(FALSE, StudioBase);
		CloseLibrary((struct Library *)DOSBase);
		CloseLibrary(StudioBase);
		return(FALSE);
	}
	
	/** Activate the sample for playback & preload the data **/
	/** AS_AUTO_DATA means that DiskIO will handle ASRead() requests **/
	
	as2=(struct ActiveSample *)ASOpen(&myregion2, AS_PLAY|AS_AUTO_DATA, StudioBase);
	if (as2==0)
	{
		printf("Error opening sample for playback!\n");
		if (we_are_initing)
			CloseAllModules(FALSE, StudioBase);
		CloseLibrary((struct Library *)DOSBase);
		CloseLibrary(StudioBase);
		return(FALSE);
	}
	
	/** Allocate any free channel for playback on **/
	
	as1->key=(struct ChanKey *)AllocChan(NULL,-1,CK_CHAN_PLAY, StudioBase);
	key1=as1->key;
	if (key1)
		ASTrigger(as1, StudioBase);		/** start playback **/
	else
		printf("Can't Allocate a play channel!\n");
	
	/** Allocate any free channel for playback on **/
	
	as2->key=(struct ChanKey *)AllocChan(NULL,-1,CK_CHAN_PLAY, StudioBase);
	key2=as2->key;
	if (key2)
		ASTrigger(as2, StudioBase);		/** start playback **/
	else
		printf("Can't Allocate a play channel!\n");
	
	/** ASClose() will wait untill the sample is finished playing **/
	/** before it frees the buffers, etc. **/
	
	rate=as1->region.parms.rate;

	ASClose(as1, StudioBase);		/** wait for it to finish & close it **/
	ASClose(as2, StudioBase);		/** wait for it to finish & close it **/

	Delay((50*6144)/rate);	/** AD1012Handler bug (wait for card buffers to empty) **/

	FreeChan(key1, StudioBase);
	FreeChan(key2, StudioBase);
#endif
	if (we_are_initing)
		CloseAllModules(FALSE, StudioBase);

	//CloseLibrary(StudioBase);
	CloseLibrary((struct Library *)DOSBase);

	return(TRUE);
}

/******** StopSample() ********/
	
void StopSample(struct Studio_record *Studio_rec, struct ActiveSample *asList[])
{
}

/******** E O F ********/
