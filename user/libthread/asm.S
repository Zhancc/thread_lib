/**
 * @file asm.S
 * @brief Assembly utility functions.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

#include <syscall_int.h>

.globl atomic_inc
.globl thread_fork_wrapper
.globl xchg

atomic_inc:
  PUSH  %ebp
  MOV   %esp,%ebp         /* Setup preamble */
  MOV   $1,%ecx           /* We are using XADD source,destination.
                             source has to be a register, destination can be
                             either register or memory location */
  LOCK                    /* Atomic exchange and add */
  XADD  %ecx,0x8(%ebp)    /* %ecx assumes the old value. 
                             Old value increases by %eax, i.e. 1 */
  MOV   %ecx,%eax         /* Return old value. */
  POP   %ebp              /* Restore %ebp */
  RET

thread_fork_wrapper:
  PUSH  %ebp
  MOV   %esp,%ebp         /* Setup preamble */
  MOV   0x4(%ebp),%ecx    /* %ecx is caller saved. We can use it as temp 
                             storage */
  INT   $THREAD_FORK_INT  /* Trap into Pebble's thread_fork */
  CMP   $0,%eax           /* The return value determines if we are in invoking
                             thread(none zero, new thread's tid) or the 
                             new thread(zero) */
  JNZ   .Return           /* Invoking thread skips next instruction and 
                             returns immediately */
  MOV   %ecx,%esp         /* New thread has its %esp replaced */
.Return:
  POP   %ebp              /* Restore %ebp */
  RET                     /* This is equivalent to POP %eip.
                             In invoking thread, new thread's tid is returned 
                             unchanged. In new thread, its %eip is overwritten
                             by the value, i.e. address of first instruction of 
                             peer_thread_init, stored at %esp. */
xchg:
  PUSH  %ebp
  MOV   %esp,%ebp         /* Setup preamble */
  MOV   0x4(%ebp),%ecx    /* source */
  MOV   0x8(%ebp),%eax    /* delta */
  XCHG  (%ecx),%eax       /* Return the source value and swap in the delta */
  POP   %ebp              /* Restore %ebp */
  RET
