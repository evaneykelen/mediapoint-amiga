//	File		: inthandler.h
//	Uses		:
//	Date		: 2-08-1993
//	Author	: S. Vanderhorst
//	Desc.		: structs and protos for the interrupts handler
//

typedef struct
{
	ULONG		ed_Sig_NextObject;
	struct	Task	*ed_Task;
} WPEVENTDATA;

typedef struct
{
	struct IOStdReq *IRB;
	struct MsgPort *Port_IDtoIEI;
	int SigNum_NO;
	int IDErr;
	struct Interrupt Int_IEWP;
	WPEVENTDATA WPEventData;
	struct Interrupt  Int_IEWorkPage;
}OBJECT_INPUT_STRUCT;

extern void *Int_IEWorkPageServer();

void RemoveInputHandler( OBJECT_INPUT_STRUCT *oi );
void FreeInputHandler( OBJECT_INPUT_STRUCT *oi );
BOOL AddInputHandler( OBJECT_INPUT_STRUCT *oi );
int SetupInputHandler( OBJECT_INPUT_STRUCT *oi );

