//	File		: report.h
//	Uses		:
//	Date		: 27 august 1994
//	Author	: ing. C. Lieshout
//	Desc.		: message struct for protocol report
//
struct PPMessage
{
	struct Message	pp_Msg;
	long				pp_count;
	long				pp_max;
	char 				*pp_mes;
	char				*pp_protocol;
};

