#include <syscall_int.h>
.globl atomic_inc
.globl thread_fork_wrapper
.globl xchg
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
	call child_init
	addl $4, %esp
1:	ret

xchg:
	movl 4(%esp), %ecx
	movl 8(%esp), %eax
	xchgl (%ecx), %eax
	ret
