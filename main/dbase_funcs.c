#include "nb:pre.h"
#include "dbase.h"

#define _PRINTF TRUE

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;

/******** OpenDBaseFile() ********/

BOOL OpenDBaseFile(struct DBaseRecord *dbase_rec, char *filename, BOOL readFile)
{
FILE *fp;
UBYTE buffer[300];
int offset, val, i, j;
BOOL go_on;

	dbase_rec->error = DBASE_NO_ERROR;

	// =============== init structure first ===============

	dbase_rec->records = NULL;
	if ( !readFile )	// prevent these ptrs from being cleared in second run
	{									// when readFile is TRUE
		dbase_rec->fields = NULL;
		dbase_rec->usedFields = NULL;
	}
	dbase_rec->report = NULL;

	// =============== open file ===============

	fp = (FILE *)fopen(filename,"r");
	if ( !fp )
	{
		dbase_rec->error = DBASE_FILE_ERROR;
		return(FALSE);
	}

	// =============== read header ===============

	ReadNBytesMax(fp, buffer, 32);

	// =============== get number of records in file ===============

	val = (int)buffer[5];
	val <<= 8;
	val += (int)buffer[4];
	dbase_rec->numRecords = val;

	if ( dbase_rec->numRecords==0 )
	{
		dbase_rec->error = DBASE_EMPTY_ERROR;
		fclose(fp);
		return(FALSE);
	}

	// =============== get number of fields of each record ===============

	val = (int)buffer[9];
	val <<= 8;
	val += (int)buffer[8];
	offset = val;
	dbase_rec->numFields = ( offset - 1 - 0x20 ) / 0x20;

	if ( dbase_rec->numFields==0 )
	{
		dbase_rec->error = DBASE_EMPTY_ERROR;
		fclose(fp);
		return(FALSE);
	}

	// =============== get record size ===============

	val = (int)buffer[11];
	val <<= 8;
	val += (int)buffer[10];
	dbase_rec->recordSize = val;

	// =============== Allocate array for field usage yes/no ===============

	if ( !readFile )	// prevent this ptr from being allocated in second run
	{
		dbase_rec->usedFields = (BOOL *)AllocMem(sizeof(BOOL)*dbase_rec->numFields, MEMF_ANY|MEMF_CLEAR);
		if ( !dbase_rec->usedFields )
		{
			dbase_rec->error = DBASE_MEMORY_ERROR;
			CloseDBaseFile(dbase_rec);
			fclose(fp);
			return(FALSE);
		}
	}

	// =============== =============== =============== =============== ===============
	// Allocate size for numFields records -- each field start with 'c' and a space (0..1)
	//																				(this is for MP scroll list functions)
	//                                        each field may have 11 characters (2...12)
	//																				11th byte is to store '\0'				(13)
	//																				12th byte is to store field size	(14)

	if ( !readFile )	// prevent this ptr from being allocated in second run
	{
		dbase_rec->fields_size = dbase_rec->numFields * 15;
		dbase_rec->fields = (UBYTE *)AllocMem(dbase_rec->fields_size, MEMF_ANY|MEMF_CLEAR);
		if ( !dbase_rec->fields )
		{
			dbase_rec->error = DBASE_MEMORY_ERROR;
			CloseDBaseFile(dbase_rec);
			fclose(fp);
			return(FALSE);
		}
	}

	// =============== Read all record names into fields array ===============

	for(i=0; i<dbase_rec->numFields; i++)
	{
		ReadNBytesMax(fp, buffer, 32);
		*( dbase_rec->fields + (i*15) + 0 ) = 'c';						// fill byte 0
		*( dbase_rec->fields + (i*15) + 1 ) = 0x21;						// fill byte 1
		CopyMem(buffer, dbase_rec->fields + (i*15) + 2, 11);	// fill byte 2...12
		*( dbase_rec->fields + (i*15) + 14 ) = buffer[0x10];	// fill byte 14
	}

	// =============== stop parsing when only the header is needed ===============

	if ( !readFile )
	{
		fclose(fp);
		return(TRUE);
	}

	// =============== Allocate size for numRecords records ===============

	dbase_rec->records_size = dbase_rec->numRecords * (dbase_rec->recordSize+dbase_rec->numFields*10);
	dbase_rec->records = (UBYTE *)AllocMem(dbase_rec->records_size, MEMF_ANY|MEMF_CLEAR);
	if ( !dbase_rec->records )
	{
		dbase_rec->error = DBASE_MEMORY_ERROR;
		CloseDBaseFile(dbase_rec);
		fclose(fp);
		return(FALSE);
	}

	// =============== Read all records into record array ===============

	ReadNBytesMax(fp, buffer, 2);

	// There's a difference between dBASE III and dBASE III Plus files.
	// In the former there's an ASCII NUL character between the $0D
	// end of header indicator and the start of the data. We support both.

	if ( buffer[1] == 0x00 ) 
		ReadNBytesMax(fp, buffer, 1);	// read an extra bye

	for(i=0; i<dbase_rec->numRecords; i++)
	{
		offset = 0;
		for(j=0; j<dbase_rec->numFields; j++)
		{
			val = (int)*( dbase_rec->fields+j*15 + 14 );	// field size
			ReadNBytesMax(fp, buffer, val);
			CopyMem(buffer, dbase_rec->records+
											i*(dbase_rec->recordSize+dbase_rec->numFields)+offset, val);
			offset += (val+1);
		}
		ReadNBytesMax(fp, buffer, 1);
	}

	// =============== allocate memory for report ===============

	if ( dbase_rec->firstRecord==-1 && dbase_rec->lastRecord==-1 )
		dbase_rec->report_size = dbase_rec->records_size;
	else
	{
		dbase_rec->report_size = ( (dbase_rec->lastRecord-dbase_rec->firstRecord) + 1 ) *
															(dbase_rec->recordSize+dbase_rec->numFields*10);
	}

	dbase_rec->report = (UBYTE *)AllocMem(dbase_rec->report_size, MEMF_ANY|MEMF_CLEAR);
	if ( !dbase_rec->report )
	{
		dbase_rec->error = DBASE_MEMORY_ERROR;
		CloseDBaseFile(dbase_rec);
		fclose(fp);
		return(FALSE);
	}

	// =============== generate report ===============

	for(i=0; i<dbase_rec->numRecords; i++)
	{
		if ( dbase_rec->firstRecord==-1 && dbase_rec->lastRecord==-1 )
			go_on=TRUE;
		else
		{
			if ( i>=dbase_rec->firstRecord && i<=dbase_rec->lastRecord )
				go_on=TRUE;
			else
				go_on=FALSE;
		}

		if ( go_on )
		{
			offset = 0;
			for(j=0; j<dbase_rec->numFields; j++)
			{
				val = (int)*( dbase_rec->fields+j*15 + 14 );	// field size
				if ( !dbase_rec->usedFields[j] )	// report this field
				{
					KillWhiteSpaces(dbase_rec->records+i*(dbase_rec->recordSize+dbase_rec->numFields)+offset,
													buffer, val);
					strcat(	dbase_rec->report, buffer );
					if ( dbase_rec->orientation == REPORT_IN_ROWS )
						strcat(dbase_rec->report,"\n");
					else
						strcat(dbase_rec->report,"  -  ");
				}
				offset += (val+1);
			}
			if ( dbase_rec->orientation == REPORT_IN_COLS )
				strcat(dbase_rec->report,"\n");
		}
	}

	// =============== Close file ===============

	fclose(fp);

	return(TRUE);
}

/******** CloseDBaseFile() ********/

void CloseDBaseFile(struct DBaseRecord *dbase_rec)
{
	if ( dbase_rec->fields )
		FreeMem(dbase_rec->fields, dbase_rec->fields_size);

	if ( dbase_rec->records )
		FreeMem(dbase_rec->records, dbase_rec->records_size);

	if ( dbase_rec->usedFields )
		FreeMem(dbase_rec->usedFields, sizeof(BOOL)*dbase_rec->numFields);

	if ( dbase_rec->report )
		FreeMem(dbase_rec->report, dbase_rec->report_size);
}

/******** ReadNBytesMax() ********/

void ReadNBytesMax(FILE *fp, char *buffer, int num)
{
int i;
char c;

	i=0;
	while( i<num && (c = getc(fp)) != EOF )
		buffer[i++] = c;
}

/******** KillWhiteSpaces() ********/

void KillWhiteSpaces(UBYTE *srcPtr, UBYTE *dstPtr, int len)
{
int i;
BOOL copy=FALSE;

	for(i=(len-1); i>=0; i--)
	{
		if ( !copy && *(srcPtr+i) != ' ' )
		{
			copy = TRUE;
			*(dstPtr+i+1) = '\0';
		}
		if ( copy )
			*(dstPtr+i) = *(srcPtr+i);
	}
	if ( !copy )	// source has nothing but spaces, those we'd like to have 
		strcpy(dstPtr,srcPtr);
}

/******** E O F ********/
