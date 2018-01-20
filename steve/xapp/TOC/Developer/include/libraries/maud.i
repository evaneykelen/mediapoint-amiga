
               IFND      MAUD_I
MAUD_I         SET       1
;---------------------------------------------------------------------------
;              IFND        EXEC_TYPES_I
;                INCLUDE   "exec/types.i"
;              ENDC
;---------------------------------------------------------------------------
;---- ID's used in FORM MAUD

ID_MAUD:       equ       'MAUD'             ;the FORM-ID
ID_MHDR:       equ       'MHDR'             ;file header chunk
ID_MDAT:       equ       'MDAT'             ;sample data chunk
ID_MINF:       equ       'MINF'             ;optional channel info chunk (future)

;---- the file header 'MHDR'

 STRUCTURE __MaudHeader,0
    ULONG      mhdr_Samples       ;number of samples stored in MDAT
    UWORD      mhdr_SampleSizeC   ;number of bits per sample as stored in MDAT
    UWORD      mhdr_SampleSizeU   ;number of bits per sample after decompression
    ULONG      mhdr_RateSource    ;clock source frequency (see maud.doc)
    UWORD      mhdr_RateDevide    ;clock devide           (see maud.doc)
    UWORD      mhdr_ChannelInfo   ;channel information (see below)
    UWORD      mhdr_Channels      ;number of channels (mono: 1, stereo: 2, ...)
    UWORD      mhdr_Compression   ;compression type (see below)
    ULONG      mhdr_Reserved1     ;MUST be set to 0 when saving
    ULONG      mhdr_Reserved2     ;MUST be set to 0 when saving
    ULONG      mhdr_Reserved3     ;MUST be set to 0 when saving
    LABEL      mhdr_SIZEOF

;---- possible values for mhdr_ChannelInfo

MCI_MONO         equ  0  ;mono
MCI_STEREO       equ  1  ;stereo
MCI_MULTIMONO    equ  2  ;mono multichannel (channels can be 2, 3, 4, ...)
MCI_MULTISTEREO  equ  3  ;stereo multichannel (channels must be 4, 6, 8, ...)
MCI_MULTICHANNEL equ  4  ;multichannel (requires additional MINF-chunks) (future)

;---- possible values for mhdr_Compression

MCOMP_NONE       equ  0  ;no compression
MCOMP_FIBDELTA   equ  1  ;'Fibonacci Delta Compression' as used in 8SVX
MCOMP_ALAW       equ  2  ;16->8 bit, European PCM standard A-Law
MCOMP_ULAW       equ  3  ;16->8 bit, American PCM standard µ-Law
MCOMP_ADPCM2     equ  4  ;16->2 bit, ADPCM compression
MCOMP_ADPCM3     equ  5  ;16->3 bit, ADPCM compression
MCOMP_ADPCM4     equ  6  ;16->4 bit, ADPCM compression
MCOMP_ADPCM5     equ  7  ;16->5 bit, ADPCM compression
MCOMP_LONGDAT    equ  8  ;16->12 bit, used for DAT-longplay

;---------------------------------------------------------------------------
               ENDC      ; MAUD_I
