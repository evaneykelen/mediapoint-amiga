/**** buttontask.c ****/

BOOL StartButtonTask(void);
void StopButtonTask(void);
BOOL IsTheButtonHit(void);

/**** icon.c ****/

BOOL SaveSessionIcon(STRPTR fullPath);
BOOL SaveCDFIcon(STRPTR fullPath);

/**** parse.c ****/

BOOL CleanUpBigFile(STRPTR bigfilePath);

/**** parsesession.c ****/

BOOL ParseSessionFile(struct SessionRecord *session_rec);
void FreeScriptAndDestLists(struct SessionRecord *session_rec);
BOOL SessionPerfFunc(	struct GenericFuncs *FuncList,
											struct ParseRecord *PR,
											STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int *val );
BOOL ScriptSessionFunc(	struct ParseRecord *PR, STRPTR s1, STRPTR s2, STRPTR s3, 
												STRPTR s4, int *val );
BOOL DestSessionFunc(	struct ParseRecord *PR, STRPTR s1, STRPTR s2, STRPTR s3, 
											STRPTR s4, int *val );
BOOL SaveSession(struct SessionRecord *session_rec, STRPTR fullPath);

/**** setup.c ****/

BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
BOOL FindNameInList(STRPTR oriPath, STRPTR name, int type);
BOOL GetRemoteAccessConfig(STRPTR path, struct SessionRecord *session_rec);
BOOL SetRemoteAccessConfig(STRPTR path, struct SessionRecord *session_rec);
void CalcValues(struct SessionRecord *session_rec, int active,
								int *numScripts, int *numDests,
								UBYTE **scriptList, UBYTE **swapList, UBYTE **ecpList, UBYTE **cdfList);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);
BOOL SelectScript(struct Window *onWindow, struct SessionRecord *session_rec,
									int selScript, UBYTE **scriptList);
BOOL SelectECPandCDF(	struct Window *onWindow, struct SessionRecord *session_rec,
											int selScr, int selDest, UBYTE **ecpList );
void UpdateEntry(	struct SessionRecord *session_rec, int selScript, int selDest,
									STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int val, int mode );
void GetEntry(struct SessionRecord *session_rec, int selScript, int selDest,
							STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int *val, int mode );
void DeleteScriptItem(struct SessionRecord *session_rec, int selScr, int selDest);
void DeleteDestItem(struct SessionRecord *session_rec, int selScr, int selDest);
void DoButtonGhosting(struct Window *window, int numScripts, int numDests);
BOOL SetOptions(struct Window *onWindow, struct SessionRecord *session_rec);
void GetSession(struct SessionRecord *session_rec, struct Window *window, STRPTR temp);
int SetCountdown(	struct Window *onWindow, struct SessionRecord *session_rec,
									STRPTR date, STRPTR time );
BOOL WaitOnCountdown(	struct Window *onWindow, struct SessionRecord *session_rec,
											STRPTR date, STRPTR time );
BOOL EditCDF(struct Window *onWindow, struct SessionRecord *session_rec, STRPTR path, STRPTR fileName);
void DisableCDFButtons(	struct Window *window, struct GadgetRecord *GR,
												struct CDF_Record *CDF_rec );

/**** upload.c ****/

BOOL MonitorUpload(struct Window *onWindow, struct SessionRecord *session_rec);
void Report(STRPTR report);
void WaitOnLastButton(struct Window *window);
BOOL ExecuteECP(STRPTR ECP, STRPTR CDF, STRPTR TempScript, STRPTR BigFile, int swap);

/******** E O F ********/
