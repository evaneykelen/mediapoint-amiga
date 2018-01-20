#include "nb:pre.h"
#include "protos.h"
#include "all.h"
#include "disp_def.h"

#define VERSI0N "\0$VER: 1.0"          
static UBYTE *vers = VERSI0N;

/**** MPEG display stuff (see display.c) ****/

extern int OpenDisplay( DISP_DEF *disp_def );
extern VOID CloseDisplay( DISP_DEF *disp_def );

/**** globals ****/

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;
struct Library *FreeAnimBase				= NULL;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
static struct TextAttr myta = { "topaz.font", 8, NULL, NULL };

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct all_record all_rec;
struct Window *window;
struct TextFont *tf;
WORD x1,y1,x2,y2;

	/**** open standard libraries ****/

	if ( !HostOpenLibs() )
		return;

	/**** parse .info file for special settings ****/

	GetInfoFile(argv[0], NULL, all_rec.devName, &(all_rec.portNr), &(all_rec.baudRate));
	//printf("[%s] %d %d\n",all_rec.devName,all_rec.portNr,all_rec.baudRate);

	/**** set up GUI ****/

	window = OpenHostWindow();
	if ( !window )
		return;

	tf = OpenFont(&myta);
	if (tf)
		SetFont(window->RPort,tf);

	x1 = window->BorderLeft + 3;
	y1 = window->BorderTop + 3;
	x2 = window->Width - window->BorderRight - 1 - 3; 
	y2 = window->Height - window->BorderBottom - 1 - 3; 

	SetAPen(window->RPort, 1);
	SetBPen(window->RPort, 0);
	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, x1+1, y2); Draw(window->RPort, x2, y2);
	Draw(window->RPort, x2, y1); Move(window->RPort, x2-1, y1+1);
	Draw(window->RPort, x2-1, y2-1);
	SetAPen(window->RPort, 2);
	Move(window->RPort, x1, y2); Draw(window->RPort, x1, y1);
	Draw(window->RPort, x2-1, y1); Move(window->RPort, x1+1, y1+1);
	Draw(window->RPort, x1+1, y2-1);

	SetWindowTitles(window,"MediaPoint CD Host",(UBYTE *)~0);

	/**** welcome user ****/

	PrintString(window, "Welcome to the MediaPoint CD Host.");
	PrintString(window, "© 1994 by MediaPoint International.");

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort(&all_rec) )
	{
		PrintString(window, "serial device not present or busy!");
		Delay(200L);
		CloseHostWindow(window);
		HostCloseLibs();
		return;
	}

	/**** monitor events ****/

	MonitorUser(window, &all_rec);

	/**** close the window ****/

	CloseHostWindow(window);
	if (tf)
		CloseFont(tf);

	/**** close specials libraries or devices ****/

	Close_SerialPort(&all_rec);

	/**** close all the libraries ****/

	HostCloseLibs();
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, struct all_record *all_rec)
{
struct IntuiMessage *message;
ULONG signal, WdwClass;
BOOL loop=TRUE;
struct CDTV_record *CDTV_rec = NULL;
struct CD_record *CD_rec = NULL;
struct MPEG_record *MPEG_rec = NULL;
BOOL CDTV=TRUE,CD=TRUE,MPEG=TRUE;
DISP_DEF disp_def;

	// CHECK FOR THE OLD CDTV

	if ( GfxBase->LibNode.lib_Version < 36 )
	{
		/**** CDTV ****/
	
		CDTV_rec = (struct CDTV_record *)AllocCDTV();
		if ( CDTV_rec )
		{
			if ( !OpenCDTV(CDTV_rec) )
			{
				//printf("OpenCDTV failed\n");
				CDTV=FALSE;
			}
		}
		else
		{
			//printf("AllocCDTV failed\n");
			CDTV=FALSE;
		}
	}
	else
		CDTV = FALSE;

	// CHECK FOR THE NEW CD³² and MPEG

	if ( GfxBase->LibNode.lib_Version >= 36 )
	{
		/**** CD ****/

		CD_rec = (struct CD_record *)AllocCD();
		if ( CD_rec )
		{
			if ( !OpenCD(CD_rec) )
			{
				//printf("OpenCD failed\n");
				CD=FALSE;
			}
		}
		else
		{
			//printf("AllocCD failed\n");
			CD=FALSE;
		}

		/**** MPEG ****/

		MPEG_rec = (struct MPEG_record *)AllocMPEG();
		if ( MPEG_rec )
		{
			if ( !OpenMPEG(CD_rec,MPEG_rec) )
			{
				//printf("OpenMPEG failed\n");
				MPEG=FALSE;
			}
		}
		else
		{
			//printf("AllocMPEG failed\n");
			MPEG=FALSE;
		}
	}
	else
	{
		CD = FALSE;
		MPEG = FALSE;	
	}

	/**** print which devices we can access ****/

	if ( CDTV )
		PrintString(window, "[×] CDTV");
	else
		PrintString(window, "[ ] CDTV");

	if ( CD )
		PrintString(window, "[×] CD");
	else
		PrintString(window, "[ ] CD");

	if ( MPEG )
		PrintString(window, "[×] MPEG");
	else
		PrintString(window, "[ ] MPEG");

	if ( MPEG )
	{
		Delay(200L);
		setmem(&disp_def,sizeof(DISP_DEF),0);
		disp_def.Flags	= DISP_SCREEN | DISP_NOPOINTER | DISP_COP_INT;
		disp_def.Width	= 320;
		disp_def.Height	= 400;
		disp_def.Depth	= 1;
		disp_def.ModeID	= LORESLACE_KEY;
		disp_def.ModeID &= ~MONITOR_ID_MASK;
		if ( OpenDisplay(&disp_def)!=0 )	// RC_OK
		{
			PrintString(window, "MPEG screen can't be opened!");
			Delay(200L);
			loop=FALSE;
		}
	}

	if ( !MPEG && !CD && !CDTV )
	{
		PrintString(window, "No devices available!");
		PrintString(window, "Can't accept commands.");
	}
	else
		PrintString(window, "Ready to accept commands...");

	// Free devices which couldn't be found

	if ( !CDTV )
	{
		if ( CDTV_rec )
			FreeCDTV((struct CDTV_record *)CDTV_rec);
		CDTV_rec = NULL;
	}

	if ( !CD )
	{
		if ( CD_rec )
			FreeCD((struct CD_record *)CD_rec);
		CD_rec = NULL;
	}

	if ( !MPEG )
	{
		if ( MPEG_rec )
			FreeMPEG((struct MPEG_record *)MPEG_rec);
		MPEG_rec = NULL;
	}

	/**** event handler ****/

	while(loop)
	{
		if ( CDTV || CD || MPEG )
			Monitor_SerialPort(window, all_rec, CDTV_rec, CD_rec, MPEG_rec);
		else
		{
			while(1)
			{
				signal = Wait( 1L << window->UserPort->mp_SigBit );
				if ( signal & (1L << window->UserPort->mp_SigBit) )
					break;
			}
		}

		while( message = (struct IntuiMessage *)GetMsg(window->UserPort) )
		{
			WdwClass = message->Class;
			ReplyMsg((struct Message *)message);
			if (WdwClass==IDCMP_CLOSEWINDOW)
				loop=FALSE;
		}
	}

	// Close MPEG

	if ( MPEG )
		CloseMPEG(MPEG_rec);
	if ( MPEG_rec )
		FreeMPEG((struct MPEG_record *)MPEG_rec);
	if ( MPEG )
		CloseDisplay(&disp_def);

	// Close CD

	if ( CD )
		CloseCD(CD_rec);
	if ( CD_rec )
		FreeCD((struct CD_record *)CD_rec);

	// Close CDTV

	if ( CDTV )
		CloseCDTV(CDTV_rec);
	if ( CDTV_rec )
		FreeCDTV((struct CDTV_record *)CDTV_rec);

	return(TRUE);
}

/******** HostOpenLibs() ********/

BOOL HostOpenLibs(void)
{
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
	if (IntuitionBase == NULL)
		return(FALSE);

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
	if (GfxBase == NULL)
		return(FALSE);

	FreeAnimBase = (struct Library *)OpenLibrary("freeanim.library", 0L);
	if ( FreeAnimBase )
		CloseLibrary((struct Library *)FreeAnimBase);

	return(TRUE);
}

/******** HostCloseLibs() ********/

void HostCloseLibs(void)
{
	if (IntuitionBase != NULL)
		CloseLibrary((struct Library *)IntuitionBase);

	if (GfxBase != NULL)
		CloseLibrary((struct Library *)GfxBase);
}

/******** PrintString() ********/

void PrintString(struct Window *window, STRPTR str)
{
WORD x1,y1,x2,y2;

	x1 = window->BorderLeft + 5;
	y1 = window->BorderTop + 5;
	x2 = window->Width - window->BorderRight - 1 - 5; 
	y2 = window->Height - window->BorderBottom - 1 - 5; 

	SetAPen(window->RPort, 1);
	SetBPen(window->RPort, 0);
	ScrollRaster(window->RPort, 0, window->RPort->TxHeight, x1, y1, x2, y2); 
	Move(window->RPort, x1+2, y2-(window->RPort->TxBaseline/2));
	Text(window->RPort, str, strlen(str));
}

/******** E O F ********/
