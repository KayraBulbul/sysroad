	.build_version macos, 14, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_phase_1                        ; -- Begin function phase_1
	.p2align	2
_phase_1:                               ; @phase_1
; %bb.0:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	adrp	x8, _sealed@PAGE
	ldrb	w10, [x8, _sealed@PAGEOFF]
	cbz	w10, LBB0_3
; %bb.1:
	mov	x8, #0                          ; =0x0
	and	w9, w1, #0xff
	mov	x11, sp
Lloh0:
	adrp	x12, _sealed@PAGE
Lloh1:
	add	x12, x12, _sealed@PAGEOFF
LBB0_2:                                 ; =>This Inner Loop Header: Depth=1
	eor	w10, w10, w9
	strb	w10, [x11, x8]
	add	x10, x12, x8
	add	x8, x8, #1
	ldrb	w10, [x10, #1]
	cbnz	w10, LBB0_2
	b	LBB0_4
LBB0_3:
	mov	x8, #0                          ; =0x0
LBB0_4:
	mov	x9, sp
	strb	wzr, [x9, x8]
	mov	x1, sp
	bl	_strcmp
	cmp	w0, #0
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.loh AdrpAdd	Lloh0, Lloh1
                                        ; -- End function
	.globl	_forge                          ; -- Begin function forge
	.p2align	2
_forge:                                 ; @forge
; %bb.0:
	mov	w8, #7                          ; =0x7
LBB1_1:                                 ; =>This Inner Loop Header: Depth=1
	mov	x9, x0
	sub	x10, x8, x0
	add	x0, x10, x0, lsl #5
	add	x8, x8, #1
	cmp	x8, #13
	b.ne	LBB1_1
; %bb.2:
	sub	x8, x8, x9
	add	x8, x8, x9, lsl #5
	sub	x0, x8, #1
	ret
                                        ; -- End function
	.globl	_phase_2                        ; -- Begin function phase_2
	.p2align	2
_phase_2:                               ; @phase_2
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	mov	x19, x0
	mov	x0, x1
	bl	_forge
	cmp	x0, x19
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
                                        ; -- End function
	.globl	_phase_3                        ; -- Begin function phase_3
	.p2align	2
_phase_3:                               ; @phase_3
; %bb.0:
	cmp	x0, #7
	b.ls	LBB3_2
; %bb.1:
	mov	w0, #0                          ; =0x0
	ret
LBB3_2:
	mov	w8, #3                          ; =0x3
Lloh2:
	adrp	x9, _table@PAGE
Lloh3:
	add	x9, x9, _table@PAGEOFF
LBB3_3:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x0, [x9, x0, lsl #3]
	subs	w8, w8, #1
	b.ne	LBB3_3
; %bb.4:
	cmp	x0, #5
	cset	w0, eq
	ret
	.loh AdrpAdd	Lloh2, Lloh3
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	cmp	w0, #4
	b.ne	LBB4_5
; %bb.1:
	mov	x19, x1
	ldr	x0, [x1, #8]
	mov	w1, #90                         ; =0x5a
	bl	_phase_1
	cbz	w0, LBB4_6
; %bb.2:
Lloh4:
	adrp	x0, l_.str.2@PAGE
Lloh5:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_puts
	ldr	x0, [x19, #16]
	mov	x1, #0                          ; =0x0
	mov	w2, #10                         ; =0xa
	bl	_strtol
	mov	w1, #4                          ; =0x4
	bl	_phase_2
	cbz	w0, LBB4_7
; %bb.3:
Lloh6:
	adrp	x0, l_.str.4@PAGE
Lloh7:
	add	x0, x0, l_.str.4@PAGEOFF
	bl	_puts
	ldr	x0, [x19, #24]
	mov	x1, #0                          ; =0x0
	mov	w2, #10                         ; =0xa
	bl	_strtol
	bl	_phase_3
	cbz	w0, LBB4_8
; %bb.4:
Lloh8:
	adrp	x0, l_.str.6@PAGE
Lloh9:
	add	x0, x0, l_.str.6@PAGEOFF
	bl	_puts
	mov	w19, #0                         ; =0x0
Lloh10:
	adrp	x0, l_.str.7@PAGE
Lloh11:
	add	x0, x0, l_.str.7@PAGEOFF
	b	LBB4_9
LBB4_5:
	mov	w19, #2                         ; =0x2
Lloh12:
	adrp	x0, l_.str@PAGE
Lloh13:
	add	x0, x0, l_.str@PAGEOFF
	b	LBB4_9
LBB4_6:
	mov	w19, #1                         ; =0x1
Lloh14:
	adrp	x0, l_.str.1@PAGE
Lloh15:
	add	x0, x0, l_.str.1@PAGEOFF
	b	LBB4_9
LBB4_7:
	mov	w19, #1                         ; =0x1
Lloh16:
	adrp	x0, l_.str.3@PAGE
Lloh17:
	add	x0, x0, l_.str.3@PAGEOFF
	b	LBB4_9
LBB4_8:
	mov	w19, #1                         ; =0x1
Lloh18:
	adrp	x0, l_.str.5@PAGE
Lloh19:
	add	x0, x0, l_.str.5@PAGEOFF
LBB4_9:
	bl	_puts
	mov	x0, x19
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh6, Lloh7
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh12, Lloh13
	.loh AdrpAdd	Lloh14, Lloh15
	.loh AdrpAdd	Lloh16, Lloh17
	.loh AdrpAdd	Lloh18, Lloh19
                                        ; -- End function
	.section	__DATA,__data
	.globl	_sealed                         ; @sealed
_sealed:
	.asciz	"43=2.0;("

	.globl	_table                          ; @table
	.p2align	3, 0x0
_table:
	.quad	3                               ; 0x3
	.quad	6                               ; 0x6
	.quad	0                               ; 0x0
	.quad	7                               ; 0x7
	.quad	1                               ; 0x1
	.quad	2                               ; 0x2
	.quad	5                               ; 0x5
	.quad	4                               ; 0x4

	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"usage: vault WORD NUMBER INDEX"

l_.str.1:                               ; @.str.1
	.asciz	"phase 1: locked"

l_.str.2:                               ; @.str.2
	.asciz	"phase 1: open"

l_.str.3:                               ; @.str.3
	.asciz	"phase 2: locked"

l_.str.4:                               ; @.str.4
	.asciz	"phase 2: open"

l_.str.5:                               ; @.str.5
	.asciz	"phase 3: locked"

l_.str.6:                               ; @.str.6
	.asciz	"phase 3: open"

l_.str.7:                               ; @.str.7
	.asciz	"The vault is open."

.subsections_via_symbols
