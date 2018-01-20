/**** cd_init ****/

struct CD_record *AllocCD(void);
void FreeCD(struct CD_record *);
BOOL OpenCD(struct CD_record *CD_rec);
void CloseCD(struct CD_record *CD_rec);
BOOL CD_PlayTrack(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_PlayTrackFromTo(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_PlayTrackStartEnd(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_Pause(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_Stop(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_Fade(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_Mute(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CD_NewCD(struct CD_record *CD_rec);
BOOL CD_GetInfo(struct CD_record *CD_rec, STRPTR str);
BOOL CD_GetTrack(struct CD_record *CD_rec, STRPTR str, int track);
BOOL CD_DiskWasChanged(struct CD_record *CD_rec);
BOOL CD_DiskIsValid(struct CD_record *CD_rec);
BOOL CD_DiskHasAudio(struct CD_record *CD_rec);

/**** cdtv_init ****/

struct CDTV_record *AllocCDTV(void);
void FreeCDTV(struct CDTV_record *);
BOOL OpenCDTV(struct CDTV_record *CDTV_rec);
void CloseCDTV(struct CDTV_record *CDTV_rec);
BOOL CDTV_PlayTrack(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_PlayTrackFromTo(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_PlayTrackStartEnd(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_Pause(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_Stop(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_Fade(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_Mute(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL CDTV_NewCD(struct CDTV_record *CDTV_rec);
BOOL CDTV_GetInfo(struct CDTV_record *CDTV_rec, STRPTR str);
BOOL CDTV_GetTrack(struct CDTV_record *CDTV_rec, STRPTR str, int track);
BOOL CDTV_DiskWasChanged(struct CDTV_record *CDTV_rec);
BOOL CDTV_DiskIsValid(struct CDTV_record *CDTV_rec);
BOOL CDTV_DiskHasAudio(struct CDTV_record *CDTV_rec);

/**** gui ****/

struct Window *OpenHostWindow(void);
void CloseHostWindow(struct Window *window);

/**** host ****/

BOOL MonitorUser(struct Window *window, struct all_record *all_rec);
BOOL HostOpenLibs(void);
void HostCloseLibs(void);
void OpenUserApplicationFonts(struct UserApplicInfo *UAI, STRPTR fontName,
															int size1, int size2);
void CloseUserApplicationFonts(struct UserApplicInfo *UAI);
void PrintString(struct Window *window, STRPTR str);

/**** hostser ****/

BOOL Monitor_SerialPort(struct Window *window, struct all_record *all_rec,
												struct CDTV_record *CDTV_rec,
												struct CD_record *CD_rec, struct MPEG_record *MPEG_rec);
void ParseTheString(struct Window *window, STRPTR recBuff,
										struct all_record *all_rec,
										struct CDTV_record *CDTV_rec,
										struct CD_record *CD_rec, struct MPEG_record *MPEG_rec);
int FindStringMax(STRPTR OrgString, STRPTR CheckString, int MaxLength);

/**** mpeg_init ****/

struct MPEG_record *AllocMPEG(void);
void FreeMPEG(struct MPEG_record *MPEG_rec);
BOOL OpenMPEG(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec);
void CloseMPEG(struct MPEG_record *MPEG_rec);
BOOL MPEG_PlayTrack(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
										int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_PlayTrackFromTo(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
													int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_PlayTrackStartEnd(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
														int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_Pause(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
								int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_Stop(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
								int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_SingleStep(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
											int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
BOOL MPEG_SlowMotion(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
											int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3);
int LSN2Track(struct CD_record *CD_rec, ULONG start);

/**** serial ****/

BOOL Open_SerialPort(struct all_record *all_rec);
void Close_SerialPort(struct all_record *all_rec);
BOOL DoIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data);
BOOL SendIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data);
void GetInfoFile(STRPTR appName, STRPTR appPath, STRPTR devName, int *portNr, int *baudRate);
BOOL DoSerIO(struct all_record *all_rec, STRPTR str, int command);
BOOL PutStringToSer(struct all_record *all_rec, STRPTR str);
BOOL GetStringFromSer(struct all_record *all_rec, STRPTR str);

/******** E O F ********/
