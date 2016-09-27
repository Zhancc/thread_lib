#include <syscall_int.h>
.globl atomic_inc
atomic_inc:
	movl 4(%esp), %eax
	movl $1, %ecx
	LOCK xaddl %ecx, (%eax)
	movl %ecx, %eax
	ret

thread_fork_wrapper:
	movl 4(%esp), %ecx
	int $THREAD_FORK_INT
	cmpl $0, %eax
	jnz 1f
	movl %ecx, %esp
1:	ret
