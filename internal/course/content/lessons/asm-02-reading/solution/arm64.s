; What `clang++ -std=c++20 -O1 -S puzzle.cpp` printed on a Mac. You do not have puzzle.cpp.
	.build_version macos, 14, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	__ZN3geo4areaEii                ; -- Begin function _ZN3geo4areaEii
	.p2align	2
__ZN3geo4areaEii:                       ; @_ZN3geo4areaEii
	.cfi_startproc
; %bb.0:
	smull	x0, w1, w0
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	__Z6bannerv                     ; -- Begin function _Z6bannerv
	.p2align	2
__Z6bannerv:                            ; @_Z6bannerv
	.cfi_startproc
; %bb.0:
Lloh0:
	adrp	x0, l_.str@PAGE
Lloh1:
	add	x0, x0, l_.str@PAGEOFF
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	__Z6answerv                     ; -- Begin function _Z6answerv
	.p2align	2
__Z6answerv:                            ; @_Z6answerv
	.cfi_startproc
; %bb.0:
	mov	w0, #650                        ; =0x28a
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	__Z4bumpv                       ; -- Begin function _Z4bumpv
	.p2align	2
__Z4bumpv:                              ; @_Z4bumpv
	.cfi_startproc
; %bb.0:
	adrp	x8, _counter@PAGE
	ldr	x9, [x8, _counter@PAGEOFF]
	add	x10, x9, #1
	str	x10, [x8, _counter@PAGEOFF]
	add	x0, x9, #3
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	__Z5greetv                      ; -- Begin function _Z5greetv
	.p2align	2
__Z5greetv:                             ; @_Z5greetv
	.cfi_startproc
; %bb.0:
Lloh2:
	adrp	x0, l_.str.1@PAGE
Lloh3:
	add	x0, x0, l_.str.1@PAGEOFF
	b	_puts
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"sysroad 4.2"

	.section	__DATA,__data
	.globl	_counter                        ; @counter
	.p2align	3, 0x0
_counter:
	.quad	40                              ; 0x28

	.section	__TEXT,__cstring,cstring_literals
l_.str.1:                               ; @.str.1
	.asciz	"hello from assembly"

.subsections_via_symbols
