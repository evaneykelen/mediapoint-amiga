#include "nb:pre.h"
#include "protos.h"
#include "setup.h"
#include "structs.h"

#define VERSI0N "\0$VER: 1.0"          
static UBYTE *vers = VERSI0N;

/**** globals ****/

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase	= NULL;
struct Library *medialinkLibBase = NULL;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
static struct TextAttr textAttr = { "topaz.font", 8, NULL, NULL };
struct MsgPort *capsPort = NULL;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** main() ********/

int main(int argc, char **argv)
{
struct Window *window;
struct TextFont *tf;
TEXT fontPath[SIZE_FULLPATH];
struct EditRecord edit_rec;

	/**** init record ****/

	edit_rec.baudRate	= 9600;
	strcpy(edit_rec.device,"serial.device");
	edit_rec.unit = 0;
	edit_rec.sevenWire = FALSE;
	edit_rec.bufferSize = 10000;
	edit_rec.priority = 127;
	edit_rec.conClass = 0;
	strcpy(edit_rec.defPath,"MP_RA:");
	strcpy(edit_rec.superPassword,"super***");

	/**** open standard libraries ****/

	if ( !MPOpenLibs() )
		return(0);

	/**** create input ports ****/

	if ( !OpenInput() )
	{
		MPCloseLibs();
		return(0);
	}

	/**** open window ****/

	Forbid();
	window = IntuitionBase->ActiveWindow;
	Permit();

	if ( window && window->WScreen->ViewPort.Modes & LACE )
		UA_DoubleGadgetDimensions(Edit_GR);

	window = OpenMPWindow(Edit_GR);
	if ( !window )
	{
		CloseInput();
		MPCloseLibs();
		return(0);
	}
	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, AREA_PEN);
	RectFill(window->RPort, window->BorderLeft, window->BorderTop, window->Width  - window->BorderRight-1, window->Height - window->BorderBottom-1 );

	/**** open font ****/

	sprintf(fontPath, "fonts:%s", APPFONT);

	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		textAttr.ta_Name = (UBYTE *)fontPath;
		textAttr.ta_YSize = 20;
		textAttr.ta_Style = FS_NORMAL;
		textAttr.ta_Flags = FPF_DESIGNED;
	}
	else
	{
		textAttr.ta_Name = (UBYTE *)fontPath;
		textAttr.ta_YSize = 10;
		textAttr.ta_Style = FS_NORMAL;
		textAttr.ta_Flags = FPF_DESIGNED;
	}

	tf = OpenDiskFont(&textAttr);
	if ( !tf )
	{
		textAttr.ta_Name = (UBYTE *)"topaz.font";
		textAttr.ta_YSize = 8;
		textAttr.ta_Flags = NULL;
	
		tf = OpenDiskFont(&textAttr);
		if ( !tf )
		{
			CloseInput();
			MPCloseLibs();
			CloseMPWindow(window);
			return(0);
		}
	}

	SetFont(window->RPort, tf);

	/**** draw gadgets ****/

	UA_DrawGadgetList(window, Edit_GR);

	/**** get current settings ****/

	if ( !GetSerialPrefs("Serial/prefs.serial", &edit_rec) )
	{
		warningES.es_TextFormat = (UBYTE *)"The 'Serial/prefs.serial' file can't be opened.";
		EasyRequest(NULL,&warningES,NULL);
	}

	/**** set gadgets ****/

	UA_SetStringGadgetToVal(window,			&Edit_GR[ 7], edit_rec.baudRate);
	UA_SetStringGadgetToString(window, 	&Edit_GR[ 8], edit_rec.device);
	UA_SetCycleGadgetToVal(window, 			&Edit_GR[ 9], edit_rec.unit);
	UA_SetCycleGadgetToVal(window, 			&Edit_GR[10], edit_rec.sevenWire);
	UA_SetStringGadgetToVal(window, 		&Edit_GR[11], edit_rec.bufferSize);
	UA_SetStringGadgetToVal(window, 		&Edit_GR[12], edit_rec.priority);
	UA_SetCycleGadgetToVal(window, 			&Edit_GR[13], edit_rec.conClass);
	UA_SetStringGadgetToString(window, 	&Edit_GR[14], edit_rec.defPath);
	UA_SetStringGadgetToString(window, 	&Edit_GR[15], edit_rec.superPassword);

	/**** monitor events ****/

	MonitorUser(window,&edit_rec);

	/**** close the window ****/

	CloseMPWindow(window);

	/**** close font ****/

	CloseFont(tf);

	/**** close all the libraries ****/

	MPCloseLibs();

	/**** close input ports ****/

	CloseInput();

	return(0);
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, struct EditRecord *edit_rec)
{
BOOL loop=TRUE, retval=FALSE;
struct EventData CED;
int ID,val;
TEXT password1[16], password2[16];

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, Edit_GR, &CED);
			switch( ID )
			{
				case 4:		// OK
do_ok:
					UA_HiliteButton(window, &Edit_GR[4]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 5:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Edit_GR[5]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 7:		// Baudrate
				case 8:		// Device
				case 11:	// Buffer size
				case 12:	// Priority
				case 14:	// Default path
				case 15:	// Super password
					UA_ProcessStringGadget(window, Edit_GR, &Edit_GR[ID], &CED);
					if ( ID==12 )
					{
						UA_SetValToStringGadgetVal(&Edit_GR[ID],&val);
						if ( val < -128 )
							val=-128;
						if ( val > 127 )
							val=127;
						UA_SetStringGadgetToVal(window, &Edit_GR[ID], val);
					}
					if ( ID==15 )
					{
						UA_SetStringToGadgetString(&Edit_GR[15], password1);
						if ( strlen(password1) < 8 )
						{
							strcpy(password2,"********");
							strncpy(password2,password1,strlen(password1));
							UA_SetStringGadgetToString(window, &Edit_GR[15], password2);
						}
						else if ( strlen(password1) > 8 )
						{
							password1[8] = '\0';
							UA_SetStringGadgetToString(window, &Edit_GR[15], password1);
						}
					}
					break;

				case 9:		// Unit
				case 10:	// Seven wire
				case 13:	// Connection class
					UA_ProcessCycleGadget(window, &Edit_GR[ID], &CED);
					break;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY )
		{
			if ( CED.Code==RAW_ESCAPE )
				goto do_cancel;
			else if ( CED.Code==RAW_RETURN )
				goto do_ok;
		}
	}

	if ( retval )
	{
		UA_SetValToStringGadgetVal(&Edit_GR[ 7],&edit_rec->baudRate);
		UA_SetStringToGadgetString(&Edit_GR[ 8],edit_rec->device);
		UA_SetValToCycleGadgetVal(&Edit_GR[ 9],&edit_rec->unit);
		UA_SetValToCycleGadgetVal(&Edit_GR[10],&edit_rec->sevenWire);
		UA_SetValToStringGadgetVal(&Edit_GR[11],&edit_rec->bufferSize);
		UA_SetValToStringGadgetVal(&Edit_GR[12],&edit_rec->priority);
		UA_SetValToCycleGadgetVal(&Edit_GR[13],&edit_rec->conClass);
		UA_SetStringToGadgetString(&Edit_GR[14],edit_rec->defPath);
		UA_SetStringToGadgetString(&Edit_GR[15],edit_rec->superPassword);

		retval = SetSerialPrefs("Serial/prefs.serial", edit_rec);
	}

	return(retval);
}

/******** GetSerialPrefs() ********/

BOOL GetSerialPrefs(STRPTR path, struct EditRecord *edit_rec)
{
FILE *fp;
TEXT str[256], sub[256];
int id;

	fp = fopen(path,"r");
	if ( fp )
	{
		while( !feof(fp) )
		{
			fgets(str, 255, fp);
			id=0;
			sscanf(str,"%s",sub);
			while( prefsCommands[id] && strcmpi(prefsCommands[id],sub) )
				id++;
			switch( id )
			{
				case PREFS_BAUDRATE:
					sscanf(str, "%s %d", sub, &edit_rec->baudRate);
					break;
				case PREFS_DEVICE:
					sscanf(str, "%s %s", sub, edit_rec->device);
					RemoveQuotes(edit_rec->device);
					break;
				case PREFS_UNIT:
					sscanf(str, "%s %d", sub, &edit_rec->unit);
					break;
				case PREFS_SEVENWIRE:
					edit_rec->sevenWire = TRUE;
					break;
				case PREFS_BUFFER_SIZE:
					sscanf(str, "%s %d", sub, &edit_rec->bufferSize);
					break;
				case PREFS_PRIORITY:
					sscanf(str, "%s %d", sub, &edit_rec->priority);
					break;
				case PREFS_CONNECTIONCLASS:
					sscanf(str, "%s %d", sub, &edit_rec->conClass);
					edit_rec->conClass--;
					break;
				case PREFS_DEFAULT_PATH:
					sscanf(str, "%s %s", sub, edit_rec->defPath);
					RemoveQuotes(edit_rec->defPath);
					break;
				case PREFS_SUPERNAME:
					sscanf(str, "%s %s", sub, edit_rec->superPassword);
					RemoveQuotes(edit_rec->superPassword);
					break;
			}
		}

		fclose(fp);

		return(TRUE);
	}
	
	return(FALSE);
}

/******** SetSerialPrefs() ********/

BOOL SetSerialPrefs(STRPTR path, struct EditRecord *edit_rec)
{
FILE *fp;

	fp = fopen(path,"w");
	if ( fp )
	{
		fprintf(fp, "BAUDRATE %d\n", edit_rec->baudRate);
		fprintf(fp, "DEVICE \"%s\"\n", edit_rec->device);
		fprintf(fp, "UNIT %d\n", edit_rec->unit);
		if ( edit_rec->sevenWire )
			fprintf(fp, "SEVENWIRE\n");
		fprintf(fp, "BUFFER_SIZE %d\n", edit_rec->bufferSize);
		fprintf(fp, "PRIORITY %d\n", edit_rec->priority);
		fprintf(fp, "CONNECTIONCLASS %d\n", edit_rec->conClass+1);
		fprintf(fp, "DEFAULT_PATH \"%s\"\n", edit_rec->defPath);
		fprintf(fp, "SUPERNAME \"%s\"\n", edit_rec->superPassword);

		fclose(fp);

		return(TRUE);
	}	
	else
	{
		warningES.es_TextFormat = (UBYTE *)"The 'Serial/prefs.serial' file can't be saved.";
		EasyRequest(NULL,&warningES,NULL);
	}

	return(FALSE);
}

/******** RemoveQuotes() ********/
/* remove trailing and ending ' or "
 */

void RemoveQuotes(STRPTR str)
{
int i,len;

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

/******** E O F ********/
