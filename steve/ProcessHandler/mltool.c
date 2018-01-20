#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include "minc:defs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "rexx_pragma.h"
#include "rexx_proto.h"
#include "rexx.h"
#include "external.h"

#define _PRINTF FALSE

/***************************************************
*Func : Scan all tooltypes
*in   : wbarg
*out  : -
*/
void GetToolTypes(struct WBArg *wbarg, STRPTR scriptName)
{
  struct DiskObject *dobj;
  char **ToolArray;
  char *ToolValue;
  
	if( (*wbarg->wa_Name) && (dobj = GetDiskObject(wbarg->wa_Name)) )	 
	{
		if ( scriptName )
		{
			findFullPath((struct FileLock *)wbarg->wa_Lock,scriptName);
			strcat(scriptName,wbarg->wa_Name);
		}

		ToolArray = (char **)dobj->do_ToolTypes;

		if(ToolValue = (char *)FindToolType(ToolArray,"MAXMEM"))
			CPrefs.maxMemSize = atoi(ToolValue);	

		if(ToolValue = (char *)FindToolType(ToolArray,"GOSUBSTACK"))
			CPrefs.gosubStackSize = atoi(ToolValue);	

		FreeDiskObject(dobj);
	}
}

/***************************************************
*Func : Check the tooltypes 
*in	  : argc, argv
*out  : -
*/
void GetWorkbenchTools(int argc, char **argv, STRPTR scriptName)
{
  struct WBStartup *WBenchMsg;
  struct WBArg *wbarg;
  int i;		
  LONG OldDirLock = -1;

	CPrefs.maxMemSize = 0; //AvailMem(MEMF_PUBLIC)-150000;
	CPrefs.gosubStackSize = 20;

	if(argc == 0)
	{
		WBenchMsg = (struct WBStartup *)argv;
		for(i = 0, wbarg=WBenchMsg->sm_ArgList; i < WBenchMsg->sm_NumArgs; i++,wbarg++)
		{
			/* if there is a directorylock for this wbarg then make it active */
			if( (wbarg->wa_Lock) && (*wbarg->wa_Name) )
				OldDirLock = CurrentDir(wbarg->wa_Lock);		
			GetToolTypes(wbarg,scriptName);

			if (OldDirLock != -1)
				CurrentDir(OldDirLock);
		}
	}

#if 0	// REMOVED ERIK 29-Sep-93

	if (CPrefs.maxMemSize == 0)	// no MAXMEM tool type found
	{
		CPrefs.maxMemSize = AvailMem(MEMF_PUBLIC)-150000;
		MLMMU_ThereIsMemRestrict(FALSE);
	}
	else
	{
		if ( CPrefs.maxMemSize > AvailMem(MEMF_PUBLIC)-100000 )
			CPrefs.maxMemSize = AvailMem(MEMF_PUBLIC)-100000;
		MLMMU_ThereIsMemRestrict(TRUE);
	}

#endif

}
