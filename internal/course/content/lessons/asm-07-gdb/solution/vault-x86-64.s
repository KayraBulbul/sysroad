	.att_syntax
	.file	"vault.cpp"
	.text
	.globl	phase_1                         # -- Begin function phase_1
	.p2align	4
	.type	phase_1,@function
phase_1:                                # @phase_1
# %bb.0:
	subq	$24, %rsp
	movzbl	sealed(%rip), %ecx
	xorl	%eax, %eax
	testb	%cl, %cl
	je	.LBB0_3
# %bb.1:
	leaq	sealed(%rip), %rdx
	.p2align	4
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	xorb	%sil, %cl
	movb	%cl, (%rsp,%rax)
	movzbl	1(%rax,%rdx), %ecx
	incq	%rax
	testb	%cl, %cl
	jne	.LBB0_2
.LBB0_3:
	movb	$0, (%rsp,%rax)
	movq	%rsp, %rsi
	callq	strcmp@PLT
	xorl	%ecx, %ecx
	testl	%eax, %eax
	sete	%cl
	movl	%ecx, %eax
	addq	$24, %rsp
	retq
.Lfunc_end0:
	.size	phase_1, .Lfunc_end0-phase_1
                                        # -- End function
	.globl	forge                           # -- Begin function forge
	.p2align	4
	.type	forge,@function
forge:                                  # @forge
# %bb.0:
	movq	$-6, %rcx
	movl	$7, %edx
	.p2align	4
.LBB1_1:                                # =>This Inner Loop Header: Depth=1
	movq	%rdi, %rax
	shlq	$5, %rax
	subq	%rdi, %rax
	leaq	(%rdx,%rax), %rdi
	decq	%rcx
	incq	%rdx
	cmpq	$-12, %rcx
	jne	.LBB1_1
# %bb.2:
	subq	%rcx, %rax
	retq
.Lfunc_end1:
	.size	forge, .Lfunc_end1-forge
                                        # -- End function
	.globl	phase_2                         # -- Begin function phase_2
	.p2align	4
	.type	phase_2,@function
phase_2:                                # @phase_2
# %bb.0:
	pushq	%rbx
	movq	%rdi, %rbx
	movq	%rsi, %rdi
	callq	forge
	xorl	%ecx, %ecx
	cmpq	%rbx, %rax
	sete	%cl
	movl	%ecx, %eax
	popq	%rbx
	retq
.Lfunc_end2:
	.size	phase_2, .Lfunc_end2-phase_2
                                        # -- End function
	.globl	phase_3                         # -- Begin function phase_3
	.p2align	4
	.type	phase_3,@function
phase_3:                                # @phase_3
# %bb.0:
	xorl	%eax, %eax
	cmpq	$7, %rdi
	ja	.LBB3_4
# %bb.1:
	movl	$3, %eax
	leaq	table(%rip), %rcx
	.p2align	4
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
	movq	(%rcx,%rdi,8), %rdi
	decl	%eax
	jne	.LBB3_2
# %bb.3:
	xorl	%eax, %eax
	cmpq	$5, %rdi
	sete	%al
.LBB3_4:
	retq
.Lfunc_end3:
	.size	phase_3, .Lfunc_end3-phase_3
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4
	.type	main,@function
main:                                   # @main
# %bb.0:
	pushq	%r14
	pushq	%rbx
	pushq	%rax
	cmpl	$4, %edi
	jne	.LBB4_1
# %bb.2:
	movq	%rsi, %r14
	movq	8(%rsi), %rdi
	movl	$90, %esi
	callq	phase_1
	movl	$1, %ebx
	testl	%eax, %eax
	je	.LBB4_3
# %bb.4:
	leaq	.L.str.2(%rip), %rdi
	callq	puts@PLT
	movq	16(%r14), %rdi
	xorl	%esi, %esi
	movl	$10, %edx
	callq	strtol@PLT
	movl	$4, %esi
	movq	%rax, %rdi
	callq	phase_2
	testl	%eax, %eax
	je	.LBB4_5
# %bb.6:
	leaq	.L.str.4(%rip), %rdi
	callq	puts@PLT
	movq	24(%r14), %rdi
	xorl	%esi, %esi
	movl	$10, %edx
	callq	strtol@PLT
	movq	%rax, %rdi
	callq	phase_3
	testl	%eax, %eax
	je	.LBB4_7
# %bb.8:
	leaq	.L.str.6(%rip), %rdi
	callq	puts@PLT
	leaq	.L.str.7(%rip), %rdi
	xorl	%ebx, %ebx
	jmp	.LBB4_9
.LBB4_1:
	movl	$2, %ebx
	leaq	.L.str(%rip), %rdi
	jmp	.LBB4_9
.LBB4_3:
	leaq	.L.str.1(%rip), %rdi
	jmp	.LBB4_9
.LBB4_5:
	leaq	.L.str.3(%rip), %rdi
	jmp	.LBB4_9
.LBB4_7:
	leaq	.L.str.5(%rip), %rdi
.LBB4_9:
	callq	puts@PLT
	movl	%ebx, %eax
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	retq
.Lfunc_end4:
	.size	main, .Lfunc_end4-main
                                        # -- End function
	.type	sealed,@object                  # @sealed
	.data
	.globl	sealed
sealed:
	.asciz	"43=2.0;("
	.size	sealed, 9

	.type	table,@object                   # @table
	.globl	table
	.p2align	4, 0x0
table:
	.quad	3                               # 0x3
	.quad	6                               # 0x6
	.quad	0                               # 0x0
	.quad	7                               # 0x7
	.quad	1                               # 0x1
	.quad	2                               # 0x2
	.quad	5                               # 0x5
	.quad	4                               # 0x4
	.size	table, 64

	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"usage: vault WORD NUMBER INDEX"
	.size	.L.str, 31

	.type	.L.str.1,@object                # @.str.1
.L.str.1:
	.asciz	"phase 1: locked"
	.size	.L.str.1, 16

	.type	.L.str.2,@object                # @.str.2
.L.str.2:
	.asciz	"phase 1: open"
	.size	.L.str.2, 14

	.type	.L.str.3,@object                # @.str.3
.L.str.3:
	.asciz	"phase 2: locked"
	.size	.L.str.3, 16

	.type	.L.str.4,@object                # @.str.4
.L.str.4:
	.asciz	"phase 2: open"
	.size	.L.str.4, 14

	.type	.L.str.5,@object                # @.str.5
.L.str.5:
	.asciz	"phase 3: locked"
	.size	.L.str.5, 16

	.type	.L.str.6,@object                # @.str.6
.L.str.6:
	.asciz	"phase 3: open"
	.size	.L.str.6, 14

	.type	.L.str.7,@object                # @.str.7
.L.str.7:
	.asciz	"The vault is open."
	.size	.L.str.7, 19

	.section	".note.GNU-stack","",@progbits
