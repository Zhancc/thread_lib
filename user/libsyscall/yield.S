/**
 *  @file yield.S
 *  @brief Wrapper for int yield(int tid)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global yield

yield:
  PUSH  %ebp       
  MOV   %esp,%ebp         /* Setup preamble */
  PUSH  %esi              /* Callee saved register */
  MOV   0x8(%ebp),%esi    /* Move tid into %esi */
  INT   $YIELD_INT        /* Trap into kernel's yield */
  POP   %esi              /* Restore %esi */
  POP   %ebp              /* Restore %ebp */
  RET                     /* Return to caller */
