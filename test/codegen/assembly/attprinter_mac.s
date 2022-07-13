.section	__TEXT,__text
.globl	start
start:	callq	_test
.globl	_test
_test:	movq	$10,	%rax
	movl	$10,	%eax
	addl	$20,	%edi
	subl	$30,	%ecx
	imull	$40,	%edx
	pushq	$50
	syscall
	jmp	_test2
	retq
