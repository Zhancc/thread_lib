/**
 *  @file make_runnable.S
 *  @brief Wrapper for int make_runnable(int tid)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global make_runnable

make_runnable:
  PUSH  %ebp       
  MOV   %esp,%ebp           /* Setup preamble */
  PUSH  %esi                /* Callee saved register */
  MOV   0x8(%ebp),%esi      /* Move tid into %esi */
  INT   $MAKE_RUNNABLE_INT  /* Trap into kernel's make_runnable */
  POP   %esi                /* Restore %esi */
  POP   %ebp                /* Restore %ebp */
  RET                       /* Return %eax unchanged */
