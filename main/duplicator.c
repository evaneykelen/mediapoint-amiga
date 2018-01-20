#include "nb:pre.h"

/**** externals ****/

extern struct EventData CED;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern UBYTE **msgs;

/**** gadgets ****/

extern struct GadgetRecord Duplicator_GR[];

/**** functions ****/

/******** Duplicator() ********/

void Duplicator(void)
{
struct Window *window;
BOOL loop=TRUE, retval=FALSE;
int ID, xAdd, yAdd, i, j, x, y, aw;
static int factor=2;

	aw = FirstActiveEditWindow();

	xAdd=EditWindowList[aw]->Width;
	yAdd=EditWindowList[aw]->Height;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,Duplicator_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(186);
		return;
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Duplicator_GR);

	UA_DrawSpecialGadgetText(window, &Duplicator_GR[3], msgs[Msg_Times-1], SPECIAL_TEXT_AFTER_STRING);
	UA_DrawSpecialGadgetText(window, &Duplicator_GR[4], msgs[Msg_ToX-1], SPECIAL_TEXT_AFTER_STRING);
	UA_DrawSpecialGadgetText(window, &Duplicator_GR[5], msgs[Msg_ToY-1], SPECIAL_TEXT_AFTER_STRING);

	UA_SetStringGadgetToVal(window, &Duplicator_GR[3], factor);
	UA_SetStringGadgetToVal(window, &Duplicator_GR[4], xAdd);
	UA_SetStringGadgetToVal(window, &Duplicator_GR[5], yAdd);

	/**** monitor user ****/

	goto do_3;	// allow direct entering of value

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Duplicator_GR, &CED);
			switch(ID)
			{
				case 3:
do_3:
				 if ( UA_ProcessStringGadget(window, Duplicator_GR, &Duplicator_GR[3], &CED) )
						goto do_4;
					break;

				case 4:
do_4:
				 if ( UA_ProcessStringGadget(window, Duplicator_GR, &Duplicator_GR[4], &CED) )
						goto do_5;
					break;

				case 5:
do_5:
				 if ( UA_ProcessStringGadget(window, Duplicator_GR, &Duplicator_GR[5], &CED) )
						goto do_3;
					break;

				case 6:	/* Cancel */
do_cancel:
					UA_HiliteButton(window, &Duplicator_GR[6]);
					loop=FALSE;
					break;

				case 7:	/* OK */
do_ok:
					UA_HiliteButton(window, &Duplicator_GR[7]);
					loop=FALSE;
					retval=TRUE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE) /* cancel */
				goto do_cancel;
			else if (CED.Code==RAW_RETURN) /* OK */
				goto do_ok;
		}
	}

	UA_SetValToStringGadgetVal(&Duplicator_GR[3], &factor);
	UA_SetValToStringGadgetVal(&Duplicator_GR[4], &xAdd);
	UA_SetValToStringGadgetVal(&Duplicator_GR[5], &yAdd);

	UA_CloseRequesterWindow(window,STDCOLORS);

	/**** do the actual duplicating ****/

	aw = NumActiveEditWindows();

	DrawAllHandles(LEAVE_ACTIVE);

	if ( factor>0 && retval )
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditSupportList[i]!=NULL && EditSupportList[i]->Active )
			{
				CopyAWindow(i,0,TRUE);	// copy window i to Clipboard_WL/SL[0], TRUE=del clipb.
				x = EditWindowList[i]->X;
				y = EditWindowList[i]->Y;
				for(j=0; j<factor; j++)
				{
					x += xAdd;				
					y += yAdd;				
					PasteActiveWindows(x, y);
				}
				aw--;
				if (aw<1)
					break;
			}
		}
	}

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** E O F ********/
