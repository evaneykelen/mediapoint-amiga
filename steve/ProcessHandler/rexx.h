#ifndef SR_REXX_H
#define SR_REXX_H

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

#endif
