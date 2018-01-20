/******************************************************
*Desc : Perform a ScriptTalk "PAGE" command
*		This routine will either wait for the command
*		to be finished or  
* <!> : This module is resident and re-entrant
*		Compile without -b1 and without -y options
*		Link with cres.o in stead of c.o
*		Also compile umain.c : lc -b1 umain 
*
*/

#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "demo:gen/general_protos.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

//#include "pascal:include/misctools.h"
//#include "pascal:include/txedtools.h"
//#include "pascal:include/textedit.h"
//#include "pascal:include/textstyles.h"

#include "pascal:include/toolslib.h"
#include "pascal:include/txed.h"
#include "pascal:include/txedstyles.h"
#include "pascal:include/txedtools.h"

#include "nb:capsdefines.h"
#include "nb:newdefines.h"
#include "nb:parser.h"
#include "nb:capsstructs.h"

#include "effect_images.h"
#include "parse.h"
#include "crawl/crawlstruct.h"
#include "minc:ge.h"
#include "inthandler.h"

#define VERSI0N "\0$VER: 1.3"
static UBYTE *vers = VERSI0N;

#define _PRINTF FALSE
#define _PRINTDEB FALSE
#define _PRINTSCRIPT FALSE

#define BUF_SIZE 1024
#define MAXSTRING 256

//
// External protos from pt.c
//
struct List *WP_InitFontList(void);
void WP_FreeFontList(struct List *list);
BOOL WP_ParseText(struct List *list, UBYTE *buffer, struct EditWindow *ew);
extern int FindVarContents(STRPTR varName, struct List *VIList, STRPTR answer);


int SafePutToPortQuit( struct Message *message, STRPTR portname);
void SendReady2( void );
//
// Load a file and return size and a mem pointer
//
char *load_file( char *name , LONG *file_size, struct Library *MLMMULibBase,  BOOL stay_on, BOOL check )
{
	char err[170];
	char *t_data = 0;
	FILE *fp;
	MEMTAG *memtag;
	int t;
	long fw;

// First check the name in the MLMMU lib

//	KPrintF("Page load %s\n",name );

	*file_size = 0;

	memtag = MLMMU_FindMem( name );
	if( memtag == NULL )
	{
		fp = fopen( name, "r" );
		if( fp != NULL )
		{
			fseek( fp, 0L ,SEEK_END );
			*file_size = ftell( fp)+2;		// add 2 for zero ending
			fseek( fp, 0L ,SEEK_SET );

			fread( &fw , 1, 4 , fp );
			fseek( fp, 0L ,SEEK_SET );

			if( fw == 0x50414745 || !check )			// is it a page ?
			{
				if( stay_on )
					t_data = (char *)MLMMU_AllocMem( *file_size, MEMF_PUBLIC|MEMF_STAY, name );
				else
					t_data = (char *)MLMMU_AllocMem( *file_size, MEMF_PUBLIC, name );

				if( t_data != NULL )
					t = fread( t_data , 1, (*file_size)-2 , fp );
			}
//			else
//				KPrintF("This file is not a Document !!!!!!!!1\n");

			fclose( fp );
			t_data[*file_size-2] = 0;
			return t_data;
		}
		else
		{
			strcpy(err,"File not found " );
			strcat(err, name );
			MLMMU_AddMsgToQueue(err, 1 );
			return NULL;
		}
	}
	else									// File is in the MLMMU lib
	{
//		KPrintF("Getting data from MMU\n");
		*file_size = memtag->mt_BlkSize;
		fw = (long)*(long*)memtag->mt_BlkBase;

		if( fw == 0x50414745 || !check )			// is it a page ?
		{
//			KPrintF("Getting page\n");
			if( stay_on )
				t_data = (char *)MLMMU_AllocMem( *file_size, MEMF_PUBLIC|MEMF_STAY, name );
			else
				t_data = (char *)MLMMU_AllocMem( *file_size, MEMF_PUBLIC, name );

//			if( memtag->mt_BlkBase != t_data )
//				KPrintF("What is wrong mother ??\n");
			t_data[*file_size-2] = 0;
			return t_data;
		}
		return NULL;
	}
}

int do_text_effect(	long *data, UBYTE *datablock,
											struct List *list, UBYTE *buffer,
											struct EditWindow *ew, long *wd, long *wf )
{
	int i;
	//char test[500];
	int stop;

	ew->X = wd[ 0 ];
	ew->Y = wd[ 1 ];
	ew->Width = wd[ 2 ];
	ew->Height = wd[ 3 ];

	ew->BorderColor[0] = wd[ 4 ];
	ew->BorderColor[1] = wd[ 5 ];
	ew->BorderColor[2] = wd[ 6 ];
	ew->BorderColor[3] = wd[ 7 ];
	ew->BorderWidth = wd[ 8 ];

//
// Check the presence of a pattern and the presence of the margins
// if there are no margins the fields will be -1000
// if 13 is -1000 it means there is a pattern but there is no margin info
//
	if( wd[ 12 ] != -1000 && wd[ 13 ]!= -1000 )
	{
		ew->TopMargin = wd[ 12 ];
		ew->RightMargin = wd[ 13 ];
		ew->BottomMargin = wd[ 14 ];
		ew->LeftMargin = wd[ 15 ];
	}
	else
	{
		ew->TopMargin = 0;					// everything on 0
		ew->RightMargin = 0;
		ew->BottomMargin = 0;
		ew->LeftMargin = 0;
	}

	ew->BackFillColor = wd[ 9 ];
	ew->BackFillType = wd[ 10 ]-1;

	ew->antiAliasLevel = wf[ 0 ];
	ew->justification = wf[ 1 ];
	ew->xSpacing = wf[ 2 ];
	ew->ySpacing = wf[ 3 ];
	ew->slantAmount = wf[ 4 ];
	ew->slantValue = wf[ 5 ];
	ew->underLineHeight = wf[ 6 ];
	ew->underLineOffset = wf[ 7 ];
	ew->shadowDepth = wf[ 8 ];
	ew->shadow_Pen = wf[ 9 ];
	ew->shadowType = wf[ 10 ];
	ew->shadowDirection = wf[ 11 ];

	WP_ParseText( list, buffer, ew);
	stop = create_text( data, datablock, ew );

	for( i = 0; i < 11; i++ )
	{
		wf[ i ] = 0;
		wd[ i ] = 0;
	}
	wd[11] = 0;
	wd[12] = 0;
	wd[13] = 0;
	wd[14] = 0;
	wd[15] = 0;

	buffer[0] = 0;							// clear the text just parsed
	return stop;
}

//
// Parse throught the file in memory
//
void parse( char *dat, long size , WORD *scriptargs, UBYTE *mem, struct Library *MLMMULibBase, struct List *VIlist, BOOL stay_on )
{
	struct Crawl_Record cr;

	int cut_w = FALSE;
	int stop = 0;
	int which_com;
	struct List *fontList;
	struct EditWindow ew;

	char win_found,clip_found,text_found,crawl_found;
	long t_size;
	char *buffer;			// BUF_SIZE
	char *filename;			// 100
	char *text = NULL;
	char *crawlblock = NULL;
	long crawlsize = 0;

	long *array;		// 20 used for temp and other data ( palet )
	long *win_array;	// 29 used for the window parameters
	long *clip_array;	// 20 used for the clip parameters
	long *text_array;	// 20 used for the text parameters
	long *format_array;	// 12 used for the format parameters

	char *p,*q;

	p = AllocMem(BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12), MEMF_ANY|MEMF_CLEAR);
	if ( !p )
		return;
	q=p;

	buffer = p;

	p = p + BUF_SIZE;
	filename = p;

	p = p + 100;
	array = (long *)p;

	p = p + 4*20;
	win_array = (long *)p;

	p = p + 4*29;
	clip_array = (long *)p;

	p = p + 4*20;
	text_array = (long *)p;	

	p = p + 4*20;
	format_array = (long *)p;

//KPrintF("Parsing\n");

	t_size = size;
	win_found = 0;
	clip_found = 0;
	text_found = 0;
	crawl_found = 0;

	if( scriptargs[2] == 72 )		// cut without erase
		cut_w = TRUE;

// take the bigger size or TEXTEDITSIZE there can be no more then TEXTEDITSIZE chars in a window
// the parser uses this as a maximum for the string

	text = MLMMU_AllocMem( size > TEXTEDITSIZE ? size : TEXTEDITSIZE, MEMF_PUBLIC|MEMF_CLEAR, NULL );

	if( text == NULL )
	{
		FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));
		return;
	}

	crawlblock = MLMMU_AllocMem( CRAWLBLOCK_SIZE, MEMF_PUBLIC|MEMF_CLEAR, NULL );
	if( crawlblock == NULL )
	{
		FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));
		return;
	}

	fontList = WP_InitFontList();
	if ( fontList == NULL )
	{
		MLMMU_FreeMem( text );
		FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));
		return;
	}

	ew.TEI = (struct TEInfo *)AllocMem(sizeof(struct TEInfo), MEMF_ANY);
	if ( ew.TEI == NULL )
	{
		MLMMU_FreeMem( text );
		WP_FreeFontList(fontList);
		FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));
		return;
	}

	ew.TEI->text = (struct TEChar *)AllocMem(	sizeof(struct TEChar)*TEXTEDITSIZE,
																							MEMF_ANY | MEMF_CLEAR);
	if ( ew.TEI->text == NULL )
	{
		MLMMU_FreeMem( text );
		WP_FreeFontList(fontList);
		FreeMem(ew.TEI, sizeof(struct TEInfo));
		FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));
		return;
	}

	while( t_size > 0 && !stop )
	{
		t_size -= skip( &dat, t_size );
		t_size -= read_word( &dat, t_size, buffer );
		which_com = get_command( buffer );
		switch( which_com )
		{
			case -1	: 	
							break;
			case 0	:	// pagetalk command
							t_size -= read_int_args( &dat, t_size, array , 2, 0 );
							break;
			case 1	:	//objectstart command
							break;
			case 2	:	// screen command
							t_size -= read_int_args( &dat, t_size, array , 6, -1000 );
							stop = create_screen( array, mem, scriptargs );
							if( cut_w )
								SendReady2();							// Send the go button command after create_screen ?
							break;
			case 3	:	// palette command 
							if( !cut_w )
							{
								t_size -= read_int_args( &dat, t_size, array , 1, 0 );
								t_size -= read_string_arg( &dat, t_size, buffer );
								array[1] = (long)buffer;
								set_colors( array, mem );
							}
							break;
			case 12 :		// background command
						if( cut_w )						// if so don't place this background
							break;
			case 4	:	 // window command
							cut_w = FALSE;				// from now on start the action
							if( crawl_found )
							{
								update_screen( mem );	// make sure everything is on screen
								cr.txt = text;
								stop = crawl( crawlblock, mem, &cr );
								text[0] = 0;
								crawl_found = 0;
							}
							if( win_found == 1 )				// window on hold ??
							{
								win_array[ 20 ] = -1;
								win_array[ 23 ] = -1;
								stop = create_window( win_array, mem );
							} 
							if( clip_found == 1 )				// clip on hold ??
							{
								clip_array[ 5 ] = -1;
								clip_array[ 8 ] = -1;
								stop = create_clip( clip_array, mem );
								clip_found = 0;
							}
							if( text_found == 1)
							{
								text_array[ 1 ] = -1;
								text_array[ 4 ] = -1;
								stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
								text_found = 0;
							}
							win_found = 1;
							t_size -= read_int_args( &dat, t_size, win_array , 20, -1000 );
							break;
			case 5	:	// clip command
							if( cut_w )				// if part off a background then skip this
								break;
							if( win_found == 1 )				// window on hold ??
							{
								win_array[ 20 ] = -1;
								win_array[ 23 ] = -1;
								stop = create_window( win_array, mem );
								win_found = 0;
							}
							if( clip_found == 1 )				// clip on hold ??
							{
								clip_array[ 5 ] = -1;
								clip_array[ 8 ] = -1;
								stop = create_clip( clip_array, mem );
							}
							if( text_found == 1)
							{
								text_array[ 1 ] = -1;
								text_array[ 4 ] = -1;
								stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
								text_found = 0;
							}
							clip_found = 1;
							t_size -= read_string_arg( &dat, t_size, filename );
							t_size -= read_int_args( &dat, t_size, &clip_array[1] , 4, 0 );
							clip_array[0] = (long)filename;
//				KPrintF("Filename in wp is %s\n",clip_array[0] );
							break;

			case 6 :		// text command
							text_found = 1;
							t_size -= read_add_string_arg( &dat, t_size, text, VIlist );
							break;

			case 7 :		// file command
							t_size -= read_string_arg( &dat, t_size, filename );
							break;

			case 8	:	// effect command
							t_size -= read_int_args( &dat, t_size, array , 1, 0 );
							switch( array[0] )
							{
								case 0 :
									t_size -= read_int_args( &dat, t_size, &win_array[20] , 8, 0 );
									stop = create_window( win_array, mem );
									win_found = 0;
									break;
								case 1 :
									if( win_found == 1 )				// window on hold ??
									{
										win_array[ 20 ] = -1;
										win_array[ 23 ] = -1;
										stop = create_window( win_array, mem );
										win_found = 0;
									}
									t_size -= read_int_args( &dat, t_size, &clip_array[5] , 8, 0 );
									stop = create_clip( clip_array, mem );
									clip_found = 0;
									break;
								case 2 :

// check window or clip on hold
									if( win_found == 1 )				// window on hold ??
									{
										win_array[ 20 ] = -1;
										win_array[ 23 ] = -1;
										stop = create_window( win_array, mem );
										win_found = 0;
									}
									if( clip_found == 1 )				// clip on hold ??
									{
										clip_array[ 5 ] = -1;
										clip_array[ 8 ] = -1;
										stop = create_clip( clip_array, mem );
										clip_found = 0;
									}

									t_size -= read_int_args( &dat, t_size, &text_array[1] , 8, 0 );
									stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
									text_found = 0;
									text[0] = 0;
									break;

								default :
									break;
							}
							break;

			case 9	:	// format command
							t_size -= read_int_args( &dat, t_size, format_array , 12, 0 );
							break;

			case 10	:	// crawl command
							if( win_found == 1 )				// window on hold ??
							{
								win_array[ 20 ] = -1;
								win_array[ 23 ] = -1;
								stop = create_window( win_array, mem );
								win_found = 0;
							}
							if( clip_found == 1 )				// clip on hold ??
							{
								clip_array[ 5 ] = -1;
								clip_array[ 8 ] = -1;
								stop = create_clip( clip_array, mem );
							}
							if( text_found == 1)
							{
								text_array[ 1 ] = -1;
								text_array[ 4 ] = -1;
								stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
								text_found = 0;
							}
							if( crawl_found == 1 )	// second time crawl
							{
								t_size -= read_add_string_arg( &dat, t_size, text, VIlist );
							}
							else									// crawl for the first time
							{
								cr.topmargin = 0;
								if( win_array[ 12 ] != -1000 && win_array[ 13 ]!= -1000 )
									cr.topmargin = win_array[ 12 ];
								cr.window_height = win_array[ 3 ];		// set height
								if( win_array[ 10 ] == 1 )
									cr.zero_col = win_array[ 9 ];
								else
									cr.zero_col = 0;
								cr.ypos = win_array[1];
								if( win_array[4] != -1 )
									cr.ypos += win_array[ 8 ];
								text[0] = 0;
								t_size -= read_string_arg( &dat, t_size, cr.fontName );
								strcat( cr.fontName,".font");
								t_size -= read_int_args( &dat, t_size, &cr.fontSize, 3, 0 );

								if( check_string_arg( dat, t_size ) )
								{
									t_size -= read_string_arg( &dat, t_size, filename );
//									KPrintF("Found string after crawl [%s]\n",filename);
									cr.txt = load_file( filename, &crawlsize, MLMMULibBase,stay_on,0 );
									if( cr.txt != NULL )
									{
										update_screen( mem );	// make sure everything is on screen
										stop = crawl( crawlblock, mem, &cr );
										crawl_found = 0;
										MLMMU_FreeMem( cr.txt );
									}
								}
								else
								{
//									KPrintF("Found nothing after crawl\n");
									crawl_found = 1;
								}
							}
							break;
			case 11	:	
							break;
			case 13	:	// animclip command
							if( win_found == 1 )				// window on hold ??
							{
								win_array[ 20 ] = -1;
								win_array[ 23 ] = -1;
								stop = create_window( win_array, mem );
								win_found = 0;
							}
							if( clip_found == 1 )				// clip on hold ??
							{
								clip_array[ 5 ] = -1;
								clip_array[ 8 ] = -1;
								stop = create_clip( clip_array, mem );
							}
							if( text_found == 1)
							{
								text_array[ 1 ] = -1;
								text_array[ 4 ] = -1;
								stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
								text_found = 0;
							}
							t_size -= read_string_arg( &dat, t_size, filename );
							t_size -= read_int_args( &dat, t_size, &clip_array[1] , 7, 0 );
							clip_array[0] = (long)filename;
//							KPrintF("Filename in wp is %s\n",clip_array[0] );
							stop = create_animclip( clip_array, mem );
							clip_found = 0;
							break;

			default	:	
						break;
		}
		t_size -= skip_eoln( &dat, t_size );
	}

	if( win_found == 1 )				// window on hold ??
	{
		win_array[ 20 ] = -1;
		win_array[ 23 ] = -1;
		stop = create_window( win_array, mem );
		win_found = 0;
	}
	if( clip_found == 1 )				// clip on hold ??
	{
		clip_array[ 5 ] = -1;
		clip_array[ 8 ] = -1;
		stop = create_clip( clip_array, mem );
	}
	if( text_found == 1)					// text on hold ??
	{
		text_array[ 1 ] = -1;
		text_array[ 4 ] = -1;
		stop = do_text_effect( text_array, mem, fontList, text, &ew, win_array, format_array );
		text_found = 0;
	}

	finish_doc( mem );

	if( crawl_found )
	{
		cr.txt = text;
		stop = crawl( crawlblock, mem, &cr );
		text[0] = 0;
		crawl_found = 0;
	}

	MLMMU_FreeMem( crawlblock );
	MLMMU_FreeMem( text );
	FreeMem(ew.TEI->text, sizeof(struct TEChar)*TEXTEDITSIZE);
	FreeMem(ew.TEI, sizeof(struct TEInfo));
	WP_FreeFontList(fontList);

	FreeMem(q, BUF_SIZE + 100 + 4*(20 + 29 + 20 + 20 + 12));

//	KPrintF("end Parsing\n");

}

// Set the names of the effects and give the main program
// the symbols with these names
//
void Set_Effect_Names( PROCESSINFO *ThisPI )
{
	LONG *tl;

	tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0]));
	*tl = (LONG)EffectData;	//_pl_1;

	//tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[2]));
	//*tl = (LONG)EffectData_pl_2;

	tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[4]));
	*tl = (LONG)effectNames;

	tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[6]));
	*tl = (LONG)brushNames;

	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[8] = NUMEFFECTS;
	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[9] = NUMBRUSH;

	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[10] = sizeof(effectNames);
	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[11] = sizeof(brushNames);

	tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[12]));
	*tl = (LONG)effectNumbers;

	tl = (LONG *)(&(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[14]));
	*tl = (LONG)brushNumbers;
}

/*************************************************
*	Func	:	Perform a dos command
* 	<!>	:	This command will set up a process for
*				the command and leave on its own.
*	in   :	Argv -> Ptr to PROCESSINFO.pi_Startup
*	out  :	-
*/

void main( int argc, char **argv)
{
	BOOL	stay_on,B_IntAdded;
	int	InputDeviceErr;

	OBJECT_INPUT_STRUCT obj_inp;

	char *doc_mem = NULL;
	long doc_size = 0;

	char FileName[MAXSTRING];
	char Varcont[MAXSTRING];

	UBYTE		*WipeDataSegment;
	int 		ErrSysPass,
				ErrLoadWipe;	
	char		*PictureSegment;
	
	PROCDIALOGUE	*Msg_PageDial,		// Our dialogue 
						*Msg_PageDial2,
						*Msg_RPageDial;	// Our dialogue when our guide replies

	MLSYSTEM		*MLSystem;	
	PROCESSINFO	*ThisPI;			// ptr to this processinfo blk (as used in our parent's list)
	SNR			*ThisSNR;			// used only by the TRANSITION module,not by the workerproc

	struct	MsgPort *RepP_WorkPage;	// Reply port for our parent when replying to our messages
	ULONG	Sig_PtoC,				// A parent to child signal
    		SigR_CtoP,				// A reply to a msg we send to our parent
			SigRecvd;				// Signals received
	int 	i;
	BOOL 	B_ReInit,				// if TRUE, re-initialise data
			B_Term,					// If TRUE, we are free to terminate
			B_Run,			
			B_Stop,
			B_Remove;				// If True, our guide wants us to clean up

	BOOL	VARFile, Document = FALSE;

	struct Library *MLMMULibBase = NULL;
	long oldpri;
	struct Task *task;

	obj_inp.SigNum_NO = 0;
	PictureSegment = NULL;

	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

//	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[15] = 0;

	if( ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		Set_Effect_Names( ThisPI );
		return;
	}

	/**************************************************************/
	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;
	RepP_WorkPage = NULL;
	Msg_PageDial = NULL;
	if( 
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkPage = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_PageDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_PageDial);
		if(RepP_WorkPage)
			DeletePort(RepP_WorkPage);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);

		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	Msg_PageDial2 = NULL;

// Program init

	if(ThisPI->pi_Node.ln_Type == NT_WORKERPROC)
	{
		MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,ThisPI->pi_Arguments.ar_Worker.aw_Name,FileName);
		//	Check of the file is a Document
		if( ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[15] == 1 )
			Document = TRUE;
	}
	else
	{
		Msg_PageDial2 = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL );
		MLSystem = ThisPI->pi_Arguments.ar_Module.am_MLSystem;
		if( (ThisPI->pi_Port_PtoC = (struct MsgPort *)CreatePort("Port_Transition",0)) == NULL)
		{
			DeletePort(RepP_WorkPage);
			MLMMU_FreeMem(Msg_PageDial);
			MLMMU_FreeMem(Msg_PageDial2);
			CloseLibrary(MLMMULibBase);
			ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
			return;
		}
	}

//	KPrintF("S[%s],%ld\n",FileName,AvailMem( MEMF_FAST ) );

	if ( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] == '@' )
	{
		VARFile = TRUE;
		Document = FALSE;
	}
	else
		VARFile = FALSE;

	if( Document || VARFile )		// when variable it could be a document
	{
		B_IntAdded = FALSE;
		if( SetupInputHandler(	&obj_inp ) != NO_ERROR )
			return;
	}

	WipeDataSegment = NULL;
	ErrSysPass = TRUE;
	ErrLoadWipe = TRUE;

	if( MLSystem->miscFlags & 1L << 6 )
		stay_on = TRUE;
	else
		stay_on = FALSE;

	if( stay_on && !VARFile )
	{
		if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
		{
			if( (WipeDataSegment = (UBYTE *)MLMMU_AllocMem( get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL)) != NULL)
			{
				if(ThisPI->pi_Node.ln_Type == NT_WORKERPROC)
				{
					if((ErrSysPass = (int)pass_mlsystem(MLSystem,WipeDataSegment, ThisPI->pi_Port_PtoC, effectNumbers,obj_inp.SigNum_NO,patterns )) == 0)
					{
						if(ErrSysPass == 0)
						{
							if( Document )
							{
								doc_mem = load_file( FileName, &doc_size, MLMMULibBase, stay_on,1 );
								if( doc_mem != NULL )
									ErrLoadWipe = 0;
								else
									ErrLoadWipe = 1;
							}
							else
								if( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] != 0 )
									ErrLoadWipe = (int)load_wipe(FileName,WipeDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs,1L);
						}
					}
				}
			}
		}
	}

//	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;

	// Set up the Dialogue message

	Msg_PageDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_PageDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_PageDial->pd_Msg.mn_ReplyPort = RepP_WorkPage;

	// Our guide will reply to us when we must start

	SigR_CtoP = 1 << RepP_WorkPage->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start

	if( ThisPI->pi_Node.ln_Type == NT_WORKERPROC )
		SendDialogue( Msg_PageDial,ThisPI,DCI_CHILDREADY );
	else
	{
		// Set up a second Dialogue message

		if( Msg_PageDial2 )
		{
			Msg_PageDial2->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
			Msg_PageDial2->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
			Msg_PageDial2->pd_Msg.mn_ReplyPort = RepP_WorkPage;
		}
	}

// main

	B_Stop = FALSE;
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;


	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & SIGF_ABORT)
		{
			if(ThisPI->pi_State != -1)
				B_Term = TRUE;
			else
				ThisPI->pi_State = ST_READY;
		}

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RPageDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				switch( Msg_RPageDial->pd_Cmd )
				{
					case DCC_DOPREPARE:
							Msg_RPageDial->pd_Cmd = DCI_CHILDPREPARES;
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
								B_Run = TRUE;
							Msg_RPageDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
							Msg_RPageDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOSTOP:
							Msg_RPageDial->pd_Cmd = DCI_CHILDREADY;
							if(ThisPI->pi_Node.ln_Type == NT_WORKERPROC)
							{
								B_Stop = TRUE;
								B_Run = FALSE;
							}
							else
								if(ThisPI->pi_Node.ln_Type == NT_MODULE)
								{
									B_Run = FALSE;
									if(PictureSegment != NULL)		
										ReleaseSemaphore(&MLSystem->ms_Sema_Transition);
									PictureSegment = NULL;
								}
							break;
					case DCC_DOEASYTERM:
							Msg_RPageDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RPageDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;

					case DCC_DOTRANSITION:
							if(!B_Term)
							{
								ThisSNR = Msg_RPageDial->pd_Luggage.lu_SNR;
								ThisPI->pi_Arguments.ar_Worker.aw_NumArgs =	ThisSNR->numericalArgs;
								ThisPI->pi_Arguments.ar_Worker.aw_Path = ThisSNR->objectPath;
								ThisPI->pi_Arguments.ar_Worker.aw_Name = ThisSNR->objectName;
								ThisPI->pi_Arguments.ar_Worker.aw_ExtraData = ThisSNR->extraData;
								ThisPI->pi_Arguments.ar_Worker.aw_ExtraDataSize = &ThisSNR->extraDataSize;
								if( ThisSNR->nodeType == TALK_ANIM )
								{
									B_Run = TRUE;
									ObtainSemaphore(&MLSystem->ms_Sema_Transition);
									PictureSegment = (char *)Msg_RPageDial->pd_Luggage.lu_Dial;
								}
							}
							Msg_RPageDial->pd_Cmd = DCI_TRANSITION;
							break;	

					default:
							// simply ignore what we don't understand
							Msg_RPageDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				Msg_RPageDial->pd_ChildPI = ThisPI;
				ReplyMsg((struct Message *)Msg_RPageDial);
			}
		}

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RPageDial = (PROCDIALOGUE *)GetMsg(RepP_WorkPage)) != NULL)
				Msg_RPageDial->pd_InUse = FALSE;

		if(B_Stop)
		{
			if( !ErrLoadWipe )
			{
				if( !Document )
					unload_file(WipeDataSegment);
				else
					if( doc_mem != NULL )
					{
						MLMMU_FreeMem( doc_mem );
						doc_mem = NULL;
					}
			}
			ErrLoadWipe = TRUE;	

			if( !ErrSysPass )
				release_slide2( WipeDataSegment );

			ErrSysPass = TRUE;
			MLMMU_FreeMem( WipeDataSegment );
			WipeDataSegment = NULL;
			B_Stop = FALSE;
		}

		if( !B_Remove && !B_Term )
		{
			if( B_Run )
			{
				// see if the picture is NOT in memory yet
				// Last effort to get the picture into memory!
				if( ErrLoadWipe )
				{
					if(MLMMU_AvailMem(MEMF_PUBLIC) > 50000)
					{
						if( WipeDataSegment == NULL )
							WipeDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(), MEMF_PUBLIC|MEMF_CLEAR,NULL);
						if( WipeDataSegment != NULL )
						{
							if( ErrSysPass != 0 )
								ErrSysPass = (int)pass_mlsystem(MLSystem,WipeDataSegment, ThisPI->pi_Port_PtoC, effectNumbers, obj_inp.SigNum_NO, patterns );
							if( ErrSysPass == 0 )
							{
								if( ( ThisPI->pi_Node.ln_Type == NT_WORKERPROC ) && ( ErrLoadWipe != 0 ) && ThisPI->pi_Arguments.ar_Worker.aw_Name[0] != 0 )
								{
									if( VARFile )
									{
//										KPrintF("VARFile 1 true\n");
										FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_Name,MLSystem->VIList, Varcont );
//										KPrintF("Found var [%s]\n",Varcont );
										strcpy( FileName, Varcont );
									}

									doc_size = 1;
									if( VARFile || Document )
										doc_mem = load_file( FileName, &doc_size, MLMMULibBase, stay_on,1 );

									if( doc_mem != NULL )
									{
										ErrLoadWipe = 0;
										Document = TRUE;
									}
									else
										if( doc_size != 0 )		// something was there just not a document
										{
											ErrLoadWipe = (int)load_wipe( FileName,WipeDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs,0L );
//											KPrintF("Load file [%s]\n",FileName );
										}
										else
											ErrLoadWipe = 1;

								}
								else
								{
									if(  ThisPI->pi_Node.ln_Type != NT_WORKERPROC && ThisSNR->nodeType == TALK_ANIM )
									{
										ErrLoadWipe = load_mem_wipe(PictureSegment,WipeDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs);
									}
								}
							}
						}
					}
				}

				if( ErrLoadWipe == 0 )
				{
					if( PictureSegment != NULL )
					{
						// setting ErrLoadWipe to FALSE and a guru will welcome you!
						// actually, it is not permitted to unload a PictureSegment

						ErrLoadWipe = TRUE;
					}
					else
					{
						// obtain semaphore when we're a workerprocess
						// when being a moduleprocess then this sema was obtained
						// earlier.
						ObtainSemaphore( &MLSystem->ms_Sema_Transition );
					}

					if( VARFile )
					{
//						KPrintF("VARFile 2 true\n");

						FindVarContents(ThisPI->pi_Arguments.ar_Worker.aw_Name,MLSystem->VIList, Varcont );

						if( strcmp( FileName, Varcont ) )			// Is the file loaded the right one
						{
							if( !Document )		// Unload document ????
								unload_file(WipeDataSegment);
							else
								if( doc_mem != NULL )
								{
									MLMMU_FreeMem( doc_mem );
									doc_mem = NULL;
									Document = FALSE;
								}
							strcpy( FileName, Varcont );
							doc_size = 1;
							if( VARFile || Document )
								doc_mem = load_file( FileName, &doc_size, MLMMULibBase, stay_on,1 );
							if( doc_mem != NULL )
							{
								ErrLoadWipe = 0;
								Document = TRUE;
							}
							else
								if( doc_size != 0 )		// something was ther just not a document
									ErrLoadWipe = (int)load_wipe( FileName,WipeDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs,0L );
								else
									ErrLoadWipe = 1;
						}
					}

					if( ErrLoadWipe == 0 || PictureSegment != NULL )
						if( Document )
						{
							task = FindTask( 0 );
							oldpri = SetTaskPri(task, 20 );

							B_IntAdded = AddInputHandler( &obj_inp );

							parse( doc_mem, doc_size, ThisPI->pi_Arguments.ar_Worker.aw_NumArgs, WipeDataSegment, MLMMULibBase, MLSystem->VIList, stay_on );

							if( B_IntAdded )
								RemoveInputHandler( &obj_inp );

							SetTaskPri( task,oldpri );
						}
						else
						{
							task = FindTask( 0 );
							oldpri = SetTaskPri(task, 20 );
//KPrintF("TRAN WIPE IN\n");
							wipe_in( WipeDataSegment );
							SetTaskPri( task,oldpri );
						}

					ReleaseSemaphore( &MLSystem->ms_Sema_Transition );
					PictureSegment = NULL;

// Running is ready you can wait for the jiffies here

				}
				else
				{
					if( PictureSegment != NULL )
					{
						ReleaseSemaphore( &MLSystem->ms_Sema_Transition );
						PictureSegment = NULL;
					}
				}
				B_Run = FALSE;
			}

			if( B_ReInit )
			{
				if( MLMMU_AvailMem(MEMF_PUBLIC) > 50000 )
				{
					if( WipeDataSegment == NULL )
						WipeDataSegment = (UBYTE *)MLMMU_AllocMem(get_varsize(),MEMF_PUBLIC|MEMF_CLEAR,NULL);

					if( WipeDataSegment != NULL )
					{
						if( ErrSysPass )
							ErrSysPass = (int)pass_mlsystem(MLSystem,WipeDataSegment, ThisPI->pi_Port_PtoC, effectNumbers, obj_inp.SigNum_NO, patterns );
						if( !ErrSysPass )
						{
							if( ErrLoadWipe )
							{
								if( ( ThisPI->pi_Node.ln_Type == NT_WORKERPROC) && ( ErrLoadWipe != 0 ) )
								{
									if( Document )
									{
										doc_mem = load_file( FileName, &doc_size, MLMMULibBase, stay_on,1 );
										if( doc_mem != NULL )
											ErrLoadWipe = 0;
										else
											ErrLoadWipe = 1;
									}
									else
										if( ThisPI->pi_Arguments.ar_Worker.aw_Name[0] != 0 )
											ErrLoadWipe = (int)load_wipe( FileName,WipeDataSegment,ThisPI->pi_Arguments.ar_Worker.aw_NumArgs,0L );
								}
							}	
						}
					}
				}
				B_ReInit = SendDialogue(Msg_PageDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;
			for(i = 0; i < DIAL_MAXPTOC; i++)
				if( ThisPI->pi_Node.ln_Type == NT_WORKERPROC )
				{
					if( ((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse )
					{
						B_Term = FALSE;
					}
				}

			if( Msg_PageDial->pd_InUse || ( ( ThisPI->pi_Node.ln_Type == NT_MODULE) && Msg_PageDial2->pd_InUse ) )
			{	
				B_Term = FALSE;
			}
		}

		// Check if there are still messages in the portlist
		// if so then signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	if(ThisPI->pi_Node.ln_Type == NT_MODULE)
		while((Msg_RPageDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
		{

			//if( Msg_RPageDial->pd_Cmd == DCC_DOTERM || Msg_RPageDial->pd_Cmd == DCC_DOEASYTERM )
			//	KPrintF("Double Term WP\n");

			Msg_RPageDial->pd_Cmd = DCI_IGNORE;
			ReplyMsg((struct Message *)Msg_RPageDial);
		}	


	if( !ErrLoadWipe && !doc_mem )
	{		
//		KPrintF("WP[:]unload %s\n",FileName );
		unload_file(WipeDataSegment);
	}

	if(!ErrSysPass)
		release_slide2(WipeDataSegment);

	if( Document || VARFile )
		FreeInputHandler( &obj_inp );

	if( doc_mem != NULL )
	{
//		KPrintF("WP[:]unload document %s\n",FileName );
		MLMMU_FreeMem( doc_mem );
	}

//KPrintF("Doing the FreeMem1\n");
	MLMMU_FreeMem(WipeDataSegment);
//KPrintF("Doing the FreeMem2\n");
	MLMMU_FreeMem(Msg_PageDial2);
	MLMMU_FreeMem(Msg_PageDial);
	DeletePort(RepP_WorkPage);
	CloseLibrary(MLMMULibBase);

	if(ThisPI->pi_Node.ln_Type == NT_MODULE)
	{
		DeletePort(ThisPI->pi_Port_PtoC);
		ThisPI->pi_Port_PtoC = NULL;
	}

//	KPrintF("T[%s],%ld\n",FileName,AvailMem( MEMF_FAST ) );

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
}
