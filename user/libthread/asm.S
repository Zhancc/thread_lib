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
    MOV   0x4(%esp),%eax    /* int *m is stored at 0x4(%esp)*/
    MOV   $1,%ecx           /* We are using XADD source,destination.
                               source has to be a register, destination can be
                               either register or memory location */
    LOCK                    /* Atomic exchange and add */
    XADD  %ecx,(%eax)       /* m++ atomically */
    MOV   %ecx,%eax         /* Return m's old value. */
    RET

thread_fork_wrapper:
    MOV   0x4(%esp),%ecx    /* %ecx is caller saved. We can use it as temp 
                               storage. void *peer_thread_esp is stored 
                               at 0x4(%esp) */
    INT   $THREAD_FORK_INT  /* Trap into Pebble's thread_fork */
    CMP   $0,%eax           /* The return value determines if we are in invoking
                               thread(none zero, new thread's tid) or the 
                               new thread(zero) */
    JNZ   .Return           /* Invoking thread skips next instruction and 
                               returns immediately */
    MOV  %ecx, %esp              /* Push tcb_ptr so new thread can invoke
                               peer_thread_init */
    CALL  peer_thread_init
    ADD   $4,%esp           /* Reclaims stack */
.Return:
    RET                     /* This is equivalent to POP %eip.
                               In invoking thread, new thread's tid is returned 
                               unchanged. In new thread, its %eip is overwritten
                               by the value, i.e. address of first instruction of 
                               peer_thread_init, stored at %esp. */
xchg:
    MOV   0x4(%esp),%ecx    /* source */
    MOV   0x8(%esp),%eax    /* delta */
    XCHG  (%ecx),%eax       /* Return the source value and swap in the delta */
    RET
