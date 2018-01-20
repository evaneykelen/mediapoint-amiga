/*********************************************************
*File : external.h
*
*Desc : External vars
*/

/*********************************************************
*	Defs for Potgo resource	
*/
extern struct Library *PotgoBase;
extern ULONG PotBits;

// from file : dongle.c
GLOBAL int AllocDongle( void);
GLOBAL void FreeDongle( void);
GLOBAL void WriteDongle( UWORD , UWORD );
GLOBAL UWORD ReadDongle( UWORD );
GLOBAL int main( int , char **);

// from file : dongleio.c
GLOBAL int GetInput( void);
GLOBAL void SetOut( UBYTE *, UBYTE );
GLOBAL void ClrOut( UBYTE *, UBYTE );
GLOBAL void SendE2Header( UWORD );
GLOBAL UWORD ReadE2Data( void);
GLOBAL void SetupPortLines( void);
GLOBAL void Wait_Te_wPulse( void);
GLOBAL void WriteE2Data( UWORD );
GLOBAL void EraseE2( UWORD );
GLOBAL void E2OutputEn( void);
GLOBAL void E2OutputDis( void);

