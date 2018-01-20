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

BOOL DisplayFye(char *FileName, int delay);

/**** screen ****/

ULONG fcs_cleanup (ULONG error, struct FyeScreen * fs, struct FyeBase *);
ULONG FyeCreateScreen(struct FyeScreen **fsptr,
											struct PictHeader *ph,
											PLANEPTR *bitmap,
											ULONG mode, struct FyeBase *);

/******** E O F ********/
