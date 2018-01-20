/**** MPEG record ****/

struct MPEG_record
{
	struct IOMPEGReq *IOReq;
	struct IOMPEGReq *IOReq2;
	struct MsgPort  *IOPort;
	struct Device *MPEG_Device;
};

/******** E O F ********/
