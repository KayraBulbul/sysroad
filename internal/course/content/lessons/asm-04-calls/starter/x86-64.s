# Five functions, compiled by clang -O1 for x86-64 (your PC).
#
# Arguments: %rdi, %rsi, %rdx, %rcx, %r8, %r9, then the stack.
# %rbx, %rbp and %r12 to %r15 survive a call. @PLT marks a call to a
# function in another file.

foxtrot:
	movq	%rdi, %rax
	addq	8(%rsp), %rax
	imulq	16(%rsp), %rax
	retq

golf:
	pushq	%rax
	callq	square@PLT
	incq	%rax
	popq	%rcx
	retq

hotel:
	pushq	%r14
	pushq	%rbx
	pushq	%rax
	movq	%rsi, %rbx
	callq	square@PLT
	movq	%rax, %r14
	movq	%rbx, %rdi
	callq	square@PLT
	addq	%r14, %rax
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	retq

india:
	testq	%rdi, %rdi
	jle	.LBB3_1
	pushq	%rbx
	leaq	-1(%rdi), %rax
	movq	%rdi, %rbx
	movq	%rax, %rdi
	callq	india
	movq	%rax, %rdi
	callq	square@PLT
	addq	%rbx, %rax
	popq	%rbx
	retq
.LBB3_1:
	xorl	%eax, %eax
	retq

juliet:
	subq	$40, %rsp
	movq	%rdi, %rsi
	movq	%rsp, %rdi
	callq	fill@PLT
	movq	24(%rsp), %rax
	addq	(%rsp), %rax
	addq	$40, %rsp
	retq
