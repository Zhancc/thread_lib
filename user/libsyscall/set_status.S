/**
 *  @file set_status.S
 *  @brief Wrapper for void set_status(int status)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global set_status

set_status:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  MOV   0x8(%ebp),%esi  /* Move status into %esi */
  INT   $SET_STATUS_INT /* Trap into kernel's set_status */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return to caller */

