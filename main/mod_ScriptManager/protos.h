/**** copyfiles ****/

BOOL CopyFilesInBigFile(struct ParseRecord *PR, BOOL systemFiles);
LONG GetFreeSpace(STRPTR dest);
BOOL ExecuteCommand(STRPTR cmd);
void CreateLongDir(STRPTR newPath);
LONG GetVolumeSize(STRPTR path);
BOOL DiffTime(STRPTR file1, STRPTR file2);
BOOL CheckSourceAndDest(STRPTR source, STRPTR dest);

/**** deepscan.c ****/

void OpenDiskList(void);
void CloseDiskList(void);
BOOL OpenScanMem(void);
void CloseScanMem(void);
void PlaceInList(STRPTR volume);
BOOL FindNameInList(STRPTR oriPath, STRPTR name, int type);

/**** parse1.c ****/

BOOL CreateBigFile(	STRPTR scriptPath, STRPTR bigFile, STRPTR tempScript,
										BYTE mode, STRPTR destPath, BOOL systemFiles, STRPTR scriptName );
BOOL SMPerfFunc(struct GenericFuncs *FuncList, struct ParseRecord *PR,
								FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs, STRPTR buffer,
								BYTE mode, STRPTR destPath);
void AddSystemFilesToBigFile(FILE *fpw1, STRPTR destPath, STRPTR drawer, BYTE mode);
int GetDateStamp(STRPTR file1);
void MyTurnAssignIntoDir(STRPTR ass);
int my_getpath(BPTR lock, char *path);
BOOL InsertRightDisk(STRPTR path, STRPTR destDevice, STRPTR msg);
BOOL WaitForDisk(struct Window *window, STRPTR destDevice, STRPTR msg);
void RA_BigFile(FILE *fp, STRPTR argStr, STRPTR command,
								STRPTR path1, STRPTR path2, int fileSize, int dateStamp);
void InitFontsList(void);
void AddToFontsList(STRPTR fontPath);
void DeInitFontsList(void);
BOOL IsFontInList(STRPTR fontPath);

/**** parse2.c ****/

BOOL AnimFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL ArexxDosFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
											STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL SoundFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL PageFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL BinaryFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL MailFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL XappFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL ClipFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL CrawlFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL TextFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL ClipAnimFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
											STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL FontFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL SystemFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL IFFFuncSpec(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
									STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL ScriptFuncSpec(struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath);
BOOL JustWriteIt(struct ParseRecord *PR, FILE *fpw, STRPTR whiteSpcs, STRPTR buffer);
void SplitFullPath(STRPTR fullPath, STRPTR path, STRPTR filename);
void RemoveQuotes(STRPTR str);
int GetFileSize(STRPTR path);
BOOL CheckThisFont(	struct ParseRecord *PR, FILE *fpw1, FILE *fpw2, STRPTR whiteSpcs,
										STRPTR buffer, BYTE mode, STRPTR destPath, STRPTR fontStr );
void FetchString(STRPTR buffer, STRPTR dest, int max, int *count);
void FetchInteger(STRPTR buffer, int *dest, int *count);
BOOL TheFileIsNotThere(STRPTR path);

/**** parse3.c ****/

BOOL InterpretBigFile(STRPTR bigfilePath, BYTE mode, int *systemSize, int *dataSize,
											int *largest, BOOL systemFiles);

/**** parse4.c ****/

BOOL ParsePageFile(	FILE *fpw1, STRPTR whiteSpcs, STRPTR buffer, BYTE mode,
										STRPTR destPath, STRPTR pagePath );

extern void Report(STRPTR report);

#if 0
/**** setup.c ****/

BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
BOOL MonitorCreateRunTime(struct Window *window);
BOOL MonitorMissingFiles(struct Window *window);
BOOL MonitorCalcSize(struct Window *window);
BOOL MonitorUpload(struct Window *window);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);
void Report(STRPTR report);
void WaitOnLastButton(struct Window *window);
#endif

/******** E O F ********/
