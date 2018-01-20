#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/interrupts.h>
#include <hardware/cia.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <cdtv/cdtv.h>

#define TAB_KEYDOWN 0x42
#define MODULENAME "CDoff2"
#define IDSTRING "CDoff V1.00\n\r"
#define STACKSIZE 64
#define BOOTPRI 35
#define RESIDENT 1
#define STRSIZE 32

#define EXECNAME "exec.library"
#define LIBRARY_VERSION 0

extern struct CIA __far ciaa;
static struct TaskBlock
{
	struct Task tb_TSK;
	struct MsgPort tb_PRT;
	struct IOStdReq tb_IOR;
	struct Interrupt tb_INT;
	ULONG tb_STK[ STACKSIZE ];
};

static struct KickMod
{
	struct MemChunk km_OverwriteMe;
	APTR km_ModuleTab[3];
	struct Resident km_ROMTag;
	struct MemList km_ML;
	struct TaskBlock *km_TB;
	int count;
	UBYTE km_ModuleName[STRSIZE],
			km_InputName[STRSIZE],
			km_IdString[STRSIZE];
};

static VOID __regargs RemHandler( struct KickMod*,UWORD );
static LONG __regargs AddHandler( struct KickMod* );
static struct InputEvent* __asm EventHandler( register __a0 struct InputEvent*,register __a1 struct MsgPort*);
static VOID __asm Install( register __a0 struct KickMod*);
static VOID __stdargs Task( struct KickMod*);
static VOID EndCode();

LONG __saveds main()
{

struct KickMod *km;
UWORD memsize;
UBYTE *cpydest,*cpysrc;
struct ExecBase *eb;
struct Process *proc;
struct Message *wbmsg = NULL;

	proc = ( struct Process *)FindTask(NULL);
	if( proc->pr_CLI == NULL )
	{
		WaitPort(&proc->pr_MsgPort );
		wbmsg = GetMsg(&proc->pr_MsgPort);
	}
	if( eb = ( struct ExecBase *)OpenLibrary(EXECNAME,LIBRARY_VERSION ) )
	{
		if( !FindPort( MODULENAME ) )
		{
			if( km = ( struct KickMod *) AllocMem(memsize=sizeof(struct KickMod ) + ((ULONG)EndCode - (ULONG)Install), MEMF_CHIP|MEMF_CLEAR ) )
			{
				km->count = 0;
				km->km_ROMTag.rt_MatchWord = RTC_MATCHWORD;
				km->km_ROMTag.rt_MatchTag = &km->km_ROMTag;
				km->km_ROMTag.rt_EndSkip = (APTR ) ( (UBYTE *) km + memsize);
				km->km_ROMTag.rt_Flags = RTF_COLDSTART;
				km->km_ROMTag.rt_Type = NT_TASK;
				km->km_ROMTag.rt_Version = 37;
				km->km_ROMTag.rt_Pri = BOOTPRI;
				km->km_ROMTag.rt_Name = km->km_ModuleName;
				km->km_ROMTag.rt_IdString = km->km_IdString;
				km->km_ROMTag.rt_Init = (APTR)((UBYTE*)km + sizeof(struct KickMod));
				km->km_ML.ml_Node.ln_Type = NT_MEMORY;
				km->km_ML.ml_ME[0].me_Un.meu_Addr = (APTR)km;
				km->km_ML.ml_NumEntries = 1;
				km->km_ML.ml_ME[0].me_Length = memsize;
				strcpy(km->km_ModuleName,MODULENAME);
				strcpy(km->km_IdString,IDSTRING);
				strcpy(km->km_InputName,"input.device");

				cpysrc = (UBYTE *)Install;
				cpydest = (UBYTE*)km->km_ROMTag.rt_Init;
				while( cpysrc != (UBYTE*)EndCode ) *cpydest++ = *cpysrc++;
#ifdef RESIDENT
				Forbid();
				km->km_ModuleTab[0] = (APTR)&km->km_ROMTag;
				if( eb->KickTagPtr)
				{
					km->km_ModuleTab[1] = (APTR)((ULONG)eb->KickTagPtr | 1 << 31 );
				}
				else
					km->km_ModuleTab[1] = NULL;
				if( eb->KickMemPtr )
					km->km_ML.ml_Node.ln_Succ = ( struct Node * ) eb->KickMemPtr;
				eb->KickTagPtr = (APTR)km->km_ModuleTab;
				eb->KickMemPtr = (APTR)&km->km_ML;
				eb->KickCheckSum = SumKickData();
				Permit();
#endif RESIDENT
				InitResident( &km->km_ROMTag,
				(unsigned long)km);
			}
		}
		CloseLibrary( &eb->LibNode );
	}
	if( wbmsg)
	{
		Forbid();
		ReplyMsg(wbmsg);
	}
	return(0);
}

static VOID __asm Install( register __a0 struct KickMod *km )
{
	char *naam;
	struct ExecBase *eb;
	struct MsgPort *IOPort;
	struct IOStdReq *IOReq;
	BYTE Err;
	int i;
	LONG sigbit;

	if( km || ( km = ( struct KickMod*)((ULONG)Install - sizeof( struct KickMod))))
	{
/*		if( km->km_TB = ( struct TaskBlock*)AllocMem(sizeof(struct TaskBlock),MEMF_PUBLIC|MEMF_CLEAR))
		{
			if( km->km_TB->tb_PRT.mp_SigBit = AllocSignal(-1))
			{
				km->km_TB->tb_PRT.mp_Node.ln_Name = km->km_ModuleName;
				km->km_TB->tb_PRT.mp_Node.ln_Type = NT_MSGPORT;
				km->km_TB->tb_PRT.mp_Flags = PA_SIGNAL;
				km->km_TB->tb_PRT.mp_SigTask = &km->km_TB->tb_TSK;
				AddPort( &km->km_TB->tb_PRT );
			}
		}
*/
	if( km->count > 5 )
		ColdReboot();
	else
		km->count++;
	}
	return;
}

static VOID EndCode(){}
