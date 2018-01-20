extern struct GfxBase 	*GfxBase;
extern struct Library 	*MLMMULibBase;
extern struct CapsPrefs CPrefs;
extern ULONG 			alloc;

GLOBAL int PageCntr;

// System setup vars
GLOBAL int 			RunMode;
GLOBAL int			SNRStackSize;
GLOBAL ULONG		TempoType;
GLOBAL BOOL			ObjectDateCheck;
GLOBAL int			PreLoadLevel;			// Preload level
GLOBAL ULONG		MaxMemSize;


GLOBAL int RunState;
GLOBAL SNR *SNR_TrackPlay;

#ifdef SR_REXX_H
GLOBAL AREXXCONTEXT *PublicRexxContext;
GLOBAL struct RexxMsg *PublicMsg_Arexx;
#endif


GLOBAL struct MsgPort *Port_PCtoGE;
GLOBAL struct MsgPort *Port_PCtoS;

#ifdef SR_REXX_H
GLOBAL REXXERROR RexxError[];
GLOBAL REXXCMD RexxCmds[];
#endif

// from file : b:message.c
GLOBAL void Message(char *fmt, ...);


#ifdef SR_REXX_H

// from file : rexx.c
GLOBAL void ParseCmdLine( char *, IDENTIFIER *);
GLOBAL struct RexxMsg *GetARexxMsg( AREXXCONTEXT *);
GLOBAL void ReplyARexxMsg( AREXXCONTEXT *, struct RexxMsg *, char *, LONG );
GLOBAL BOOL SetARexxLastError( AREXXCONTEXT *, struct RexxMsg *, char *);
GLOBAL BOOL SendARexxMsg( AREXXCONTEXT *, char *, BOOL );
GLOBAL void FreeARexx( AREXXCONTEXT *);
GLOBAL AREXXCONTEXT *InitARexx( void);
GLOBAL void ArexxTalks( AREXXCONTEXT *, struct ScriptInfoRecord *);

// from file : rexxcmd.c
GLOBAL void HandleGoto( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void SetTimeCode( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetTimeCode( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetTimeCodeAsc( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void QuitScript( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void SetEscape( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void SetCursor( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void SetMouse( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetPageNr( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetNrPages( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetState( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetVAR( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void SetVAR( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);
GLOBAL void GetNewScript( IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int  *);

#endif

// from file : mltool.c
GLOBAL void GetToolTypes(struct WBArg *wbarg, STRPTR scriptName);
GLOBAL void GetWorkbenchTools(int argc, char **argv, STRPTR scriptName);

// from file : addmodule.c
GLOBAL int AddModule( struct List *, struct List *, char *, struct ScriptInfoRecord *, struct MsgPort *, char *, PROCESSINFO **, MLSYSTEM *);

// from file : procerror.c
GLOBAL void ProcessError( int );

// from file : loadsegment.c
GLOBAL BPTR MLLoadSeg( struct List *, char *, ULONG *);
GLOBAL void MLUnLoadSeg( struct List *, BPTR );
GLOBAL BOOL MLGetQuietSeg( struct List *, BPTR );

// from file : initsegment.c
GLOBAL BOOL ScanToolTypes( char *, ULONG *, ULONG *, BOOL *);
GLOBAL int	GetPreResidentModules( struct List *, struct FileLock *, struct FileInfoBlock *);
GLOBAL int	GetAfterResidentModules( struct List *, struct FileLock *, struct FileInfoBlock *);
GLOBAL struct List *LoadMLSegments( struct List *, struct FileLock *, ULONG  );
GLOBAL void UnLoadMLSegments( struct List *, ULONG );

// from file : proccont.c
GLOBAL PROCDIALOGUE *GetFreeDial( PROCESSINFO *);
GLOBAL SYNCDIALOGUE *GetFreeSyncDial( void);
GLOBAL GEDIALOGUE *GetFreeGEDial( void);
GLOBAL void TerminateChildren( void);
GLOBAL void CleanupChildren( void);
GLOBAL int AddSyncProcessor( struct List *, struct List *, struct ScriptInfoRecord *, struct MsgPort *, PROCESSINFO **);
GLOBAL int AddGlobalEventProcessor( struct List *, struct List *, struct ScriptInfoRecord *, struct MsgPort *);
GLOBAL int AddTransition( struct List *, struct List *, struct ScriptInfoRecord *, struct MsgPort *);
GLOBAL BOOL MakeCopyOfVars( void);
GLOBAL void FreeCopyOfVars( void);
GLOBAL void PrintVC( VARCONTENTS *Vc);
GLOBAL VARCONTENTS *VCCopyOf( VIR *);
GLOBAL void Add( VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
GLOBAL void Subtract( VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
GLOBAL void Multiply( VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
GLOBAL void Divide( VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
GLOBAL void CalculateOn( VARCONTENTS *, VARCONTENTS *, VARCONTENTS *, UWORD		);
GLOBAL BOOL CheckCondition( VARCONTENTS *, VARCONTENTS *, UWORD );
GLOBAL VCR *CheckConditionsOn( struct List *, VARCONTENTS *);
GLOBAL VCR *CalculateVariablesOfSNR( struct List *);
GLOBAL int FreeProcessController(int,BOOL);
GLOBAL BOOL SetupMMU( void);
GLOBAL int InitProcessController(struct ScriptInfoRecord *, BOOL);
GLOBAL SNR *FindObject( struct List *, SNR *);
GLOBAL int AddGuides( struct List *);
GLOBAL int RemoveChild( void);
GLOBAL void ChildReply( void);
GLOBAL void ChildTalks( void);
GLOBAL void AnalyseGlobalMsg( GEVENTDIALOGUE *, BOOL );
GLOBAL void GETalks( void);
GLOBAL void SendLocalEvents( struct List *);
GLOBAL int SynchroTalks( void);
GLOBAL void GetSynchroReply( void);
GLOBAL void GetGEReply( void);
GLOBAL void PushToStack( SNRSTACK *, SNR *, ULONG , SNR *);
GLOBAL SNR *PullFromStack( SNRSTACK *);
GLOBAL ULONG PeekStackJumpType( SNRSTACK *);
GLOBAL SNR *PeekStackHitSNR( SNRSTACK *);
GLOBAL SNR *PeekStackSNR( SNRSTACK *);
GLOBAL SNR *TakeIndexedObject( SNR *, UWORD , BOOL );
GLOBAL void DoObjectDateCheck( SNR *);
GLOBAL SNR *ScriptForwardFollower( SNR *, SNRSTACK *, BOOL , BOOL , BOOL );
GLOBAL SNR *ScriptReverseFollower( SNR *, BOOL );
GLOBAL void ObjectPlayer( void);
GLOBAL void ObjectLoader( void);
GLOBAL void ObjectPuncher( void);
GLOBAL void PunchParObjects( struct List *);
GLOBAL int pc_ProcessScript( struct ScriptInfoRecord *, SNR *, BOOL, BOOL, BOOL);
GLOBAL BOOL SpecialAllocMLSystem(void);
GLOBAL void SpecialFreeMLSystem(void);
