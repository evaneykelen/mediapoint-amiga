
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <libraries/dos.h>

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include "text.h"
#include "report.h"

int CXBRK(void) { return(0); }
int chkabort(void ){return(0);}

#define WIDTH 400
#define HEIGHT 105

struct Library *IntuitionBase;
struct GfxBase *GfxBase;
struct Window *win;

void main( void )
{
	char tt[256];
	long max,count,sigs,t;
	struct MsgPort *xymp;
	struct PPMessage *msg;
	ULONG portsig, usersig, signal,winsig;
	BOOL ABORT = FALSE;

	GfxBase = ( struct GfxBase *)OpenLibrary("graphics.library",37 );
	if( GfxBase	!= NULL )
	{
		IntuitionBase = OpenLibrary("intuition.library",37 );
		if( IntuitionBase != NULL )
		{
			win = OpenWindowTags(NULL,	WA_Left, 20, WA_Top, 20,WA_Width, WIDTH,WA_Height, HEIGHT,
					WA_DragBar, TRUE,
					WA_CloseGadget, TRUE ,
					WA_IDCMP, IDCMP_CLOSEWINDOW ,
					WA_Title, "Protocol progress" ,
					TAG_DONE, 0 );

			if( win != NULL )
			{
				xymp = CreatePort("ProtPort", 0);
				if (xymp != NULL )
				{
					InitPens( );

					PrintText( win, "Protocol:",11,15 );
					TekenBorder( win, 11, 80, WIDTH-22, 15 );

					portsig = 1 << xymp->mp_SigBit;
					usersig = SIGBREAKF_CTRL_F;          /* User can break with CTRL-F */
					winsig = 1L << win->UserPort->mp_SigBit;

					sigs = portsig | usersig | winsig;

					while( !ABORT )
					{
						signal = Wait( sigs );     /* sleep 'till someone signals */

				  		if (signal & portsig)
						{
//printf("sig\n" );
							while ( msg = (struct PPMessage *)GetMsg((struct MsgPort *)xymp))
							{
//printf("do update\n");
								if( msg->pp_protocol )
									PrintText( win, msg->pp_protocol,100,15 );
								if( msg->pp_mes )
								{
									PrintText( win, msg->pp_mes,11,30 );
									ClearBox( win,13,82,WIDTH-26,11 );
								}
								if( msg->pp_count != -1 )
								{
									max = msg->pp_max;
									count = msg->pp_count;
									sprintf(tt,"Size = %ld Received = %ld    ", max, count);
									PrintText( win, tt,11,45 );
									t = ( WIDTH - 26 ) * count;
									t = t / max;
									SetBox( win,13,82,t,12 );
								}
								ReplyMsg( (struct Message *)msg );
							}
						}
						if (signal & usersig)
						{
							while (msg = (struct PPMessage *)GetMsg((struct MsgPort *)xymp));  /* just in case */
							ABORT = TRUE;
						}

						if (signal & winsig)
						{
							while (msg = (struct PPMessage *)GetMsg((struct MsgPort *)xymp));  /* just in case */
							ABORT = TRUE;
						}
					}
					DeletePort((struct MsgPort *)xymp);
				}
				else
					printf("Couldn't create port ProtPort\n");
				CloseWindow( win );
			}
			else
				printf("Couldn't open window\n");
			CloseLibrary( IntuitionBase );
		}
		CloseLibrary( (struct Library *)GfxBase );
	}	
}
