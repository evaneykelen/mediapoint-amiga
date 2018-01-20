/*********************************************************
*File : Commands.h
*
*Desc : E2prom command codes
*/

#define Te_wLENGTH 500	/* Nr of clockpulses before erase has been done */

/* 9306/9307/9346-E2Prom command headers
  the headers are already converted to 9 bit format (plus startbit)
   so that the may be or'd with the lower 6 bits of the address if needed */
#define READ 0x0180		/* read an address */
#define WRITE 0x0140		/* write an address */
#define ERASE 0x01C0		/* erase an address, must preceed the WRITE*/
#define EWEN 0x0130		/*erase/write enable, no address bits */
#define EWDS 0x0100		/*erase/write disable, no address bits */
#define ERAL 0x0120		/*erase all data, no address bits */ 
#define WRAL 0x0110		/*write all addresses with data, no address bits*/

