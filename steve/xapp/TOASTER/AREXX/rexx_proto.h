struct RexxMsg *CreateRexxMsg(struct MsgPort *, char *, char *);
void *CreateArgstring(char *, long);
void DeleteRexxMsg(struct RexxMsg *);
void DeleteArgstring(char *);
BOOL IsRexxMsg(struct Mesage *);
