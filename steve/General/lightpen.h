//	File		: lightpen.h
//	Uses		:
//	Date		: 21 june 1994
//	Author	: ing. C. Lieshout
//	Desc.		: structs and protos for lightpen movement
//

#ifndef	EXEC_INTERRUPTS_H
#include "exec/interrupts.h"
#endif

struct lp
{
	long	x;
	long	y;
	struct Interrupt int_50hz;
};

long setlightpen( struct lp *lp );
void removelightpen( struct lp *lp );
