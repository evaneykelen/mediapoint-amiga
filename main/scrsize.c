#include "nb:pre.h"

#define MONLISTWIDTH 80

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern struct Gadget PropSlider1;
extern UWORD chip gui_pattern[];

/**** static globals ****/

TEXT colorList[75];

/**** gadgets ****/

extern struct GadgetRecord FormatRequester_GR[];
extern struct CycleRecord Format_Color_CR1;
extern struct CycleRecord Format_Color_CR2;

/**** functions ****/

/******** MonitorScreenSize() ********/

BOOL MonitorScreenSize(BOOL *choseOptimize, int colorUsage, BOOL forceReopen)
{
struct Window *window;
BOOL loop, retVal, dblClicked;
int ID, val, line, i;
struct CapsPrefs copy_prefs;
static BOOL optimize=TRUE;
struct ScrollRecord SR;
int numMons, numDisp, topEntry, listPos;
UBYTE *monList;
ULONG *IDS;
UBYTE selectionList[100];

	retVal = FALSE;
	loop = TRUE;
	CopyMem(&CPrefs, &copy_prefs, sizeof(struct CapsPrefs));
	numMons = 0;
	numDisp = 8;
	topEntry = 0;
	line = 0;
	
	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,FormatRequester_GR,colorUsage);
	if (!window)
	{
		UA_WarnUser(157);
		return(FALSE);
	}

	/**** get # monitors ****/

	monList = (UBYTE *)AllocMem(MONLISTWIDTH*100,MEMF_ANY);
	if ( !monList )
	{
		UA_CloseRequesterWindow(window,colorUsage);
		return(FALSE);
	}

	IDS = (ULONG *)AllocMem(100*sizeof(ULONG),MEMF_ANY);
	if ( !IDS )
	{
		FreeMem(monList,MONLISTWIDTH*100);
		UA_CloseRequesterWindow(window,colorUsage);
		return(FALSE);
	}

	listPos = -1;
	numMons = MakeDisplayList(	monList,IDS,100,CPrefs.pageMonName,&listPos,
															CPrefs.pageMonitorID, FALSE );
	if ( numMons==0 )
	{
		numMons = MakeDisplayList(	monList,IDS,100,CPrefs.pageMonName,&listPos,
																CPrefs.pageMonitorID, TRUE );
		if ( numMons==0 )
		{
			FreeMem(IDS,100*sizeof(ULONG));
			FreeMem(monList,MONLISTWIDTH*100);
			UA_CloseRequesterWindow(window,colorUsage);
			return(FALSE);
		}
	}

	for(i=0; i<100; i++)
		selectionList[i] = 0;

	if ( listPos>=0 && listPos<100 )
		selectionList[ listPos ] = 1;
	else
	{
		selectionList[ 0 ] = 1;
		listPos = 0;
	}

	line = listPos;

	/**** render gadgets ****/

	UA_DrawGadgetList(window, FormatRequester_GR);

	UA_DrawSpecialGadgetText(window, &FormatRequester_GR[ 9], msgs[Msg_Colors-1],   SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &FormatRequester_GR[10], msgs[Msg_Overscan-1], SPECIAL_TEXT_TOP);

	if ( optimize )	
		UA_InvertButton(window, &FormatRequester_GR[12]);

	/**** slider gadget ****/

	PropSlider1.LeftEdge = FormatRequester_GR[8].x1+4;
	PropSlider1.TopEdge	 = FormatRequester_GR[8].y1+2;
	PropSlider1.Width		 = FormatRequester_GR[8].x2-FormatRequester_GR[8].x1-7;
	PropSlider1.Height	 = FormatRequester_GR[8].y2-FormatRequester_GR[8].y1-3;
	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge += 2;
		PropSlider1.Height -= 4;
	}
	InitPropInfo((struct PropInfo *)PropSlider1.SpecialInfo,(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);
	UA_SetPropSlider(window, &PropSlider1, numMons, numDisp, topEntry);

	/**** init scroll record ****/

	SR.GR							= &FormatRequester_GR[7];
	SR.window					= window;
	SR.list						= monList;
	SR.sublist				= NULL;
	SR.selectionList	= selectionList;
	SR.entryWidth			= MONLISTWIDTH;
	SR.numDisplay			= numDisp;
	SR.numEntries			= numMons;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR,topEntry,NULL);

	/**** print info about current mode ****/

	GetDisplayInfoText(window,*(IDS+listPos),CPrefs.overScan,CPrefs.pageMonName);

	/**** monitor user ****/

	if ( forceReopen )
		UA_DisableButton(window, &FormatRequester_GR[6], gui_pattern);	

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		dblClicked=FALSE;
		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			UA_ScrollStandardList(&SR,&topEntry,&PropSlider1,NULL,&CED);
		}
		else if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, FormatRequester_GR, &CED);
			switch(ID)
			{
				case 5:
do_ok:
					UA_HiliteButton(window, &FormatRequester_GR[5]);	
					loop=FALSE;
					retVal=TRUE;
					break;

				case 6:
do_cancel:
					UA_HiliteButton(window, &FormatRequester_GR[6]);	
					loop=FALSE;
					retVal=FALSE;
					break;

				case 7:	// list
					line = UA_SelectStandardListLine(&SR,topEntry,FALSE,&CED,FALSE,FALSE);
					if ( line != -1 )
					{
						GetDisplayInfoText(	window,*(IDS+line+topEntry),CPrefs.overScan,
																CPrefs.pageMonName);
						if ( dblClicked )
							goto do_ok;
					}
					break;

				case 9:		// #colors
					UA_ProcessCycleGadget(window, &FormatRequester_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&FormatRequester_GR[ID], &val);
					InterpretColors(val, CPrefs.pageMonitorID);
					break;
					
				case 10:	// overscan
					UA_ProcessCycleGadget(window, &FormatRequester_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&FormatRequester_GR[ID], &val);
					CPrefs.overScan = val;
					GetDisplayInfoText(window,*(IDS+line+topEntry),CPrefs.overScan,CPrefs.pageMonName);
					break;

				case 12:	// optimize
					UA_InvertButton(window, &FormatRequester_GR[ID]);
					if ( optimize )
						optimize=FALSE;
					else
						optimize=TRUE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,colorUsage);

	if (	retVal && !forceReopen &&
				copy_prefs.PageScreenWidth == CPrefs.PageScreenWidth &&
				copy_prefs.PageScreenHeight == CPrefs.PageScreenHeight &&
				copy_prefs.PageScreenDepth == CPrefs.PageScreenDepth &&
				copy_prefs.PageScreenModes == CPrefs.PageScreenModes &&
				copy_prefs.pageMonitorID == CPrefs.pageMonitorID &&
				copy_prefs.overScan == CPrefs.overScan	)
	{
		retVal = FALSE;	// you don't fool me!
		// Resolution not changed, OK hit, no screen size change but yes to optimize
		if ( optimize )
			OptimizePalette(TRUE);
	}

	if ( !retVal )
		CopyMem(&copy_prefs, &CPrefs, sizeof(struct CapsPrefs));

	FreeMem(IDS,100*sizeof(ULONG));
	FreeMem(monList,MONLISTWIDTH*100);

	*choseOptimize = optimize;

	if ( forceReopen )
		UA_EnableButtonQuiet(&FormatRequester_GR[6]);	
	
	return(retVal);
}

/******** MakeDisplayList() ********/

int MakeDisplayList(UBYTE *monitorList, ULONG *IDS, UBYTE maxMonitors, STRPTR monName,
										int *listPos, ULONG currentID, BOOL doAll)
{
ULONG ID;
int ct,i;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
struct ExtendedNode *en;
TEXT monitorName[50];

	ID = INVALID_ID;
	ct = 0;
	do
	{
		ID = NextDisplayInfo(ID);
		if ( ID != INVALID_ID )
		{
			if ( doAll || (ID & MONITOR_ID_MASK) )
			{
				if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),DTAG_DISP,ID) )
				{
					if ( dispinfo.NotAvailable==0L )
					{
						if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
						{
							if ( GetDisplayInfoData(NULL,(UBYTE *)&nameinfo,sizeof(struct NameInfo),DTAG_NAME,ID) )
							{
								if ( GetDisplayInfoData(NULL,(UBYTE *)&moninfo,sizeof(struct MonitorInfo),DTAG_MNTR,ID) )
								{
									if ( (diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1) >= 320 &&
											 (diminfo.Nominal.MaxY-diminfo.Nominal.MinY+1) <= 580 )
									{
										en = &(moninfo.Mspc->ms_Node);

										strcpy(monitorName,en->xln_Name);
										for(i=0; i<50; i++)
										{
											if ( monitorName[i]=='.' )
											{
												monitorName[i]='\0';
												break;
											}
										}

										if ( !strcmpi(monitorName, monName) )
										{
											strcpy(monitorName,nameinfo.Name);
											for(i=0; i<50; i++)
											{
												if ( monitorName[i]==':' )
												{
													strcpy(monitorName,&monitorName[i+1]);
													break;
												}
											}

											sprintf(monitorList+ct*MONLISTWIDTH, "%s", monitorName, ID);
											*(IDS+ct) = ID;

											if ( currentID == ID )
												*listPos = ct;

											ct++;
											if (ct==maxMonitors)
												return(ct);
										}
									}
								}
							}
						}
					}		
				}
			}
		}
	}
	while( ID != INVALID_ID );

	return(ct);
}

/******** GetDisplayInfoText() ********/

BOOL GetDisplayInfoText(struct Window *window, ULONG ID, int overScan, STRPTR monName)
{
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
WORD w,h;
TEXT info[256];
int i,nc,ehb,ham,ham8;
struct CycleRecord *CR;

	if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),DTAG_DISP,ID) )
	{
		if ( dispinfo.NotAvailable==0L )
		{
			if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
			{
				if ( GetDisplayInfoData(NULL,(UBYTE *)&nameinfo,sizeof(struct NameInfo),DTAG_NAME,ID) )
				{
					if ( GetDisplayInfoData(NULL,(UBYTE *)&moninfo,sizeof(struct MonitorInfo),DTAG_MNTR,ID) )
					{
						CPrefs.pageMonitorID = ID;

						if ( overScan == 0 )				// No overscan
						{
							w = diminfo.Nominal.MaxX - diminfo.Nominal.MinX + 1;
							h = diminfo.Nominal.MaxY - diminfo.Nominal.MinY + 1;
						}
						else if ( overScan == 1 )	// TxtOScan - editable via preferences
						{
							w = diminfo.TxtOScan.MaxX - diminfo.TxtOScan.MinX + 1;
							h = diminfo.TxtOScan.MaxY - diminfo.TxtOScan.MinY + 1;
						}
						else if ( overScan == 2 )	// StdOScan - editable via preferences
						{
							w = diminfo.StdOScan.MaxX - diminfo.StdOScan.MinX + 1;
							h = diminfo.StdOScan.MaxY - diminfo.StdOScan.MinY + 1;
						}
						else if ( overScan == 3 )	// MaxOScan - fixed, hardware dependent
						{
							w = diminfo.MaxOScan.MaxX - diminfo.MaxOScan.MinX + 1;
							h = diminfo.MaxOScan.MaxY - diminfo.MaxOScan.MinY + 1;
						}
						else if ( overScan == 4 )	// VideoOScan - fixed, hardware dependent
						{
							w = diminfo.VideoOScan.MaxX - diminfo.VideoOScan.MinX + 1;
							h = diminfo.VideoOScan.MaxY - diminfo.VideoOScan.MinY + 1;
						}

						sprintf(info, "%s: %d × %d\n", monName,w,h);

						UA_PrintInBox(window, &FormatRequester_GR[11],
													FormatRequester_GR[11].x1, FormatRequester_GR[11].y1,
													FormatRequester_GR[11].x2, FormatRequester_GR[11].y2,
													info, PRINT_CENTERED);

						// set CPrefs

						CPrefs.PageScreenWidth = w;
						CPrefs.PageScreenHeight = h;

						if ( dispinfo.PropertyFlags & DIPF_IS_LACE )
							CPrefs.PageScreenModes |= LACE;
						else
							CPrefs.PageScreenModes &= ~LACE;

						if ( CPrefs.PageScreenHeight >= 400 )
							CPrefs.PageScreenModes |= LACE;				

						for(i=0; i<75; i++)
							colorList[i] = '\0';
						nc = 2;
						for(i=0; i<diminfo.MaxDepth; i++)
						{
							sprintf(&colorList[i*5],"%d",nc);
							nc <<= 1;
						}

						if ( !ModeNotAvailable(CPrefs.pageMonitorID | EXTRAHALFBRITE_KEY) )
						{
							strcpy(&colorList[i*5],"EHB");
							ehb = i;	// used later to set #colors cycle gadget
							i++;
						}
						else CPrefs.PageScreenModes &= ~EXTRAHALFBRITE_KEY; 

						if ( !ModeNotAvailable(CPrefs.pageMonitorID | HAM_KEY) )
						{
							strcpy(&colorList[i*5],"HAM");
							ham = i;	// used later to set #colors cycle gadget
							i++;
							if ( diminfo.MaxDepth >= 8 )
							{
								strcpy(&colorList[i*5],"HAM8");
								ham8 = i;	// used later to set #colors cycle gadget
								i++;
							}
						}
						else CPrefs.PageScreenModes &= ~HAM_KEY; 

						CR = (struct CycleRecord *)FormatRequester_GR[9].ptr;
						CR->ptr = colorList;
						CR->number = i;
						CR->width = 5;

						nc = CPrefs.PageScreenDepth-1;	// depth eg 6 -> nc = 5 -> 64 colors
						if ( CPrefs.PageScreenModes & EXTRAHALFBRITE_KEY )
							nc=ehb;
						if ( (CPrefs.PageScreenModes & HAM_KEY) && CPrefs.PageScreenDepth==6 )
							nc=ham;
						if ( (CPrefs.PageScreenModes & HAM_KEY) && CPrefs.PageScreenDepth==8 )
							nc=ham8;

						if ( nc > (CR->number-1) )
						{
							nc = (CR->number-1);
							CPrefs.PageScreenDepth = nc+1;
						}

						// Set #colors and overscan buttons

						CR->active = -1;	// force redraw							
						UA_SetCycleGadgetToVal(window, &FormatRequester_GR[9], nc);

						UA_SetCycleGadgetToVal(window, &FormatRequester_GR[10], CPrefs.overScan);

						return(TRUE);
					}
				}
			}
		}		
	}

	return(FALSE);
}

/******** InterpretColors() ********/

void InterpretColors(int val, ULONG ID)
{
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;

	if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),DTAG_DISP,ID) )
	{
		if ( dispinfo.NotAvailable==0L )
		{
			if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
			{
				if ( GetDisplayInfoData(NULL,(UBYTE *)&nameinfo,sizeof(struct NameInfo),DTAG_NAME,ID) )
				{
					if ( GetDisplayInfoData(NULL,(UBYTE *)&moninfo,sizeof(struct MonitorInfo),DTAG_MNTR,ID) )
					{
						/*
							"2|   4|   "	// ECS Super
						   -----+++++
							"2|   4|   8|   16|  "	// ECS Hires
						   -----+++++-----+++++
							"2|   4|   8|   16|  32|  EHB| HAM|  "	// ECS Lores
						   -----+++++-----+++++-----+++++------
							"2|   4|   8|   16|  32|  EHB| 64|  128| 256| HAM| HAM8| "	// AA
							 -----+++++-----+++++-----+++++-----+++++-----+++++------
						*/

						// What a strange but refreshingly new way to discover depth!

						CPrefs.PageScreenModes &= ~EXTRAHALFBRITE_KEY; 
						CPrefs.PageScreenModes &= ~HAM_KEY; 

						if ( !strncmp( &colorList[val*5], "256", 3 ) )
							CPrefs.PageScreenDepth = 8;
						else if ( !strncmp( &colorList[val*5], "128", 3 ) )
							CPrefs.PageScreenDepth = 7;
						else if ( !strncmp( &colorList[val*5], "64", 2 ) )
							CPrefs.PageScreenDepth = 6;
						else if ( !strncmp( &colorList[val*5], "32", 2 ) )
							CPrefs.PageScreenDepth = 5;
						else if ( !strncmp( &colorList[val*5], "16", 2 ) )
							CPrefs.PageScreenDepth = 4;
						else if ( !strncmp( &colorList[val*5], "8", 1 ) )
							CPrefs.PageScreenDepth = 3;
						else if ( !strncmp( &colorList[val*5], "4", 1 ) )
							CPrefs.PageScreenDepth = 2;
						else if ( !strncmp( &colorList[val*5], "2", 1 ) )
							CPrefs.PageScreenDepth = 1;
						else if ( !strncmp( &colorList[val*5], "EHB", 3 ) )
						{
							CPrefs.PageScreenDepth = 6;
							CPrefs.PageScreenModes |= EXTRAHALFBRITE_KEY; 
						}
						else if ( !strncmp( &colorList[val*5], "HAM8", 4 ) )
						{
							CPrefs.PageScreenDepth = 8;
							CPrefs.PageScreenModes |= HAM_KEY; 
						}
						else if ( !strncmp( &colorList[val*5], "HAM", 3 ) )
						{
							CPrefs.PageScreenDepth = 6;
							CPrefs.PageScreenModes |= HAM_KEY; 
						}
					}
				}
			}
		}		
	}
}

/******** E O F ********/
