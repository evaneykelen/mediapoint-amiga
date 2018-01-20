typedef struct
{
  struct MsgPort *ARexxPort;
  struct Library *RexxSysBase;
  long Outstanding;
  char PortName[24];
  char ErrorName[28];
  char Extension[8];
} AREXXCONTEXT;


#define REXX_RETURN_ERROR ((struct RexxMsg *)-1L)

#pragma libcall RexxContext->RexxSysBase CreateRexxMsg 90 09803
#pragma libcall RexxContext->RexxSysBase CreateArgstring 7E 0802
#pragma libcall RexxContext->RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxContext->RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxContext->RexxSysBase IsRexxMsg A8 801
#pragma libcall RexxContext->RexxSysBase ClearRexxMsg 9C 0802

/**** rexx1.c ****/

struct RexxMsg *GetARexxMsg_V1(AREXXCONTEXT *RexxContext);
void ReplyARexxMsg_V1(AREXXCONTEXT *RexxContext, struct RexxMsg *rmsg, char *RString, LONG Error);
short SendARexxMsg_V1(AREXXCONTEXT *RexxContext, char *RString, struct FileHandle *FH);
void FreeARexx_V1(AREXXCONTEXT *RexxContext);
short SetARexxLastError(AREXXCONTEXT *RexxContext, struct RexxMsg *rmsg, char *ErrorString);

/**** rexx2.c ****/

struct RexxMsg *GetARexxMsg_V2(AREXXCONTEXT *RexxContext);
BOOL ReplyARexxMsg_V2(AREXXCONTEXT *RexxContext, struct RexxMsg *msg,
											LONG return_code, char *result_string,
											LONG error_code );
struct RexxMsg *SendARexxMsg_V2(AREXXCONTEXT *RexxContext, char *port_name,
																char *command, ULONG flags);
void FreeARexx_V2(AREXXCONTEXT *RexxContext);
AREXXCONTEXT *InitARexx(char *RexxName);
BOOL FreeARexxMsg_V2(AREXXCONTEXT *RexxContext, struct RexxMsg *msg);
struct RexxMsg *WaitForReply_V2(AREXXCONTEXT *RexxContext);
BOOL IssueRexxCmd_V2(	STRPTR appName, STRPTR port_name, STRPTR command, STRPTR resultStr,
											ULONG *RC);

