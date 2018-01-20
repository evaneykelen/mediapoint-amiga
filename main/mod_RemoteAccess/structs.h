#define LOGFILE "ram:logfile"
#define RA_CONFIG "RemoteAccess.config"
#define MAXSCRIPTS 64

struct SessionRecord
{
	struct List scriptList;
	TEXT sessionName[SIZE_FULLPATH];
	TEXT sessionPath[SIZE_FULLPATH];
	int upload_all_files;
	int delayed_upload;
	int skip_system_files;
	int upload_multiple_scripts;
};

struct ScriptListNode
{
	struct Node node;	// embedded node
	struct List destList;
	TEXT scriptPath[SIZE_FULLPATH];
	TEXT scriptName[SIZE_FULLPATH];
	int swap;
};

struct DestListNode
{
	struct Node node;	// embedded node
	TEXT ecpPath[SIZE_FULLPATH];
	TEXT ecpName[SIZE_FULLPATH];
	TEXT cdfPath[SIZE_FULLPATH];
	TEXT cdfName[SIZE_FULLPATH];
};

/******** E O F ********/
