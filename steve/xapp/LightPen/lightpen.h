//	File		: lightpen.h
//	Uses		:
//	Date		: 17 july 1994
//	Author	: ing. C. Lieshout
//	Desc.		: structs and protos for the lightpen interrupts
//

#ifndef	EXEC_INTERRUPTS_H
#include "exec/interrupts.h"
#endif

struct lp
{
	long	x;
	long	y;
	long	oldx;
	long 	oldy;
	WORD	offset_x;
	WORD	offset_y;

	long	signal;
	long	signum;
	long	task;

	struct Interrupt int_50hz;
	struct Interrupt int_TimA;

	int dev;
	struct IOStdReq	*InputIO;
	struct MsgPort		*InputMP;
	struct InputEvent	*FakeEvent;
};

long setlightpen( struct lp *lp );
void removelightpen( struct lp *lp );

// protos for set_mouse.c

int init_mouse( struct lp *lp );
void set_mouse( struct lp *lp);
void free_mouse( struct lp *lp );

