/***********************************************************
*File : Dongleio.c
*	
*Desc : lowlevel routines
*Tab = 8
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

/*********************************************************
	HARDWARE DESCRIPTIONS

VCC   -> +5V 				;joyport pin 7
GND   ->				;joyport pin 8
CS    -> Paula Potgo bit 14/15		;joyport pin 9
DI/DO -> Paula Potgo bit 12/13		;joyport pin 5
SK    -> CIAA bfe001 bit 7		;joyport pin 6	

e2prom 9306/9346 pin assignments 

   1 _ _ 8
CS -| U |- VCC
SK -|   |- nc
DI -|   |- nc
DO -|   |- GND
     ---

JoyPort pin#    E2prom pin #
1 NC            6 NC		
2 NC            7 NC
3 NC
4 NC
5 ------------- 3 + 4
6 ------------- 2
7 ------------- 8
8 ------------- 5
9 ------------- 1


Since the electrical rise/fall characteristics of the PAULA Potgo pins
are terrible it was impossible to use these pins for Serial Clock out.
The e2prom thought it was sometimes doubleclocked and responded with
sending multiple bits on one cycle. (Hardware made two cycles out of one)
This is due to the capacitors 0.047 uF and 100 pF which are attached to its
I/O pins.

CS and DI/DO are however less demanding and can easily use PAULA IO.

*/

/*********************************************************
*	Defs for Potgo resource	
*/
struct Library *PotgoBase;
ULONG PotBits;
 
/**************************************************
*Func : Read the potgo registers directly
*in   : -
*out  : 0/1
*/		
int GetInput( void)
{
    /* direct read, since no resource routines are available for 
       reading potgo registers */
    if( (*(UWORD *)0xdff016) & 0x1000)	
	return(1);
    return(0);
}

/*********************************************************
*	Defs for CIAA
*	No resource is available for this bit
*	Therefore we have to crawl into the hardware
*	directly. 
*/


/*********************************************************
*Func : Set a bit
*in   : Addr -> address at which the bit to be set is located
*	Bit -> Bit value
*out  : -
*/
void SetOut( Addr, Bit)
UBYTE *Addr;
UBYTE Bit;
{
    *Addr |= Bit;
}

/*********************************************************
*Func : clr a bit
*in   : Addr -> address at which the bit to be set is located
*	Bit -> Bit value
*out  : -
*/
void ClrOut( Addr, Bit)
UBYTE *Addr;
UBYTE Bit;
{
    *Addr &= ~Bit;
}


/*********************************************************
*Func : Send a packet to the rom with the command
*       See E2prom.h for packet types 
*in   : The packet to be send
*out  : -
*/
void SendE2Header( HeadPack)
UWORD HeadPack;
{
   int HeadBits;

   HeadBits = 0;
   HeadPack<<=1;
   HeadPack&=0xFFFE;
   
   CS_OUT; 
   DATA_OUT;

   SK_LOW;
   DATA_LOW;
   CS_LOW;

   /*send first cycle */
   DELAY;
   CS_HIGH;      	/*CS high*/

   while(HeadBits < 10)
   {
      SK_HIGH;   	/*CLK up*/
  		DELAY;
      SK_LOW;      	/*CLK down */

      if( (HeadPack<<=1) & 0x0400) 
          DATA_HIGH;
      else
          DATA_LOW;

      HeadBits++;
		DELAY;
   }
}/*SendE2Header*/

/*********************************************************
*Func : Read data from the e2prom
*       the address is not needed since it was sent by
*       the SendE2Header routine as a part of the HeadPack
*in   : - 
*out  : 16 bit Data
*/
UWORD ReadE2Data( void)
{
   UWORD DataIn = 0;
   int DataBits;

   DATA_IN;

   DataBits = 0;

   while(DataBits < 16)
   {
      DataIn <<= 1;

      SK_HIGH;
  		DELAY;
      SK_LOW;

      if(DATA_INPUT)
         DataIn |=0x0001;
      else
         DataIn &=0xFFFE;

      DataBits++;   
  		DELAY;
   };   
   SK_HIGH;
   DELAY;
   SK_LOW;
   CS_LOW;
   return(DataIn);

} /* ReadE2Data */

/******************************************************
*Func : Set the port bits to an initial state
*in   : -
*out  : -
*/
void SetupPortLines( void)
{
   SK_LOW;
   CS_LOW;
}

#if _DONGLEWRITE_ON

/*********************************************************
*Func : Wait approximately 30 ms for the e2prom to hold 
*	program its data
*in   : -
*out  : -
*/	 
void Wait_Te_wPulse( )
{
   int Time_Out_Cntr;

   /* dev only */
   Time_Out_Cntr = 0;

   CS_LOW;      /*CS low */

   /* wait a while (30 ms) and hold CS low while Sending clockpulses */
   /*then pull CS high and we're ready */   

   while(++Time_Out_Cntr < Te_wLENGTH)
   {
      SK_HIGH;
  		DELAY;
      SK_LOW;   
  		DELAY;
   }
   /* send final clock pulse */
   CS_HIGH;
  	DELAY;
   SK_HIGH;
 	DELAY;
   SK_LOW;
}

/*********************************************************
*Func : Send a word of 16 bit data to the e2prom
*	The address to be written was already specified
*	in SendE2Header's Headpack
*in   : WordOut -> 16 bit Data to be send to the e2prom
*out  : -
*/
void WriteE2Data( WordOut)
UWORD WordOut;
{
   int Bits;

   Bits = 0;

   while(Bits < 16)
   {
      if(WordOut & 0x8000)
         DATA_HIGH;   
      else
         DATA_LOW;

      SK_HIGH;
  		DELAY;
      SK_LOW;

      WordOut <<= 1;
      Bits++;
   }         

   Wait_Te_wPulse();

   CS_LOW;   

} /* WriteE2Data*/

/*********************************************************
*Func : Erase and address in the e2prom, needs to be done
*	for writing
*in   : DestAddr -> address to be erased
*out  : -
*/
void EraseE2( DestAddr)
UWORD DestAddr;
{
   SendE2Header(ERASE|DestAddr);      
   Wait_Te_wPulse();                     /*wait so E2prom can finish */    
}

/*********************************************************
*Func : After this command has been send, the e2prom is
*	ready to receive data which has to be written
*	into its memory
*in   : -
*out  : -
*/
void E2OutputEn( )
{
   SendE2Header(EWEN);
   CS_LOW;
}

/*********************************************************
*Func : After this command has been send, the e2prom will
*	block any write or erase commands issued by a user
*in   : -
*out  : -
*/
void E2OutputDis( void)
{
   SendE2Header(EWDS);
   CS_LOW;
}

#endif /* DONGLEWRITE_ON */
