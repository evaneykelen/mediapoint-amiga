/********************************************************
*File : input.c
*/
#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
#include <intuition/intuitionbase.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <resources/potgo.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/keymap_protos.h>
#include <clib/potgo_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "minc:types.h"
#include "minc:defs.h"
#include "minc:errors.h"
#include "minc:process.h"
#include "minc:ge.h"
#include "minc:system.h"
#include "external.h"
#include "nb:capsstructs.h"

#define LIGHTPEN_ON FALSE

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

#define MILLION 1000000

// currently, the highest key on the keyboard is 0x67 
#define	MAX_RAWKEYS 0x68

#define _PRINTF FALSE

extern void *Int_GlobEProcServer();

#define _ON_ 	TRUE
#define _OFF_	FALSE

#define OUTRY 1L << 15
#define DATRY 1L << 14
#define OUTRX 1L << 13
#define DATRX 1L << 12
extern struct Custom far custom;

UWORD *emptySprite=NULL;
STATIC void SwitchSprite(EVENTDATA *GEventData, BOOL state);
extern BYTE CreateBeep(void);
extern VOID DeleteBeep(void);
extern VOID Beep(int,int);
extern void InterpretAssignment(STRPTR, MLSYSTEM *);

void place_button( WORD *arguments, MLSYSTEM *mlsystem );

void RemapKeyPad(UWORD *);
MOUSEJUMPREC *FollowMouse(EVENTDATA *GEventData, struct InputEvent *ie, BOOL *immediate);
void KillScreenButtons(EVENTDATA *GED);

/******************************************************
*Func : See if the mouse pointer is currently on a screen 
*		button
*int  : GED
*out  : NULL -> There is no screen button under
*				the mouse pointer.
*		else -> ptr to mouse jump rec
*/
MOUSEJUMPREC *FindScreenButton(EVENTDATA *GED, BOOL ForFollowMouse, BOOL *immediate)
{
  int i;
  MOUSEJUMPREC *ME_JumpList;
  WORD MouseX, MouseY;
  struct View *view;
  struct ViewPort *vp;
  struct Screen *screen;
  struct RendezVousRecord *rvrec;

	if ( !GED->temp3[2] )
		return(NULL);

	if ( !GED->temp3[3] )
		return(NULL);
	GED->temp3[3] = FALSE;

	// NEW -- DO NOT ACCEPT MOUSE/KEY EVENTS UNTIL PAGE IS DONE WITH RENDERING
	rvrec = (struct RendezVousRecord *)GED->miscPtr;
	if ( rvrec->capsprefs->mousePointer & 128 )
	{
		if ( !AttemptSemaphore( &GED->mlsystem->ms_Sema_Transition ) )
		{
			GED->temp3[3] = TRUE;
			return(NULL);
		}
		else
			ReleaseSemaphore( &GED->mlsystem->ms_Sema_Transition );
	}
	// NEW -- DO NOT ACCEPT MOUSE/KEY EVENTS UNTIL PAGE IS DONE WITH RENDERING

	i = 0;
	ME_JumpList = GED->ed_ME_LocJumpList;
	screen = (struct Screen *)GED->temp2;
	MouseX = screen->MouseX;
	MouseY = screen->MouseY;

	Forbid();
	view = GED->ed_GfxBase->ActiView;
	Permit();
	vp = view->ViewPort;

	if ( ((screen->Width >= 400) || vp->Modes & HIRES) && (screen->Width < 640) )
		MouseX *= 2;

	if ( (screen->Height < 400) && ( vp->Modes & LACE || vp->DHeight>=400 ) )
		MouseY *= 2;

	MouseX -= vp->DxOffset;
	MouseY -= vp->DyOffset;

	while(ME_JumpList[i].mj_Width)
	{
		if( ME_JumpList[i].mj_B_Used &&
			(MouseX >= ME_JumpList[i].mj_X) && 
			(MouseX <= (ME_JumpList[i].mj_X + ME_JumpList[i].mj_Width)) && 
			(MouseY >= ME_JumpList[i].mj_Y) &&
			(MouseY <= (ME_JumpList[i].mj_Y + ME_JumpList[i].mj_Height)) 
		  )
		{
			// button found - now see if auto select is on (only for mouse follow func)

			if (ForFollowMouse)
			{
				if ( immediate && (ME_JumpList[i].mj_RenderType & RENDERTYPE_IMMEDIATE) )
					*immediate = TRUE;

				if ( !(ME_JumpList[i].mj_RenderType & RENDERTYPE_AUTO) )	// auto == follow
				{
					GED->temp3[3] = TRUE;
					return(NULL);
				}
			}
			else
				KillScreenButtons(GED);

			GED->temp3[3] = TRUE;
			return(&ME_JumpList[i]);
		}	

		i++;
	}

	GED->temp3[3] = TRUE;
	return(NULL);
}

/******************************************************
*Func : Kill button list
*int  :
*out  :
*		
*/
void KillScreenButtons(EVENTDATA *GED)
{
  int i;
  MOUSEJUMPREC *ME_JumpList;

	Forbid();
	GED->temp3[2] = FALSE;
	i = 0;
	ME_JumpList = GED->ed_ME_LocJumpList;
	if ( ME_JumpList )
	{
		while(ME_JumpList[i].mj_Width)
		{
			ME_JumpList[i].mj_B_Used = FALSE;
			i++;
		}
	}
	Permit();
}

/******************************************************
*Func : Look up which MOUSEJUMPREC is associated with this key
*int  : GED
*out  : NULL -> This key is not in the list
*		else -> ptr to mouse jump rec
*/
MOUSEJUMPREC *FindKeyButton(EVENTDATA *GED, int rawkeyCode)
{
  int i;
  MOUSEJUMPREC *ME_JumpList;
  int raw;
  struct RendezVousRecord *rvrec;

	// NEW -- DO NOT ACCEPT MOUSE/KEY EVENTS UNTIL PAGE IS DONE WITH RENDERING
	rvrec = (struct RendezVousRecord *)GED->miscPtr;
	if ( rvrec->capsprefs->mousePointer & 128 )
	{
		if ( !AttemptSemaphore( &GED->mlsystem->ms_Sema_Transition ) )
			return(NULL);
		else
			ReleaseSemaphore( &GED->mlsystem->ms_Sema_Transition );
	}
	// NEW -- DO NOT ACCEPT MOUSE/KEY EVENTS UNTIL PAGE IS DONE WITH RENDERING

	i = 0;
	ME_JumpList = GED->ed_ME_LocJumpList;

	while(ME_JumpList[i].mj_Width)
	{
		if ( ME_JumpList[i].mj_keyCode != -1 )
			raw = ASCIIToRawKey( GED, ME_JumpList[i].mj_keyCode );
		else
			raw = ME_JumpList[i].mj_rawkeyCode;

		if ( raw == rawkeyCode )
		{
			KillScreenButtons(GED);
			return(&ME_JumpList[i]);
		}

		i++;
	}

	return(NULL);
}

/******************************************************
*Func : 
*int  : 
*out  : 
*		
*/
void SendButtonHilight( MLSYSTEM *mlsystem, MOUSEJUMPREC *mj, BOOL forFollow )
{
WORD numericalArgs[10];

	// MAKE SOUND...

	if ( forFollow && mj->mj_AudioCue>=1 )
		Beep(20000,2);
	else
	{
		if ( mj->mj_AudioCue == 1)		// beep
			Beep(25000,1);
		else if ( mj->mj_AudioCue == 2)	// true
			Beep(20000,1);
		else if ( mj->mj_AudioCue == 3)	// false
			Beep(7000,1);
	}

	// ...AND RENDER THINGS ON SCREEN...

	if (	( mj->mj_RenderType & RENDERTYPE_INVERT ) ||
				( mj->mj_RenderType & RENDERTYPE_BOX ) ||
				( mj->mj_RenderType & RENDERTYPE_FATBOX ) )
	{
		numericalArgs[0] = mj->mj_RenderType;
		numericalArgs[1] = mj->mj_X;
		numericalArgs[2] = mj->mj_Y;
		numericalArgs[3] = mj->mj_Width;
		numericalArgs[4] = mj->mj_Height;
		numericalArgs[5] = 0;	// color
		numericalArgs[6] = 5;	// fat line weight
		numericalArgs[7] = 3;	// delay
		numericalArgs[8] = 0;	// # blinks
		if ( mj->mj_RenderType & RENDERTYPE_STAY )
			numericalArgs[9] = FALSE;
		else
			numericalArgs[9] = TRUE;

		// attempt to get screen semaphore
		if ( AttemptSemaphore( &mlsystem->ms_Sema_Transition ) )
		{
			place_button( numericalArgs, mlsystem );
			ReleaseSemaphore( &mlsystem->ms_Sema_Transition );
		}
	}
}

/**************************************************
*Func : Convert a ASCII code into a rawkey code
*in   : ASCIICode -> 8 bit ascii code in integer format 
*out  : RawKey -> rawkey code, as from the keyboard
*/
int ASCIIToRawKey(EVENTDATA *GED, int ASCIICode)
{
UBYTE src_buffer[3];
UBYTE dst_buffer[10];
struct Library *KeymapBase;

	KeymapBase = (struct Library *)GED->ed_KeymapBase;
	if ( !KeymapBase )
		return(0);
	
	src_buffer[0] = (UBYTE)ASCIICode,
	src_buffer[1] = '\0';

	if ( MapANSI(src_buffer,1, dst_buffer,3, NULL)==1 )
		return( dst_buffer[0] );
	else
		return( 0 );
}

/*****************************************************
*Func : Set up a jumptable with SNRs for each key
*in   : EvList -> ptr to eentlist (either global or local)
*		IE_JumpList -> Ptr to rawkey to SNR conversion table 
*		ME_JumpList -> Ptr to coords to SNR converion table
*out  : TRUE -> Mouse coord Events are part of the eventlist
*		FALSE -> No mouse coord events
*/
BOOL MakeJumpList(GED, EvList, IE_JumpList, ME_JumpList)
EVENTDATA *GED;
struct ScriptEventRecord **EvList;
EVENTJUMPREC *IE_JumpList;
MOUSEJUMPREC *ME_JumpList;
{ 
  BOOL B_MouseOn;
  int i;
  struct ScriptEventRecord 	*CurSER;

	for(i = 0; i < MAX_RAWKEYS; i++)
		IE_JumpList[i].ej_LabelSNR = NULL;

	if(ME_JumpList)
		for(i = 0; i < MAX_GLOBAL_EVENTS; i++)
			ME_JumpList[i].mj_B_Used = FALSE;

	CurSER = *EvList;
	B_MouseOn = FALSE;	
	i = 0;
	while(CurSER != NULL)
	{
		if(CurSER->keyCode != -1)
		{
			// in case of a ascii represented key, convert it to rawkey
			
			// copy its ScriptNodeRecord ptr
			IE_JumpList[ASCIIToRawKey(GED,CurSER->keyCode)].ej_LabelSNR = 
			 CurSER->labelSNR;
			IE_JumpList[ASCIIToRawKey(GED,CurSER->keyCode)].ej_JumpRecType = JRT_KEYEVENT;
		}
		else
		{
			if(CurSER->rawkeyCode != -1)
			{
				IE_JumpList[CurSER->rawkeyCode].ej_LabelSNR = CurSER->labelSNR;
				IE_JumpList[CurSER->rawkeyCode].ej_JumpRecType = JRT_KEYEVENT;
			}
		}	

		if(ME_JumpList)	
		{
			if(CurSER->width)
			{	
				B_MouseOn = TRUE;
				ME_JumpList[i].mj_X = CurSER->x;
				ME_JumpList[i].mj_Y = CurSER->y;
				ME_JumpList[i].mj_Width = CurSER->width;
				ME_JumpList[i].mj_Height = CurSER->height;
				ME_JumpList[i].mj_RenderType = CurSER->renderType;
				ME_JumpList[i].mj_AudioCue = CurSER->audioCue;
				ME_JumpList[i].mj_LabelSNR = CurSER->labelSNR;
				ME_JumpList[i].mj_TypeBits = CurSER->typeBits;
				ME_JumpList[i].mj_JumpRecType = JRT_SCREENBUTTON;
				ME_JumpList[i].mj_B_Used = TRUE;

				ME_JumpList[i].mj_keyCode = CurSER->keyCode;					// ADDED ERIK
				ME_JumpList[i].mj_rawkeyCode = CurSER->rawkeyCode;				// ADDED ERIK
				if ( CurSER->assignment[0] )									// ADDED ERIK
					strcpy(ME_JumpList[i].mj_assignment, CurSER->assignment);	// ADDED ERIK
				else															// ADDED ERIK
					ME_JumpList[i].mj_assignment[0]='\0';						// ADDED ERIK

				i++;
			}
		}
		EvList++;		// next  event
		CurSER = *EvList;
	}

	if(ME_JumpList)	
		ME_JumpList[i].mj_Width = 0;

	return(B_MouseOn);
}

/**************************************************
*Func : main entry for the global event processor
*		InputEvent handling is set up here
*		The jumplist is initialised
*in   : -
*out  : Error in PI->ar_Argument.ag_RetErr
*/
void main( argc, argv)
int argc;
char **argv;
{
  int				Err_InputDev;
  PROCESSINFO		*ThisPI;
  MLSYSTEM			*MLSystem;
  GEVENTDIALOGUE 	*Msg_GEDial;	// used to send commands to the processcontroller
  GEDIALOGUE		*Msg_RGEDial;	// an message from a guide/pc
  BOOL 				GEventEnabled;	// if TRUE, events from the inputevent. 
									// interrupt are passed on to the proccontroller 
									// else they will be flushed.
  struct MsgPort	*Port_IDtoIEI,	// Port for input device
					*Port_GEtoPC,	// Port through which the global event handler
									// can reach the processcontroller.
					*RepP_GEtoPC,	// Replyport for the processcontroller after
									// it has received a message on its Port_GEtoPC.
					*Port_toGE;		// Public global event port which may be used
									// future. e.g. for global event data refresh etc 	
  ULONG 			SigR_GEtoPC,	// Signal, reply from PC 
					Sig_toGE,		// Signal, message from PC
  					SigRecvd;		// signals received
  int				SigNum_ItoGE;
  struct ScriptInfoRecord	*SIR;
  MOUSEJUMPREC		*ME_LocJumpList;
  EVENTJUMPREC		*IE_GlobJumpList,// the rawkeycode is the index for this array 
					*IE_LocJumpList;
  EVENTDATA 		GEventData; 	// Data field for both the Int_GlobEProc and 
									// the GEventTask
  struct IOStdReq 	*InputRequestBlock;
  struct Interrupt 	Int_GlobEProc;
  struct Custom 	*ge_Custom;
  /**** access to MP public port ****/
  struct MsgPort *port;
  struct Node *node;
  struct List *list;
  struct RendezVousRecord *rvrec;
  struct PotgoBase *PotgoBase;
  ULONG potbits;
  struct Library *medialinkLibBase;

	ge_Custom = (struct Custom *)0xdff000;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr( argc, argv)) == NULL)
		return;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;
	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	GEventData.temp1 = NULL;	// NEW - used to store last followed window
	GEventData.temp2 = (LONG)rvrec->pagescreen;	// NEW - used to store pointer to playScreen

	GEventData.miscPtr = rvrec;	// NEW

	SIR = ThisPI->pi_Arguments.ar_Module.am_SIR;
    MLSystem = ThisPI->pi_Arguments.ar_Module.am_MLSystem;

	GEventData.mlsystem = MLSystem;	// NEW

	GEventData.ed_JumpRec = NULL;
	GEventData.ed_Task = ThisPI->pi_Process;

	GEventData.ed_IntuitionBase = NULL;
	GEventData.ed_GfxBase = NULL;
	Msg_GEDial = NULL;
	IE_GlobJumpList = NULL;
	ME_LocJumpList = NULL;
	IE_LocJumpList = NULL;
	Port_toGE = NULL;
	RepP_GEtoPC = NULL;
	SigNum_ItoGE = -1;
	Port_GEtoPC = NULL;
	Port_IDtoIEI = NULL;
	InputRequestBlock = NULL;
	Err_InputDev = TRUE;

	if( 
		((GEventData.ed_IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0)) == NULL) ||	
		((GEventData.ed_GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0)) == NULL) ||	
		((GEventData.ed_KeymapBase = (struct Library *)OpenLibrary("keymap.library",0)) == NULL) ||	
		((Msg_GEDial = (GEVENTDIALOGUE *)AllocMem(sizeof(GEVENTDIALOGUE),MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((IE_GlobJumpList = (EVENTJUMPREC *)AllocMem(MAX_RAWKEYS * sizeof(EVENTJUMPREC),MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((ME_LocJumpList = (MOUSEJUMPREC *)AllocMem(MAX_GLOBAL_EVENTS * sizeof(MOUSEJUMPREC),MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((IE_LocJumpList = (EVENTJUMPREC *)AllocMem(MAX_RAWKEYS * sizeof(EVENTJUMPREC),MEMF_PUBLIC|MEMF_CLEAR)) == NULL) ||
		((Port_toGE = (struct MsgPort *)CreatePort("Port_GlobEProc",0)) == NULL) ||
		((RepP_GEtoPC = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((SigNum_ItoGE = AllocSignal(-1)) == -1) ||
		((Port_GEtoPC = (struct MsgPort *)FindPort("Port_GEtoPC")) == NULL) ||
    	((Port_IDtoIEI = CreatePort("Port_IDtoIEI",0)) == NULL) ||
    	((InputRequestBlock = (struct IOStdReq *)CreateStdIO(Port_IDtoIEI)) == NULL) ||
	    ((Err_InputDev = OpenDevice("input.device",0,InputRequestBlock,0)) != 0) 
	  )
	{
		if(Err_InputDev == 0)
			CloseDevice(InputRequestBlock);
		if(InputRequestBlock)
			DeleteStdIO(InputRequestBlock);
		if(Port_IDtoIEI)
			DeletePort(Port_IDtoIEI);
		if(SigNum_ItoGE != -1)
			FreeSignal(SigNum_ItoGE);
		if(RepP_GEtoPC)
			DeletePort(RepP_GEtoPC);
		if(Port_toGE)
			DeletePort(Port_toGE);
		if(IE_LocJumpList)
			FreeMem(IE_LocJumpList, MAX_RAWKEYS * sizeof(EVENTJUMPREC));
		if(ME_LocJumpList)
			FreeMem(ME_LocJumpList, MAX_GLOBAL_EVENTS * sizeof(MOUSEJUMPREC));
		if(IE_GlobJumpList)
			FreeMem(IE_GlobJumpList, MAX_RAWKEYS * sizeof(EVENTJUMPREC));
		if(Msg_GEDial)
			FreeMem(Msg_GEDial,sizeof(GEVENTDIALOGUE));
		if(GEventData.ed_IntuitionBase)
			CloseLibrary(GEventData.ed_IntuitionBase);
		if(GEventData.ed_GfxBase)
			CloseLibrary(GEventData.ed_GfxBase);
		if(GEventData.ed_KeymapBase)
			CloseLibrary(GEventData.ed_KeymapBase);
 		ThisPI->pi_Arguments.ar_RetErr = ERR_INPUTHANDLER;
		return;
	}

	// set system signals
	Sig_toGE = 1 << Port_toGE->mp_SigBit;			// from outer world to us
	SigR_GEtoPC = 1 << RepP_GEtoPC->mp_SigBit;		// replyport to us
	GEventData.ed_Sig_ItoGE = 1 << SigNum_ItoGE;	// event inputhandler to main

	// set jumplist data
	GEventData.ed_IE_GlobJumpList = IE_GlobJumpList;
    GEventData.ed_ME_LocJumpList = ME_LocJumpList;
	GEventData.ed_IE_LocJumpList = IE_LocJumpList;

	// mouse/ keyboard check flags

	GEventData.ed_ControlBits = CBB_ESCAPE;
	if ( rvrec->capsprefs->mousePointer & 1 )		// cursor
		GEventData.ed_ControlBits |= CBB_CURSOR;
	if ( rvrec->capsprefs->mousePointer & 2 )		// mouse
		GEventData.ed_ControlBits |= CBB_MOUSE;
	if ( rvrec->capsprefs->mousePointer & 4 )		// cursor+mouse
		GEventData.ed_ControlBits |= (CBB_CURSOR|CBB_MOUSE);

	GEventData.ed_B_LJLUsed = FALSE;
	GEventData.ed_B_ProcessEvent = FALSE;
	GEventData.ed_B_MouseOn = FALSE;

	GEventData.temp3[0] = 0L;		// used
	GEventData.temp3[1] = TRUE;		// used
	GEventData.temp3[2] = FALSE;	// used 	
	GEventData.temp3[3] = TRUE;

	emptySprite = (UWORD *)AllocMem(20L,MEMF_CHIP|MEMF_CLEAR);
	if ( !emptySprite )
		return;

	if ( rvrec->capsprefs->mousePointer & 16 )
		SwitchSprite(&GEventData,_ON_);

	//if ( (rvrec->capsprefs->mousePointer & 16) || (rvrec->capsprefs->mousePointer & 64) )	// show sprite
	//	SwitchSprite(&GEventData,_ON_);
	//else
	//	SwitchSprite(&GEventData,_OFF_);

	// INIT AUDIO - START ADDED ERIK
	if ( !CreateBeep() )
		return;
	// INIT AUDIO - END ADDED ERIK

	MakeJumpList(&GEventData, SIR->globallocalEvents, IE_GlobJumpList, NULL);

	// set up input event server interrupt
    Int_GlobEProc.is_Data = (APTR)&GEventData;
    Int_GlobEProc.is_Code = (void *)Int_GlobEProcServer;
    Int_GlobEProc.is_Node.ln_Pri = 52;
    Int_GlobEProc.is_Node.ln_Name = "Int_GlobEProc";
    InputRequestBlock->io_Command = IND_ADDHANDLER;
    InputRequestBlock->io_Data = (APTR)&Int_GlobEProc;
    DoIO(InputRequestBlock);

	// Init gameport

	PotgoBase=(struct PotgoBase *)OpenResource(POTGONAME);
	if ( !PotgoBase )
		return;
	potbits = AllocPotBits(OUTRY | DATRY | OUTRX | DATRX );
	if ( potbits != (OUTRY | DATRY | OUTRX | DATRX ) )
	{
		FreePotBits(potbits);
		return;
	}
	WritePotgo(0xFFFFFFFFL,potbits);

	// start processing input events	

	GEventData.ed_B_ProcessEvent = TRUE;
	GEventEnabled = TRUE;
	SigRecvd = 0;

	while( !(SigRecvd & SIGF_ABORT))
	{
		SigRecvd = Wait(SIGF_ABORT | Sig_toGE | SigR_GEtoPC | GEventData.ed_Sig_ItoGE);

		// signal from InputEvent interrupt ?
		if(GEventEnabled && (SigRecvd & GEventData.ed_Sig_ItoGE))
		{
			if( GEventData.ed_JumpRec && GEventData.ed_Cmd == RCMDNONE )
			{
				if ( (((MOUSEJUMPREC *)GEventData.ed_JumpRec)->mj_JumpRecType == JRT_SCREENBUTTON) )
						SendButtonHilight( MLSystem, (MOUSEJUMPREC *)GEventData.ed_JumpRec, TRUE );
			}
			else
			{
				Msg_GEDial->gd_Msg.mn_Node.ln_Type = NT_MESSAGE;
				Msg_GEDial->gd_Msg.mn_Length = sizeof(GEVENTDIALOGUE);
				Msg_GEDial->gd_Msg.mn_ReplyPort = RepP_GEtoPC;

				// set other values like new ej_LabelSNR here
				Msg_GEDial->gd_Cmd = GEventData.ed_Cmd;		
				// whereto should de pc jump, if needed

				if(GEventData.ed_Cmd == RCMDSTOPnJUMP)
				{
					// START NEW - ADDED ERIK
					if(GEventData.ed_JumpRec)
					{
						Msg_GEDial->gd_Label.gl_SNR = ((EVENTJUMPREC *)GEventData.ed_JumpRec)->ej_LabelSNR;
						if(Msg_GEDial->gd_Label.gl_SNR->nodeType == TALK_STARTSER)
							Msg_GEDial->gd_Label.gl_SNR = (SNR *)Msg_GEDial->gd_Label.gl_SNR->list;
						// START NEW
						if ( rvrec->capsprefs->mousePointer & 64 )	// auto sprite
							SwitchSprite(&GEventData,_OFF_);
						// END NEW
					}
					else
					{
						Msg_GEDial->gd_Label.gl_SNR = NULL;
					}
					// END NEW - ADDED ERIK
				}
				else	
				{
					Msg_GEDial->gd_Label.gl_SNR = NULL;
				}

				if(GEventData.ed_JumpRec != NULL)
				{
					if ( ((MOUSEJUMPREC *)GEventData.ed_JumpRec)->mj_JumpRecType == JRT_SCREENBUTTON )
					{
						// CALCULATE VARIABLES
						if ( ((MOUSEJUMPREC *)GEventData.ed_JumpRec)->mj_assignment[0] )
						{
							InterpretAssignment(((MOUSEJUMPREC *)GEventData.ed_JumpRec)->mj_assignment,MLSystem);
						}
					}
				}
	
				// send the new information to the processcontroller
				PutMsg(Port_GEtoPC,(struct Message *)Msg_GEDial);

				if(GEventData.ed_JumpRec != NULL)
				{
					if ( (((MOUSEJUMPREC *)GEventData.ed_JumpRec)->mj_JumpRecType == JRT_SCREENBUTTON) )
					{
						// play audio sample, highlite button etc.
						if(	GEventData.ed_Cmd == RCMDSTOPnJUMP ||
								GEventData.ed_Cmd == RCMDPREVOBJ || GEventData.ed_Cmd == RCMDNEXTOBJ )
							SendButtonHilight( MLSystem, (MOUSEJUMPREC *)GEventData.ed_JumpRec, FALSE );
						//else if ( GEventData.ed_Cmd == RCMDNONE )
						//	SendButtonHilight( MLSystem, (MOUSEJUMPREC *)GEventData.ed_JumpRec, TRUE );
					}
				}

				// don't send incoming events untill a reply has been received
				GEventEnabled = FALSE;
			}
		}
		else
		{
			// replysignal from process controller ?
			if(SigRecvd & SigR_GEtoPC)
			{
				GEventEnabled = TRUE;
				// send new gp event right away
				// testing purposes only
				//GEventData.ed_Cmd = RCMDNEXTOBJ;
				//Signal(GEventData.ed_Task,GEventData.ed_Sig_ItoGE);
			}

			// dialogue from ... ?
			if(SigRecvd & Sig_toGE)
			{
				while( (Msg_RGEDial = (GEDIALOGUE *)GetMsg(Port_toGE)) != NULL)
				{
					switch(Msg_RGEDial->gd_Cmd)
					{
						case EDC_NEWLOCALEVENTS:
#if _PRINTF
							KPrintF("EDC_NEWLOCALEVENTS\n");
#endif
							GEventData.temp3[2] = FALSE;
							GEventData.ed_B_ProcessEvent = FALSE;
							GEventData.temp1 = NULL; // NEW - used to store last followed window
Forbid();	// NEW
							GEventData.ed_B_MouseOn = MakeJumpList(	&GEventData, Msg_RGEDial->gd_Luggage.gl_LocEvList,
																	IE_LocJumpList,ME_LocJumpList );
Permit();	// NEW
							GEventData.ed_B_LJLUsed = TRUE;
							GEventData.ed_B_ProcessEvent = TRUE;
							break;

						case EDC_CLRLOCALEVENTS:
#if _PRINTF
							KPrintF("EDC_CLRLOCALEVENTS\n");
#endif
							// Hide mouse
							if ( rvrec->capsprefs->mousePointer & 64 )	// auto sprite
								SwitchSprite(&GEventData,_OFF_);
							GEventData.ed_B_LJLUsed = FALSE;
							GEventData.ed_B_MouseOn = FALSE;
							KillScreenButtons(&GEventData);
							break;

						case EDC_USELOCALEVENTS:
#if _PRINTF
							KPrintF("EDC_USELOCALEVENTS\n");
#endif
							if ( GEventData.ed_B_MouseOn )
							{
								// Show mouse
								if ( rvrec->capsprefs->mousePointer & 64 )	// auto sprite
									SwitchSprite(&GEventData,_ON_);
							}
							else
							{
								// Hide mouse
								if ( rvrec->capsprefs->mousePointer & 64 )	// auto sprite
									SwitchSprite(&GEventData,_OFF_);
							}	
							GEventData.temp3[2] = TRUE;
							break;

						default:
#if _PRINTF
							KPrintF("default\n");
#endif
							if(Msg_RGEDial->gd_Cmd & CBB_SETCLR)
								GEventData.ed_ControlBits |= (Msg_RGEDial->gd_Cmd ^ CBB_SETCLR);
							else
								GEventData.ed_ControlBits &= (~Msg_RGEDial->gd_Cmd);
							break;	
					}
					// check for a reply port since we can't trust unknown xapps
					if( ((struct Message *)Msg_RGEDial)->mn_ReplyPort)
						ReplyMsg((struct Message *)Msg_RGEDial);
				}
			}

			// Clean up the Msg queue
			while(GetMsg(RepP_GEtoPC));	
		}
	}

	Forbid();
	if ( GEventData.ed_IntuitionBase->ActiveWindow )
	{
		if ( (rvrec->capsprefs->mousePointer & 16) || (rvrec->capsprefs->mousePointer & 64) )
			ClearPointer(GEventData.ed_IntuitionBase->ActiveWindow);
	}
	Permit();

	// Free gameport

	FreePotBits(potbits);

	// INIT AUDIO - START ADDED ERIK
	DeleteBeep();
	// INIT AUDIO - END ADDED ERIK

	// Do post playing killing of other programs
	// PAR
	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
	if ( medialinkLibBase )
	{
		if ( FindPort("DDR") )
			UA_IssueRexxCmd_V1("MP_Input","address DDR STOP",NULL,FALSE,0);
		CloseLibrary((struct Library *)medialinkLibBase);
	}

	// Close other stuff

	InputRequestBlock->io_Command = IND_REMHANDLER;
	InputRequestBlock->io_Data = (APTR)&Int_GlobEProc;
 	DoIO(InputRequestBlock);
	CloseDevice(InputRequestBlock);
	DeleteStdIO(InputRequestBlock);
	FreeSignal(SigNum_ItoGE);
	DeletePort(Port_IDtoIEI);
	DeletePort(RepP_GEtoPC);

	Forbid();
	// START NEW
	while( (Msg_RGEDial = (GEDIALOGUE *)GetMsg(Port_toGE)) != NULL)
	{
		if( ((struct Message *)Msg_RGEDial)->mn_ReplyPort)
		{
			ReplyMsg((struct Message *)Msg_RGEDial);
			//KPrintF("verloren berichtje\n");
		}
	}
	// END BEW
	DeletePort(Port_toGE);
	Permit();

	FreeMem(IE_LocJumpList, MAX_RAWKEYS * sizeof(EVENTJUMPREC));
	FreeMem(ME_LocJumpList, MAX_GLOBAL_EVENTS * sizeof(MOUSEJUMPREC));
	FreeMem(IE_GlobJumpList, MAX_RAWKEYS * sizeof(EVENTJUMPREC));
	FreeMem(Msg_GEDial,sizeof(GEVENTDIALOGUE));
	CloseLibrary(GEventData.ed_IntuitionBase);
	CloseLibrary(GEventData.ed_GfxBase);
	CloseLibrary(GEventData.ed_KeymapBase);
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;

	if ( emptySprite )
		FreeMem(emptySprite,20L);

	return;
}

/***************************************************
*Func : Handle the inputevent stream
*in   : InEvent -> ptr to an event
*		GEventData -> ptr to EVENTDATA struct
*out  : InEvent
*/
struct InputEvent *Int_GlobEProcHandler( InEvent, GEventData )
struct InputEvent *InEvent;
EVENTDATA *GEventData;
{
UWORD KeyCode;
UWORD Qual;
MOUSEJUMPREC *MouseJumpRec, *jr;
BOOL immediate=FALSE, passIt;
ULONG t;
struct RendezVousRecord *rvrec;
void *ed_JumpRec;

	rvrec = (struct RendezVousRecord *)GEventData->miscPtr;

	if ( rvrec->capsprefs->input_delay )	// non NULL
	{
		if ( InEvent->ie_Class==IECLASS_TIMER )
		{
			t = (InEvent->ie_TimeStamp.tv_secs * MILLION) + InEvent->ie_TimeStamp.tv_micro;
			if ( (t - GEventData->temp3[0]) > rvrec->capsprefs->input_delay )
			{
				GEventData->temp3[0] = t;
				GEventData->temp3[1] = TRUE;
			}
		}

		if (	InEvent->ie_Class==IECLASS_RAWKEY ||
					(InEvent->ie_Class==IECLASS_RAWMOUSE &&
					(InEvent->ie_Code==IECODE_LBUTTON || InEvent->ie_Code==IECODE_RBUTTON)) )
		{
			if ( !GEventData->temp3[1] && InEvent->ie_Code!=0x45 )	// immed. esc.
				return(0L);
			else
				GEventData->temp3[1]=FALSE;
		}
	}

	switch(InEvent->ie_Class)
	{
    case IECLASS_RAWKEY:
    	KeyCode = (UWORD)InEvent->ie_Code;
			Qual = InEvent->ie_Qualifier;

do_keys:
			RemapKeyPad(&KeyCode);

			// emergency exit LALT+CTRL+ESC
			if(
					KeyCode == 0x45 && 
					(Qual & (IEQUALIFIER_LALT | IEQUALIFIER_CONTROL)) ==
					(IEQUALIFIER_LALT | IEQUALIFIER_CONTROL)
			  )
			{
				if(GEventData->ed_ControlBits & CBB_ESCAPE)
				{
					GEventData->temp3[0] = 0;
					GEventData->temp3[1] = 0;
		    	InEvent->ie_Code = 0;
					InEvent->ie_Class = IECLASS_NULL;
					GEventData->ed_JumpRec = NULL;
					GEventData->ed_Cmd = RCMDESCAPE_SPECIAL;
					Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
				}
			}	
			else
			{
				if(!(KeyCode & IECODE_UP_PREFIX))
				{	
					passIt=TRUE;

					if(
						(GEventData->ed_B_LJLUsed && (GEventData->ed_IE_LocJumpList[(int)KeyCode].ej_LabelSNR != NULL)) ||
						(GEventData->ed_IE_GlobJumpList[(int)KeyCode].ej_LabelSNR != NULL)
					  )
					{
						jr=NULL;

						if(GEventData->ed_B_LJLUsed && (GEventData->ed_IE_LocJumpList[(int)KeyCode].ej_LabelSNR != NULL))
						{
							GEventData->ed_JumpRec = (void *)&GEventData->ed_IE_LocJumpList[(int)KeyCode];

							// ADDED ERIK
							if ( rvrec->capsprefs->mousePointer )
							{
								jr = FindKeyButton(GEventData, KeyCode);
								if ( jr )
									GEventData->ed_JumpRec = (void *)jr;
								else
									passIt=FALSE;

							}
							// ADDED ERIK
						}
						else
							GEventData->ed_JumpRec = (void *)&GEventData->ed_IE_GlobJumpList[(int)KeyCode];

						if ( jr && jr->mj_TypeBits & TB_PREVPAGE )
						{
							GEventData->ed_Cmd = RCMDPREVOBJ;
						}
						else if ( jr && jr->mj_TypeBits & TB_NEXTPAGE )
						{
							GEventData->ed_Cmd = RCMDNEXTOBJ;
						}
						else
						{
							GEventData->ed_Cmd = RCMDSTOPnJUMP;
						}
// ADDED KC
						if ( passIt )
// END ADDED
							Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
					}	
					else
					{
						// check special preprogrammed keys
						switch(KeyCode)
						{
							case 0x4e:	//crsr right
								if(GEventData->ed_ControlBits & CBB_CURSOR)
								{
									if(GEventData->ed_ControlBits & CBB_SHFTCRSR)
									{
										if(Qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CAPSLOCK))
										{
											GEventData->ed_Cmd = RCMDNEXTOBJ;
											GEventData->ed_JumpRec = NULL;
											Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
										}
									}
									else
									{
										GEventData->ed_Cmd = RCMDNEXTOBJ;
										GEventData->ed_JumpRec = NULL;
										Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
									}
								}
								break;
							case 0x4f:	//crsr left
								if(GEventData->ed_ControlBits & CBB_CURSOR)
								{
									if(GEventData->ed_ControlBits & CBB_SHFTCRSR)
									{
										if(Qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CAPSLOCK))
										{
											GEventData->ed_Cmd = RCMDPREVOBJ;
											GEventData->ed_JumpRec = NULL;
											Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
										}
									}
									else
									{
										GEventData->ed_Cmd = RCMDPREVOBJ;
										GEventData->ed_JumpRec = NULL;
										Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
									}
								}
								break;
							case 0x45:	//escape
								if(GEventData->ed_ControlBits & CBB_ESCAPE)
								{
									GEventData->temp3[0] = 0;
									GEventData->temp3[1] = 0;
						    	InEvent->ie_Code = 0;
									InEvent->ie_Class = IECLASS_NULL;
									GEventData->ed_JumpRec = NULL;
									GEventData->ed_Cmd = RCMDESCAPE;
									Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
								}
								break;
						}
					}	
				}
			}
			break;

	    case IECLASS_RAWMOUSE:
			// mouse buttons must be used for button interactivity control
   		KeyCode = (UWORD)InEvent->ie_Code;
do_mouse:
			if(GEventData->ed_B_MouseOn)
			{
				switch(KeyCode)
				{
					case IECODE_LBUTTON:
						if ( rvrec->capsprefs->mousePointer )
						{
							if((MouseJumpRec = FindScreenButton(GEventData,FALSE,NULL)) != NULL)
							{
								if(MouseJumpRec->mj_TypeBits & TB_PREVPAGE)
								{
									GEventData->ed_Cmd = RCMDPREVOBJ;
									// NEW - Prev&Next page also highlists button
									GEventData->ed_JumpRec = (void *)MouseJumpRec;
								}
								else										
								{
									if(MouseJumpRec->mj_TypeBits & TB_NEXTPAGE)
									{
										GEventData->ed_Cmd = RCMDNEXTOBJ;
										// NEW - Prev&Next page also highlists button
										GEventData->ed_JumpRec = (void *)MouseJumpRec;
									}
									else									
									{
										GEventData->ed_Cmd = RCMDSTOPnJUMP;
										GEventData->ed_JumpRec = (void *)MouseJumpRec;
									}
								}
								Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
							}
						}
						break;
				}
			}
			else
			{
				if(GEventData->ed_ControlBits & CBB_MOUSE)
				{
					switch(KeyCode)
					{
						case IECODE_LBUTTON:
								GEventData->ed_Cmd = RCMDPREVOBJ;
								GEventData->ed_JumpRec = NULL;
								Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
								break;
						case IECODE_RBUTTON:
								GEventData->ed_Cmd = RCMDNEXTOBJ;
								GEventData->ed_JumpRec = NULL;
								Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
								break;
					}
				}
			}
			break;

		case IECLASS_TIMER:
			Qual = 0;

			if ( TestJoystick(GEventData,InEvent,&KeyCode) )
				goto do_keys;

#if LIGHTPEN_ON
			if ( TestLightpen(GEventData,InEvent,&KeyCode) )
				goto do_mouse;
#endif

			if ( rvrec->capsprefs->mousePointer )
			{
				ed_JumpRec = (void *)FollowMouse(GEventData, InEvent, &immediate);
				if ( ed_JumpRec )
				{
					GEventData->ed_JumpRec = ed_JumpRec;
					if ( immediate )
						GEventData->ed_Cmd = RCMDSTOPnJUMP;
					else
						GEventData->ed_Cmd = RCMDNONE;
					Signal(GEventData->ed_Task,GEventData->ed_Sig_ItoGE);
				}
			}

			break;
	}

	if (	InEvent->ie_Class==IECLASS_RAWKEY ||
				(InEvent->ie_Class==IECLASS_RAWMOUSE && InEvent->ie_Code!=IECODE_NOBUTTON) )
		return(0L);
	else
		return(InEvent);
}

/******** SwitchSprite() ********/

STATIC void SwitchSprite(EVENTDATA *GEventData, BOOL state)
{
struct Window *win;

	Forbid();
	win = GEventData->ed_IntuitionBase->ActiveWindow;
	Permit();
	if ( win )
	{
		if ( state )
			ClearPointer(win);
		else if ( emptySprite )
			SetPointer(win,emptySprite,0,16,0,0);
	}
}

/******** RemapKeyPad() ********/

void RemapKeyPad(UWORD *KeyCode)
{
	if ( *KeyCode==0x5c )				// slash
		*KeyCode=0x3a;
	else if ( *KeyCode==0x3d )	// 7
		*KeyCode=0x07;
	else if ( *KeyCode==0x3e )	// 8
		*KeyCode=0x08;
	else if ( *KeyCode==0x3f )	// 9
		*KeyCode=0x09;
	else if ( *KeyCode==0x4a )	// minus
		*KeyCode=0x0b;
	else if ( *KeyCode==0x2d )	// 4
		*KeyCode=0x04;
	else if ( *KeyCode==0x2e )	// 5
		*KeyCode=0x05;
	else if ( *KeyCode==0x2f )	// 6
		*KeyCode=0x06;
	else if ( *KeyCode==0x1d )	// 1
		*KeyCode=0x01;
	else if ( *KeyCode==0x1e )	// 2
		*KeyCode=0x02;
	else if ( *KeyCode==0x1f )	// 3
		*KeyCode=0x03;
	else if ( *KeyCode==0x0f )	// 0
		*KeyCode=0x0a;
	else if ( *KeyCode==0x3c )	// period
		*KeyCode=0x39;
	else if ( *KeyCode==0x43 )	// enter
		*KeyCode=0x44;
}

/******** TestJoystick() ********/

BOOL TestJoystick(EVENTDATA *GEventData, struct InputEvent *ie, UWORD *keys)
{
struct RendezVousRecord *rvrec;
UWORD *JOY1DAT = (UWORD *)0xdff00c;
UWORD *JOY2DAT = (UWORD *)0xbfe001;
UWORD *JOY3DAT = (UWORD *)0xdff016;
ULONG t,diff;
STATIC ULONG ts = 0L;
int game=-1;

	rvrec = (struct RendezVousRecord *)GEventData->miscPtr;
  
	// skip fast if not used -- saves time!

	if ( rvrec->capsprefs->gameport_used )
	{
		if ( rvrec->capsprefs->gameport[8] && (*JOY2DAT & 0x8000) == 0x0 )
			game=8;		// FIRE 1
		else if ( rvrec->capsprefs->gameport[9] && (*JOY3DAT & 0x4000) == 0x0 )
			game=9;		// FIRE 2 (RIGHT MOUSE BUTTON IN GAMEPORT)
		else if ( rvrec->capsprefs->gameport[10] && (*JOY3DAT & 0x1000) == 0x0 )
			game=10;	// FIRE 3 (MIDDLE MOUSE BUTTON IN GAMEPORT)
		else if ( rvrec->capsprefs->gameport[1] && (*JOY1DAT & 0x0103) == 0x0103 )
			game=1;		// RIGHT UP
		else if ( rvrec->capsprefs->gameport[5] && (*JOY1DAT & 0x0301) == 0x0301 )
			game=5;		// LEFT DOWN
		else if ( rvrec->capsprefs->gameport[2] && (*JOY1DAT & 0x0003) == 0x0003 )
			game=2;		// RIGHT
		else if ( rvrec->capsprefs->gameport[3] && (*JOY1DAT & 0x0002) == 0x0002 )
			game=3;		// RIGHT DOWN
		else if ( rvrec->capsprefs->gameport[4] && (*JOY1DAT & 0x0001) == 0x0001 )
			game=4;		// DOWN
		else if ( rvrec->capsprefs->gameport[6] && (*JOY1DAT & 0x0300) == 0x0300 )
			game=6;		// LEFT
		else if ( rvrec->capsprefs->gameport[7] && (*JOY1DAT & 0x0200) == 0x0200 )
			game=7;		// LEFT UP
		else if ( rvrec->capsprefs->gameport[0] && (*JOY1DAT & 0x0100) == 0x0100 )
			game=0;		// UP
		if (game!=-1)
		{
			t = (ie->ie_TimeStamp.tv_secs * MILLION) + ie->ie_TimeStamp.tv_micro;
			diff = t - ts;
			if ( diff >= rvrec->capsprefs->gameport_delay )
			{
				ts = t;
				*keys = ASCIIToRawKey( GEventData, rvrec->capsprefs->gameport[game] );
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** FollowMouse() ********/

MOUSEJUMPREC *FollowMouse(EVENTDATA *GEventData, struct InputEvent *ie, BOOL *immediate)
{
MOUSEJUMPREC *mjr;

	// GEventData->
	//  APTR temp1;	// temporary storage used by gp:input.c
	//  LONG temp2;	// temporary storage used by gp:input.c

	mjr = FindScreenButton(GEventData,TRUE,immediate);
	if ( mjr && (mjr != GEventData->temp1) )
	{
		GEventData->temp1 = mjr;
		return( mjr );
	}
	else if ( !mjr )
		GEventData->temp1 = NULL;

	return(NULL);
}

#if LIGHTPEN_ON

/******** TestLightpen() ********/

BOOL TestLightpen(EVENTDATA *GEventData, struct InputEvent *ie, UWORD *keys)
{
struct RendezVousRecord *rvrec;
UWORD *JOY1DAT = (UWORD *)0xdff00c;
ULONG t,diff;
STATIC ULONG ts = 0L;
int light=-1;

	rvrec = (struct RendezVousRecord *)GEventData->miscPtr;

	if ( (*JOY1DAT & 0x0003) == 0x0003 )
		light = IECODE_RBUTTON;
	else if ( (*JOY1DAT & 0x0300) == 0x0300 )
		light = IECODE_LBUTTON;

	if (light!=-1)
	{
		t = (ie->ie_TimeStamp.tv_secs * MILLION) + ie->ie_TimeStamp.tv_micro;
		diff = t - ts;
		if ( diff >= rvrec->capsprefs->input_delay )
		{
			ts = t;
			*keys = light;
			return(TRUE);
		}
	}

	return(FALSE);
}

#endif

/******** E O F ********/
