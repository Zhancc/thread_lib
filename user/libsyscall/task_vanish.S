/**
 *  @file task_vanish.S
 *  @brief Wrapper for void task_vanish(int status)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global task_vanish

task_vanish:
  PUSH  %ebp       
  MOV   %esp,%ebp         /* Setup preamble */
  PUSH  %esi              /* Callee saved register */
  MOV   0x8(%ebp),%esi    /* Move status into %esi */
  INT   $TASK_VANISH_INT  /* Trap into kernel's set_status */
  POP   %esi              /* Restore %esi */
  POP   %ebp              /* Restore %ebp */
  RET                     /* Return to caller */
