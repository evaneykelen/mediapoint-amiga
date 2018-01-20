#include "nb:pre.h"
#include "protos.h"

#define MONLISTWIDTH 80

/**** externals ****/

extern struct RendezVousRecord *rvrec;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct EventData CED;
extern UWORD chip mypattern1[];
extern UBYTE **msgs;
extern struct MsgPort *capsport;

/**** functions ****/

/******** MakeMonitorList() ********/

int MakeMonitorList(UBYTE *monitorList, ULONG *IDS, UBYTE maxMonitors, BOOL laced,
										WORD width, WORD height, ULONG monitorID, STRPTR monName, int *listPos)
{
ULONG ID;
int ct,i;
struct DimensionInfo diminfo;
struct DisplayInfo dispinfo;
struct NameInfo nameinfo;
struct MonitorInfo moninfo;
TEXT genlock[50];
struct ExtendedNode *en;
TEXT monitorName[50], monListItem[MONLISTWIDTH], prev_monListItem[MONLISTWIDTH];
BOOL ok;

	ID = INVALID_ID;
	ct = 0;
	prev_monListItem[0]='\0';
	do
	{
		ID = NextDisplayInfo(ID);
		if ( ID != INVALID_ID )
		{
			if ( ID & MONITOR_ID_MASK )
			{
				if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),DTAG_DISP,ID) )
				{
					ok=TRUE;
					if ( !laced && (dispinfo.PropertyFlags & DIPF_IS_LACE) )
						ok=FALSE;
					if ( ok && dispinfo.NotAvailable==0L )
					{
						if ( dispinfo.PropertyFlags & DIPF_IS_GENLOCK )
							strcpy(genlock,msgs[Msg_P_Genlockable-1]);
						else
							strcpy(genlock,msgs[Msg_P_NotGenlockable-1]);

						if ( GetDisplayInfoData(NULL,(UBYTE *)&diminfo,sizeof(struct DimensionInfo),DTAG_DIMS,ID) )
						{
							if ( (diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1) >= 320 &&
									 (width==-1 || (diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1)==width) && 
									 (height==-1 || (diminfo.Nominal.MaxY-diminfo.Nominal.MinY+1)<=height) )
							{
								if ( GetDisplayInfoData(NULL,(UBYTE *)&nameinfo,sizeof(struct NameInfo),DTAG_NAME,ID) )
								{
									if ( GetDisplayInfoData(NULL,(UBYTE *)&moninfo,sizeof(struct MonitorInfo),DTAG_MNTR,ID) )
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

										if ( laced )
											sprintf(monListItem, "%s, %d × %d, %s",
															monitorName,
															diminfo.Nominal.MaxX-diminfo.Nominal.MinX+1,
															diminfo.Nominal.MaxY-diminfo.Nominal.MinY+1,
															genlock);
										else
											sprintf(monListItem, "%s, %s", monitorName, genlock);

										if ( prev_monListItem[0]=='\0' || strcmpi(prev_monListItem,monListItem) )
										{
											strcpy(monitorList+ct*MONLISTWIDTH,monListItem);
											strcpy(prev_monListItem,monListItem);

											*(IDS+ct) = ID;

											if ( (monitorID==0L || ID==monitorID) && !strcmpi(monName,monitorName) )
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

/******** E O F ********/
