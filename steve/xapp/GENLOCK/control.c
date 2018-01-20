#include "nb:pre.h"
#include <graphics/videocontrol.h>
#include "structs.h"
#include "demo:gen/wait50hz.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;
extern struct RendezVousRecord *rvrec;

/**** functions ****/

/******** PerformActions() ********/

BOOL PerformActions(struct Genlock_record *gl_rec, struct Window *window,
										PROCESSINFO *ThisPI)
{
	if ( window )
		GL_GenlockOn(gl_rec,window->WScreen,NULL);
	else
		GL_GenlockOn(gl_rec,NULL,ThisPI);

	if (window)	// preview mode
	{
		Delay(100L);
		GL_GenlockOff(window->WScreen);
	}

	return(TRUE);
}

/******** GL_GenlockOn() ********/

void GL_GenlockOn(struct Genlock_record *gl_rec, struct Screen *screen,
									PROCESSINFO *ThisPI)
{
struct TagItem ti[270];
int i,t,pens;
ULONG chroma=0L;
struct GfxBase *GfxBase;
BOOL AA, GENLOCK;
MLSYSTEM *MLSystem;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if ( !GfxBase )
		return;

	if ((GfxBase->ChipRevBits0 & SETCHIPREV_AA)==SETCHIPREV_AA)
		AA = TRUE;	
	else
		AA = FALSE;

	GENLOCK = FALSE;

/*
	This is done in ph:procinit because Cees needs it.
	ti[0].ti_Tag  = VTAG_ATTACH_CM_SET;
	ti[0].ti_Data = NULL;
	ti[1].ti_Tag  = VTAG_VIEWPORT_EXTRA_SET;
	ti[1].ti_Data = NULL;
	ti[2].ti_Tag  = VTAG_NORMAL_DISP_SET
	ti[2].ti_Data = 0;
*/

	t=0;

	switch ( gl_rec->mode )
	{
		case 0:	// normal (everywhere color 0 is video)
			ti[t].ti_Tag	= VTAG_BORDERNOTRANS_CLR;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMA_PEN_SET;
			ti[t].ti_Data = 0;
			t++;
			GENLOCK = TRUE;	// There's video in border -> Transitions must do special things
			break;

		case 1:	// total video, no amiga anywhere
			chroma = VTAG_CHROMA_PEN_SET;
			ti[t].ti_Tag	= VTAG_BORDERNOTRANS_CLR;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
			ti[t].ti_Data	= NULL;
			t++;
			GENLOCK = TRUE;	// There's video in border -> Transitions must do special things
			break;

		case 2:	// total amiga, no video anywhere
			//chroma = VTAG_CHROMA_PEN_CLR;
			ti[t].ti_Tag	= VTAG_BORDERNOTRANS_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMA_PEN_CLR;
			ti[t].ti_Data	= 0;
			t++;
			break;

		case 3:	// only video in border
			//chroma = VTAG_CHROMA_PEN_CLR;
			ti[t].ti_Tag	= VTAG_BORDERBLANK_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMA_PEN_CLR;
			ti[t].ti_Data	= 0;
			t++;
			GENLOCK = TRUE;	// There's video in border -> Transitions must do special things
			break;

		case 4:	// video and amiga anywhere BUT border
			ti[t].ti_Tag	= VTAG_BORDERBLANK_CLR;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_BORDERNOTRANS_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
			ti[t].ti_Data	= NULL;
			t++;
			ti[t].ti_Tag	= VTAG_CHROMA_PEN_SET;
			ti[t].ti_Data = 0;
			t++;
			break;
	}

	if ( chroma )
	{
		if ( AA ) 
			pens = 256;
		else
			pens = 32;
		for(i=0; i<pens; i++)
		{
			ti[t+i].ti_Tag	= chroma;
			ti[t+i].ti_Data = i;
		}
		t=t+pens;
	}

	ti[t].ti_Tag	= VTAG_END_CM;
	ti[t].ti_Data = NULL;
	t++;

	if ( screen )
	{
		if ( VideoControl(screen->ViewPort.ColorMap, ti)==NULL )
		{
			MakeScreen(screen);
			RethinkDisplay();
		}
	}
	else
	{
		Forbid();
		MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;
		CopyMem(ti, &MLSystem->taglist[3], sizeof(struct TagItem)*t);
		if ( gl_rec->alwaysLaced )
			MLSystem->miscFlags |= 0x00000010L;		// always laced is default
		else
			MLSystem->miscFlags &= ~0x00000010L;
		if ( GENLOCK )
			MLSystem->miscFlags |= 0x00000020L;
		else
			MLSystem->miscFlags &= ~0x00000020L;
		Permit();
/*
{
char str[50];
sprintf(str,"IN GENLOCK %x %d\n",MLSystem->taglist,t);
KPrintF(str);
}
*/
	}

	CloseLibrary((struct Library *)GfxBase);
}

/******** GL_GenlockOff() ********/

void GL_GenlockOff(struct Screen *screen)
{
struct TagItem ti[270];
int i,t,pens;
BOOL AA;

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if ( !GfxBase )
		return;

	if ((GfxBase->ChipRevBits0 & SETCHIPREV_AA)==SETCHIPREV_AA)
		AA = TRUE;	
	else
		AA = FALSE;

	t=0;

	ti[t].ti_Tag	= VTAG_BORDERBLANK_CLR;
	ti[t].ti_Data	= NULL;
	t++;

	ti[t].ti_Tag	= VTAG_CHROMAKEY_SET;
	ti[t].ti_Data	= NULL;
	t++;

	if ( AA ) 
		pens = 256;
	else
		pens = 32;
	for(i=0; i<pens; i++)
	{
		ti[t+i].ti_Tag	= VTAG_CHROMA_PEN_CLR;
		ti[t+i].ti_Data = i;
	}
	t=t+32;

	ti[t].ti_Tag	= VTAG_END_CM;
	ti[t].ti_Data = NULL;

	if ( VideoControl(screen->ViewPort.ColorMap,ti)==NULL )
	{
		MakeScreen(screen);
		RethinkDisplay();
	}

	CloseLibrary((struct Library *)GfxBase);
}

/******** E O F ********/
