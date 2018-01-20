#include "gen:support_protos.h"

/**** control ****/

BOOL ControlCDTV(struct CDTV_record *CDTV_rec);
void ShowTrack(struct Window *window, struct CDTV_record *CDTV_rec);
void GetNewCD(struct Window *window, struct CDTV_record *CDTV_rec);

/**** hostser ****/

BOOL Monitor_SerialPort(struct Window *window, struct CDTV_record *CDTV_rec);
void PrintString(struct Window *window, STRPTR str);
void ParseTheString(struct Window *window, STRPTR recBuff,
										struct CDTV_record *CDTV_rec);
void SendIsPlayingInfo(struct CDTV_record *CDTV_rec);
void SendGetCDInfo(struct CDTV_record *CDTV_rec);
int FindStringMax(STRPTR OrgString, STRPTR CheckString, int MaxLength);

/**** lowlevel ****/

BOOL OpenCDTV(struct CDTV_record *CDTV_rec);
void CloseCDTV(struct CDTV_record *CDTV_rec);
void DoIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data);
VOID SendIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data);
BOOL CDTV_PlayTrack(struct CDTV_record *CDTV_rec);
BOOL CDTV_PlayTrackFromTo(struct CDTV_record *CDTV_rec);
BOOL CDTV_PlayTrackStartEnd(struct CDTV_record *CDTV_rec);
BOOL CDTV_Pause(struct CDTV_record *CDTV_rec);
BOOL CDTV_Stop(struct CDTV_record *CDTV_rec);
void CDTV_GetPrevSong(struct CDTV_record *CDTV_rec, int *song);
void CDTV_GetNextSong(struct CDTV_record *CDTV_rec, int *song);
BOOL CDTV_IsPlaying(struct CDTV_record *CDTV_rec, BOOL getPos,
										STRPTR track, STRPTR index);
void CDTV_GetCDInfo(struct CDTV_record *CDTV_rec,
										STRPTR numTracks, STRPTR duration);
BOOL CDTV_Fade(struct CDTV_record *CDTV_rec, int speed, int inout);
BOOL CDTV_Reset(struct CDTV_record *CDTV_rec);
BOOL CDTV_Mute(struct CDTV_record *CDTV_rec, int mode);
BOOL CDTV_FrontPanel(struct CDTV_record *CDTV_rec, int mode);

/**** serial ****/

BOOL Open_SerialPort(struct CDTV_record *CDTV_rec);
void Close_SerialPort(struct CDTV_record *CDTV_rec);
void GetInfoFile(STRPTR appName, STRPTR appPath, STRPTR devName, int *portNr, int *baudRate);
void SendSerCmd(struct CDTV_record *CDTV_rec, int cmd, int arg1, int arg2);
BOOL SendStringViaSer(struct CDTV_record *CDTV_rec, STRPTR str);
BOOL GetTwoStringsFromSer(struct CDTV_record *CDTV_rec, STRPTR strA, STRPTR strB);
void SwitchTicksOn(struct Window *window);
void SwitchTicksOff(struct Window *window);

/******** E O F ********/
