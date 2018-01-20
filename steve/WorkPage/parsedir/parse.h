// File		: parse.h
// Uses		:
//	Date		: 17 February 1993
// Autor		: C. Lieshout
// Desc.		: Structs and protos for the document parser
//

typedef struct parse_info
{
	int type;

	int parameters[14];

	char *data;
	int datasize;

	struct parse_info *next;

}P_INFO;
