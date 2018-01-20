	incdir wp:inclibs/
	include exec_lib.i
	incdir include:
	include	dos/dos.i

start:
	move.l	#SIGBREAKF_CTRL_F,d0
	move.l	$4.w,a6
	jsr	_LVOWait(a6)
	moveq	#0,d0
	rts
