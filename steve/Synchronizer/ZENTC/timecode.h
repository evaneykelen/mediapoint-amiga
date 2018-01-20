/**********************************

     TimeCode.h  7-July-89

     Revised: 16-Aug-90 
              17-Feb-92 Mode sw added for 24,25,30 Frames
              23-Mar-93 Protos  added for TcFuncs
              18-Apr-93 VITC/LTC control added
               
     Author : Martin Kay
            : Zen Computer Services
                     
              UUCP cbmuk!cbmuka!zencs!martnkay
              
              44 (0)61 - 793 1931
             
***********************************/

#ifndef TIMECODE_H
#define TIMECODE_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_TYPES_H
#include <exec/ports.h>
#endif

struct TimeFields {              /* Hour Min Sec Frame digit fields */
   unsigned h1 : 4;   unsigned h0 : 4;
   unsigned m1 : 4;   unsigned m0 : 4;
   unsigned s1 : 4;   unsigned s0 : 4;
   unsigned f1 : 4;   unsigned f0 : 4; };
   
union BCDTCode {                 /* Timecode in BCD form */
   struct TimeFields TFields;
   ULONG Frame; } ;

struct UserFields {              /* User Bits */
   unsigned U7 : 4;   unsigned U6 : 4;
   unsigned U5 : 4;   unsigned U4 : 4;
   unsigned U3 : 4;   unsigned U2 : 4;
   unsigned U1 : 4;   unsigned U0 : 4; };
   
union BCDUCode {
   struct UserFields UFields;
   ULONG UBits; } ;

struct TCMsg {                   /* The complete Timecode message */
   struct Message Msg;
   union BCDTCode TCode;
   union BCDUCode UCode; 
   USHORT Class, Code;
   char *StatusText;
   } ;

               /* Class defines */

#define  TCNORM     1   /* ie Read from external hardware */
#define  TCSET      2   /* Set external timecode, as per Code field */
#define  TCSTATUS   3   /* Get char string as per Code field */
#define  TCSTART    4   /* Generator control */
#define  TCSTOP     5   /* Generator control */
#define  TCQUIT     6   /* End driver program */
#define  TCMODE     7   /* Hardware mode VITC/LTC/CTRL etc */
#define  TCVLN1    64   /* Code = VITC Line 1  */
#define  TCVLN2    65   /* Code = VITC Line 2  */
#define  TCVMODE   66   /* Code = Block or Selected Lines  */

               /* Code defines */
#define  TCRDEF     0   /* Read Default Timecode   (Class = NORM) */
#define  TCRLTC     1   /* Read LTC only           (Class = NORM) */
#define  TCRVTC     2   /* Read VITC only          (Class = NORM) */
#define  TCRINT     2   /* Code is Interpolated    (Class = NORM) */

#define  TCPROG     1   /* Program Name            (Class = STATUS) */
#define  TCHARD     2   /* Hardware Name           (Class = STATUS) */
#define  TCXTRA     3   /* Extra Information       (Class = STATUS) */ 
#define  TCVER      4   /* Version Number          (Class = STATUS) */
#define  TCDATE     5   /* Version Date            (Class = STATUS) */


#define  TCREAL     1   /* Set current time        (Class = SET) */ 
#define  TCNEXT     2   /* Set time for next start (Class = SET) */ 
#define  TCOFF      3   /* Add offset              (Class = SET) */ 
#define  TCFRATE    4   /* Set Frame Rate          (Class = SET) */


#define  TCLTC      1   /* Read LTC                (Class = MODE) */ 
#define  TCVITC     2   /* Read VITC               (Class = MODE) */ 
#define  TCCTRL     3   /* Read Control Track      (Class = MODE) */ 
#define  TCAUTO     4   /* Switch for best results (Class = MODE) */ 

#define  TCVBLK     0   /* VITC Block Mode         (Class = VMODE) */ 
#define  TCVSEL     1   /* VITC Selected Lines     (Class = VMODE) */ 

#define  TC25       0   /* 25 Frames/sec */
#define  TC24       1   /* 24 Frames/sec */
#define  TC30       2   /* 30 Frames/sec */
#define  TC30D      3   /* 30 Drop Frame */

#ifndef SMPTE_H /* Sunrize Studio16 versions of these functions */

extern ULONG AddTimeCode(union BCDTCode,union BCDTCode,UBYTE);

/* result = AddTimeCode(time1,time2,format)
   
   if time1 = 0x01000114  
      time2 = 0x00000916
      format = TC25
   
   then result = 0x01001005      
*/

extern ULONG SubTimeCode(union BCDTCode,union BCDTCode,UBYTE);

/* result = SubTimeCode(time1,time2,format)
   
   if time1 = 0x01000114  
      time2 = 0x00000916
      format = TC25
   
   then result = 0x00595123      
*/

#endif

ULONG CodeToFrames(union BCDTCode,UBYTE);

/* result = CodeToFrames(time,format)
   
   if time = 0x00000114  
      format = TC25
   
   then result = 39      
*/

ULONG FramesToCode(ULONG,UBYTE);

/* result = FramesToCode(frames,format)
   
   if frames = 258
      format = TC25
   
   then result = 0x00001008      
*/


