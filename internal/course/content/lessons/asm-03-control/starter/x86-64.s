# Five functions, compiled by clang -O1 for x86-64 (your PC).
#
# cmpq B, A then jg jumps when A > B. Labels that start with .LBB are
# jump targets inside a function.

alpha:
	movq	%rsi, %rax
	cmpq	%rsi, %rdi
	cmovgq	%rdi, %rax
	retq

bravo:
	movl	$4, %eax
	cmpl	$89, %edi
	jg	.LBB1_3
	movl	$3, %eax
	cmpl	$74, %edi
	jg	.LBB1_3
	xorl	%eax, %eax
	cmpl	$50, %edi
	setge	%al
	addl	%eax, %eax
.LBB1_3:
	retq

charlie:
	movl	$1, %eax
	testq	%rsi, %rsi
	je	.LBB2_2
.LBB2_1:
	imulq	%rdi, %rax
	decq	%rsi
	jne	.LBB2_1
.LBB2_2:
	retq

delta:
	xorl	%eax, %eax
	cmpq	$1, %rdi
	je	.LBB3_3
.LBB3_1:
	movl	%edi, %ecx
	leaq	(%rdi,%rdi,2), %rdx
	incq	%rdx
	sarq	%rdi
	testb	$1, %cl
	cmovneq	%rdx, %rdi
	incq	%rax
	cmpq	$1, %rdi
	jne	.LBB3_1
.LBB3_3:
	retq

echo:
	movq	%rdi, %rax
	testq	%rsi, %rsi
	je	.LBB4_4
	movq	%rsi, %rdx
.LBB4_2:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.LBB4_2
	movq	%rcx, %rax
.LBB4_4:
	retq
