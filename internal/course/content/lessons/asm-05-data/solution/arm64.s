; The same six functions, compiled by clang -O1 for arm64 (your Mac).
;
; [x0, #8] is the memory at address x0 + 8. The first argument, a
; pointer, arrives in x0. madd x0, x9, x10, x0 is x0 = x9 * x10 + x0.

_kilo:
	cmp	x1, #1
	b.lt	LBB0_4
	mov	x8, #0
LBB0_2:
	ldr	x9, [x0], #8
	add	x8, x9, x8
	subs	x1, x1, #1
	b.ne	LBB0_2
	mov	x0, x8
	ret
LBB0_4:
	mov	x8, #0
	mov	x0, x8
	ret

_lima:
	add	x8, x0, x1, lsl #2
	ldr	w9, [x8]
	ldr	w8, [x8, #8]
	add	w0, w8, w9
	ret

_mike:
	ldr	x8, [x1]
	ldr	x9, [x0]
	add	x8, x9, x8
	str	x8, [x0]
	ret

_november:
	cmp	x1, #1
	b.lt	LBB3_4
	mov	x8, x0
	mov	x0, #0
	add	x8, x8, #16
LBB3_2:
	ldur	x9, [x8, #-8]
	ldrsw	x10, [x8]
	madd	x0, x9, x10, x0
	add	x8, x8, #24
	subs	x1, x1, #1
	b.ne	LBB3_2
	ret
LBB3_4:
	mov	x0, #0
	ret

_oscar:
	ldr	w8, [x0]
	ldrsh	w9, [x0, #4]
	ldrsb	w10, [x0, #6]
	add	w8, w8, w9
	add	w0, w8, w10
	ret

_papa:
	cbz	x0, LBB5_3
LBB5_1:
	ldr	x8, [x0]
	cmp	x8, x1
	b.eq	LBB5_4
	ldr	x0, [x0, #16]
	cbnz	x0, LBB5_1
LBB5_3:
	mov	x0, #-1
	ret
LBB5_4:
	ldr	x0, [x0, #8]
	ret
