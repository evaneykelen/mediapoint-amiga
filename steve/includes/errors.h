#ifndef MINC_ERRORS_H
#define  MINC_ERRORS_H

enum
{
  NO_ERROR,
  ERR_NOMEM,		// general memory error

  ERR_QUIT_REQUEST,	// QUIT and RESTART request from synchronizer
  // addition errors from pc_ProcessScript	
  ERR_MEMVILIST = 199,

  // errors from GlobEProc	
  ERR_MEMMSGGEDIAL = 200,
  ERR_MEMJUMPLIST,
  ERR_PORTPCTOGE,
  ERR_REPPORTGETOPC,
  ERR_INPUTEVENTSERVER,

  // Errors from pc_ProcessScript()
  ERR_MLMMU,
  ERR_SYSTEMMEM,
  ERR_PORTPROCCONT,
  ERR_REPPORTPROCCONT,
  ERR_CHILDTOPARENTPORT,
  ERR_REPPORT,
  ERR_MEMSYNCDIAL,
  ERR_REPPORTPARENTTOCHILD,
  ERR_MEMPROCDIALLIST,			// 220
  ERR_MEMPILIST,
  ERR_MLSEGMENTS,
  ERR_MAINDIRMEM,	

  // Errors from serguide()
  ERR_NOGLOBALVARMEM,
  ERR_REPPORTGUIDE,
  ERR_GUIDETOPROCCONTPORT,
  ERR_MEMDIALOGUE,
  ERR_SYNCDIALMEM,
  ERR_SYNCPORT,
  ERR_SYNCTASKPORT,			// 230
  ERR_SYNCREPPORT,

  // Errors from Synchronizer	
  ERR_SYNCHRO,

  ERR_PORTTRANSITION,

  // Errors from XaPPs
  ERR_WORKER = 300,

  // Errors with string data
  ERR_NOTEMPO_EDITOR = 400,
  ERR_NOXAPPMEM,				// no memory for the xapp
  ERR_XAPPNOTFOUND,				// xapp couldn't be found
  ERR_GOSUBSTACKOVERFLOW,		// Stack overflow on gosubstack
  ERR_NOGRAPHICSMEM,
  ERR_INPUTHANDLER,
  ERR_EMPTYSCRIPT,
  ERR_ILLEGALTIMECODE			// start 00:00:00:00, end 00:00:00:00 timecode found
};
#endif
