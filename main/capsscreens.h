/******** CAPSSCREENS.H ********/

struct NewScreen NewScreenStructure = {
	0,0,
	320,200,
	2,
	0,1,
	NULL,	/* viewmodes set by caps prefs */
	CUSTOMSCREEN | SCREENBEHIND,
	NULL,	/* font */
	(UBYTE *)APPNAME,
	NULL,
	NULL
};

struct NewWindow NewWindowStructure = {
	1,1,
	1,1,
	0,1,
	NULL,
	WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP | WFLG_NOCAREREFRESH,
	NULL,
	NULL,
	NULL, /* Page Window */
	NULL,
	NULL,
	5,5,
	640,200,
	CUSTOMSCREEN
};

/******** E O F ********/
