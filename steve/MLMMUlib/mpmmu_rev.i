VERSION		EQU 1
REVISION	EQU 5
DATE		MACRO
				dc.b 'Thursday 31-Mar-94'
			ENDM
VERS		MACRO
				dc.b 'MPMMU library 1.5'
			ENDM
VSTRING		MACRO
				dc.b 'MPMMU library 1.5 (Thursday 31-Mar-94)\n\r',13,10,0
			ENDM
