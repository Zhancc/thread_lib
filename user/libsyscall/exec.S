/**
 *  @file exec.S
 *  @brief Wrapper for int exec(char *execname, char **argvec)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global exec

exec:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  PUSH  0xc(%ebp)       /* argvec */
  PUSH  0x8(%ebp)       /* execname */
  MOV   %esp,%esi       /* Move address of the "system call packet" into %esi.
                           Since we pushed two arguments onto the stack, the
                           address is simply the stach pointer %esp */
  INT   $EXEC_INT       /* Trap into kernel's exec, return value in %eax */
  LEA   0x8(%esp),%esp  /* If the code hits this point, the syscall has failed
                           and we need to clean up calling stack */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return w/ %eax unchanged from the trap */
