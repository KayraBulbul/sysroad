; The same five functions, compiled by clang -O1 for arm64 (your Mac).
;
; cmp A, B then b.gt jumps when A > B. Labels that start with LBB are
; jump targets inside a function. wzr is a register that is always 0.

_alpha:
	cmp	x0, x1
	csel	x0, x0, x1, gt
	ret

_bravo:
	mov	w8, #4
	mov	w9, #3
	mov	w10, #2
	cmp	w0, #49
	csel	w10, w10, wzr, gt
	cmp	w0, #74
	csel	w9, w9, w10, gt
	cmp	w0, #89
	csel	w0, w8, w9, gt
	ret

_charlie:
	cbz	x1, LBB2_4
	mov	x8, x0
	mov	w0, #1
LBB2_2:
	mul	x0, x0, x8
	subs	x1, x1, #1
	b.ne	LBB2_2
	ret
LBB2_4:
	mov	w0, #1
	ret

_delta:
	cmp	x0, #1
	b.ne	LBB3_2
	mov	x0, #0
	ret
LBB3_2:
	mov	x8, #0
LBB3_3:
	asr	x9, x0, #1
	add	x10, x0, x0, lsl #1
	tst	x0, #0x1
	csinc	x0, x9, x10, eq
	add	x8, x8, #1
	cmp	x0, #1
	b.ne	LBB3_3
	mov	x0, x8
	ret

_echo:
	cbz	x1, LBB4_3
LBB4_1:
	mov	x8, x1
	udiv	x9, x0, x1
	msub	x1, x9, x1, x0
	mov	x0, x8
	cbnz	x1, LBB4_1
	mov	x0, x8
LBB4_3:
	ret
