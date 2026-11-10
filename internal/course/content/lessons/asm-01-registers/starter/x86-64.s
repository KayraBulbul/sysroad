# Six functions, compiled by clang -O1 for x86-64 (your PC).
#
# AT&T syntax: the source comes first and the destination last.
# Arguments arrive in %rdi, %rsi, %rdx, in that order. The result is
# whatever is in %rax at the retq.

first:
	leaq	(%rdi,%rsi), %rax
	retq

second:
	movq	%rdi, %rax
	addq	%rsi, %rsi
	subq	%rsi, %rax
	retq

third:
	leaq	(%rdi,%rdi,4), %rax
	addq	$7, %rax
	retq

fourth:
	movq	%rdi, %rax
	subq	%rsi, %rax
	imulq	%rdx, %rax
	retq

fifth:
	leal	(%rdi,%rdi,2), %eax
	subl	%esi, %eax
	addl	$100, %eax
	retq

sixth:
	shlq	$4, %rdi
	sarq	$2, %rsi
	leaq	(%rsi,%rdi), %rax
	retq
