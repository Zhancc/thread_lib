/**
 *  @file get_ticks.S
 *  @brief Wrapper for unsigned int get_ticks(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global get_ticks

get_ticks:
  PUSH %ebp
  MOV %esp,%ebp       /* Setup preamble */
  INT $GET_TICKS_INT  /* Trap into kernel's get_ticks */
  POP %ebp            /* Restore %ebp */
  RET                 /* Return %eax unchanged to caller */
  
