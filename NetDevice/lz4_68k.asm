;---------------------------------------------------------------------------------------------
; lz4_68k.asm: an LZ4 block decoder and encoder for the plain 68000.
;
; Copyright (c) 2026 Stefan Skotte and contributors. MIT licence (LICENSE in this directory).
; Written from the LZ4 block format description (github.com/lz4/lz4, doc/lz4_Block_format.md).
; The decoder's fast path uses the technique of Arnaud Carre's lz4_fastest (MIT,
; github.com/arnaud-carre/lz4-68k): a token jump table into unrolled copy blocks. No code was
; copied from it or from any other implementation.
;
; Assemble-time option: LZ4_68K_UNCHECKED_READS = 1 (e.g. vasm -DLZ4_68K_UNCHECKED_READS=1)
; drops the decoder fast path's two match-source checks (the window and offset 0). Writes stay
; inside [dst, dst_end) and src is never read past its end; only where a match copies FROM is
; no longer checked, so a malformed block may read memory below the window. Default 0: checked.
;
; Motorola syntax for vasm (vasmm68k_mot -m68000; -Faout or -Fhunk) and Devpac-style
; assemblers. 68000 only: no 68010+ instructions, and no word or long access at an odd address
; whatever the alignment of the buffers. The register contract and the buffer rules are in
; lz4_68k.h; the encoder's exact behaviour is the C model lz4_68k.c.
;
; Entry points (register call; every register not carrying a result is preserved):
;   lz4_decode          a0 src, a1 dst, a2 dst_end, a3 window start, d0.l src length
;                       -> d0.l decoded length, or -1 (malformed block, or it would not fit)
;   lz4_encode          a0 src, a1 dst, a2 table, d0.l n, d1.l cap, d2.l table entries
;                       -> d0.l block size, or 0 if the block would be larger than cap
;   lz4_encode_linked   the same, plus a3 = the window start (matches may reach back to it)
; and C (stack) entries _lz4_decode, _lz4_encode, _lz4_encode_linked, see lz4_68k.h.
;---------------------------------------------------------------------------------------------

	ifnd	LZ4_68K_UNCHECKED_READS
LZ4_68K_UNCHECKED_READS	equ	0
	endif

	section	code,code

	xdef	lz4_decode
	xdef	lz4_encode
	xdef	lz4_encode_linked
	xdef	_lz4_decode
	xdef	_lz4_encode
	xdef	_lz4_encode_linked

;---------------------------------------------------------------------------------------------
; C entries (m68k-amigaos gcc / vbcc: arguments on the stack, d0-d1/a0-a1 scratch).
;
; long lz4_decode(const void *src, unsigned long srclen, void *dst, void *dst_end,
;                 const void *window);
_lz4_decode:
	movem.l	a2-a3,-(sp)
	move.l	12(sp),a0
	move.l	16(sp),d0
	move.l	20(sp),a1
	move.l	24(sp),a2
	move.l	28(sp),a3
	bsr	lz4_decode
	movem.l	(sp)+,a2-a3
	rts

; unsigned long lz4_encode(const void *src, unsigned long n, void *dst, unsigned long cap,
;                          unsigned short *table, unsigned long entries);
_lz4_encode:
	movem.l	d2/a2,-(sp)
	move.l	12(sp),a0
	move.l	16(sp),d0
	move.l	20(sp),a1
	move.l	24(sp),d1
	move.l	28(sp),a2
	move.l	32(sp),d2
	bsr	lz4_encode
	movem.l	(sp)+,d2/a2
	rts

; unsigned long lz4_encode_linked(const void *src, unsigned long n, void *dst,
;                                 unsigned long cap, unsigned short *table,
;                                 unsigned long entries, const void *window);
_lz4_encode_linked:
	movem.l	d2/a2-a3,-(sp)
	move.l	16(sp),a0
	move.l	20(sp),d0
	move.l	24(sp),a1
	move.l	28(sp),d1
	move.l	32(sp),a2
	move.l	36(sp),d2
	move.l	40(sp),a3
	bsr	lz4_encode_linked
	movem.l	(sp)+,d2/a2-a3
	rts

;=============================================================================================
; DECODER
;
; Registers: a0 ip, a1 op, a2 dst_end, a3 window start, a4 src end, a5 a continuation address,
; a6 match source; d0 the token (bits 8-31 always 0), d2 the offset (bits 16-31 always 0), d7
; the fast budget (below), d1/d3/d4 scratch. d5 and d6 are not used.
;
; The fast path. Its unit is a "slot": 19 bytes of input and 32 of output. A sequence with a
; literal nibble and a match nibble below 15 fits one slot (token, 14 literals, 2 offset bytes;
; 14 + 18 bytes out), and so does one with match nibble 15 up to its first length byte. While
; src_end - ip >= 19 + 32 * d7 and dst_end - op >= 32 + 32 * d7 hold before a sequence, the
; sequence and d7 more slots are known to fit, so a short sequence pays one DBF for both
; bounds, and none of them can be the block's last one (that one ends exactly at src_end with
; its literals). A token jumps (one 256-entry table) straight into the literal copy of the
; block for its match nibble: block M is 14 unrolled literal moves, entered 2 * L bytes before
; their end, then the offset, the window check and M + 4 unrolled match moves.
; A literal or match nibble of 15 with a first length byte below 255 (runs up to 269 / 273
; bytes) takes ceil(extra / 32) more slots from d7 and is copied by one jump into a long run of
; unrolled moves (longs when src and dst have the same parity). When d7 cannot pay, or a
; length byte is 255, the sequence goes to a fully checked loop, which hands back to the fast
; path whenever the margins allow (.recount), and decodes the block's tail.
;
; The window check: a match source must lie in [window, op). The fast path tests the lower
; bound and an offset of 0 (the upper bound); a match source below address 0 (an offset larger
; than op itself) cannot happen when window >= $10000, so a window below $10000 sends the
; whole block to the checked loop, which tests the full range.
;=============================================================================================

DBLK	equ	104			; bytes per match-length block (padded)
SLOTI	equ	19			; input bytes per slot
FMAX	equ	273			; the longest fast copy: a match of 19 + 254

; The common head of every match block: the literal copy, the offset, the offset-0 test.
dhead	macro				; \1 = the match nibble
.mb\1:
	rept	14
	move.b	(a0)+,(a1)+
	endr
.ml\1:	move.b	(a0)+,d1		; the offset, little-endian
	move.b	(a0)+,-(sp)
	move.w	(sp)+,d2
	move.b	d1,d2
	if	LZ4_68K_UNCHECKED_READS=0
	beq.s	.zt\1			; low byte 0: the offset may be 0
	endif
.zr\1:	move.l	a1,a6
	sub.l	d2,a6
	if	LZ4_68K_UNCHECKED_READS=0
	cmpa.l	a3,a6
	bcs.s	.et\1			; reaches below the window
	endif
	endm

; The block's far exits, in reach of its short branches.
dtramp	macro
.zt\1:	bra.w	.zoff
.et\1:	bra.w	.derr
	endm

dmatch	macro				; \1 = the match nibble (0..14)
	dhead	\1
	rept	\1+4
	move.b	(a6)+,(a1)+
	endr
	dbf	d7,.dfast
	bra.w	.recount
	dtramp	\1
	if	*-.mb\1>DBLK
	fail	"DBLK too small"
	endif
	dcb.b	DBLK-(*-.mb\1),0
	endm

lz4_decode:
	movem.l	d1-d7/a0-a6,-(sp)
	move.l	a1,-(sp)		; the dst start, for the result
	cmpa.l	a1,a2
	bcs.w	.derr			; dst_end below dst
	cmpa.l	a1,a3
	bhi.w	.derr			; the window starts after dst
	lea	0(a0,d0.l),a4
	moveq	#0,d0
	moveq	#0,d2
	cmpa.l	#$10000,a3
	bcs.w	.lowwin			; a low window: the checked loop does it all
	lea	-SLOTI(a4),a6
	move.l	a6,d5			; d5: the last ip a slot may start at
	move.l	a2,d6
	sub.l	a1,d6
	cmp.l	#$f0000,d6		; (so that d7 = margin / 32 fits 15 bits)
	bls.s	.dl1
	move.l	#$f0000,d6
.dl1:	add.l	a1,d6
	moveq	#32,d3
	sub.l	d3,d6			; d6: the last op a slot may start at
.recount:
	move.l	d5,d3
	sub.l	a0,d3			; src margin
	move.l	d6,d4
	sub.l	a1,d4			; dst margin
	cmp.l	d4,d3
	blt.s	.rc1
	move.l	d4,d3
.rc1:	tst.l	d3
	bmi.w	.tail
	lsr.l	#5,d3
	move.w	d3,d7
.dfast:	move.b	(a0)+,d0		; token
	move.w	d0,d1
	add.w	d1,d1
	move.w	.dtab(pc,d1.w),d1
	jmp	.dtab(pc,d1.w)
.dtab:
DT_T	set	0
	rept	256
DT_L	set	DT_T>>4
DT_M	set	DT_T&15
	if	DT_L=15
	dc.w	.ll0+DT_M*8-.dtab
	else
	dc.w	.mb0+DT_M*DBLK+2*(14-DT_L)-.dtab
	endif
DT_T	set	DT_T+1
	endr

	dmatch	0
	dmatch	1
	dmatch	2
	dmatch	3
	dmatch	4
	dmatch	5
	dmatch	6
	dmatch	7
	dmatch	8
	dmatch	9
	dmatch	10
	dmatch	11
	dmatch	12
	dmatch	13
	dmatch	14

; Match nibble 15 (block 15, laid out like the others up to .zr15).
	dhead	15
	moveq	#0,d3
	move.b	(a0)+,d3		; the first length byte (inside the slot)
	cmp.b	#31,d3
	bhi.w	.mxbig
	add.w	#19,d3			; 19..50 bytes: at most 32 beyond the slot, one slot more
	subq.w	#1,d7
	bmi.w	.mxroom			; d7 cannot pay: check the room itself
	bra.w	.mcopy
	dtramp	15
	if	*-.mb15>DBLK
	fail	"DBLK too small"
	endif
	dcb.b	DBLK-(*-.mb15),0

; Literal nibble 15: per match nibble a stub sets the continuation (that block's .ml).
DLL	macro
.ll\1:	lea	.ml\1(pc),a5
	bra.w	.llit
	endm
	DLL	0
	DLL	1
	DLL	2
	DLL	3
	DLL	4
	DLL	5
	DLL	6
	DLL	7
	DLL	8
	DLL	9
	DLL	10
	DLL	11
	DLL	12
	DLL	13
	DLL	14
	DLL	15

; The long copies, entered by a computed jump. Byte runs end at .fbend, long runs at .flend
; (then 0..3 more bytes); both go on at (a5).
.fbyte:	rept	FMAX
	move.b	(a0)+,(a1)+
	endr
.fbend:	jmp	(a5)
.fbytes:				; d3.w bytes, any parity
	add.w	d3,d3
	neg.w	d3
	jmp	.fbend(pc,d3.w)
.flong:	rept	FMAX/4
	move.l	(a0)+,(a1)+
	endr
.flend:	add.w	d4,d4
	neg.w	d4
	jmp	.ftend(pc,d4.w)
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
.ftend:	jmp	(a5)

; Literal nibble 15, (a5) = the match block to go on with.
.llit:	moveq	#0,d3
	move.b	(a0)+,d3		; the first length byte (inside the slot)
	cmp.b	#31,d3
	bhi.s	.llbig
	add.w	#15,d3			; 15..46 bytes: at most 32 beyond the slot, one slot more
	subq.w	#1,d7
	bmi.s	.llchk			; (else fall into .fcopy)

; .fcopy: d3.w bytes (15..FMAX) from (a0)+ to (a1)+, then jmp (a5). Longs when a0 and a1 have
; the same parity. Clobbers d1, d3, d4.
.fcopy:	move.w	a0,d4
	move.w	a1,d1
	eor.w	d1,d4
	lsr.w	#1,d4
	bcs.w	.fbytes			; different parity: bytes
	move.w	a0,d4
	lsr.w	#1,d4
	bcc.s	.fev
	move.b	(a0)+,(a1)+
	subq.w	#1,d3
.fev:	moveq	#3,d4
	and.w	d3,d4			; the tail bytes
	lsr.w	#2,d3			; the longs
	add.w	d3,d3
	neg.w	d3
	jmp	.flend(pc,d3.w)

; A long literal whose first length byte is 32..255.
.llbig:	addq.b	#1,d3
	beq.s	.llback			; 255: the checked way
	moveq	#31,d4			; the run is 14 + d3 bytes: d3 beyond the slot
	add.w	d3,d4
	lsr.w	#5,d4
	add.w	#14,d3
	sub.w	d4,d7
	bpl.s	.fcopy
	bra.s	.llchk
; A first length byte of 255: back to the token, the checked loop takes the sequence.
.llback:
	subq.l	#2,a0
	lea	.recount(pc),a5		; one checked sequence, then the fast path again
	bra.w	.slow
; A long literal the budget cannot pay for: check its room, copy it, and finish the
; sequence the checked way.
.llchk:	move.l	a4,d4
	sub.l	a0,d4
	cmp.l	d3,d4
	bcs.w	.derr			; the literals pass the src end
	move.l	a2,d4
	sub.l	a1,d4
	cmp.l	d3,d4
	bcs.w	.derr			; or the dst end
	lea	.llmid(pc),a5
	bra.s	.fcopy
.llmid:	lea	.recount(pc),a5
	bra.w	.slend


; Block 15's first length byte was 32..255.
.mxbig:	addq.b	#1,d3
	beq.w	.mx255			; 255: more length bytes, the checked way
	add.w	#18,d3			; the match: 51..273 bytes
	moveq	#13,d4			; d3 - 18 beyond the slot
	add.w	d3,d4
	lsr.w	#5,d4
	sub.w	d4,d7
	bpl.w	.mcopy
; Block 15's match, the budget could not pay: check the room, then copy and recount.
.mxroom:
	moveq	#0,d7			; so the copy's DBF falls through to .recount
	move.l	a2,d4
	sub.l	a1,d4
	cmp.l	d3,d4
	bcs.w	.derr
	bra.w	.mcopy

; .mcopy: d3.w bytes (19..273) from (a6)+ to (a1)+, a6 = a1 - d2; then the next sequence.
; Longs when the offset is even and at least 4. Clobbers d1, d3, d4. (Its long run and tail
; come first, so that its computed jump reaches them.)
.mlg:	rept	FMAX/4
	move.l	(a6)+,(a1)+
	endr
.mle:	add.w	d4,d4
	neg.w	d4
	jmp	.mte(pc,d4.w)
	move.b	(a6)+,(a1)+
	move.b	(a6)+,(a1)+
	move.b	(a6)+,(a1)+
.mte:	dbf	d7,.dfast
	bra.w	.recount
.mcopy:	btst	#0,d2
	bne.s	.mcb			; odd: different parity, bytes
	cmp.w	#4,d2
	bcs.s	.mcb			; offset 2: overlapping, bytes
	move.w	a1,d4
	lsr.w	#1,d4
	bcc.s	.mce
	move.b	(a6)+,(a1)+
	subq.w	#1,d3
.mce:	moveq	#3,d4
	and.w	d3,d4			; the tail bytes
	lsr.w	#2,d3			; the longs
	add.w	d3,d3
	neg.w	d3
	jmp	.mle(pc,d3.w)
.mcb:	moveq	#31,d1
	and.w	d3,d1
	lsr.w	#5,d3
	add.w	d1,d1
	neg.w	d1
	jmp	.mbe(pc,d1.w)
.mbg:	rept	32
	move.b	(a6)+,(a1)+
	endr
.mbe:	dbf	d3,.mbg
	dbf	d7,.dfast
	bra.w	.recount

; Block 15's first length byte was 255: the rest of the length, checked.
.mx255:	move.l	#19+255,d3
	bsr.w	.extlen
	move.l	a2,d4
	sub.l	a1,d4
	cmp.l	d3,d4
	bcs.w	.derr
	bsr.w	.copymatch
	bra.w	.recount

; An offset whose low byte is 0, in block M: an error if the high byte is 0 too, else back to
; the block's .zr (M is the token's match nibble).
.zoff:	tst.w	d2
	beq.w	.derr
	moveq	#15,d1
	and.w	d0,d1
	mulu	#DBLK,d1
	lea	.zr0(pc),a6
	jmp	0(a6,d1.w)

; .extlen: d3 += the 255-run at ip (bytes, checked against the src end). Clobbers d4.
.extlen:
	moveq	#0,d4
.ex1:	cmpa.l	a4,a0
	bcc.s	.exerr
	move.b	(a0)+,d4
	add.l	d4,d3
	cmp.b	#255,d4
	beq.s	.ex1
	rts
.exerr:	addq.l	#4,sp			; drop the return address
	bra.w	.derr

; .copylit: d3.l bytes (may be 0) from (a0)+ to (a1)+. Longs when both have the same parity.
; Clobbers d3-d1.
.copylit:
	cmp.l	#16,d3
	bcs.s	.clb
	move.w	a0,d4
	move.w	a1,d1
	eor.w	d1,d4
	lsr.w	#1,d4
	bcs.s	.clb			; different parity: bytes
	move.w	a0,d4
	lsr.w	#1,d4
	bcc.s	.cle
	move.b	(a0)+,(a1)+
	subq.l	#1,d3
.cle:	move.l	d3,d4
	lsr.l	#2,d4			; longs
	and.l	#3,d3			; then 0..3 bytes
	moveq	#15,d1
	and.w	d4,d1
	lsr.l	#4,d4
	add.w	d1,d1
	neg.w	d1
	jmp	.cll(pc,d1.w)
.clg:	rept	16
	move.l	(a0)+,(a1)+
	endr
.cll:	subq.l	#1,d4
	bcc.s	.clg
.clb:	moveq	#63,d1
	and.w	d3,d1
	lsr.l	#6,d3
	add.w	d1,d1
	neg.w	d1
	bra.w	.clj
.clh:	rept	64
	move.b	(a0)+,(a1)+
	endr
.clc:	subq.l	#1,d3
	bcc.w	.clh
	rts
.clj:	jmp	.clc(pc,d1.w)

; .copymatch: d3.l bytes from (a6)+ to (a1)+, a6 = a1 - d2 (d2 >= 1; the areas may overlap).
; Longs when the offset is at least 4 and even. Clobbers d3-d1.
.copymatch:
	cmp.l	#16,d3
	bcs.s	.cmb
	cmp.w	#4,d2
	bcs.s	.cmb
	btst	#0,d2
	bne.s	.cmb
	move.w	a1,d4
	lsr.w	#1,d4
	bcc.s	.cme
	move.b	(a6)+,(a1)+
	subq.l	#1,d3
.cme:	move.l	d3,d4
	lsr.l	#2,d4
	and.l	#3,d3
	move.w	d4,d1
	and.w	#7,d1
	lsr.l	#3,d4
	add.w	d1,d1
	neg.w	d1
	jmp	.cml(pc,d1.w)
.cmg:	rept	8
	move.l	(a6)+,(a1)+
	endr
.cml:	subq.l	#1,d4
	bcc.s	.cmg
.cmb:	move.w	d3,d1
	and.w	#15,d1
	lsr.l	#4,d3
	add.w	d1,d1
	neg.w	d1
	jmp	.cmc(pc,d1.w)
.cmh:	rept	16
	move.b	(a6)+,(a1)+
	endr
.cmc:	subq.l	#1,d3
	bcc.s	.cmh
	rts

; The checked loop: one sequence with every read and write tested, then on at (a5): .recount
; after a long run the fast path could not pay for, else .slow itself (the block's tail: the
; margins only shrink, so once short they stay short; and whole blocks with a low window).
.lowwin:
	lea	.slow(pc),a5
	bra.s	.slow

; The block's tail: one sequence at a time with exact checks. A short one (both nibbles below
; 15, not the last) goes through its fast block with d7 = 0, which comes back here through
; .recount; anything else is one checked sequence.
.tail:	move.l	a4,d3
	sub.l	a0,d3			; input left
	ble.w	.derr			; no token where one must be
	move.b	(a0),d0			; the token (not taken yet)
	moveq	#15,d4
	and.w	d0,d4			; M
	moveq	#0,d1
	move.b	d0,d1
	lsr.w	#4,d1			; L
	cmp.w	#15,d1
	beq.s	.tsl			; long literals: checked
	addq.w	#1,d1
	cmp.l	d1,d3
	beq.s	.tfin			; token and literals are all that is left: the last one
	addq.w	#2,d1			; token, literals, offset
	cmp.w	#15,d4
	beq.s	.tsl
	cmp.l	d1,d3
	bls.s	.tsl			; too short: checked (an error)
	add.w	d4,d1
	addq.w	#1,d1			; L + M + 4 out
	move.l	a2,d3
	sub.l	a1,d3
	cmp.l	d1,d3
	bcs.s	.tsl
	moveq	#0,d7
	bra.w	.dfast
.tfin:	subq.w	#1,d1			; L (0..14)
	move.l	a2,d3
	sub.l	a1,d3
	cmp.l	d1,d3
	bcs.w	.derr			; passes the dst end
	addq.l	#1,a0
	subq.w	#1,d1
	bmi.s	.ddone
.tf1:	move.b	(a0)+,(a1)+
	dbf	d1,.tf1
	bra.s	.ddone
.tsl:	lea	.tail(pc),a5
.slow:	cmpa.l	a4,a0
	bcc.w	.derr			; no token where one must be
	move.b	(a0)+,d0
	moveq	#0,d3
	move.b	d0,d3
	lsr.b	#4,d3
	cmp.b	#15,d3
	bne.s	.sl1
	bsr.w	.extlen
.sl1:	move.l	a4,d4
	sub.l	a0,d4
	cmp.l	d3,d4
	bcs.w	.derr
	move.l	a2,d4
	sub.l	a1,d4
	cmp.l	d3,d4
	bcs.w	.derr
	bsr.w	.copylit
.slend:	cmpa.l	a4,a0
	beq.s	.ddone			; the last sequence ends here
	move.l	a4,d4
	sub.l	a0,d4
	subq.l	#2,d4
	bcs.s	.derr
	move.b	(a0)+,d1
	move.b	(a0)+,-(sp)
	move.w	(sp)+,d2
	move.b	d1,d2
	tst.w	d2
	beq.s	.derr			; offset 0
	move.l	a1,d4
	sub.l	a3,d4
	cmp.l	d2,d4
	bcs.s	.derr			; reaches below the window
	moveq	#15,d3
	and.w	d0,d3
	addq.w	#4,d3
	cmp.w	#15+4,d3
	bne.s	.sl2
	bsr.w	.extlen
.sl2:	move.l	a2,d4
	sub.l	a1,d4
	cmp.l	d3,d4
	bcs.s	.derr
	move.l	a1,a6
	sub.l	d2,a6
	bsr.w	.copymatch
	jmp	(a5)

.ddone:	move.l	a1,d0
	sub.l	(sp)+,d0
	movem.l	(sp)+,d1-d7/a0-a6
	rts
.derr:	addq.l	#4,sp
	moveq	#-1,d0
	movem.l	(sp)+,d1-d7/a0-a6
	rts

;=============================================================================================
; ENCODER
;
; The C model lz4_68k.c is the specification: same arguments, same table, same bytes. Greedy,
; one hash probe per position: the 4 bytes at a position are read as X = b0:b2, Y = b1:b3 with
; two MOVEPs (any address, byte accesses), the table offset is (2X + Y) & mask, the table
; holds 16-bit positions XOR $8000 from the window start, never cleared (a candidate is
; compared before use). Skip strength 6 (the step grows every 64 misses), forward extension
; with CMPM.L while the offset is even, backward extension into the pending literals, exact
; cap check.
;
; Registers: a0 ip, a1 op, a2 table, a3 window + $8000 (so 0(a3,d.w) with a biased position
; is its address), a4 anchor, a5 dst + cap, a6 scratch; d0 ip's position XOR $8000, d5 the
; table mask, d6 the step, d7 the miss count of this step, d1-d4 scratch.
; Stack: 0 dst start, 4 the last position a match may start at (src end - 12), 8 src end,
; 12 the window start.
;=============================================================================================

lz4_encode:
	movem.l	d1-d7/a0-a6,-(sp)
	move.l	a0,a3			; the window is src itself
	bra.s	lz4_enc
lz4_encode_linked:
	movem.l	d1-d7/a0-a6,-(sp)
lz4_enc:
	move.l	a3,-(sp)		; 12: the window start
	lea	0(a0,d0.l),a6
	move.l	a6,-(sp)		; 8: src end
	lea	-12(a6),a6
	move.l	a6,-(sp)		; 4: the last match start
	move.l	a1,-(sp)		; 0: dst start
	cmp.l	#$ffff,d0
	bhi.w	.efail			; n over 65535
	cmpa.l	a0,a3
	bhi.w	.efail			; the window starts after src
	move.l	8(sp),d3
	sub.l	a3,d3
	cmp.l	#$10000,d3
	bhi.w	.efail			; positions from the window would pass 65535
	lea	0(a1,d1.l),a5		; the op limit
	move.l	a0,a4			; anchor
	adda.l	#$8000,a3
	move.w	d2,d5
	subq.w	#1,d5
	add.w	d5,d5			; the mask (byte offsets into the table)
	moveq	#13,d1
	cmp.l	d1,d0
	bcs.w	.elast			; under 13 bytes: literals only
	move.w	a0,d0
	sub.w	a3,d0			; ip's position XOR $8000

; The search: blocks of up to 64 positions at one step. After a match, or at the start:
.esearch:
	moveq	#1,d6
	move.l	4(sp),d7
	sub.l	a0,d7			; bytes to the last match start
	bmi.w	.elast
	moveq	#63,d1
	cmp.l	d1,d7
	bls.s	.e1
	move.l	d1,d7
.e1:	movep.w	0(a0),d3		; X = b0:b2
	movep.w	1(a0),d4		; Y = b1:b3
; Step 1: the next position's X is this one's Y, so one MOVEP per position. Unrolled twice,
; the two halves swapping the roles of d3 and d4; d7 = positions - 1, halved.
	lsr.w	#1,d7
	bcs.s	.eA			; an even number of positions: start with A
	exg	d3,d4			; odd: start with B
	bra.s	.eB
.eA:	move.w	d3,d1			; X = d3, Y = d4
	add.w	d1,d1
	add.w	d4,d1
	and.w	d5,d1			; (2X + Y) & mask
	move.w	0(a2,d1.w),d2		; the candidate
	move.w	d0,0(a2,d1.w)		; this position
	cmp.w	d0,d2
	bge.s	.eAm			; at or past this position: none
	cmp.b	2(a3,d2.w),d3		; b2 first: most candidates fail here
	bne.s	.eAm
	lea	0(a3,d2.w),a6
	movep.w	0(a6),d1
	cmp.w	d3,d1
	bne.s	.eAm
	movep.w	1(a6),d1
	cmp.w	d4,d1
	beq.w	.efound
.eAm:	addq.l	#1,a0
	addq.w	#1,d0
	movep.w	1(a0),d3		; the next Y
.eB:	move.w	d4,d1			; X = d4, Y = d3
	add.w	d1,d1
	add.w	d3,d1
	and.w	d5,d1
	move.w	0(a2,d1.w),d2
	move.w	d0,0(a2,d1.w)
	cmp.w	d0,d2
	bge.s	.eBm
	cmp.b	2(a3,d2.w),d4
	bne.s	.eBm
	lea	0(a3,d2.w),a6
	movep.w	0(a6),d1
	cmp.w	d4,d1
	bne.s	.eBm
	movep.w	1(a6),d1
	cmp.w	d3,d1
	beq.w	.efound
.eBm:	addq.l	#1,a0
	addq.w	#1,d0
	movep.w	1(a0),d4
	dbf	d7,.eA
	moveq	#2,d6			; 64 misses (or the end): step 2
; Steps of 2 and more.
.eblock:
	move.l	4(sp),d7
	sub.l	a0,d7			; bytes to the last match start
	bmi.w	.elast
.estep:	moveq	#0,d1
	move.w	d6,d1
	lsl.l	#6,d1
	cmp.l	d1,d7
	bcs.s	.est1
	moveq	#63,d7			; 64 positions fit
	bra.s	.elp
.est1:	divu	d6,d7			; positions left - 1 (quotient < 64)

.elp:	movep.w	0(a0),d3		; X = b0:b2
	movep.w	1(a0),d4		; Y = b1:b3
	move.w	d3,d1
	add.w	d1,d1
	add.w	d4,d1
	and.w	d5,d1			; (2X + Y) & mask
	move.w	0(a2,d1.w),d2		; the candidate
	move.w	d0,0(a2,d1.w)		; this position
	cmp.w	d0,d2
	bge.s	.emiss			; at or past this position: none
	cmp.b	2(a3,d2.w),d3
	bne.s	.emiss
	lea	0(a3,d2.w),a6
	movep.w	0(a6),d1
	cmp.w	d3,d1
	bne.s	.emiss
	movep.w	1(a6),d1
	cmp.w	d4,d1
	beq.s	.efound
.emiss:	adda.w	d6,a0
	add.w	d6,d0
	dbf	d7,.elp
	addq.w	#1,d6			; 64 misses at this step (or the end): one more
	bra.w	.eblock

; A match: a0 = its start, a6 = the earlier copy, 4 bytes equal.
.efound:
	move.l	a0,d1			; the match start
	move.l	a0,d2
	sub.l	a6,d2			; the offset
	addq.l	#4,a0
	addq.l	#4,a6
	move.l	8(sp),d3
	subq.l	#5,d3
	sub.l	a0,d3			; bytes to compare, up to 5 before the end (>= 3)
	btst	#0,d2
	bne.s	.efb			; odd offset: bytes
	move.w	a0,d4
	lsr.w	#1,d4
	bcc.s	.efl
	cmpm.b	(a6)+,(a0)+
	bne.s	.efm1
	subq.w	#1,d3
.efl:	move.w	d3,d4
	lsr.w	#2,d4
	beq.s	.efb			; under 4 left
	subq.w	#1,d4
.efl1:	cmpm.l	(a6)+,(a0)+
	dbne	d4,.efl1
	beq.s	.efl2
	subq.l	#4,a0			; this long differs: find the byte
	subq.l	#4,a6
.efl3:	cmpm.b	(a6)+,(a0)+
	beq.s	.efl3
	bra.s	.efm1
.efl2:	and.w	#3,d3
.efb:	subq.w	#1,d3
	bcs.s	.efdone
.efb1:	cmpm.b	(a6)+,(a0)+
	dbne	d3,.efb1
	beq.s	.efdone
.efm1:	subq.l	#1,a0			; back to the first differing byte
.efdone:
	; backwards: while start > anchor, start - offset > window, and the bytes before agree
	move.l	d1,a6
	cmpa.l	a4,a6
	bls.s	.ebkd0			; no literals pending: nothing to extend into
	move.l	12(sp),d4
	add.l	d2,d4
	cmp.l	a4,d4
	bcc.s	.ebk0
	move.l	a4,d4			; the bound: max(anchor, window + offset)
.ebk0:	neg.l	d2
.ebk:	cmpa.l	d4,a6
	bls.s	.ebkd
	move.b	-1(a6),d3
	cmp.b	-1(a6,d2.l),d3
	bne.s	.ebkd
	subq.l	#1,a6
	bra.s	.ebk
.ebkd:	neg.l	d2			; the offset again; a6 = the match start
.ebkd0:	move.l	a6,d3
	sub.l	a4,d3			; lit
	move.l	a0,d4
	sub.l	a6,d4
	subq.l	#4,d4			; ml
	; room: 5 + lit + (lit + ml) / 128 is never below the real need
	move.l	d3,d1
	add.l	d4,d1
	lsr.l	#7,d1
	add.l	d3,d1
	addq.l	#5,d1
	move.l	a5,d6
	sub.l	a1,d6
	cmp.l	d6,d1
	bhi.w	.eexact
.eemit:	moveq	#15,d1
	cmp.l	d1,d3
	bcc.s	.elong
	cmp.l	d1,d4
	bcc.s	.elong
	move.b	d3,d1			; both lengths below 15: the token, literals, offset
	lsl.b	#4,d1
	or.b	d4,d1
	move.b	d1,(a1)+
	add.w	d3,d3
	neg.w	d3
	jmp	.esl(pc,d3.w)
	rept	14
	move.b	(a4)+,(a1)+
	endr
.esl:	move.b	d2,(a1)+		; the offset, little-endian
	move.w	d2,-(sp)
	move.b	(sp)+,(a1)+
	move.l	a0,a4			; anchor = the match end
	move.w	a0,d0
	sub.w	a3,d0
	bra.w	.esearch
.elong:	moveq	#15,d1			; the token
	cmp.l	d1,d3
	bcc.s	.et1
	move.w	d3,d1
.et1:	lsl.b	#4,d1
	moveq	#15,d6
	cmp.l	d6,d4
	bcc.s	.et2
	move.w	d4,d6
.et2:	or.b	d6,d1
	move.b	d1,(a1)+
	moveq	#15,d1
	cmp.l	d1,d3
	bcs.s	.et3
	move.l	d3,d1
	sub.l	#15,d1
	bsr.w	.eputlen
.et3:	bsr.w	.ecopy			; the literals, anchor -> op
	move.b	d2,(a1)+		; the offset, little-endian
	move.w	d2,-(sp)
	move.b	(sp)+,(a1)+
	moveq	#15,d1
	cmp.l	d1,d4
	bcs.s	.et4
	move.l	d4,d1
	sub.l	#15,d1
	bsr.w	.eputlen
.et4:	move.l	a0,a4			; anchor = the match end
	move.w	a0,d0
	sub.w	a3,d0
	bra.w	.esearch

; The cheap room estimate failed: the exact need, 3 + lit + ext(lit) + ext(ml).
.eexact:
	move.l	d3,d1
	bsr.s	.eextlen
	move.l	d1,-(sp)
	move.l	d4,d1
	bsr.s	.eextlen
	add.l	(sp)+,d1
	add.l	d3,d1
	addq.l	#3,d1
	cmp.l	d6,d1
	bhi.s	.efail
	bra.w	.eemit

; .eextlen: d1 = the bytes after a nibble of 15 for a length d1: 0 below 15, else
; (d1 - 15) / 255 + 1.
.eextlen:
	moveq	#15,d6
	cmp.l	d6,d1
	bcs.s	.eex0
	sub.l	d6,d1
	divu	#255,d1
	and.l	#$ffff,d1
	addq.l	#1,d1
	move.l	a5,d6			; (d6 = the room again)
	sub.l	a1,d6
	rts
.eex0:	moveq	#0,d1
	move.l	a5,d6
	sub.l	a1,d6
	rts

; The last literals, anchor .. src end.
.elast:	move.l	8(sp),d3
	sub.l	a4,d3			; lit
	move.l	d3,d1
	lsr.l	#7,d1
	add.l	d3,d1
	addq.l	#2,d1			; 2 + lit + lit/128 is never below 1 + lit + ext(lit)
	move.l	a5,d6
	sub.l	a1,d6
	cmp.l	d6,d1
	bls.s	.el1
	move.l	d3,d1
	bsr.s	.eextlen
	add.l	d3,d1
	addq.l	#1,d1
	cmp.l	d6,d1
	bhi.s	.efail
.el1:	moveq	#15,d1
	cmp.l	d1,d3
	bcc.s	.el2
	move.w	d3,d1
.el2:	lsl.b	#4,d1
	move.b	d1,(a1)+
	moveq	#15,d1
	cmp.l	d1,d3
	bcs.s	.el3
	move.l	d3,d1
	sub.l	#15,d1
	bsr.s	.eputlen
.el3:	bsr.s	.ecopy
	move.l	a1,d0
	sub.l	(sp),d0			; the block's size
	lea	16(sp),sp
	movem.l	(sp)+,d1-d7/a0-a6
	rts
.efail:	moveq	#0,d0
	lea	16(sp),sp
	movem.l	(sp)+,d1-d7/a0-a6
	rts

; .eputlen: d1.l >= 0 as an LZ4 length run (255s, then the rest) at (a1)+. Clobbers d1.
.eputlen:
	cmp.l	#255,d1
	bcs.s	.epl1
	st	(a1)+
	sub.l	#255,d1
	bra.s	.eputlen
.epl1:	move.b	d1,(a1)+
	rts

; .ecopy: d3.l bytes (may be 0) from (a4)+ to (a1)+. Longs when both have the same parity.
; Clobbers d1, d3, d6.
.ecopy:	cmp.l	#16,d3
	bcs.s	.ecb
	move.w	a4,d6
	move.w	a1,d1
	eor.w	d1,d6
	lsr.w	#1,d6
	bcs.s	.ecb			; different parity: bytes
	move.w	a4,d6
	lsr.w	#1,d6
	bcc.s	.ece
	move.b	(a4)+,(a1)+
	subq.l	#1,d3
.ece:	move.l	d3,d6
	lsr.l	#2,d6			; longs
	and.l	#3,d3			; then 0..3 bytes
	moveq	#15,d1
	and.w	d6,d1
	lsr.l	#4,d6
	add.w	d1,d1
	neg.w	d1
	jmp	.ecl(pc,d1.w)
.ecg:	rept	16
	move.l	(a4)+,(a1)+
	endr
.ecl:	subq.l	#1,d6
	bcc.s	.ecg
.ecb:	moveq	#63,d1
	and.w	d3,d1
	lsr.l	#6,d3
	add.w	d1,d1
	neg.w	d1
	bra.w	.ecj
.ech:	rept	64
	move.b	(a4)+,(a1)+
	endr
.ecc:	subq.l	#1,d3
	bcc.w	.ech
	rts
.ecj:	jmp	.ecc(pc,d1.w)
