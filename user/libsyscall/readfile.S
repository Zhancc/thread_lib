/**
 *  @file exec.S
 *  @brief Wrapper for int readfile(char *filename, 
 *                                  char *buf, 
 *                                  int count, 
 *                                  int offset)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global readfile

readfile:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  PUSH  0x14(%ebp)      /* offset */
  PUSH  0x10(%ebp)      /* count */
  PUSH  0xc(%ebp)       /* buf */
  PUSH  0x8(%ebp)       /* filename */
  MOV   %esp,%esi       /* Move address of the "system call packet" into %esi */
  INT   $READLINE_INT   /* Trap into kernel's readline, return value in %eax */
  LEA   0x10(%esp),%esp /* Reclaim the stack */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return %eax unchanged to caller */
