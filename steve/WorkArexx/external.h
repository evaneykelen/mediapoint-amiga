// from file : rexx.c
GLOBAL char *MakeRexxCmd( int , PROCESSINFO *, char *);
GLOBAL char *ARexxName( AREXXCONTEXT *);
GLOBAL void ParseCmdLine( char *, IDENTIFIER *);
GLOBAL int GetARexxMsg( AREXXCONTEXT *);
GLOBAL void ReplyARexxMsg( AREXXCONTEXT *, struct RexxMsg *, char *, LONG );
GLOBAL BOOL SetARexxLastError( AREXXCONTEXT *, struct RexxMsg *, char *);
GLOBAL struct RexxMsg *SendARexxMsg( AREXXCONTEXT *, char *, BOOL );
GLOBAL void FreeARexx( AREXXCONTEXT *);
GLOBAL AREXXCONTEXT *InitARexx( char *);
GLOBAL AREXXCONTEXT *PerformRexxCmd( char *);

// from file : workarexx.c
GLOBAL void main( int , char **);

// from file : geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

