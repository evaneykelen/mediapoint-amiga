#include "nb:pre.h"

/**** defines ****/

#define MILLION	1000000

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct Library *medialinkLibBase;
extern struct Process *process;
extern struct ExecBase *SysBase;
extern ULONG allocFlags;
extern struct Screen *scriptScreen;
extern struct Window *scriptWindow;
extern struct MsgPort *capsPort;
extern struct MsgPort *broker_mp;
extern struct Library *CxBase;

/**** globals ****/

static struct Task *ClockTask;
static TEXT datetimeStr[32];
static struct Window *clockWindow;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** startClockTask() ********/

BOOL startClockTask(void)
{
int d,month,y;
struct NewWindow NewWindowStructure;

	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(scriptWindow->RPort, largeFont);

	NewWindowStructure.LeftEdge			= 454;
	NewWindowStructure.Width				= 76;
	NewWindowStructure.TopEdge			= scriptWindow->TopEdge+
																		scriptWindow->Height-
																		scriptWindow->RPort->TxHeight+1;
	NewWindowStructure.Height				= scriptWindow->RPort->TxHeight-1;
	NewWindowStructure.DetailPen		= 0;
	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= 0;
	NewWindowStructure.Flags				= WFLG_NOCAREREFRESH | WFLG_BORDERLESS | WFLG_SIMPLE_REFRESH;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen				= scriptScreen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	clockWindow = (struct Window *)OpenWindow(&NewWindowStructure);
	if (clockWindow==NULL)
	{
		UA_WarnUser(122);
		return(FALSE);
	}

	SetDrMd(clockWindow->RPort, JAM1);

	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(clockWindow->RPort, largeFont);
	else
		SetFont(clockWindow->RPort, smallFont);

	for(d=0; d<32; d++)
		datetimeStr[d] = ' ';
	SystemDate(&d, &month, &y);
	DayMonthYearToString(datetimeStr, d, month, y);
	SetAPen(scriptWindow->RPort, HI_PEN);
	SetDrMd(scriptWindow->RPort, JAM1);
	Move(	scriptWindow->RPort, 541,
				scriptWindow->Height - scriptWindow->RPort->TxHeight + scriptWindow->RPort->TxBaseline + 1);
	Text(scriptWindow->RPort, datetimeStr, strlen(datetimeStr));

	ClockTask = (struct Task *)CreateTask(ML_CLOCK_TASK,0,ClockServer,4096);
	if (ClockTask==NULL)
	{
		UA_WarnUser(123);
		CloseWindow(clockWindow);
		return(FALSE);
	}

	/* Wait for ringback signal. */

	Wait(SIGBREAKF_CTRL_C);

	/* clock server has `died'. */

	if(!ClockTask)
	{
		UA_WarnUser(124);
		CloseWindow(clockWindow);
		return(FALSE);
	}

	SetBit(&allocFlags, CLOCK_FLAG);

	SetFont(scriptWindow->RPort, smallFont);

	return(TRUE);
}

/******** stopClockTask() ********/

void stopClockTask(void)
{
	if( TestBit(allocFlags, CLOCK_FLAG) )
	{
		UnSetBit(&allocFlags, CLOCK_FLAG);
		Signal(ClockTask, SIGBREAKF_CTRL_C);
		Wait(SIGBREAKF_CTRL_C);
		CloseWindow(clockWindow);
	}
}

/******** ClockServer() ********/

VOID __saveds ClockServer(void)
{
struct timerequest *TimeRequest;
struct MsgPort *TimePort;
BYTE KeepGoing = TRUE;
LONG ThisHour, ThisMinute;
struct RastPort rp_back, rp_high;
CxMsg *msg;	// broker msg

	CopyMem(clockWindow->RPort, &rp_back, sizeof(struct RastPort));
	CopyMem(clockWindow->RPort, &rp_high, sizeof(struct RastPort));

	// SetAPen seems to be a pretty hefty time-consuming function, that's why...

	SetAPen(&rp_back, BGND_PEN);
	SetAPen(&rp_high, HI_PEN);

	/**** Create a timer device request ****/

	if(TimePort = (struct MsgPort *)CreatePort(ML_CLOCK_NAME,0L))
	{
		if(TimeRequest = (struct timerequest *)CreateExtIO(TimePort,sizeof(struct timerequest)))
		{
			if(!OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)TimeRequest,0))
			{
				/**** Signal our father process that we're running ****/

				Signal((struct Task *)process,SIGBREAKF_CTRL_C);

				/**** Keep on displaying ****/

				while(KeepGoing)
				{
					/**** Are we to quit? ****/

					if(SetSignal(0,0) & SIGBREAKF_CTRL_C)
					{
						KeepGoing = FALSE;
						SetSignal(0,SIGBREAKF_CTRL_C);
					}

					/**** Get the current time ****/

					TimeRequest->tr_node.io_Command = TR_GETSYSTIME;
					DoIO((struct IORequest *)TimeRequest);

					/**** Print the current time ****/

					ThisHour   = (TimeRequest->tr_time.tv_secs % 86400) / 3600;
					ThisMinute = (TimeRequest->tr_time.tv_secs % 3600) / 60;

					/**** print time ****/

					HoursMinsSecsTenthsToString(datetimeStr, ThisHour,ThisMinute,TimeRequest->tr_time.tv_secs % 60, 0);
					datetimeStr[8] = '\0';
					RectFill(&rp_back, 0, 0, 75, clockWindow->Height-1);
					Move(&rp_high, 0, clockWindow->RPort->TxBaseline);
					Text(&rp_high, datetimeStr, strlen(datetimeStr));

					/**** Wait a second ****/

					if(KeepGoing)
					{
						TimeRequest->tr_node.io_Command	= TR_ADDREQUEST;
						TimeRequest->tr_time.tv_secs	= 0;
						TimeRequest->tr_time.tv_micro	= MILLION/4;
						DoIO((struct IORequest *)TimeRequest);
					}

					/**** free any messages that the broker send ****/

					if ( CxBase!=NULL )
					{
						while(msg = (CxMsg *)GetMsg(broker_mp))
							ReplyMsg((struct Message *)msg);
					}
				}
				CloseDevice((struct IORequest *)TimeRequest);
			}
			DeleteExtIO((struct IORequest *)TimeRequest);
		}
		DeletePort(TimePort);
	}

	/**** Signal the father process that we're done ****/
	/**** and quietly remove ourselves ****/

	Forbid();
	Signal((struct Task *)process,SIGBREAKF_CTRL_C);
	ClockTask = NULL;
	RemTask(SysBase -> ThisTask);
}

/******** E O F ********/
