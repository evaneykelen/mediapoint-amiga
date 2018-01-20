/***********************************************************
*File : Dongle.c
*	
*		Dongle.h
*Desc : Programmerlevel routines for dongle interfacing 
*/
#include <stdio.h>
#include <exec/types.h>
#include <resources/potgo.h>
#include <ctype.h>

#include <proto/battmem.h>
#include <proto/cia.h>
#include <proto/potgo.h>
#include <proto/misc.h>

#include <pragmas/battmem_pragmas.h>
#include <pragmas/cia_pragmas.h>
#include <pragmas/potgo_pragmas.h>
#include <pragmas/misc_pragmas.h>

#include "commands.h"
#include "defs.h"
#include "external.h"

#if _DONGLEWRITE_ON

#include "codes.h"

#endif

/**********************************************************
*Func : Allocate the Potgo bits from the resource
*in   : -
*out  : TRUE -> ok
*/
int AllocDongle( void)
{
  UBYTE CIAA_DDRA;

    if( (PotgoBase =(struct Library *) OpenResource(POTGONAME)) == NULL) 
		return(FALSE);

    PotBits = AllocPotBits(OUTRY|DATRY|OUTRX|DATRX);

    if(PotBits != (OUTRY|DATRY|OUTRX|DATRX))
    {
		FreePotBits(PotBits);
		return(FALSE);
    }

	// initialise port
   	CIAA_DDRA = *(UBYTE *)0xbfe201;	
   	*(UBYTE *)0xbfe201 = CIAA_DDRA | SK;
	SetupPortLines();
   	*(UBYTE *)0xbfe201 = CIAA_DDRA;
	
    return(TRUE);
}

/**********************************************************
*Func : return the potgo to the system
*in   : -
*out  : -
*/
void FreeDongle( void)
{
    if(PotBits == (OUTRY|DATRY|OUTRX|DATRX))
        FreePotBits(PotBits);
}

#if _DONGLEWRITE_ON

/**********************************************************
*Func : Program a address in the dongle with a 16 bit value
*in   : DestAddr -> which address, 0-15
*	Data -> Data-16 bit
*out  : -
*/
void WriteDongle( DestAddr,Data)
UWORD DestAddr;
UWORD Data;
{
  UBYTE CIAA_DDRA;

   /* CIAA_DDRA is not supported by any of the resources, lets take it over anyway */
   /* get the original DDRA setting, probably 0x03 */ 
   CIAA_DDRA = *(UBYTE *)0xbfe201;	
  
   /* now set the DDRA bit 7 for the FIRE1 to output since we use it as Serial Clock out */
   *(UBYTE *)0xbfe201 = CIAA_DDRA | SK;
   E2OutputEn();

   EraseE2(DestAddr);

   SendE2Header(WRITE|DestAddr);   	/* write to address n */
   WriteE2Data(Data);         		/* word written */

   E2OutputDis();

   /* restore DDRA to original setting */
   *(UBYTE *)0xbfe201 = CIAA_DDRA;
}

#endif

/*********************************************************
*Func : Read an address from the dongle and return its
*	16 bit value
*in   : SourceAddr -> Which address is to be read 0-15 for 9306
*					          0-63 for 9346
*out  : 16 bit Data
*/
UWORD ReadDongle( SourceAddr)
UWORD SourceAddr;
{
  UWORD Data;
  UBYTE CIAA_DDRA;

   /* CIAA_DDRA is not supported by any of the resources, lets take it over anyway */
   /* get the original DDRA setting, probably 0x03 */ 
   CIAA_DDRA = *(UBYTE *)0xbfe201;	
  
   /* now set the DDRA bit 7 for the FIRE1 to output since we use it as Serial Clock out */
   *(UBYTE *)0xbfe201 = CIAA_DDRA | SK;

   SendE2Header(READ|SourceAddr);
   Data = ReadE2Data();

   /* restore DDRA to original setting */
   *(UBYTE *)0xbfe201 = CIAA_DDRA;

   return(Data);
}

#if _MAIN_ON

/*********************************************************
*Example of how to implement the routines in your software
*/

/*********************************************************
*Func : as above
*in   : -
*out  : TRUE -> OK
*	FALSE -> Error
*/
int main( argc, argv)
int argc;
char **argv;
{
  int i,j;

    if(!AllocDongle())
	return(FALSE);

#if _DONGLEWRITE_ON
    for(i = 0; i < 16; i++)
    {
	   	WriteDongle(i,Codes[i]);
   		printf("Programmed with %04x, Data is %04x, Result = %s\n",Codes[i],ReadDongle(i),
	   		(ReadDongle(i) == Codes[i] ? "OK": "FAIL"));   
    }
#else
/*
    for(i = 0; i < 16; i++)
   	printf("Data is %04x\n",ReadDongle(i));
*/
    for(j = 0; j < 4; j++)
	    for(i = 0; i < 16; i++)
	   		ReadDongle(i);
#endif

    FreeDongle();
    return(TRUE);
}

#endif
