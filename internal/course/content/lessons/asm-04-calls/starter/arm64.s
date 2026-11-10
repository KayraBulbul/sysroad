; The same five functions, compiled by clang -O1 for arm64 (your Mac).
;
; Arguments: x0 to x7. x19 to x28 survive a call. x30 holds the return
; address and x29 the frame pointer, and both are saved around calls.

_foxtrot:
	add	x8, x6, x0
	mul	x0, x8, x7
	ret

_golf:
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
	bl	_square
	add	x0, x0, #1
	ldp	x29, x30, [sp], #16
	ret

_hotel:
	stp	x20, x19, [sp, #-32]!
	stp	x29, x30, [sp, #16]
	add	x29, sp, #16
	mov	x19, x1
	bl	_square
	mov	x20, x0
	mov	x0, x19
	bl	_square
	add	x0, x0, x20
	ldp	x29, x30, [sp, #16]
	ldp	x20, x19, [sp], #32
	ret

_india:
	cmp	x0, #0
	b.le	LBB3_2
	stp	x20, x19, [sp, #-32]!
	stp	x29, x30, [sp, #16]
	add	x29, sp, #16
	mov	x19, x0
	sub	x0, x0, #1
	bl	_india
	bl	_square
	add	x0, x0, x19
	ldp	x29, x30, [sp, #16]
	ldp	x20, x19, [sp], #32
	ret
LBB3_2:
	mov	x0, #0
	ret

_juliet:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]
	add	x29, sp, #32
	mov	x1, x0
	mov	x0, sp
	bl	_fill
	ldr	x8, [sp]
	ldr	x9, [sp, #24]
	add	x0, x9, x8
	ldp	x29, x30, [sp, #32]
	add	sp, sp, #48
	ret
