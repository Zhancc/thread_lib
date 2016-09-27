/**
 *  @file set_cursor_pos.S
 *  @brief Wrapper for int set_cursor_pos(int row, int col)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global set_cursor_pos

set_cursor_pos:
  PUSH  %ebp       
  MOV   %esp,%ebp           /* Setup preamble */
  PUSH  %esi                /* Callee saved register */
  PUSH  0xC(%ebp)           /* col */
  PUSH  0x8(%ebp)           /* row */
  MOV   %esp,%esi           /* Move address of calling packet into %esi */
  INT   $SET_CURSOR_POS_INT /* Trap into kernel's set_term_color */
  LEA   0x8(%esp),%esp      /* Reclaim stack */
  POP   %esi                /* Restore %esi */
  POP   %ebp                /* Restore %ebp */
  RET                       /* Return %eax unchanged to caller */
