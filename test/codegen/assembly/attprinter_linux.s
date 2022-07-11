.text
.globl	_start
_start:
	callq	test
.globl	test
test:
	movq	$10, %rax
	movl	$10, %eax
	addl	$20, %edi
	subl	$30, %ecx
	imull	$40, %edx
	pushq	$50
	syscall
	jmp	test2
	retq
