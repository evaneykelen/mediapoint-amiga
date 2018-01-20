#include "nb:pre.h"
#include "sm_seg.h"
#include "protos.h"

#define VERSION	"\0$VER: MediaPoint ScriptManager 1.0"
#define STP (char *)	// scalar to pointer

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct UserApplicInfo UAI;
struct EventData CED;
struct FileListInfo FLI;
TEXT scanDevs[SIZE_FILENAME*10];
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };

/**** disable CTRL-C break ****/

int CXBRK(void) { return(0); }
void chkabort(void) { return; }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct MsgPort *port;
struct Node *node;
struct List *list;
int i,j;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		exit(0);

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;
	msgs							= (UBYTE **)rvrec->msgs;

	/**** translate gadgets ****/

	//TranslateGR(about_GR);

	/**** do some pre-processing ****/

	UA_GetDevicesAndAssigns(&FLI);

	for(i=0; i<10; i++)
		scanDevs[i*SIZE_FILENAME] = '\0';

	for(j=0,i=0; i<FLI.numDevices; i++)
	{
		if (	stricmp(FLI.deviceList+i*SIZE_FILENAME,"DF0:") &&
					stricmp(FLI.deviceList+i*SIZE_FILENAME,"DF1:") &&
					stricmp(FLI.deviceList+i*SIZE_FILENAME,"DF2:") &&
					stricmp(FLI.deviceList+i*SIZE_FILENAME,"DF3:") &&
					stricmp(FLI.deviceList+i*SIZE_FILENAME,"PC0:") &&
					stricmp(FLI.deviceList+i*SIZE_FILENAME,"PC1:") )
		{
			strcpy(&scanDevs[j*SIZE_FILENAME],FLI.deviceList+i*SIZE_FILENAME);
			j++;
			if (j>7)
				break;
		}
	}

	UA_FreeDevicesAndAssigns(&FLI);

	/**** play around ****/

	doYourThing();

	/**** and leave the show ****/

	exit(0);
}

/******** doYourThing() ********/

void doYourThing(void)
{
	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open a window ****/

	UAI.windowModes = 2;	/* open on MY screen */

	if (rvrec->ehi->activeScreen == STARTSCREEN_PAGE)
		UAI.userScreen = rvrec->pagescreen;
	else if (rvrec->ehi->activeScreen == STARTSCREEN_SCRIPT)
		UAI.userScreen = rvrec->scriptscreen;
	else
		return;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(SM_Std_GR);
		UA_DoubleGadgetDimensions(SM_1_GR);
		UA_DoubleGadgetDimensions(SM_2_GR);
		UA_DoubleGadgetDimensions(SM_3_GR);
		UA_DoubleGadgetDimensions(SM_4_GR);
	}

	UAI.windowX			 = -1;	/* -1 means center on screen */
	UAI.windowY			 = -1;	/* -1 means center on screen */
	UAI.windowWidth	 = SM_Std_GR[0].x2;
	UAI.windowHeight = SM_Std_GR[0].y2;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, SM_Std_GR);
	UA_DrawGadgetList(UAI.userWindow, SM_1_GR);

	MonitorUser(UAI.userWindow);

	/**** close the window ****/

	UA_CloseWindow(&UAI);
}

/******** MonitorUser() ********/

void MonitorUser(struct Window *window)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
BOOL loop;
int ID,page=1;

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);
	loop=TRUE;
	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			while(message = (struct IntuiMessage *)GetMsg(window->UserPort))
			{
				CED.Class			= message->Class;
				CED.Code 			= message->Code;
				CED.Qualifier	= message->Qualifier;
				CED.MouseX 		= message->MouseX;
				CED.MouseY 		= message->MouseY;
				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code==SELECTDOWN)
						{
							ID = UA_CheckGadgetList(window, SM_Std_GR, &CED);
							switch(ID)
							{
								case 6:	// Exit
									UA_HiliteButton(window, &SM_Std_GR[ID]);
									loop=FALSE;
									break;
							}
							if (page==1)
								CheckPage1(&page);
							else if (page==2)
								CheckPage2();
							else if (page==3)
								CheckPage3();
						}
						break;
				}
			}
		}
	}
}

/******** TranslateGR() ********/

void TranslateGR(struct GadgetRecord *GR)
{
struct GadgetRecord *nextGR;
struct CycleRecord *CR;
int i, offset;

	nextGR = GR;
	while(nextGR)
	{
		i=0;
		while( nextGR[i].x1 != -1 )
		{
			if ( nextGR[i].type == PREV_GR )
			{
				nextGR = nextGR[i].ptr;
				break;
			}
			if ( nextGR[i].txt != NULL )
			{
				offset = (int)nextGR[i].txt;
				nextGR[i].txt = msgs[offset];
			}
			if ( nextGR[i].type == CYCLE_GADGET && nextGR[i].ptr != NULL )
			{
				CR = (struct CycleRecord *)nextGR[i].ptr;
				offset = (int)CR->ptr;
				CR->ptr = msgs[offset];
			}
			i++;
		}
	}
}

/******* CheckPage1() ********/

void CheckPage1(int *page)
{
int ID;

	ID = UA_CheckGadgetList(UAI.userWindow, SM_1_GR, &CED);
	switch(ID)
	{
		case 0:	// Check script
			UA_HiliteButton(UAI.userWindow, &SM_1_GR[ID]);
			UA_ClearButton(UAI.userWindow, &SM_Std_GR[4], AREA_PEN);
			UA_ClearButton(UAI.userWindow, &SM_Std_GR[5], AREA_PEN);
			DrawPage2Gadgets();
			*page=2;
			break;

		case 1:	// Create run-time
			UA_HiliteButton(UAI.userWindow, &SM_1_GR[ID]);
			UA_ClearButton(UAI.userWindow, &SM_Std_GR[4], AREA_PEN);
			UA_ClearButton(UAI.userWindow, &SM_Std_GR[5], AREA_PEN);
			DrawPage3Gadgets();
			*page=3;
			break;
	}
}

/******** SplitFullPath() ********/

void SplitFullPath(STRPTR fullPath, STRPTR path, STRPTR filename)
{
char a[FNSIZE], b[FMSIZE], c[FNSIZE], d[FESIZE];
int i,len;

	if ( UA_FindString(fullPath, ":")==-1 )
	{
		getcd(0, path);
		stccpy(filename, fullPath, SIZE_FILENAME);
		return;
	}

	if (strlen(fullPath)<3)
	{
		path[0] = '\0';
		filename[0] = '\0';
		return;
	}

	/* remove trailing and ending " */
	if ( fullPath[0] == '\"' )
	{
		len = strlen(fullPath)-2;
		for(i=0; i<len; i++)
			fullPath[i] = fullPath[i+1];
		fullPath[len] = '\0';
	}

	strsfn(fullPath, a,b,c,d);

	if (b[0]!='\0')
		sprintf(path, "%s%s/", a,b);
	else
		sprintf(path, "%s", a);

	UA_ValidatePath(path);

	if (d[0]!='\0')
		sprintf(filename, "%s.%s", c,d);
	else
		sprintf(filename, "%s", c);
}

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** PerfFunc() ********/
/* input:    pointer to an array of pointers to functions, a pointer to
 *           a filled ParseRecord which holds the currently parsed command
 *           and a pointer to the current ScriptInfoRecord.
 * output:   -
 * function: jumps to a function, carrying along the PR and SIR pointers.
 */

struct GenericFuncs
{
	void (*func)(APTR, APTR);
};

void PerfFunc(struct GenericFuncs *FuncList, struct ParseRecord *PR, struct ScriptInfoRecord *SIR)
{
void (*func)(APTR, APTR);

	func = FuncList[PR->commandCode].func;
	if (func!=NO_FUNCTION)
		(*(func))(PR,SIR);
}

/******** E O F ********/
