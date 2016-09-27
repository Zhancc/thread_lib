/**
 *  @file print.S
 *  @brief Wrapper for int print(int size, char *buf)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global print

print:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  PUSH  0xc(%ebp)       /* buf */
  PUSH  0x8(%ebp)       /* size */
  MOV   %esp,%esi       /* Move address of the "system call packet" into %esi */
  INT   $PRINT_INT      /* Trap into kernel's print, return value in %eax */
  LEA   0x8(%esp),%esp  /* Clean up calling stack */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return to caller */
