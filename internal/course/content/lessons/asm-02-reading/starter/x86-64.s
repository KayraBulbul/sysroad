# What `clang++ -std=c++20 -O1 -S puzzle.cpp` printed on a PC. You do not have puzzle.cpp.
	.att_syntax
	.file	"puzzle.cpp"
	.text
	.globl	_ZN3geo4areaEii                 # -- Begin function _ZN3geo4areaEii
	.prefalign	4, .Lfunc_end0, nop
	.type	_ZN3geo4areaEii,@function
_ZN3geo4areaEii:                        # @_ZN3geo4areaEii
	.cfi_startproc
# %bb.0:
	movslq	%edi, %rcx
	movslq	%esi, %rax
	imulq	%rcx, %rax
	retq
.Lfunc_end0:
	.size	_ZN3geo4areaEii, .Lfunc_end0-_ZN3geo4areaEii
	.cfi_endproc
                                        # -- End function
	.globl	_Z6bannerv                      # -- Begin function _Z6bannerv
	.prefalign	4, .Lfunc_end1, nop
	.type	_Z6bannerv,@function
_Z6bannerv:                             # @_Z6bannerv
	.cfi_startproc
# %bb.0:
	leaq	.L.str(%rip), %rax
	retq
.Lfunc_end1:
	.size	_Z6bannerv, .Lfunc_end1-_Z6bannerv
	.cfi_endproc
                                        # -- End function
	.globl	_Z6answerv                      # -- Begin function _Z6answerv
	.prefalign	4, .Lfunc_end2, nop
	.type	_Z6answerv,@function
_Z6answerv:                             # @_Z6answerv
	.cfi_startproc
# %bb.0:
	movl	$650, %eax                      # imm = 0x28A
	retq
.Lfunc_end2:
	.size	_Z6answerv, .Lfunc_end2-_Z6answerv
	.cfi_endproc
                                        # -- End function
	.globl	_Z4bumpv                        # -- Begin function _Z4bumpv
	.prefalign	4, .Lfunc_end3, nop
	.type	_Z4bumpv,@function
_Z4bumpv:                               # @_Z4bumpv
	.cfi_startproc
# %bb.0:
	movq	counter(%rip), %rax
	leaq	1(%rax), %rcx
	movq	%rcx, counter(%rip)
	addq	$3, %rax
	retq
.Lfunc_end3:
	.size	_Z4bumpv, .Lfunc_end3-_Z4bumpv
	.cfi_endproc
                                        # -- End function
	.globl	_Z5greetv                       # -- Begin function _Z5greetv
	.prefalign	4, .Lfunc_end4, nop
	.type	_Z5greetv,@function
_Z5greetv:                              # @_Z5greetv
	.cfi_startproc
# %bb.0:
	leaq	.L.str.1(%rip), %rdi
	jmp	puts@PLT                        # TAILCALL
.Lfunc_end4:
	.size	_Z5greetv, .Lfunc_end4-_Z5greetv
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"sysroad 4.2"
	.size	.L.str, 12

	.type	counter,@object                 # @counter
	.data
	.globl	counter
	.p2align	3, 0x0
counter:
	.quad	40                              # 0x28
	.size	counter, 8

	.type	.L.str.1,@object                # @.str.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.1:
	.asciz	"hello from assembly"
	.size	.L.str.1, 20

	.ident	"clang version 23.1.3"
	.section	".note.GNU-stack","",@progbits
	.addrsig
