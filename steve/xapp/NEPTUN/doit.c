#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"
#include "demo:gen/general_protos.h"
#include <libraries/dosextens.h>
#include "protos.h"
#include <graphics/videocontrol.h>

extern struct Library *medialinkLibBase;

STATIC void SwitchGenlockOn(PROCESSINFO *ThisPI);

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct Neptun_record nep_rec;

	Forbid();
	if ( !FindPort("Neptun") )
	{
		Permit();
		return(0);
	}
	Permit();

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
		if ( medialinkLibBase )
		{
			SwitchGenlockOn(ThisPI);

			nep_rec.version=0;
			Forbid();
			GetNepVersion(&nep_rec);
			Permit();

			GetVarsFromPI(&nep_rec, ThisPI);
			Forbid();
			PerformActions(&nep_rec, ThisPI);
			Permit();

			CloseLibrary((struct Library *)medialinkLibBase);
		}
	}

	return(0);
}

/******** GetVarsFromPI() ********/

void GetVarsFromPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI)
{
	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %s %d %d %d %d",

					&nep_rec->page,

					&nep_rec->computer1,
					&nep_rec->genlock_amiga,

					&nep_rec->video1,
					&nep_rec->normal_invert,

					&nep_rec->overlay1,
					&nep_rec->normal_alpha,

					&nep_rec->computer2,
					&nep_rec->fadein_fadeout1,

					&nep_rec->video2,
					&nep_rec->fadein_fadeout2,

					&nep_rec->computer3,
					nep_rec->duration1,
					&nep_rec->from1,
					&nep_rec->pct1f,
					&nep_rec->to1,
					&nep_rec->pct1t,

					&nep_rec->video3,
					nep_rec->duration2,
					&nep_rec->from2,
					&nep_rec->pct2f,
					&nep_rec->to2,
					&nep_rec->pct2t );
}

/******** PutVarsToPI() ********/

void PutVarsToPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI)
{
	sprintf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
						"%d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %s %d %d %d %d",

						nep_rec->page,

						nep_rec->computer1,
						nep_rec->genlock_amiga,

						nep_rec->video1,
						nep_rec->normal_invert,

						nep_rec->overlay1,
						nep_rec->normal_alpha,

						nep_rec->computer2,
						nep_rec->fadein_fadeout1,

						nep_rec->video2,
						nep_rec->fadein_fadeout2,

						nep_rec->computer3,
						nep_rec->duration1,
						nep_rec->from1,
						nep_rec->pct1f,
						nep_rec->to1,
						nep_rec->pct1t,

						nep_rec->video3,
						nep_rec->duration2,
						nep_rec->from2,
						nep_rec->pct2f,
						nep_rec->to2,
						nep_rec->pct2t );
}

/******** PerformActions() ********/

BOOL PerformActions(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI)
{
BOOL success=TRUE;
TEXT which[16], str[256], dur[12];
int from,to,i,do1,do2,pct;

	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET REMOTE",NULL,NULL);

	if ( nep_rec->page==0 )
	{
		if ( nep_rec->video1 )
		{
			if ( nep_rec->normal_invert==0 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET KEY NORMAL",NULL,NULL);
			if ( nep_rec->normal_invert==1 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET KEY INVERT",NULL,NULL);
		}
		if ( nep_rec->computer1 )
		{
			if ( nep_rec->genlock_amiga==0 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET MONITOR GENLOCK",NULL,NULL);
			if ( nep_rec->genlock_amiga==1 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET MONITOR AMIGA",NULL,NULL);
		}
		if ( nep_rec->overlay1 )
		{
			if ( nep_rec->normal_alpha==0 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET ALPHA OFF",NULL,NULL);
			if ( nep_rec->normal_alpha==1 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET ALPHA ON",NULL,NULL);
		}
	}
	else if ( nep_rec->page==1 )
	{
		if ( nep_rec->video2 )
		{
			success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET VIDEO TIME 02.00 SEC",NULL,NULL);
			if ( nep_rec->fadein_fadeout1==0 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","FADE VIDEO UP",NULL,NULL);
			if ( nep_rec->fadein_fadeout1==1 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","FADE VIDEO DOWN",NULL,NULL);
		}
		if ( nep_rec->computer2 )
		{
			success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","SET AMIGA TIME 02.00 SEC",NULL,NULL);
			if ( nep_rec->fadein_fadeout2==0 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","FADE AMIGA UP",NULL,NULL);
			if ( nep_rec->fadein_fadeout2==1 )
				success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","FADE AMIGA DOWN",NULL,NULL);
		}
	}
	else if ( nep_rec->page==2 )
	{
		for(i=0; i<2; i++)
		{
			which[0]='0';
			if (i==0 && nep_rec->video3 )
				strcpy(which,"VIDEO");
			if (i==1 && nep_rec->computer3 )
				strcpy(which,"COMPUTER");
			if ( which[0] )
			{
				str[0]='\0';

				if (i==0)
					pct=nep_rec->pct1f;
				if (i==1)
					pct=nep_rec->pct2f;
				
				from = (pct*256)/100;
				if ( nep_rec->version==1 )	// BUG FIX
				{
					if ( from<=0 )
						from=1;
					if ( from>=255 )
						from=254;
				}

				if (i==0)
					pct=nep_rec->pct1t;
				if (i==1)
					pct=nep_rec->pct2t;

				to = (pct*256)/100;
				if ( nep_rec->version==1 )	// BUG FIX
				{
					if ( to<=0 )
						to=1;
					if ( to>=255 )
						to=254;
				}

				if ( i==0 )
					strcpy(dur,nep_rec->duration1);
				if ( i==1 )
					strcpy(dur,nep_rec->duration2);

				if ( i==0 )
					do1 = nep_rec->from1;
				if ( i==1 )
					do1 = nep_rec->from2;

				if ( i==0 )
					do2 = nep_rec->to1;
				if ( i==1 )
					do2 = nep_rec->to2;

				if ( do1 && !do2 )
				{
					sprintf(str, "SET %s TIME %s SEC", which, dur);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
					sprintf(str, "SET %s FADER %d", which, from);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
				}
				if ( do1 && do2 )
				{
					sprintf(str, "SET %s TIME %s SEC", which, dur);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
					sprintf(str, "SET %s FADER %d", which, from);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
					sprintf(str, "FADE %s TO %d", which, to);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
				}
				if ( !do1 && do2 )
				{
					sprintf(str, "SET %s TIME %s SEC", which, dur);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
					sprintf(str, "FADE %s TO %d", which, to);
					success=UA_IssueRexxCmd_V2("MP_NEP","Neptun",str,NULL,NULL);
				}
			}
		}
	}

	return(success);
}

/******** GetNepValues() ********/

BOOL GetNepValues(struct Neptun_record *nep_rec, int which)
{
TEXT result[50];
BOOL success=TRUE;
int val;

	switch(which)
	{
		case 1:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET MONITOR STATUS",result,NULL);
						if (success)
						{
							if ( nep_rec->version==1 )
							{
								if ( !strcmpi(result,"GENLOCK") )
									nep_rec->genlock_amiga = 1;	// ERROR IN NEPTUN AREXX HOST!
								else
									nep_rec->genlock_amiga = 0;	// ERROR IN NEPTUN AREXX HOST!
							}
							else if ( nep_rec->version==2 )
							{
								if ( !strcmpi(result,"GENLOCK") )
									nep_rec->genlock_amiga = 0;
								else
									nep_rec->genlock_amiga = 1;
							}
						}
						break;

		case 2:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET KEY STATUS",result,NULL);
						if (success)
						{
							if ( !strcmpi(result,"NORMAL") )
								nep_rec->normal_invert = 0;
							else
								nep_rec->normal_invert = 1;
						}
						break;

		case 3:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET ALPHA STATUS",result,NULL);
						if (success)
						{
							if ( !strcmpi(result,"OFF") )
								nep_rec->normal_alpha = 0;
							else
								nep_rec->normal_alpha = 1;
						}
						break;

		case 4:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET VIDEO POTI",result,NULL);
						if (success)
						{
							sscanf(result,"%d",&val);	// 0...255
							nep_rec->pct1f = (val*100)/256;
						}
						break;

		case 5:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET AMIGA POTI",result,NULL);
						if (success)
						{
							sscanf(result,"%d",&val);	// 0...255
							nep_rec->pct2f = (val*100)/256;
						}
						break;

		case 6:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET VIDEO POTI",result,NULL);
						if (success)
						{
							sscanf(result,"%d",&val);	// 0...255
							nep_rec->pct1t = (val*100)/256;
						}
						break;

		case 7:	success=UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET AMIGA POTI",result,NULL);
						if (success)
						{
							sscanf(result,"%d",&val);	// 0...255
							nep_rec->pct2t = (val*100)/256;
						}
						break;
	}

	return(success);
}

/******** GetNepVersion() ********/

void GetNepVersion(struct Neptun_record *nep_rec)
{
TEXT result[50];

	result[0] = '\0';
	if ( UA_IssueRexxCmd_V2("MP_NEP","Neptun","GET VERSION",result,NULL) )
	{
		if ( strlen(result)==9 )
			nep_rec->version = 1;	// old
		else
			nep_rec->version = 2;	// new
	}
}

/******** SwitchGenlockOn() ********/

STATIC void SwitchGenlockOn(PROCESSINFO *ThisPI)
{
struct TagItem ti[10];
struct GfxBase *GfxBase;
MLSYSTEM *MLSystem;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if ( !GfxBase )
		return;

	ti[0].ti_Tag	= VTAG_BORDERNOTRANS_CLR;
	ti[0].ti_Data	= NULL;

	ti[1].ti_Tag	= VTAG_CHROMAKEY_SET;
	ti[1].ti_Data	= NULL;

	ti[2].ti_Tag	= VTAG_CHROMA_PEN_SET;
	ti[2].ti_Data = 0;

	ti[3].ti_Tag	= VTAG_END_CM;
	ti[3].ti_Data = NULL;

	Forbid();

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;
	CopyMem(ti, &MLSystem->taglist[3], sizeof(struct TagItem)*4);
	MLSystem->miscFlags |= 0x00000020L;

	Permit();

	CloseLibrary((struct Library *)GfxBase);
}

/******** E O F ********/
