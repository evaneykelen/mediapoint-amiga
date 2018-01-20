#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include <dos/dostags.h>

/**** externals ****/

extern struct Library *medialinkLibBase;
extern UBYTE **msgs;
extern struct RendezVousRecord *rvrec;
extern struct GadgetRecord RA2_GR[];
extern struct EventData CED;
extern UWORD chip mypattern1[];
extern struct Gadget PropSlider1;
extern int buttonTouched;

/**** globals ****/

FILE *logfile;
struct Window *reportWdw;

/**** functions ****/

/******** MonitorUpload() ********/

BOOL MonitorUpload(struct Window *onWindow, struct SessionRecord *session_rec)
{
BOOL retval = FALSE, syst, buttontask;
struct Window *window;
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
TEXT fullPath[SIZE_FULLPATH], ecpPath[SIZE_FULLPATH], cdfPath[SIZE_FULLPATH];
TEXT reportStr[256];
int errors;

	rvrec->miscfunc = Report;
	rvrec->miscfunc2 = IsTheButtonHit;
	rvrec->returnCode = TRUE;
	errors=0;
	if ( session_rec->skip_system_files )
		syst = SYSTEMFILES_NO;
	else
		syst = SYSTEMFILES_YES;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA2_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA2_GR);
	UA_DisableButton(window, &RA2_GR[5], mypattern1);	// Abort
	reportWdw = window;

	/**** start button task ****/

	buttontask = StartButtonTask();

	/**** follow list of script and destinations ****/

	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		Report(NULL);	// Clear status bar

		/********************************/
		/**** START - PROCESS SCRIPT ****/

		UA_MakeFullPath(sln_work_node->scriptPath, sln_work_node->scriptName, fullPath);

		Report("------------------------------------------------------------------------------------------------------");
		sprintf(reportStr,msgs[Msg_RA_PrepScript-1],sln_work_node->scriptName);
		Report(reportStr);

		logfile = (FILE *)fopen(LOGFILE,"a");
		retval = CreateBigFile(	fullPath, BIGFILE, TEMPSCRIPT,
														MODE_UPLOAD, "ALIAS:", syst, NULL );
		if ( logfile )
		{
			fclose( logfile );
			logfile=NULL;
		}

		if ( !retval )
		{
			errors++;

			sprintf(reportStr, msgs[Msg_RA_Missing1-1], sln_work_node->scriptName);
			Report(reportStr);

			sprintf(reportStr,msgs[Msg_RA_Missing2-1], sln_work_node->scriptName);
			Report(reportStr);
		}

		/**** END - PROCESS SCRIPT ******/
		/********************************/

		dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;
		if ( sln_work_node->destList.lh_TailPred != (struct Node *)&sln_work_node->destList )
		{
			while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
			{
				/******************************/
				/**** START - PROCESS DEST ****/

				if ( retval )
				{
					// LAUNCH ECP

					sprintf(reportStr,msgs[Msg_RA_LaunchingECP-1],dln_work_node->ecpName);
					Report(reportStr);

					UA_EnableButton(window, &RA2_GR[5]);	// Abort

					UA_MakeFullPath(dln_work_node->cdfPath, dln_work_node->cdfName, cdfPath);
					UA_MakeFullPath(dln_work_node->ecpPath, dln_work_node->ecpName, ecpPath);

					if ( !ExecuteECP(ecpPath, cdfPath, TEMPSCRIPT, BIGFILE, sln_work_node->swap) )
					{
						errors++;
						Report(msgs[Msg_RA_ECP_Error-1]);
						if ( Fault(IoErr(),msgs[Msg_RA_DOS_error-1],reportStr,255) )
							Report(reportStr);
					}

					UA_DisableButton(window, &RA2_GR[5], mypattern1);	// Abort
				}

				/**** END - PROCESS DEST ******/
				/******************************/
				dln_work_node = dln_next_node;

				if ( !session_rec->upload_multiple_scripts )
					break;	// SINGLE SCRIPT MODE --> QUIT NOW
			}
		}
		else
		{
			errors++;
			sprintf(reportStr,msgs[Msg_RA_ECP_CDF_Missing-1]);
			Report(reportStr);
		}

		CleanUpBigFile(BIGFILE);	// REMOVE ALL .TMP FILES

		sln_work_node = sln_next_node;

		if ( !session_rec->upload_multiple_scripts )
			break;	// SINGLE SCRIPT MODE --> QUIT NOW
	}

	/**** Change Abort into OK ****/

	if ( buttonTouched )
		Report( msgs[Msg_RA_Aborted-1] );

	RA2_GR[5].txt = msgs[Msg_OK-1];	
	UA_EnableButton(window, &RA2_GR[5]);

	/**** start button task ****/

	if ( buttontask )
		StopButtonTask();

	/**** Say goodbye ****/

	Report("------------------------------------------------------------------------------------------------------");
	if ( errors )
	{
		Report("····························································");
		if ( errors==1 )
			sprintf(reportStr,msgs[Msg_RA_ErrorRep1-1],errors);
		else
			sprintf(reportStr,msgs[Msg_RA_ErrorRep2-1],errors);
		Report(reportStr);
		Report("····························································");
	}

	WaitOnLastButton(window);
	UA_CloseRequesterWindow(window, STDCOLORS);
	DeleteFile(LOGFILE);

	return(retval);
}

/******** Report() ********/

void Report(STRPTR report)
{
struct Window *window;
int x,w,pct;
TEXT repstr[256];
FILE *fp;

	window = reportWdw;	// STATIC GLOBAL

	if ( !report)
	{
		SetAPen(window->RPort, AREA_PEN);
		RectFill(window->RPort, RA2_GR[6].x1+3, RA2_GR[6].y1+2, RA2_GR[6].x2-3,	RA2_GR[6].y2-2);
		return;
	}

	if( report[0] != '~' &&
			report[0] != '^' &&
			report[0] != '%' )	// ~ means don't scroll up, ^ means only scroll up, % NOT scroll up BUT DO store
		stccpy(repstr,report,254);
	else
		stccpy(repstr,&report[ 1 ],254);

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	// update the status bar or print a text

	if ( isdigit( repstr[0] ) )
	{
		SetAPen(window->RPort, BGND_PEN);
		sscanf(repstr,"%d",&pct);
		if (pct)
		{
			if (pct>100)
				pct=100;
			w = RA2_GR[6].x2 - RA2_GR[6].x1 - 6;
			x = w * pct;
			x /= 100;
			if ( ( RA2_GR[6].x1 + x ) > ( RA2_GR[6].x1 + w ) )
				x = w;
			RectFill(window->RPort, RA2_GR[6].x1+3, RA2_GR[6].y1+2, RA2_GR[6].x1+3+x,	RA2_GR[6].y2-2);
		}
	}
	else
	{
		SetAPen(window->RPort, LO_PEN);
		SetBPen(window->RPort, AREA_PEN);
		if( report[0] != '~' && report[0] != '%' )
		{
			ScrollRaster(	window->RPort, 0, window->RPort->TxHeight,
										RA2_GR[4].x1+2, RA2_GR[4].y1+2,
										RA2_GR[4].x2-2, RA2_GR[4].y2-2);
		}
		else
		{
			SetAPen(window->RPort, AREA_PEN);
			if (window->WScreen->ViewPort.Modes & LACE)
				RectFill(	window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2, RA2_GR[4].y2-5);
			else
				RectFill( window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2,	RA2_GR[4].y2-5);
			SetAPen(window->RPort, LO_PEN);
		}

		if( report[0] != '~' && report[0] != '^' )
		{
			fp = (FILE *)fopen(LOGFILE,"a");
			if (fp)
			{
				fprintf(fp,"%s\n",repstr);
				fclose(fp);
			}
		}

		if( report[0] != '^' )
		{
			UA_ShortenString(window->RPort, repstr, (RA2_GR[4].x2-RA2_GR[4].x1)-32);
			if (window->WScreen->ViewPort.Modes & LACE)
				Move(	window->RPort, RA2_GR[4].x1+4, RA2_GR[4].y2-7 );
			else
				Move(	window->RPort, RA2_GR[4].x1+4, RA2_GR[4].y2-6 );
			Text(window->RPort, repstr, strlen(repstr));
		}
	}

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);
}

/******** WaitOnLastButton() ********/

void WaitOnLastButton(struct Window *window)
{
struct ScrollRecord SR;
int numDisp=10,numLines,topEntry,size;
UBYTE **list;
TEXT str[512];
FILE *fp;
BOOL loop=TRUE;

	list = (UBYTE **)AllocMem(sizeof(UBYTE *)*1024,MEMF_CLEAR|MEMF_ANY);

	numLines=0;
	fp = (FILE *)fopen(LOGFILE,"r");
	if ( list && fp )
	{
		while( numLines<1000 && !feof(fp) )
		{
			fgets(str, 511, fp);
			if ( feof(fp) )
				break;
			if ( strlen(str) )
			{
				if ( strlen(str)<=1 )
					strcpy(str,"   ");
				list[numLines] = (UBYTE *)AllocMem(strlen(str),MEMF_CLEAR|MEMF_ANY);
				if ( list[numLines] )
				{
					stccpy(list[numLines],str,strlen(str));
					numLines++;
				}
				else
					break;
			}
		}
	}
	if ( fp )
		fclose(fp);

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	if ( numLines )
	{
		PropSlider1.LeftEdge= RA2_GR[7].x1+4;
		PropSlider1.TopEdge	= RA2_GR[7].y1+2;
		PropSlider1.Width		= RA2_GR[7].x2-RA2_GR[7].x1-7;
		PropSlider1.Height	= RA2_GR[7].y2-RA2_GR[7].y1-3;
		if ( UA_IsWindowOnLacedScreen(window) )
		{
			PropSlider1.TopEdge	+= 2;
			PropSlider1.Height	-= 4;
		}
		InitPropInfo((struct PropInfo *)PropSlider1.SpecialInfo, (struct Image *)PropSlider1.GadgetRender);
		AddGadget(window, &PropSlider1, -1L);

		SR.GR						= &RA2_GR[4];
		SR.window				= window;
		SR.list					= NULL;
		SR.sublist				= NULL;
		SR.selectionList	= NULL;
		SR.entryWidth		= -1;
		SR.numDisplay		= numDisp;
		SR.numEntries		= numLines;

		UA_PrintStandardList(NULL,-1,NULL);	// init static

		topEntry=0;
		if (numLines>numDisp)
			topEntry = numLines-numDisp;

		UA_ClearButton(window,&RA2_GR[4],AREA_PEN);

		UA_PrintStandardList(&SR,topEntry,list);
		UA_SetPropSlider(window, &PropSlider1, numLines, numDisp, topEntry);
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( numLines && (CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP ) )
			UA_ScrollStandardList(&SR,&topEntry,&PropSlider1,list,&CED);
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			if ( UA_CheckGadgetList(window, RA2_GR, &CED) == 5 )
			{
do_ok:
				UA_HiliteButton(window, &RA2_GR[5]);
				loop=FALSE;
				break;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
	}

	numLines=0;
	while( list[numLines] )
	{
		size = strlen(list[numLines])+1;
		FreeMem(list[numLines],size);
		numLines++;
	}
	if ( list )
		FreeMem(list,sizeof(UBYTE *)*1024);

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);
}

/******** ExecuteECP() ********/
/*
 * ECP and CDF are fullpaths
 *
 */

BOOL ExecuteECP(STRPTR ECP, STRPTR CDF, STRPTR TempScript, STRPTR BigFile, int swap)
{
struct FileHandle	*DOSHandle;
BOOL retVal=FALSE;
TEXT cmdline[512];
int error;

	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if (DOSHandle)
	{
		strcpy(cmdline, "Stack 40000\n ");
		strcat(cmdline, "\""); strcat(cmdline, ECP); strcat(cmdline, "\"");
		strcat(cmdline, " ");
		strcat(cmdline, "\""); strcat(cmdline, CDF); strcat(cmdline, "\"");
		strcat(cmdline, " "); strcat(cmdline, TempScript);
		strcat(cmdline, " "); strcat(cmdline, BigFile);
		if ( swap )
		{
			strcat(cmdline, " "); strcat(cmdline, "0");
		}
		else
		{
			strcat(cmdline, " "); strcat(cmdline, "1");
		}
		strcat(cmdline, " "); strcat(cmdline, "ram:log");

		error = SystemTags(cmdline, SYS_Input, NULL, SYS_Output, DOSHandle, TAG_DONE);
		if ( error == 0 )
			retVal = TRUE;			

		Close((BPTR)DOSHandle);
	}

	return(retVal);
}

/******** E O F ********/
