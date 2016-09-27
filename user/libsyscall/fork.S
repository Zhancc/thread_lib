/**
 *  @file fork.S
 *  @brief Wrapper for int fork(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global fork

fork:
  PUSH  %ebp
  MOV   %esp,%ebp     /* Setup preamble */
  INT   $FORK_INT     /* Trap into kernel's fork */
  POP   %ebp          /* Restore %ebp */
  RET                 /* Return w/ %eax unchanged */
