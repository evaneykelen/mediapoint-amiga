struct EditRecord
{
	int baudRate;
	TEXT device[40];
	int unit;
	int sevenWire;
	int bufferSize;
	int priority;
	int conClass;
	TEXT defPath[70];
	TEXT superPassword[20];
} edit_rec;

enum
{
	PREFS_BAUDRATE, PREFS_DEVICE, PREFS_UNIT, PREFS_SEVENWIRE, PREFS_BUFFER_SIZE,
	PREFS_PRIORITY, PREFS_CONNECTIONCLASS, PREFS_DEFAULT_PATH, PREFS_SUPERNAME,
};

UBYTE *prefsCommands[] =
{
	"BAUDRATE", "DEVICE", "UNIT", "SEVENWIRE", "BUFFER_SIZE",
	"PRIORITY", "CONNECTIONCLASS", "DEFAULT_PATH", "SUPERNAME",
	NULL
};

/******** E O F ********/
