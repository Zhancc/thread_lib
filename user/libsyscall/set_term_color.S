/**
 *  @file set_term_color.S
 *  @brief Wrapper for int set_term_color(int color)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global set_term_color

set_term_color:
  PUSH  %ebp       
  MOV   %esp,%ebp           /* Setup preamble */
  PUSH  %esi                /* Callee saved register */
  MOV   0x8(%ebp),%esi      /* Move status into %esi */
  INT   $SET_TERM_COLOR_INT /* Trap into kernel's set_term_color */
  POP   %esi                /* Restore %esi */
  POP   %ebp                /* Restore %ebp */
  RET                       /* Return %eax unchanged to caller */
