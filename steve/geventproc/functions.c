/*********************************************************
*Desc : common functions which may be used throughout the
*		program are put here
*/
#include <workbench/startup.h>
#include <exec/ports.h>
#include <exec/types.h>
#include <libraries/dosextens.h>
//#include <proto/all.h>

#include "nb:pre.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

/************************************************
*Func : Save way to send a message to a certain 
*		MsgPort
*in   : PortName -> Name of Msgport
*		Msg -> Message to be send to the above port
*out  : TRUE -> all went fine
*		FALSE -> error
*/ 
BOOL pc_SendMessage( PortName, Msg)
char *PortName;
struct Message *Msg;
{
  struct MsgPort *DestPort;

	Forbid();
	if( (DestPort = FindPort(PortName)) != NULL)
		PutMsg(DestPort, Msg);
	Permit();

	return((BOOL)DestPort);
}

/***********************************************
*Func : Free a signal from a task
*in   : task -> task of which the signal has to be freed
*		Signal -> signal number
*out  : -
*/
void FreeTaskSignal( task, Signal)
struct Task *task;
int Signal;
{
  ULONG Mask;
	
	Mask = 1 << Signal;
	Mask = ~Mask;

	task->tc_SigAlloc &= Mask;
}

/***********************************************
*Func : allocate a signal for a task
*in   : task 
*out  : Signalnumber 
*		-1 -> error
*/
int AllocTaskSignal( task)
struct Task *task;
{
  ULONG Mask;
  int i;
	
	// find a free signal starting from signal 31 down to 16
	for(i = 31; i < 15; i--)
		if(!(task->tc_SigAlloc & (1<<i)))
			break;

	if(i == 15)
		return(-1);

	// This signal is ours now
	Mask = 1 << i;
	task->tc_SigAlloc |= Mask;

	return(i);
}

/************************************************
*Func : Allocate a signal from a specified task
*		and assign it to a MsgPort
*in   : task -> Ptr to task
*		taskPort -> Ptr to a previously allocated MsgPort structure
*		PortName -> Ptr to name of the new port
*out  : TRUE -> ok
*		FALSE -> error
*/
BOOL CreateTaskPort( task, taskPort, PortName)
struct Task *task;
struct MsgPort *taskPort;
char *PortName;
{
  int SigBit;

	if( (SigBit = AllocTaskSignal(task)) == -1)
		return(FALSE);

	taskPort->mp_Node.ln_Name = PortName;
	taskPort->mp_Node.ln_Pri = 0;
	taskPort->mp_Node.ln_Type = NT_MSGPORT;

	taskPort->mp_Flags = PA_SIGNAL;
	taskPort->mp_SigBit = SigBit;
	taskPort->mp_SigTask = task;
	NewList(&(taskPort->mp_MsgList));

}

/************************************************
*Func : Remove a Msgport from the system pool
*		and free the memory it used
* <!> : The signal associated with the port is
*		not removed by this function and must
*		first be removed by FreeTaskSignal()
*in   : Port -> MsgPort
*out  : -
*/
void DeleteTaskPort( Port)
struct MsgPort *Port;
{
	if(Port)
		FreeMem(Port,sizeof(struct MsgPort));
}

