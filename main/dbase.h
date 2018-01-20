/******** DBASE.H ********/

#define REPORT_IN_ROWS	1
#define REPORT_IN_COLS	2

#define DBASE_NO_ERROR			0		// everything a-ok
#define DBASE_FILE_ERROR		1		// unable to open or read file
#define DBASE_EMPTY_ERROR		2		// no records
#define DBASE_MEMORY_ERROR	3		// no memory

struct DBaseRecord
{
	int numRecords;
	int numFields;
	int recordSize;

	UBYTE *fields;
	int fields_size;

	UBYTE *records;
	int records_size;

	BOOL *usedFields;			// if TRUE then this field is NOT used

	UBYTE *report;
	int report_size;

	int firstRecord;
	int lastRecord;
	int orientation;

	UBYTE error;
};

/******** E O F ********/
