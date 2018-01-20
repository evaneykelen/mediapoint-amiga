/******************* PlayerCode.h ***********************
 *							*
 *    Advanced Player Device : device-specific commands	*
 *							*
 *    Last Modified: 12-Oct-89				*
 *							*
 ********************************************************/

#ifndef PLAYERCODE_H
#define PLAYERCODE_H

/* Currently assigned command codes to the Ariadne Video Player Device.

   Unassigned codes are to be considered reserved for future expansion.

   For summary of command effects refer to documentation.
*/

/* Codes 0-63 'DEVICE' System calls : serviced by DEVICE */

#define PDCMD_MAYREAD   CMD_NONSTD        /* EQU $09 */
#define PDCMD_MAX       PDCMD_MAYREAD

/* added by me */

#define AC_DCLEAR	0x05





#define AC_DPLAYR       0x0A
#define AC_DTYPE        0x0B
#define AC_DLOCK        0x0C
#define AC_DULOCK       0x0D
#define AC_DQUERY       0x0E
#define AC_DVECT        0x0F

#define AC_PSELCT       0x10
#define AC_PQUERY       0x11
#define AC_PIDENT       0x12
#define AC_PUSLCT       0x13

#define AC_PNULL        0x14

#define AC_XVWAIT       0x20
#define AC_XNULL        0x21

#define AC_MRESET       0x30
#define AC_MALLOC       0x31
#define AC_MADDLC       0x32
#define AC_MENDLC       0x33
#define AC_MFREEN       0x34
#define AC_MFREEL       0x35
#define AC_MCALLN       0x36
#define AC_MCALLS       0x37
#define AC_MLOAD        0x38
#define AC_MSAVE        0x39
#define AC_MEDIT        0x3A
#define AC_MNAME        0x3B
#define AC_MSTOP        0x3D
#define AC_MSTART       0x3E
#define AC_MDIR         0x3F

/* Codes 64-255 Player 'ATOM' calls : serviced by DRIVER */

#define AC_ATOMLIMIT    0x40

#define AC_ARESET       0x40
#define AC_AREADY       0x41
#define AC_AROFF        0x42
#define AC_ARCLRR       0x43

#define AC_AHVINT       0x46
#define AC_AHVEXT       0x47
#define AC_AHJDIS       0x48
#define AC_AHJENB       0x49
#define AC_AHJECT       0x4A
#define AC_AHRENB       0x4B
#define AC_AHRDIS       0x4C
#define AC_AHMON        0x4D
#define AC_AHUSER       0x4E
#define AC_AHREMT       0x4F

#define AC_APLOAD       0x50
#define AC_APRPLY       0x51
#define AC_APUSER       0x52
#define AC_APROM        0x53
#define AC_APSTAT       0x54
#define AC_APFAST       0x55
#define AC_APSLOW       0x56

#define AC_AQTYPE       0x58
#define AC_AQSTAT       0x59
#define AC_AQUSER       0x5A
#define AC_AQENDM       0x5B

#define AC_AABEEP       0x60
#define AC_AAMUTE       0x61
#define AC_AACX         0x62
#define AC_AA1ON        0x63
#define AC_AA1OFF       0x64
#define AC_AA2ON        0x65
#define AC_AA2OFF       0x66
#define AC_AAUDEX       0x67

#define AC_AVPION       0x68
#define AC_AVPIOF       0x69
#define AC_AVCION       0x6A
#define AC_AVCIOF       0x6B
#define AC_AVVION       0x6C
#define AC_AVVIOF       0x6D
#define AC_AVOVLY       0x6E
#define AC_AVTXT        0x6F

#define AC_AFREQ        0x70
#define AC_AFINFO       0x71
#define AC_AFSTOP       0x72
#define AC_AFGOST       0x73
#define AC_AFGOPL       0x74
#define AC_AFGOCN       0x75
#define AC_AFRETI       0x76
#define AC_AFRETS       0x77

#define AC_ACREQ        0x78
#define AC_ACGOST       0x79
#define AC_ACGOPL       0x7A

#define AC_ATREQ        0x7C
#define AC_ATINFO       0x7D
#define AC_ATGOPL       0x7E
#define AC_ATRETI       0x7F

#define AC_ASFAST       0x80
#define AC_ASSLOW       0x81
#define AC_ASSTIL       0x82
#define AC_ASSTOP       0x83

#define AC_AJUMPF       0x86
#define AC_AJUMPR       0x87
#define AC_AJUMPQ       0x88
#define AC_AJPFJF       0x89
#define AC_AJPFJR       0x8A
#define AC_AJPRJF       0x8B
#define AC_AJPRJR       0x8C
#define AC_AJHTJF       0x8D
#define AC_AJHTJR       0x8E

#define AC_AXCFWD       0x90
#define AC_AXCRVS       0x91

#define AC_AMGO         0x92
#define AC_AMPNF        0x93
#define AC_AMPNR        0x94
#define AC_AMPSF        0x95
#define AC_AMPSR        0x96
#define AC_AMPFF        0x97
#define AC_AMPFR        0x98
#define AC_AMSTF        0x99
#define AC_AMSTR        0x9A

#define  AC_MAXCMD      0x9A

/***** Error returns *****/

#define AC_MACROLIMIT   0x100

#define AE_NOERR 0x00   /* No error                                     */
#define AE_LIBOP 0x01   /* Library open error                           */
#define AE_FLOCK 0x02   /* Cannot Lock driver file (not found)          */
#define AE_FLOAD 0x03   /* Load driver failed                           */
#define AE_NEDRV 0x04   /* Nonexistent driver id                        */
#define AE_FEXCL 0x05   /* Device excluded by another process           */
#define AE_FTASK 0x06   /* Task initialisation failed                   */
#define AE_FIDNT 0x07   /* Not identified as driver                     */
#define AE_NDRIV 0x08   /* No driver Loaded                             */
#define AE_DLOCK 0x09   /* Driver config locked                         */
#define AE_NTIMP 0x0A   /* Command Not Available                        */
#define AE_NEJCT 0x0B   /* Eject disabled                               */
#define AE_BADIN 0x0C   /* Bad player return code                       */
#define AE_BADFN 0x0D   /* Internal Driver Failure                      */
#define AE_TRAYO 0x0E   /* Media ejected or absent                      */
#define AE_BADAK 0x0F   /* Bad Cmd ack                                  */
#define AE_NOCFG 0x10   /* Can't find Player-Config file                */
#define AE_NOMEM 0x11   /* Can't expand Macro Buffer                    */
#define AE_NOALC 0x12   /* Can't save ATOM - no Macro                   */
#define AE_MACER 0x13	/* Macro allocation error                       */
#define AE_NOMID 0x14	/* No valid Macro identity                      */
#define AE_NESTX 0x15   /* Macro Nesting exceeded                       */
#define AE_ABORT 0x16   /* Macro Aborted                                */
#define AE_MXERR 0x17   /* Macro Execution error                        */
#define AE_NTFRM 0x18   /* NOT FRAME error                              */
#define AE_NTTIM 0x19   /* NOT TIME error                               */
#define AE_NTCHA 0x1A   /* NOT CHAPTER error                            */
#define AE_MEDIT 0x1B   /* Device or Macros locked                      */
#define AE_NOIPT 0x1C   /* Input Error in special vector                */
#define AE_MACMD 0x1D   /* Can't use command in Macro                   */
#define AE_MANEX 0x1E   /* Can't save a call to non-existent Macro      */
#define AE_MANUM 0x1F   /* Can't save a call to Macro with same number  */
#define AE_MANAM 0x20   /* Can't save a call to Macro with same name    */
#define AE_NSAVE 0x21   /* No Save Name                                 */
#define AE_NOMAC 0x22   /* No Macros to save                            */
#define AE_FMACS 0x23   /* Can't open macro file                        */
#define AE_FWRIT 0x24   /* Cant write to macro file                     */
#define AE_FREAD 0x25   /* Cant read macro file                         */
#define AE_NOTMF 0x26   /* Not a macro file                             */
#define AE_MLOAD 0x27   /* Can't recontruct macro record                */

#define AE_LASTERROR 0x28

/* Player Status Flags (returned in Argx after READY or RESET) */

#define PSF_FRAME    0x0001   /* SET==FRAMECODED | CLEAR==TIMECODED  */
#define PSF_CHAPTER  0x0002   /* CHAPTER CODED DATA PRESENT          */
#define PSF_NOCODES  0x0004   /* UNCODED MEDIA                       */
#define PSF_EXT      0x0008   /* VARIABLE SPEED TIMECODED ESCAPE     */
#define PSF_CH1      0x0010   /* AUDIO 1 ON AFTER READY/RESET        */
#define PSF_CH2      0x0020   /* AUDIO 2 ON AFTER READY/RESET        */
#define PSF_VID      0x0040   /* VIDEO ON AFTER READY/RESET          */
#define PSF_STILL    0x0080   /* PLAYER STILL AFTER READY/RESET      */
#define PSF_VTR      0x0100   /* VIDEO TAPE MEDIA DETECT             */
#define PSF_REC      0x0200   /* PLAYER CAN RECORD FLAG              */
#define PSF_JMP      0x0400   /* PLAYER CAN INSTANT JUMP FLAG        */
#define PSF_USR      0x0800   /* USER CAN OVERIDE DEVICE FLAG        */
#define PSF_MULTI    0x1000   /* STOP/INFO FEATURES NOT ASYNCHRONOUS */
#define PSF_TXT      0x2000   /* ENCODED TELETEXT DETECT             */
#define PSF_PRG      0x4000   /* ENCODED PROGRAM DETECT              */
#define PSF_FMM      0x8000   /* FMM ON AUDIO DETECT                 */

#define PSF_ARIADNE  0x1FFF0000  /* reserved for Ariadne use */

#define PSF_DEV0     0x20000000  /* Developers use flag 0 */
#define PSF_DEV1     0x40000000  /* Developers use flag 1 */
#define PSF_DEV2     0x80000000  /* Developers use flag 2 */

/* Error escape codes */

/* if TRUE error ignored - if FALSE error reported 
   e.g. Frame request with CLV disc - NOT TIME error 
        PLAY CHA 1 with no chapter codes - NOT CHAPTER error 
*/

#define AX_FRAME   0x8000      /* ignore NOT FRAME error    */
#define AX_TIME    0x4000      /* ignore NOT TIME error     */
#define AX_CHAPTER 0x2000      /* ignore NOT CHAPTER error  */

/* player identifier codes */

#define PHILIPS               0xF000
#define SONY                  0xE000
#define PIONEER	              0xD000

#define CUSTOMPLAYER          0x0000

#define PHILIPS41_PLAYER      (0x0001 + PHILIPS)
#define PHILIPS83_PLAYER      (0x0002 + PHILIPS)
#define PHILIPS405_PLAYER     (0x0003 + PHILIPS)

#define SONY1500_PLAYER       (0x0001 + SONY)
#define SONY1000_PLAYER       (0x0002 + SONY)
#define SONY2000_PLAYER       (0x0003 + SONY)
#define SONY180_PLAYER        (0x0004 + SONY)

#define SONYVTRF_PLAYER       (0x0005 + SONY)  	       /* Umatic series 9 */

#define PIONEER42_PLAYER      (0x0001 + PIONEER)

#endif

/***** End of PlayerCode.h *****/
