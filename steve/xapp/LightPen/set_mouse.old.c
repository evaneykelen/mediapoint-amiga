//	File		:	set_mouse.c
//	Uses		:
//	Date		:	26-04-93
//	Autor		:	Devices ( P. 111 )
//	Desc.		:	Set the mouse to a certain position using the input device
//

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <stdio.h>

#ifdef LATTICE
int CXBRK( void ) { return( 0 ); }
int ckabort( void ) { return( 0 );}
#endif

struct IntuitionBase *IntuitionBase;

void set_mouse( long x, long y )
{
	struct IOStdReq	*InputIO;
	struct MsgPort		*InputMP;
	struct InputEvent	*FakeEvent;
	struct IEPointerPixel	*NewPixel;
	struct Screen *PubScreen;

	if( InputMP = CreateMsgPort() )
	{
		if((FakeEvent = (struct InputEvent *)AllocMem( sizeof( struct InputEvent ),MEMF_PUBLIC ) ) &&
			(NewPixel = (struct IEPointerPixel * )AllocMem( sizeof( struct IEPointerPixel ),MEMF_PUBLIC ) ) )
		{
			if( InputIO = CreateIORequest( InputMP,sizeof( struct IOStdReq )))
			{
				if( !OpenDevice("input.device",NULL,(struct IORequest *)InputIO, NULL ) )
				{
					if( IntuitionBase = ( struct IntuitionBase *)OpenLibrary("intuition.library",36L ) )
					{
						if( PubScreen = ( struct Screen *)LockPubScreen( NULL ) )
						{
							NewPixel->iepp_Screen = (struct Screen *)PubScreen;
							NewPixel->iepp_Position.X = x;
							NewPixel->iepp_Position.Y = y;
							FakeEvent->ie_EventAddress = (APTR)NewPixel;
							FakeEvent->ie_NextEvent = NULL;
							FakeEvent->ie_Class = IECLASS_NEWPOINTERPOS;
							FakeEvent->ie_SubClass = IESUBCLASS_PIXEL;
							FakeEvent->ie_Code = IECODE_NOBUTTON;
							FakeEvent->ie_Qualifier = NULL;

							InputIO->io_Data = (APTR)FakeEvent;
							InputIO->io_Length = sizeof( struct InputEvent );
							InputIO->io_Command = IND_WRITEEVENT;
							DoIO( ( struct IORequest *)InputIO );
							UnlockPubScreen( NULL, PubScreen );
						}
						else
							printf("No screen pointer\n");
						CloseLibrary( (struct Library * )IntuitionBase );
					}
					else
						printf("No intuition \n");
					CloseDevice( ( struct IORequest *)InputIO );
				}
				else
					printf("No input.device\n");
				DeleteIORequest( InputIO );
			}
			else
				printf("no io request\n");
			FreeMem( FakeEvent, sizeof( struct InputEvent ) );
			FreeMem( NewPixel, sizeof( struct IEPointerPixel ) );
		}
		else
			printf("no memory\n");
		DeleteMsgPort( InputMP );
	}
	else
		printf("no msgport\n");
}
