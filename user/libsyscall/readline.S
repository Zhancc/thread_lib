/**
 *  @file readline.S
 *  @brief Wrapper for int readline(int len, char *buf) 
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global readline

readline:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  PUSH  0xC(%ebp)       /* buf */
  PUSH  0x8(%ebp)       /* len */
  MOV   %esp,%esi       /* Move the address of the "argument packet" into %esi */
  INT   $READLINE_INT   /* Trap into kernel's new_page */
  LEA   0x8(%esp),%esp  /* Reclaim the call stack */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return %eax unchanged to the caller */
