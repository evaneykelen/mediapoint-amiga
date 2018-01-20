#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "protos.h"
#include "structs.h"
#define GUI_DEFS
#include "setup.h"
#include "gen:wait50hz.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/**** functions ****/

/******** PerformActions() ********/

BOOL PerformActions(struct GVR_record *GVR_rec)
{
BOOL retval=TRUE;
char cmdstr[32], tmpstr[20]; // For concatenating a command with frame code

	/**** These command do we have ****/

	#if 0
	GVR_FAST_REWIND, GVR_FAST_FORWARD, GVR_FIND, GVR_INITIALIZE, GVR_LOOP_ON,
	GVR_LOOP_OFF, GVR_PAUSE, GVR_PLAY, GVR_PLAYTO, GVR_PLAYSEG, GVR_SCAN, GVR_SCAN_REVERSE, GVR_SEEK, GVR_SLOW,
	GVR_STEP, GVR_STOP, GVR_WAIT
	#endif

	switch( GVR_rec->cmd )
	{
    case GVR_INITIALIZE:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("INIT\r"), (APTR)"INIT\r", (UWORD)CMD_WRITE);
			break;
                     
		case GVR_PLAY:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("PLYB\r"), (APTR)"PLYB\r", (UWORD)CMD_WRITE);
			break;
                                       
		case GVR_PLAYTO:
			strcpy(cmdstr,"PLYB:");
			strcpy(tmpstr,GVR_rec->end);
			cmdstr[5]=tmpstr[0];
			cmdstr[6]=tmpstr[1];
			cmdstr[7]=tmpstr[3];
			cmdstr[8]=tmpstr[4];
			cmdstr[9]=tmpstr[6];
			cmdstr[10]=tmpstr[7];
			cmdstr[11]=tmpstr[9];
			cmdstr[12]=tmpstr[10];
			cmdstr[13]=0x0D; // Carriage Return               
			cmdstr[14]=0x00;                                             
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen(cmdstr), (APTR)cmdstr, (UWORD)CMD_WRITE);
			break;

		case GVR_PLAYSEG:
			strcpy(cmdstr,"FIND:");
			strcpy(tmpstr,GVR_rec->start);
			cmdstr[5]=tmpstr[0];
			cmdstr[6]=tmpstr[1];
			cmdstr[7]=tmpstr[3];
			cmdstr[8]=tmpstr[4];
			cmdstr[9]=tmpstr[6];
			cmdstr[10]=tmpstr[7];
			cmdstr[11]=tmpstr[9];
			cmdstr[12]=tmpstr[10];
			cmdstr[13]=0x00;                                                            
			strcat(cmdstr,";PLYB:");
			strcpy(tmpstr,GVR_rec->end);
			cmdstr[19]=tmpstr[0];
			cmdstr[20]=tmpstr[1];
			cmdstr[21]=tmpstr[3];
			cmdstr[22]=tmpstr[4];
			cmdstr[23]=tmpstr[6];
			cmdstr[24]=tmpstr[7];
			cmdstr[25]=tmpstr[9];
			cmdstr[26]=tmpstr[10];
			cmdstr[27]=0x0D; // Carriage Return               
			cmdstr[28]=0x00;                                             
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen(cmdstr), (APTR)cmdstr, (UWORD)CMD_WRITE);
			break;               

		case GVR_PAUSE:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("PAUS\r"), (APTR)"PAUS\r", (UWORD)CMD_WRITE);
			break;

		case GVR_STOP:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("STOP\r"), (APTR)"STOP\r", (UWORD)CMD_WRITE);
			break;
                        
		case GVR_FAST_FORWARD:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("FFWD\r"), (APTR)"FFWD\r", (UWORD)CMD_WRITE);
			break;
                        
		case GVR_FAST_REWIND:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("REWD\r"), (APTR)"REWD\r", (UWORD)CMD_WRITE);
			break;
                        
		case GVR_SLOW:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("SLOW\r"), (APTR)"SLOW\r", (UWORD)CMD_WRITE);
			break;
                        
		case GVR_STEP:
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen("STEP\r"), (APTR)"STEP\r", (UWORD)CMD_WRITE);
			break;

		case GVR_FIND:
			strcpy(cmdstr,"FIND:");
			strcpy(tmpstr,GVR_rec->start);
			cmdstr[5]=tmpstr[0];
			cmdstr[6]=tmpstr[1];
			cmdstr[7]=tmpstr[3];
			cmdstr[8]=tmpstr[4];
			cmdstr[9]=tmpstr[6];
			cmdstr[10]=tmpstr[7];
			cmdstr[11]=tmpstr[9];
			cmdstr[12]=tmpstr[10];
			cmdstr[13]=0x0D; // Carriage Return               
			cmdstr[14]=0x00;                                             
			retval = DoAsyncIO(GVR_rec, (ULONG)strlen(cmdstr), (APTR)cmdstr, (UWORD)CMD_WRITE);
			break;            

	}

	return(retval);
}

/******** DoAsyncIO() ********/

BOOL DoAsyncIO(	struct GVR_record *GVR_rec,
								ULONG io_Length, APTR io_Data, UWORD io_Command )
{
ULONG signal, mask, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	GVR_rec->serialIO->IOSer.io_Length 	= io_Length;
	GVR_rec->serialIO->IOSer.io_Data		= io_Data;
	GVR_rec->serialIO->IOSer.io_Command	= io_Command;
	SendIO((struct IORequest *)GVR_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum = 0;
	hz_signal = set50hz(&WJIF, 250);
	mask = (1L << GVR_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);

		if ( signal & hz_signal )
			loop=FALSE;

		if( CheckIO((struct IORequest *)GVR_rec->serialIO) )
		{
			WaitIO((struct IORequest *)GVR_rec->serialIO);
			loop=FALSE;
		}
	}

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** GetFrameCode() ********/

void GetFrameCode(struct GVR_record *GVR_rec, STRPTR retStr)
{
char cmdstr[32];

	strcpy(cmdstr,"FRAM:\r");
	DoAsyncIO(GVR_rec, (ULONG)strlen(cmdstr), (APTR)cmdstr, (UWORD)CMD_WRITE);

	strcpy(retStr, "                              ");
	DoAsyncIO(GVR_rec, 18, (APTR)retStr, (UWORD)CMD_READ);
	strcpy(retStr,&retStr[7]);
}

/******** E O F ********/
