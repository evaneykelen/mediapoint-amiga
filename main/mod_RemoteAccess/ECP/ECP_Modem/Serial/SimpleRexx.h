/*
 * Simple ARexx interface...
 *
 * This is a very "Simple" interface...
*/

#ifndef	SIMPLEREXX_TM_H
#define	SIMPLEREXX_TM_H

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

/*
 * This is the handle that SimpleRexx will give you
 * when you initialize an ARexx port...
 *
 * The conditional below is used to skip this if we have
 * defined it earlier...
*/

#ifndef	AREXXCONTEXT

typedef void *AREXXCONTEXT;

#endif	/* AREXXCONTEXT */

#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

ULONG ARexxSignal(AREXXCONTEXT RexxContext);
BOOL IsARexxReply(struct RexxMsg *msg);
struct RexxMsg *GetARexxMsg(AREXXCONTEXT RexxContext);
BOOL ReplyARexxMsg(	AREXXCONTEXT RexxContext, struct RexxMsg *msg,
										LONG return_code, char *result_string,
										LONG error_code );
struct RexxMsg *SendARexxMsg(	AREXXCONTEXT RexxContext, char *port_name,
															char *command, ULONG flags);
void ARexxFree(AREXXCONTEXT RexxContext);
AREXXCONTEXT ARexxInit(char *name, char *extension, BOOL multiple);
BOOL FreeARexxMsg(AREXXCONTEXT RexxContext, struct RexxMsg *msg);
char *StrNUpper(char *dest, char *src, unsigned int len);
char *UiToStr(unsigned int num, char *buf);
struct RexxMsg *RXWaitForReply(AREXXCONTEXT RexxContext);
BOOL IssueRexxCmd(STRPTR appName, STRPTR port_name, STRPTR command, STRPTR resultStr,
									ULONG *RC);

#endif	/* SIMPLE_REXX_H */
