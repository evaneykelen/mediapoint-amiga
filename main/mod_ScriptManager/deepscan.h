//	File		:	readdir.h
//	Uses		:
//	Date		:	26-04-93
//	Autor		:	ing C. Lieshout
//	Desc.		:	defines, structs and defines for readdir.c
//

#define TYPE_DIR 1
#define TYPE_FILE 2
#define POOL_SIZE 500000

typedef struct
{
	SHORT		type;				// TYPE_DIR or TYPE_FILE
	long		next;
	long		sub;
	char		name[0];		// space for filename now empty
} directory;

typedef struct
{
	SHORT		type;				// TYPE_DIR or TYPE_FILE
	long		next;
	char		name[0];		// space for filename now empty
} files;

typedef struct
{
	char 		*pool;			// pointer to data
	long 		size;				// size of the mempool
	long		pointer;		// pointer to free area ( relative )
} mempool;

typedef struct disk
{
	char name[40];
	long pointer;
	struct disk *next;
}DISK_LIST;

/******** E O F ********/
