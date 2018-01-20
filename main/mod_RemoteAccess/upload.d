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

/**** (static) globals ****/

STATIC struct Window *reportWdw;
FILE *logfile;

/**** functions ****/

/******** MonitorUpload() ********/

BOOL MonitorUpload(struct Window *onWindow, struct SessionRecord *session_rec)
{
BOOL retval = FALSE;
struct Window *window;
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
TEXT fullPath[SIZE_FULLPATH];
TEXT reportStr[256];
int errors;

	rvrec->miscfunc = Report;
	rvrec->returnCode = TRUE;
	errors=0;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA2_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA2_GR);
	UA_DisableButton(window, &RA2_GR[5], mypattern1);	// Abort
	reportWdw = window;

	/**** follow list of script and destinations ****/

	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		Report(NULL);	// Clear status bar

		/********************************/
		/**** START - PROCESS SCRIPT ****/

		UA_MakeFullPath(sln_work_node->scriptPath, sln_work_node->scriptName, fullPath);

		Report("------------------------------------------------------------");
		sprintf(reportStr,"Preparing script '%s'.",sln_work_node->scriptName);
		Report(reportStr);

		logfile = (FILE *)fopen(LOGFILE,"a");
		retval = CreateBigFile(	fullPath, BIGFILE, TEMPSCRIPT,
														MODE_UPLOAD, "ALIAS:", SYSTEMFILES_YES, NULL );
		if ( !retval )
		{
			errors++;
			// Clarify the logfile
			if ( logfile )
				fprintf(logfile,"The files listed above are missing in '%s'.\n", sln_work_node->scriptName);
		}
		if ( logfile )
		{
			fclose( logfile );
			logfile=NULL;
		}

		if ( !retval )
		{
			sprintf(reportStr,"ERROR: files are missing in '%s'.",sln_work_node->scriptName);
			Report(reportStr);
		}

		/**** END - PROCESS SCRIPT ******/
		/********************************/

		dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;
		while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
		{
			/******************************/
			/**** START - PROCESS DEST ****/

			if ( retval )
			{
				// LAUNCH ECP

				UA_MakeFullPath(rvrec->aPtrThree, "ECP_Drawer", fullPath);

				sprintf(reportStr,"Launching ECP '%s'.",dln_work_node->ecpName);
				Report(reportStr);

				UA_EnableButton(window, &RA2_GR[5]);	// Abort

				if ( !ExecuteECP(	fullPath, dln_work_node->cdfName, dln_work_node->ecpName,
													TEMPSCRIPT, BIGFILE) )
				{
					errors++;
					Report("Unable to launch ECP.");
					if ( Fault(IoErr(),"DOS error",reportStr,255) )
						Report(reportStr);
				}

				UA_DisableButton(window, &RA2_GR[5], mypattern1);	// Abort
			}

			/**** END - PROCESS DEST ******/
			/******************************/
			dln_work_node = dln_next_node;
		}

		sprintf(reportStr,"Finishing upload.");
		Report(reportStr);

		CleanUpBigFile(BIGFILE);	// REMOVE ALL .TMP FILES

		sln_work_node = sln_next_node;

		if ( !session_rec->upload_multiple_scripts )
			break;	// SINGLE SCRIPT MODE --> QUIT NOW
	}

	/**** Say goodbye ****/

	Report("------------------------------------------------------------");
	if ( errors )
	{
		Report("····························································");
		sprintf(reportStr,"WARNING: There were %d errors reported!",errors);
		Report(reportStr);
		Report("····························································");
	}

	for(errors=0; errors<1000; errors++)
	{
		sprintf(reportStr,"~`'[_^| test test %d",errors);
		Report(reportStr);
	}

	WaitOnLastButton(window);
	UA_CloseRequesterWindow(window, STDCOLORS);

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

	if( report[0] != '~' )
		strcpy(repstr,report);
	else
		strcpy(repstr,&report[ 1 ] );

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
		if( report[0] != '~' )
		{
			//ScrollRaster(	window->RPort, 0, window->RPort->TxBaseline+5,
			ScrollRaster(	window->RPort, 0, window->RPort->TxHeight,
										RA2_GR[4].x1+2, RA2_GR[4].y1+2,
										RA2_GR[4].x2-2, RA2_GR[4].y2-2);
		}
		else
		{
			SetAPen(window->RPort, AREA_PEN);
SetAPen(window->RPort, 5);
			if (window->WScreen->ViewPort.Modes & LACE)
				RectFill(	window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2, RA2_GR[4].y2-5);
			else
				RectFill( window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2,	RA2_GR[4].y2-5);
#if 0
			if (window->WScreen->ViewPort.Modes & LACE)
				RectFill(window->RPort, RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxBaseline-14, RA2_GR[4].x2-2, RA2_GR[4].y2-2);
			else
				RectFill(window->RPort, RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxBaseline-7, RA2_GR[4].x2-2,	RA2_GR[4].y2-2);
#endif
			SetAPen(window->RPort, LO_PEN);
		}

//#if 0
			SetAPen(window->RPort, HI_PEN);
			if (window->WScreen->ViewPort.Modes & LACE)
				RectFill(	window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2, RA2_GR[4].y2-5 );
			else
				RectFill(	window->RPort,
									RA2_GR[4].x1+2, RA2_GR[4].y2-window->RPort->TxHeight-4,
									RA2_GR[4].x2-2,	RA2_GR[4].y2-5 );
			SetAPen(window->RPort, LO_PEN);
//#endif

		if( report[0] != '~' )
		{
			fp = (FILE *)fopen(LOGFILE,"a");
			if (fp)
			{
				fprintf(fp,"%s\n",repstr);
				fclose(fp);
			}
		}

		UA_ShortenString(window->RPort, repstr, (RA2_GR[4].x2-RA2_GR[4].x1)-32);
		if (window->WScreen->ViewPort.Modes & LACE)
			Move(	window->RPort, RA2_GR[4].x1+4, RA2_GR[4].y2-7 );
		else
			Move(	window->RPort, RA2_GR[4].x1+4, RA2_GR[4].y2-6 );
		Text(window->RPort, repstr, strlen(repstr));
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
int numDisp=10,numLines,topEntry;
UBYTE **list;
TEXT str[512];
FILE *fp;

	RA2_GR[5].txt = msgs[Msg_OK-1];	// Change Abort into OK
	UA_EnableButton(window, &RA2_GR[5]);

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
				stccpy(list[numLines],str,strlen(str));
KPrintF("[%s]\n",list[numLines]);
				numLines++;
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

	while(1)
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
				return;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
	}

	if ( list )
		FreeMem(list,sizeof(UBYTE *)*1024);

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);
}

/******** ExecuteECP() ********/

BOOL ExecuteECP(STRPTR sysPath, STRPTR CDF, STRPTR ECP, STRPTR TempScript, STRPTR BigFile)
{
struct FileHandle	*DOSHandle;
BOOL retVal=FALSE;
TEXT cmdline[512];
TEXT path[SIZE_FULLPATH];
int error;

	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if (DOSHandle)
	{
		strcpy(cmdline, "Stack 40000\n ");
		UA_MakeFullPath(sysPath, ECP, path);
		strcat(cmdline, path);
		strcat(cmdline, " "); strcat(cmdline, CDF);
		strcat(cmdline, " "); strcat(cmdline, TempScript);
		strcat(cmdline, " "); strcat(cmdline, BigFile);
		strcat(cmdline, " "); strcat(cmdline, "1");
		strcat(cmdline, " "); strcat(cmdline, "ram:log");

		error = SystemTags(cmdline, SYS_Input, NULL, SYS_Output, DOSHandle, TAG_DONE);
		if ( error == 0 )
			retVal = TRUE;			

		Close((BPTR)DOSHandle);
	}

	return(retVal);
}

/******** E O F ********/
