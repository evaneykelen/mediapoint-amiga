
;---------------------------------------------------------------------------
TOCCATA_LIB_VERSION  equ   0                ;the current version of toccata.library
;---------------------------------------------------------------------------
; the public-part of toccata.library

    STRUCTURE __ToccataBase,LIB_SIZE
               APTR      _tb_BoardAddress   ;The address of the Toccata board 
                                            ;  (should not be used)
               APTR      _tb_HardInfo       ;Pointer to struct HardInfo (PRIVATE)
                                            ;  or NULL if there is no hardware.
                                            ;Check this entry to see if there is a
                                            ;  toccata board installed or not!

    LABEL   _tb_PUBSIZE           ; you should never use this

; subsequent fields in the library base are TOCCATA PRIVATE
;---------------------------------------------------------------------------
PAT_Min                equ  TAG_USER+200
PAT_MixAux1Left        equ  PAT_Min+2
PAT_MixAux1Right       equ  PAT_Min+3
PAT_MixAux2Left        equ  PAT_Min+4
PAT_MixAux2Right       equ  PAT_Min+5
PAT_InputVolumeLeft    equ  PAT_Min+6
PAT_InputVolumeRight   equ  PAT_Min+7
PAT_OutputVolumeLeft   equ  PAT_Min+8
PAT_OutputVolumeRight  equ  PAT_Min+9
PAT_LoopbackVolume     equ  PAT_Min+10
PAT_Mode               equ  PAT_Min+11
PAT_Frequency          equ  PAT_Min+12
PAT_Input              equ  PAT_Min+13      ;see TINPUT_xxx below
PAT_MicGain            equ  PAT_Min+14

TINPUT_Line    equ       0
TINPUT_Aux1    equ       1
TINPUT_Mic     equ       2
TINPUT_Mix     equ       3
;---------------------------------------------------------------------------
TT_Min               equ  TAG_USER+300

TT_Window        equ  TT_Min+0      ;Default: NULL
TT_BufferPri     equ  TT_Min+1      ;Default: 2
TT_IoPri         equ  TT_Min+2      ;Default: 1
TT_IrqPri        equ  TT_Min+3      ;Default: 105
TT_BufferSize    equ  TT_Min+4      ;Default: 100 KByte
TT_Frequency     equ  TT_Min+6      ;sample frequency, defaults to global settings
TT_Mode          equ  TT_Min+7      ;mode, defaults to global settings
TT_Length        equ  TT_Min+8      ;number of bytes to capture/playback
TT_FileName      equ  TT_Min+11     ;File to load from/save to
TT_Save          equ  TT_Min+15     ;Callback function CB_Save()  (capture only)
TT_Load          equ  TT_Min+16     ;Callback function CB_Load()  (playback only)
TT_Level         equ  TT_Min+17     ;Callback function CB_Level() (level only)
TT_ErrorTask     equ  TT_Min+18     ;task to notify if an error occurs
TT_ErrorMask     equ  TT_Min+19     ;signal mask to send
TT_SmartPlay     equ  TT_Min+20     ;BOOL, do not stop playing if CB_Load() is too slow
TT_RawTask       equ  TT_Min+21     ;Task to notify if the raw buffer has to be refilled
TT_RawMask       equ  TT_Min+22     ;signal mask to send
TT_RawBuffer1    equ  TT_Min+23     ;first buffer for raw playback
TT_RawBuffer2    equ  TT_Min+24     ;second buffer for raw playback
TT_RawIrqSize    equ  TT_Min+25     ;amount of data transferred by one raw interrupt
TT_RawReply      equ  TT_Min+26     ;function to call if buffer has been refilled
;---------------------------------------------------------------------------
;possible values for PAT_Mode

TMODE_LINEAR_8     equ     0         ; 8 bit linear
TMODE_LINEAR_16    equ     1         ;16 bit linear
TMODE_ALAW         equ     2         ; 8 bit A-Law compressed
TMODE_ULAW         equ     3         ; 8 bit µ-Law compressed

    BITDEF     TMODE,STEREO,3        ;set this bit in PAT_Mode if you want stereo

TMODE_LINEAR_8_S   equ     8         ; 8 bit linear stereo
TMODE_LINEAR_16_S  equ     9         ;16 bit linear stereo
TMODE_ALAW_S       equ     10        ; 8 bit A-Law compressed stereo
TMODE_ULAW_S       equ     11        ; 8 bit µ-Law compressed stereo

TMODE_MASK         equ     7
;---------------------------------------------------------------------------
; flags for T_Stop():

 BITDEF TS,DONTSAVECACHE,0     ;stop capture immediately, do not save cached data
;---------------------------------------------------------------------------
; You may use this macro to call the functions in toccata.library. To avoid
; conflicts with existing functions, they have a slightly unusual name (T_xxx).
CALLTOC:     MACRO
               move.l    a6,-(a7)
               movea.l   _ToccataBase(a5),a6
               jsr       _LVOT_\1(a6)
               movea.l   (a7)+,a6
             ENDM
;---------------------------------------------------------------------------



