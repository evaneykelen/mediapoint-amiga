//	File		: waitsec.h
//	Uses		:
//	Date		: 18 july 1993
//	Author	: ing. C. Lieshout
//	Desc.		: structs and protos for a 50hz wait interrupt
//

#ifndef	EXEC_INTERRUPTS_H
#include "exec/interrupts.h"
#endif

struct wjif
{
	long	on;
	long	signal;
	long	signum;
	long	task;
	long	val;
	long	endval;
	struct Interrupt int_50hz;
};

long install50hz( struct wjif *wj, long value );
void set50hz( struct wjif *wj, long value );
void remove50hz( struct wjif *wj );
