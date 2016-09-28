/**
 *  @file sleep.S
 *  @brief Wrapper for int sleep(int tid)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global sleep

sleep:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  MOV   0x8(%ebp),%esi  /* Move tid into %esi */
  INT   $SLEEP_INT      /* Trap into kernel's sleep */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return %eax unchanged */
