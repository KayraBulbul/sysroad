# Six functions, compiled by clang -O1 for x86-64 (your PC).
#
# D(B,I,S) is the memory at address B + I*S + D. The first argument, a
# pointer, arrives in %rdi.

kilo:
	xorl	%eax, %eax
	testq	%rsi, %rsi
	jle	.LBB0_3
	xorl	%ecx, %ecx
.LBB0_2:
	addq	(%rdi,%rcx,8), %rax
	incq	%rcx
	cmpq	%rcx, %rsi
	jne	.LBB0_2
.LBB0_3:
	retq

lima:
	movl	8(%rdi,%rsi,4), %eax
	addl	(%rdi,%rsi,4), %eax
	retq

mike:
	movq	(%rsi), %rax
	addq	%rax, (%rdi)
	retq

november:
	testq	%rsi, %rsi
	jle	.LBB3_1
	addq	$16, %rdi
	xorl	%eax, %eax
.LBB3_4:
	movslq	(%rdi), %rcx
	imulq	-8(%rdi), %rcx
	addq	%rcx, %rax
	addq	$24, %rdi
	decq	%rsi
	jne	.LBB3_4
	retq
.LBB3_1:
	xorl	%eax, %eax
	retq

oscar:
	movswl	4(%rdi), %ecx
	addl	(%rdi), %ecx
	movsbl	6(%rdi), %eax
	addl	%ecx, %eax
	retq

papa:
	movq	$-1, %rax
	testq	%rdi, %rdi
	je	.LBB5_4
.LBB5_2:
	cmpq	%rsi, (%rdi)
	je	.LBB5_3
	movq	16(%rdi), %rdi
	testq	%rdi, %rdi
	jne	.LBB5_2
.LBB5_4:
	retq
.LBB5_3:
	movq	8(%rdi), %rax
	retq
