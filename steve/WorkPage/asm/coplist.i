* Structure to store a copperlist in fastmem
* or even in compressed form
*

* the different flags

COPF_SHF	= $0001		; only a short frame in this list
COPF_LOFSHF	= $0002		; short and long frame
COPF_COMP	= $0004		; list is compressed

	RSRESET
cop_block:	rs.w	0
cop_flags:	rs.l	1		; the type of cops in this list
cop_mem:	rs.l	1		; pointer to base memory
cop_size:	rs.l	1		; size of the memory block
cop_point:	rs.l	1		; points to current free space
cop_remain:	rs.l	1		; the remaining bytes in this list
cop_count:	rs.l	1		; the number of cops in this list
cop_current:	rs.l	1	; points to the current coplist structure
cop_played:	rs.l	1	; how many coplists are played until now
cop_next:	rs.l	1		; pointer to next coplist table
cop_largest:	rs.w	1		; largest coplist in longs
cop_chipmem:	rs.l	1		; pointer to mem for chip copper lists
cop_chipsize:	rs.l	1		; total number of bytes allocated
cop_SIZEOF:	rs.w	0
