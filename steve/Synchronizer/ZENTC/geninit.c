/******************************************************
*Desc : This is the general init module of MediaLink
*       All XaPPS should use the functions from this
*       module to communicate with MediaLink processguides 
* <!> : All tabs (size 4) have been replaced by spaces
*/

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
#include <graphics/view.h>
#include <graphics/rastport.h>


#define WORKERNAME "SynchroMLTC"

// Some protos, defines and dialogue structures we need
#include "minc:types.h"
#include "minc:process.h"

/**************************************************
*Func : Find out the environment in which this process 
*       has been started. If it is not from MediaLink
*       tell the user about it and return NULL.
*		If a programmer wants to test it software from
*		the CLI then he should setup a PROCESSINFO
*		structure using AllocMem and put his parameters
*		into the sugestive PI->pi_Arguments.ar_Worker fields.
*		Notice that when runnig from the CLI, it is not possible
*		to communicate with other processes since all other fields
*		are not valid. 
*in   : argc -> if != 0 then CLI
*               if == 0 then workbench or ML
*out  : NULL -> CLI or WORKBENCH process
*               else ptr to PROCESSINFO structure 
*/
PROCESSINFO *ml_FindBaseAddr( argc, argv)
int argc;
char **argv;
{
  PROCESSINFO *ThisPI;      // the PI struct of this process in our parents PIList

    ThisPI = NULL;

    //check if we are started from the CLI, the Workbench or a MediaLink guide
    if(argc)
        printf("MediaLink aPP\nCommand: %s\nUnable to start from CLI.\n", WORKERNAME);
    else
    {
        // If the size of the message we received is smaller than the ProcessInfo
        // structure then this has to be a workbench-run application
        if(((struct WBStartup *)argv)->sm_Message.mn_Length <
            ( sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD)))
        {
            printf("MediaLink aPP\nCommand: %s\nUnable to start from WORKBENCH.\n",WORKERNAME);
            Delay(100);
        }
        else
            ThisPI = (PROCESSINFO *)((ULONG)argv - (ULONG)sizeof(struct Node) - sizeof(UWORD));
    }
    return(ThisPI);
}

