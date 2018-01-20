	IFND	MEDIAPOINT_PREFS_I
MEDIAPOINT_PREFE_I	SET	1
**
**	$VER: mediapoint/pascal/include/prefs.i 01.001 (02.17.94)
**
**	Contains partial structure
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

  STRUCTURE CapsPrefs,0
	ULONG	cp_PageScreenWidth
	ULONG	cp_PageScreenHeight
	ULONG	cp_PageScreenDepth
	ULONG	cp_PageScreenModes
	ULONG	cp_extraPageScreenModes
	UBYTE cp_PagePalNtsc
	UBYTE cp_padXX
	APTR	cp_PageCM

	ENDC	; MEDIAPOINT_PREFS_I
