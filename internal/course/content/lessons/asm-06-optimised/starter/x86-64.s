# Five functions, compiled by clang -O2 for x86-64 (your PC).
#
# %dil is the low byte of %rdi, and %al the low byte of %rax. The table
# that uniform reads is at the bottom.

quebec:
	movl	%edi, %ecx
	movl	$3435973837, %eax
	imulq	%rcx, %rax
	shrq	$35, %rax
	retq

romeo:
	leaq	(%rdi,%rdi,4), %rax
	leaq	(%rax,%rax,2), %rax
	retq

sierra:
	leal	3(%rdi), %eax
	testl	%edi, %edi
	cmovnsl	%edi, %eax
	sarl	$2, %eax
	retq

tango:
	addb	$-48, %dil
	cmpb	$10, %dil
	setb	%al
	retq

uniform:
	decl	%edi
	movl	$-1, %eax
	cmpl	$11, %edi
	ja	.LBB4_2
	movl	%edi, %eax
	leaq	.Lswitch.table.uniform(%rip), %rcx
	movzwl	(%rcx,%rax,2), %eax
.LBB4_2:
	retq
.Lswitch.table.uniform:
	.short	0
	.short	31
	.short	59
	.short	90
	.short	120
	.short	151
	.short	181
	.short	212
	.short	243
	.short	273
	.short	304
	.short	334
