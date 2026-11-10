; The same five functions, compiled by clang -O2 for arm64 (your Mac).
;
; movk writes 16 bits into the middle of a register and keeps the rest:
; it builds a constant too big for one instruction. umull multiplies two
; 32-bit values into a 64-bit result. The table that uniform reads is at
; the bottom.

_quebec:
	mov	w8, #52429
	movk	w8, #52428, lsl #16
	umull	x8, w0, w8
	lsr	x0, x8, #35
	ret

_romeo:
	lsl	x8, x0, #4
	sub	x0, x8, x0
	ret

_sierra:
	add	w8, w0, #3
	cmp	w0, #0
	csel	w8, w8, w0, mi
	asr	w0, w8, #2
	ret

_tango:
	and	w8, w0, #0xff
	sub	w8, w8, #48
	cmp	w8, #10
	cset	w0, lo
	ret

_uniform:
	sub	w8, w0, #1
	cmp	w8, #11
	b.hi	LBB4_2
	adrp	x9, l_switch.table.uniform@PAGE
	add	x9, x9, l_switch.table.uniform@PAGEOFF
	ldrh	w0, [x9, w8, uxtw #1]
	ret
LBB4_2:
	mov	w0, #-1
	ret

l_switch.table.uniform:
	.short	0
	.short	31
	.short	59
	.short	90
	.short	120
	.short	151
	.short	181
	.short	212
	.short	243
	.short	273
	.short	304
	.short	334
