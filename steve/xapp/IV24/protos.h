/**** doit.c ****/

BOOL XappDoIt(PROCESSINFO *ThisPI, struct IV24_actions *IV24);
BOOL IV_OpenPip(struct FyeBase *FyeBase, struct PipData *pip, int mode);
BOOL IV_ClosePip(struct FyeBase *FyeBase);
BOOL IV_MovePip(struct FyeBase *FyeBase, struct PipData *pip, int x, int y);
BOOL IV_FlyPipUp(struct FyeBase *FyeBase, struct PipData *pip, int x, int y, int addY);
BOOL IV_FlyPipDown(struct FyeBase *FyeBase, struct PipData *pip, int x, int y, int addY);
BOOL IV_FlyPipFromTop(struct FyeBase *FyeBase, struct PipData *pip, int x, int y,
											int addY, int height, int Mode);
BOOL IV_FlyPipFromBottom(	struct FyeBase *FyeBase, struct PipData *pip, int x, int y,
													int addY,	int height, int Mode);
BOOL IV_SetAmigaCompKeyer(struct FyeBase *FyeBase, UBYTE val);
BOOL IV_SetExtCompKeyer(struct FyeBase *FyeBase, UBYTE val);

/**** fye.c ****/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void RenderPage(struct Window *window, struct IV24_actions *IV24);
void ProcessPage1(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED);
void ProcessPage2(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED);
void ProcessPage3(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED, UWORD *mypattern1);
void ProcessPage4(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED);
void ProcessPage5(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED);
void DisplayModeInfo(struct Window *window, struct IV24_actions *IV24);
BOOL CheckBoard(struct Window *window);

/**** getargs ****/

void GetExtraData(PROCESSINFO *ThisPI, struct IV24_actions *IV24);
void PutExtraData(PROCESSINFO *ThisPI, struct IV24_actions *IV24);

/**** iff.c ****/

ULONG FyeReadIffPictHeader(ULONG file, struct PictHeader **phptr, struct Library *);
ULONG riph_cleanup (ULONG error, struct PictHeader *ph);
char UnpackRLL(BYTE **pSource,BYTE **pDest,int *srcBytes0,int dstBytes0);
char SkipRLL(BYTE **pSource,int *srcBytes0,int dstBytes0);
void FyeCleanupReadIff (struct PictHeader *ph);
ULONG FyeReadIffPictBody ( 	struct PictHeader *ph,
														PLANEPTR * bitmap,
														ULONG bitplanestart,
														ULONG nbrofbitplanes,
														ULONG mode, struct Library *);

/**** fyeview ****/

BOOL PickPicture(struct Window *window, STRPTR picpath, UWORD *mypattern1);
BOOL DisplayFye(char *FileName, int delay);

/**** screen ****/

ULONG fcs_cleanup (ULONG error, struct FyeScreen * fs, struct FyeBase *);
ULONG FyeCreateScreen(struct FyeScreen **fsptr,
											struct PictHeader *ph,
											PLANEPTR *bitmap,
											ULONG mode, struct FyeBase *);

/******** E O F ********/
