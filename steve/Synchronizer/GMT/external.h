// from file : synchrogmt.c
GLOBAL SYNCGUIDE *FindSyncGuide( struct List *, PROCESSINFO *);
GLOBAL void main( int , char **);

// from file : geninit.c
GLOBAL PROCESSINFO *ml_FindBaseAddr( int , char **);

// from file : tempoeditor.c
GLOBAL void ReturnPunch( struct List *, TIMEREQUEST *, struct MsgPort *, struct MsgPort *, int *);
GLOBAL void TempoEditor( PROCESSINFO	*);
GLOBAL struct InputEvent *Int_IEHandler( struct InputEvent *, EVENTDATA *);

