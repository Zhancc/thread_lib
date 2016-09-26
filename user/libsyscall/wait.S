/**
 *  @file wait.S
 *  @brief Wrapper for int wait(int *status_ptr)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global wait

wait:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  MOV   0x8(%ebp),%esi  /* Move status_ptr into %esi */
  INT   $WAIT_INT       /* Trap into kernel's wait, return value in %eax */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return w/ %eax unchanged */
