_LVOIEEEDPFix	=	-30
_LVOIEEEDPFlt	=	-36
_LVOIEEEDPCmp	=	-42
_LVOIEEEDPTst	=	-48
_LVOIEEEDPAbs	=	-54
_LVOIEEEDPNeg	=	-60
_LVOIEEEDPAdd	=	-66
_LVOIEEEDPSub	=	-72
_LVOIEEEDPMul	=	-78
_LVOIEEEDPDiv	=	-84
_LVOIEEEDPFloor	=	-90
_LVOIEEEDPCeil	=	-96
CALLIEEEDOUB	MACRO
	MOVE.L	_MathIeeeDoubBasBase,A6
	JSR	_LVO\1(A6)
	ENDM
IEEEDOUBNAME	MACRO
	DC.B	'mathieeedoubbas.library',0
	ENDM
