	IFND	STACKVARS_I
STACKVARS_I	SET	1
**
**	$VER: stackvars.i 0.040 (29.01.94)
**
**	Data storage macros.
**
**	(C) Copyright 1993 Bits On My Byte.
**	    All Rights Reserved
**

**
** Stack Structure Building Macros
**

STACKSTRUCT MACRO		; structure name, initial offset
            IFC     '\1',''
SOFFSET	    SET     0
            ENDC
            IFNC    '\1',''
SOFFSET     SET     8+(\2*4)	; skip rts & link addresses
            ENDC
	    ENDM

_FPTR	    MACRO		; function pointer (32 bits - all bits valid)
SOFFSET     SET     SOFFSET-4
\1	    EQU     SOFFSET
	    ENDM

_BOOL	    MACRO		; boolean (16 bits)
SOFFSET     SET     SOFFSET-2
\1	    EQU     SOFFSET
	    ENDM

_BYTE	    MACRO		; byte (8 bits)
SOFFSET     SET     SOFFSET-1
\1	    EQU     SOFFSET
	    ENDM

_UBYTE	    MACRO		; unsigned byte (8 bits)
SOFFSET     SET     SOFFSET-1
\1	    EQU     SOFFSET
	    ENDM

_WORD	    MACRO		; word (16 bits)
SOFFSET     SET     SOFFSET-2
\1	    EQU     SOFFSET
	    ENDM

_UWORD	    MACRO		; unsigned word (16 bits)
SOFFSET     SET     SOFFSET-2
\1	    EQU     SOFFSET
	    ENDM

_LONG	    MACRO		; long (32 bits)
SOFFSET     SET     SOFFSET-4
\1	    EQU     SOFFSET
	    ENDM

_ULONG	    MACRO		; unsigned long (32 bits)
SOFFSET     SET     SOFFSET-4
\1	    EQU     SOFFSET
	    ENDM

_FLOAT	    MACRO		; C float (32 bits)
SOFFSET     SET     SOFFSET-4
\1	    EQU     SOFFSET
	    ENDM

_DOUBLE	    MACRO		; C double (64 bits)
SOFFSET	    SET	    SOFFSET-8
\1	    EQU	    SOFFSET
	    ENDM

_APTR	    MACRO		; untyped pointer (32 bits - all bits valid)
SOFFSET     SET     SOFFSET-4
\1	    EQU     SOFFSET
	    ENDM

_RPTR	    MACRO		; unsigned relative pointer (16 bits)
SOFFSET     SET     SOFFSET-2
\1	    EQU     SOFFSET
	    ENDM

_LABEL	    MACRO		; Define a label without bumping the offset
\1	    EQU     SOFFSET
	    ENDM

_STRUCT	    MACRO		; Define a sub-structure
SOFFSET     SET     SOFFSET-\2
\1	    EQU     SOFFSET
	    ENDM

* _ALIGNWORD  MACRO		; Align structure offset to nearest word
* SOFFSET     SET     (SOFFSET+1)&$fffffffe
* 	    ENDM
*
* _ALIGNLONG  MACRO		; Align structure offset to nearest longword
* SOFFSET     SET     (SOFFSET+3)&$fffffffc
* 	    ENDM

	ENDC	; STACKVARS_I
