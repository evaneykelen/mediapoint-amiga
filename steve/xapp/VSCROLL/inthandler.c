//	File		: inthandler.c
//	Uses		: inthandler.h
//	Date		: -1993
//	Author	: S. Vanderhorst & C. Lieshout
//	Desc.		: Setup a interrupt handler
//
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <exec/interrupts.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <libraries/dosextens.h>

#include "inthandler.h"

#define ERR_WORKER 1
#define NO_ERROR 0

void RemoveInputHandler( OBJECT_INPUT_STRUCT *oi )
{
	if( oi->IRB != NULL )
	{	
		oi->IRB->io_Command = IND_REMHANDLER;
		oi->IRB->io_Data = (APTR)&oi->Int_IEWP;
		DoIO( (struct IORequest *)oi->IRB);
	}
}

void FreeInputHandler(	OBJECT_INPUT_STRUCT *oi )
{
	if( oi->IRB != NULL)
	{	
		if( oi->IDErr == 0 )
    		CloseDevice( ( struct IORequest * )oi->IRB );
		DeleteStdIO( oi->IRB );
	}
	if( oi->SigNum_NO != -1 )
		FreeSignal( oi->SigNum_NO );
	if( oi->Port_IDtoIEI != NULL )
		DeletePort( oi->Port_IDtoIEI );
}

BOOL AddInputHandler(	OBJECT_INPUT_STRUCT *oi )
{
	// set up input event server interrupt

	oi->Int_IEWP.is_Data = (APTR)&oi->WPEventData;
	oi->Int_IEWP.is_Code = (void *)Int_IEWorkPageServer;
	oi->Int_IEWP.is_Node.ln_Pri = 53;
	oi->Int_IEWP.is_Node.ln_Name = "Int_IEWorkPage";
	oi->IRB->io_Command = IND_ADDHANDLER;
	oi->IRB->io_Data = (APTR)&oi->Int_IEWP;
	DoIO( (struct IORequest * )oi->IRB );

	return(TRUE);
}

int SetupInputHandler( OBJECT_INPUT_STRUCT *oi )
{

	oi->SigNum_NO = -1;
	oi->Port_IDtoIEI = NULL;
	oi->IRB = NULL;
	oi->IDErr = TRUE;

	if( ( oi->SigNum_NO = AllocSignal(-1) ) == -1 )
		return( ERR_WORKER );
	if(((oi->Port_IDtoIEI = CreatePort(0,0)) == NULL) )
		return( ERR_WORKER );
	if((oi->IRB = (struct IOStdReq *)CreateStdIO( oi->Port_IDtoIEI)) == NULL)
		return( ERR_WORKER );
	if((oi->IDErr = OpenDevice("input.device",0,(struct IORequest *)oi->IRB,0)) != 0)
		return( ERR_WORKER );

	oi->WPEventData.ed_Task = ( struct Task * )FindTask( NULL );
	oi->WPEventData.ed_Sig_NextObject = 1<<( oi->SigNum_NO );
	return( NO_ERROR );
}

struct InputEvent *Int_IEWorkPageHandler(	struct InputEvent *InEvent,
														WPEVENTDATA *WPEventData )
{
	UWORD KeyCode,check;
	struct InputEvent *EventList;

	EventList = InEvent;
	check = 0;
	while(InEvent)
	{
		switch(InEvent->ie_Class)
		{
	    	case IECLASS_RAWKEY:
	    		KeyCode = (UWORD)InEvent->ie_Code;
				switch(KeyCode)
				{
					case 0x45:	// ESC
							Signal(WPEventData->ed_Task,WPEventData->ed_Sig_NextObject);
							check = 1;
							break;
				}
				break;
	    	case IECLASS_RAWMOUSE:
			    KeyCode = (UWORD)InEvent->ie_Code;

				switch(KeyCode)
				{
					case IECODE_LBUTTON:
							Signal(WPEventData->ed_Task,WPEventData->ed_Sig_NextObject);
							check = 1;
							break;
				}
				break;
			default:
				break;
		}

		InEvent = InEvent->ie_NextEvent;
	}

	if( check )
		return( 0 );
	else
		return(EventList);
}
