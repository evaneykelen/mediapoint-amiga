
	xdef	_InitStudio16UserEnv
_InitStudio16UserEnv:
	movea.l	16(a7),a6						; get 4th argument
	jmp	-564(a6)

	xdef	_ASOpen
_ASOpen:
	movea.l	12(a7),a6						; get 3rd argument
	jmp	-480(a6)

	xdef	_AllocChan
_AllocChan:
	movea.l	16(a7),a6						; get 4th argument
	jmp	-312(a6)

	xdef	_ASTrigger
_ASTrigger:
	movea.l	8(a7),a6						; get 2nd argument
	jmp	-36(a6)

	xdef	_CloseAllModules
_CloseAllModules:
	movea.l	8(a7),a6						; get 2nd argument
	jmp	-660(a6)

	xdef	_ASStop
_ASStop:
	movea.l	8(a7),a6						; get 2nd argument
	jmp	-30(a6)

	xdef	_ASClose
_ASClose:
	movea.l	8(a7),a6						; get 2nd argument
	jmp	-330(a6)

	xdef	_FreeChan
_FreeChan:
	movea.l	8(a7),a6						; get 2nd argument
	jmp	-96(a6)

	END
