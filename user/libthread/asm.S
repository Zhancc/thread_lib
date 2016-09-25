.globl atomic_inc
atomic_inc:
	movl 4(%esp), %eax
	movl $1, %ecx
	LOCK xaddl %ecx, (%eax)
	movl %ecx, %eax
	ret
