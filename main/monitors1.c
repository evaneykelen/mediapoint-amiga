/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct FileInfoBlock *FIB;
extern UBYTE **msgs;
extern struct Library *medialinkLibBase;
extern ULONG allocFlags;

/**** statics ****/

TEXT DefMonName[50];

/**** functions ****/

/******** SetMonitorDefaults() ********/

BOOL SetMonitorDefaults(void)
{
int result;
struct DimensionInfo DimInfo;

	CPrefs.overScan = 0;

	DefaultMonName(CPrefs.scriptMonName, DEFAULT_MONITOR_ID, &CPrefs.scriptMonitorID);
	CPrefs.scriptMonitorID |= HIRES_KEY;

	DefaultMonName(CPrefs.pageMonName, DEFAULT_MONITOR_ID, &CPrefs.pageMonitorID);
	CPrefs.pageMonitorID |= HIRES_KEY;

	DefaultMonName(CPrefs.playerMonName, DEFAULT_MONITOR_ID, &CPrefs.playerMonitorID);

	strcpy(DefMonName, CPrefs.playerMonName);

	result = GetDisplayInfoData(NULL, (UBYTE *)&DimInfo, sizeof(struct DimensionInfo),
															DTAG_DIMS, CPrefs.scriptMonitorID | HIRES);
	if ( result==0 )
		return(FALSE);

	CPrefs.PageScreenWidth				= DimInfo.Nominal.MaxX - DimInfo.Nominal.MinX + 1;
	CPrefs.PageScreenHeight				= DimInfo.Nominal.MaxY - DimInfo.Nominal.MinY + 1;
	CPrefs.PageScreenDepth				= 4;
	CPrefs.PageScreenModes				= HIRES;
	if ( CPrefs.PageScreenHeight >= 400 )
		CPrefs.PageScreenModes |= LACE;				

	CPrefs.ScriptScreenWidth			= CPrefs.PageScreenWidth;
	CPrefs.ScriptScreenHeight			= CPrefs.PageScreenHeight;
	CPrefs.ScriptScreenDepth			= 3;
	CPrefs.ScriptScreenModes			= HIRES;
	if ( CPrefs.ScriptScreenHeight >= 400 )
		CPrefs.ScriptScreenModes |= LACE;				

	CPrefs.ThumbnailScreenWidth		= CPrefs.PageScreenWidth;
	CPrefs.ThumbnailScreenHeight	= CPrefs.PageScreenHeight;
	CPrefs.ThumbnailScreenDepth		= 4;
	CPrefs.ThumbnailScreenModes		= HIRES;
	if ( CPrefs.ThumbnailScreenHeight >= 400 )
		CPrefs.ThumbnailScreenModes |= LACE;				

	if (CheckPAL("Workbench"))
	{
		CPrefs.ScriptPalNtsc = PAL_MODE;
		CPrefs.PagePalNtsc = PAL_MODE;
		CPrefs.PlayerPalNtsc = PAL_MODE;
	}	
	else
	{
		CPrefs.ScriptPalNtsc = NTSC_MODE;
		CPrefs.PagePalNtsc = NTSC_MODE;
		CPrefs.PlayerPalNtsc = NTSC_MODE;
	}

	return(TRUE);
}

/******** SetMonitorFromConfig() ********/

BOOL SetMonitorFromConfig(void)
{
int result;
struct DimensionInfo DimInfo;

	/**** PAGE ****/

	if ( !GetIDWithMonName(CPrefs.pageMonName, &CPrefs.pageMonitorID, 640, -1, 0) )
	{
		DefaultMonName(CPrefs.pageMonName, DEFAULT_MONITOR_ID, &CPrefs.pageMonitorID);
		CPrefs.pageMonitorID |= HIRES_KEY;
	}

	result = GetDisplayInfoData(NULL, (UBYTE *)&DimInfo, sizeof(struct DimensionInfo),
															DTAG_DIMS, CPrefs.pageMonitorID);
	if ( result==0 )
		return(FALSE);

	CPrefs.PageScreenWidth = DimInfo.Nominal.MaxX - DimInfo.Nominal.MinX + 1;
	CPrefs.PageScreenHeight = DimInfo.Nominal.MaxY - DimInfo.Nominal.MinY + 1;

	if ( CPrefs.PageScreenDepth > DimInfo.MaxDepth )
		CPrefs.PageScreenDepth = DimInfo.MaxDepth;

	if ( CPrefs.PageScreenHeight >= 400 )
		CPrefs.PageScreenModes |= LACE;				

	/**** SCRIPT ****/

	if ( !GetIDWithMonName(CPrefs.scriptMonName, &CPrefs.scriptMonitorID, CPrefs.ScriptScreenWidth, CPrefs.ScriptScreenHeight, 0) )
	{
		DefaultMonName(CPrefs.scriptMonName, DEFAULT_MONITOR_ID, &CPrefs.scriptMonitorID);
		CPrefs.scriptMonitorID |= HIRES_KEY;
	}

	result = GetDisplayInfoData(NULL, (UBYTE *)&DimInfo, sizeof(struct DimensionInfo),
															DTAG_DIMS, CPrefs.scriptMonitorID | CPrefs.ScriptScreenModes);
	if ( result==0 )
		return(FALSE);

	CPrefs.ScriptScreenWidth = DimInfo.Nominal.MaxX - DimInfo.Nominal.MinX + 1;
	CPrefs.ScriptScreenHeight = DimInfo.Nominal.MaxY - DimInfo.Nominal.MinY + 1;

	if ( CPrefs.ScriptScreenDepth > DimInfo.MaxDepth )
		CPrefs.ScriptScreenDepth = DimInfo.MaxDepth;

	if ( CPrefs.ScriptScreenHeight >= 400 )
		CPrefs.ScriptScreenModes |= LACE;				

	CPrefs.ThumbnailScreenWidth = CPrefs.ScriptScreenWidth;
	CPrefs.ThumbnailScreenHeight = CPrefs.ScriptScreenHeight;
	CPrefs.ThumbnailScreenModes = CPrefs.ScriptScreenModes;

	if ( CPrefs.ThumbnailScreenDepth > DimInfo.MaxDepth )
		CPrefs.ThumbnailScreenDepth = DimInfo.MaxDepth;

	/**** PLAYER ****/

	if ( !GetIDWithMonName(CPrefs.playerMonName, &CPrefs.playerMonitorID, -1, -1, 0) )
		strcpy(CPrefs.playerMonName,DefMonName);

	return(TRUE);
}

/******** CheckPAL() ********/
/*
 * Code lifted from AmigaMail Page V - 10, volume II
 *
 */

BOOL CheckPAL(STRPTR screenname)
{
struct Screen *screen;
ULONG modeID = LORES_KEY;
struct DisplayInfo displayinfo;
BOOL IsPAL;

	if (CPrefs.SystemTwo)
	{
		if (screen=LockPubScreen(screenname))
		{
			if ((modeID = GetVPModeID(&(screen->ViewPort))) != INVALID_ID)
			{
				if (!	(	(modeID & MONITOR_ID_MASK) == NTSC_MONITOR_ID ||
								(modeID & MONITOR_ID_MASK) == PAL_MONITOR_ID))
					modeID = LORES_KEY;
			}
			UnlockPubScreen(NULL, screen);
		}

		if (	GetDisplayInfoData(NULL, (UBYTE *)&displayinfo,
					sizeof(struct DisplayInfo), DTAG_DISP, modeID))
		{
			if (displayinfo.PropertyFlags & DIPF_IS_PAL)
				IsPAL = TRUE;
			else
				IsPAL = FALSE;
		}
	}
	else
		IsPAL = (GfxBase->DisplayFlags & PAL) ? TRUE : FALSE;

	return(IsPAL);
}

/******** GetDimsFromMode1And2And3() ********/
/*
 * This function provides backward compatible values for old-style documents
 * with an old SCREEN command.
 * Used in pagetalk.c
 *
 */

BOOL GetDimsFromMode1And2And3(int mode1, int mode2, int mode3, WORD *w, WORD *h)
{
struct DimensionInfo DimInfo;
ULONG modeID;
int result;

	if ( CPrefs.PagePalNtsc == PAL_MODE )
		modeID = PAL_MONITOR_ID;
	else
		modeID = NTSC_MONITOR_ID;

	if ( mode1==3 || mode1==4 )	// HIRES or HIRESLACE
		modeID |= HIRES_KEY;
	if ( mode1==2 || mode1==4 || mode1==6 )	// LORESLACE HIRESLACE SUPERLACE
		modeID |= LACE;
	if ( mode1==5 || mode1==6 )
		modeID |= SUPER_KEY;

	//modes = GetModesFromMode1And2(mode1,mode2);

	result = GetDisplayInfoData(NULL, (UBYTE *)&DimInfo, sizeof(struct DimensionInfo), DTAG_DIMS, modeID);
	if ( result==0 )
		return(FALSE);

	if ( mode3 == 0 )				// No overscan
	{
		*w = DimInfo.Nominal.MaxX - DimInfo.Nominal.MinX + 1;
		*h = DimInfo.Nominal.MaxY - DimInfo.Nominal.MinY + 1;
	}
	else if ( mode3 == 1 )	// MaxOScan - fixed, hardware dependent
	{
		//CPrefs.overScan = 3;
		*w = DimInfo.MaxOScan.MaxX - DimInfo.MaxOScan.MinX + 1;
		*h = DimInfo.MaxOScan.MaxY - DimInfo.MaxOScan.MinY + 1;
	}
	else if ( mode3 == 2 )	// VideoOScan - fixed, hardware dependent
	{
		//CPrefs.overScan = 4;
		*w = DimInfo.VideoOScan.MaxX - DimInfo.VideoOScan.MinX + 1;
		*h = DimInfo.VideoOScan.MaxY - DimInfo.VideoOScan.MinY + 1;
	}

	return(TRUE);
}

/******** GetModesFromMode1And2() ********/
/*
 * Used in pagetalk.c
 *
 */

ULONG GetModesFromMode1And2(int mode1, int mode2)
{
ULONG modes;

	modes = 0L;

	if ( mode1==1 )
		modes = LORES_KEY;
	else if ( mode1==2 )
		modes = LORESLACE_KEY;
	else if ( mode1==3 )
		modes = HIRES_KEY;
	else if ( mode1==4 )
		modes = HIRESLACE_KEY;
	else if ( mode1==5 )
		modes = SUPER_KEY;
	else if ( mode1==6 )
		modes = SUPERLACE_KEY;

	if ( mode2==1 )
		modes |= HAM_KEY;
	else if ( mode2==2 )
		modes |= EXTRAHALFBRITE_KEY;

	return( modes );
}	

/******** GetInfoOnModeID() ********/

BOOL GetInfoOnModeID(ULONG modeID, WORD *w, WORD *h, WORD *d, WORD overscan)
{
ULONG ID;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;

	ID = INVALID_ID;
	do
	{
		ID = NextDisplayInfo(ID);
		if ( ID != INVALID_ID )
		{
			//if ( ID & MONITOR_ID_MASK )
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
									if ( modeID == ID )
									{
										GetOverscanValues(&diminfo, overscan, w, h);
										//*w = diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1;
										//*h = diminfo.Nominal.MaxY-diminfo.Nominal.MinY+1;
										*d = diminfo.MaxDepth;
										return(TRUE);
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

	return(FALSE);
}

/******** DefaultMonName() ********/

void DefaultMonName(STRPTR name, ULONG ID, ULONG *monID)
{
int i;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
struct ExtendedNode *en;
TEXT monitorName[50];

	// First get name of default monitor

	monitorName[0]='\0';
	*monID = ID;

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
						en = &(moninfo.Mspc->ms_Node);
						strcpy(monitorName,en->xln_Name);
					}
				}
			}		
		}
	}

	if ( monitorName[0]=='\0' )
		return;

	for(i=0; i<50; i++)
	{
		if ( monitorName[i]=='.' )
		{
			monitorName[i]='\0';
			break;
		}
	}

	// Then get ID of matching real (not default) monitor

	strcpy(name, monitorName);
	GetIDWithMonName(name, monID, -1, -1, 0);
}

/******** GetIDWithMonName() ********/
/*
 * monitorName eg 'pal' **NOT** 'pal.monitor'
 *
 */

BOOL GetIDWithMonName(STRPTR name, ULONG *monID, WORD w, WORD h, int oscan)
{
ULONG ID;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
struct ExtendedNode *en;
TEXT monitorName[50];
WORD dw,dh,diffW=9999,diffH=9999;	//,prevDW=9999,prevDH=9999;
BOOL retval=FALSE;

	strcpy(monitorName, name);
	strcat(monitorName, ".monitor");

	ID = INVALID_ID;
	do
	{
		ID = NextDisplayInfo(ID);
		if ( ID != INVALID_ID )
		{
			if ( ID & MONITOR_ID_MASK )
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
									en = &(moninfo.Mspc->ms_Node);	
									if ( !strcmp(monitorName,en->xln_Name) )
									{
										GetOverscanValues(&diminfo,oscan,&dw,&dh);

										if ( w!=-1 && h!=-1 )
										{
											diffW = AbsWORD(dw,w);
											diffH = AbsWORD(dh,h);
										}

										if ( dh<800 && dw>=320 )
										{
											if ( (w==-1 && h==-1) || (dw==w && h==-1) || (dw>=w && diffW<320 && dh>=h) )
														/*(diffW<prevDW && diffH<prevDH) || (diffW==0 && diffH==0) )*/
											{
												*monID = ID;

												// Return immediately if something vaguely matching is found 

												if ( (w==-1 && h==-1) || (dw==w && h==-1) )
													return(TRUE);

												// Return immediately if something exactly matching is found 

												if ( dw==w && dh==h )
													return(TRUE);

												// return with ID that is smaller/equal to requested size

												retval = TRUE;
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
	}
	while( ID != INVALID_ID );

	return( retval );
}

/******** GetDimsFromIFF() ********/

BOOL GetDimsFromIFF(struct IFF_FRAME *iff, WORD *w, WORD *h, int *oscan, WORD *maxDepth,
										ULONG *monID, ULONG *modes, BOOL dontCareH)
{
ULONG ID;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
struct ExtendedNode *en;
TEXT monitorName[50];
WORD mw,mh;

	// IFF_FRAME tells me:	iff->BMH.w and iff->BMH.h
	//											iff->BMH.nPlanes
	//											iff->viewModes
	//											iff->colorMap

	strcpy(monitorName, CPrefs.pageMonName);
	strcat(monitorName, ".monitor");

	ID = INVALID_ID;
	do
	{
		ID = NextDisplayInfo(ID);
		if ( ID != INVALID_ID )
		{
			if ( ID & MONITOR_ID_MASK )
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
									en = &(moninfo.Mspc->ms_Node);	
									if ( !strcmp(monitorName,en->xln_Name) )
									{
										*oscan=-1;

										mw = diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1;
										mh = diminfo.Nominal.MaxY-diminfo.Nominal.MinY+1;
										if (	( iff->BMH.w <= mw && iff->BMH.h <= mh && mh<800 ) ||
													( dontCareH && mw >= iff->BMH.w && mh <= iff->BMH.h && mh<800 ) )
											*oscan = 0;
										else
										{
											mw = diminfo.TxtOScan.MaxX-diminfo.TxtOScan.MinX+1;
											mh = diminfo.TxtOScan.MaxY-diminfo.TxtOScan.MinY+1;
											if (	( iff->BMH.w <= mw && iff->BMH.h <= mh && mh<800 ) ||
														( dontCareH && mw >= iff->BMH.w && mh <= iff->BMH.h && mh<800 ) )
												*oscan = 1;
											else
											{
												mw = diminfo.StdOScan.MaxX-diminfo.StdOScan.MinX+1;
												mh = diminfo.StdOScan.MaxY-diminfo.StdOScan.MinY+1;
												if (	( iff->BMH.w <= mw && iff->BMH.h <= mh && mh<800 ) ||
															( dontCareH && mw >= iff->BMH.w && mh <= iff->BMH.h && mh<800 ) )
													*oscan = 2;
												else
												{
													mw = diminfo.MaxOScan.MaxX-diminfo.MaxOScan.MinX+1;
													mh = diminfo.MaxOScan.MaxY-diminfo.MaxOScan.MinY+1;
													if (	( iff->BMH.w <= mw && iff->BMH.h <= mh && mh<800 ) ||
																( dontCareH && mw >= iff->BMH.w && mh <= iff->BMH.h && mh<800 ) )
														*oscan = 3;
													else
													{
														mw = diminfo.VideoOScan.MaxX-diminfo.VideoOScan.MinX+1;
														mh = diminfo.VideoOScan.MaxY-diminfo.VideoOScan.MinY+1;
														if (	( iff->BMH.w <= mw && iff->BMH.h <= mh && mh<800 ) ||
																	( dontCareH && mw >= iff->BMH.w && mh <= iff->BMH.h && mh<800 ) )
															*oscan = 4;
													}
												}
											}
										}

										if ( *oscan != -1 )
										{
											*w = mw;
											*h = mh;
											*maxDepth = diminfo.MaxDepth;
											*monID = ID;

											*modes = 0L;
											if ( dispinfo.PropertyFlags & DIPF_IS_LACE )
												*modes = LACE;
											if ( *h >= 400 )
												*modes = LACE;				

											return(TRUE);
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

	return(FALSE);
}

/******* GetOverscanValues() ********/

void GetOverscanValues(struct DimensionInfo *DimInfo, int oscan, WORD *w, WORD *h)
{
	if (oscan==0)
	{
		*w = DimInfo->Nominal.MaxX - DimInfo->Nominal.MinX + 1;
		*h = DimInfo->Nominal.MaxY - DimInfo->Nominal.MinY + 1;
	}
	else if (oscan==1)
	{
		*w = DimInfo->TxtOScan.MaxX - DimInfo->TxtOScan.MinX + 1;
		*h = DimInfo->TxtOScan.MaxY - DimInfo->TxtOScan.MinY + 1;
	}
	else if (oscan==2)
	{
		*w = DimInfo->StdOScan.MaxX - DimInfo->StdOScan.MinX + 1;
		*h = DimInfo->StdOScan.MaxY - DimInfo->StdOScan.MinY + 1;
	}
	else if (oscan==3)
	{
		*w = DimInfo->MaxOScan.MaxX - DimInfo->MaxOScan.MinX + 1;
		*h = DimInfo->MaxOScan.MaxY - DimInfo->MaxOScan.MinY + 1;
	}
	else if (oscan==4)
	{
		*w = DimInfo->VideoOScan.MaxX - DimInfo->VideoOScan.MinX + 1;
		*h = DimInfo->VideoOScan.MaxY - DimInfo->VideoOScan.MinY + 1;
	}
}

/******** E O F ********/
