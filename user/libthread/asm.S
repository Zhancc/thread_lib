/**
 * @file asm.S
 * @brief Implementation of assembly utility functions defined in
 *        asm_internals.h and thr_internals.h
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

#include "syscall_int.h"

.globl atomic_inc
.globl xchg
.globl cmpxchg
.globl thread_fork_wrapper
.globl default_exit_entry
.globl get_ebp

atomic_inc:
  movl      0x4(%esp),%eax    /* int *m */
  movl      $1,%ecx
  lock
  xaddl     %ecx,(%eax)       /* *m++ atomically */
  movl      %ecx,%eax         /* Return old value pointed to by m */
  ret

xchg:
  movl      0x4(%esp),%ecx    /* int *source */
  movl      0x8(%esp),%eax    /* int delta */
  xchgl     (%ecx),%eax       /* Swap values and return old content */
  ret

cmpxchg:
  movl      0x4(%esp),%ecx    /* int *source */
  movl      0x8(%esp),%eax    /* int test */
  movl      0xc(%esp),%edx    /* int set */
  lock
  cmpxchgl  %edx,(%ecx)       /* %eax is an implicit operand */
  jz        .Equal
  movl      $0,%eax
  ret
.Equal:
  movl     $1,%eax
  ret 

/* Please refer to thread.c for play by play stack diagram. */
thread_fork_wrapper:
  movl      0x4(%esp),%ecx    /* void *esp */
  movl      0x8(%esp),%edx    /* tcb_t *tcb */
  int       $THREAD_FORK_INT  /* Trap into thread_fork */
  cmpl      $0,%eax           /* The return value determines if we are in 
                                 invoking thread(new thread's tid) or the new 
                                 thread(zero). */
  jnz       .Return           /* Invoking thread returns immediately */
  movl      %edx,%ebp         /* Frame pointer at start of TCB */
  movl      %ecx,%esp         /* Replace invoking thread's %esp w/ esp */
  call      peer_thread_init
  addl      $4,%esp           /* Reclaims stack, %esp points to func */
.Return:  
  ret                         /* This is equivalent to 
                                    pop %eip.
                                 In invoking thread, new thread's tid is 
                                 returned unchanged. In new thread, its %eip is 
                                 overwritten by the address of first 
                                 instruction of peer_thread_init, stored at
                                 %esp. */

default_exit_entry:
	pushl     %eax                /* Return value of thread's body func */
	pushl     $default_exit_entry /* Placeholder for return address. */
	jmp       default_exit
1:                              /* Boy you are in trouble if you get here */ 
	jmp 1b

get_ebp:
	movl     %ebp,%eax          /* Store calling function's %ebp */
	ret 
