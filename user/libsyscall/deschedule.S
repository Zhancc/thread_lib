/**
 *  @file deschedule.S
 *  @brief Wrapper for int deschedule(int *reject)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global deschedule

deschedule:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  MOV   0x8(%ebp),%esi  /* Move reject into %esi */
  INT   $DESCHEDULE_INT /* Trap into kernel's deschedule */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return %eax unchanged */
