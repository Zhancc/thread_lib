/**
 *  @file getchar.S
 *  @brief Wrapper for char getchar(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global getchar

getchar:
  PUSH  %ebp       
  MOV   %esp,%ebp     /* Setup preamble */
  INT   $GETCHAR_INT  /* Trap into kernel's getchar */
  POP   %ebp          /* Restore %ebp */
  RET                 /* Return %eax unchanged to caller */

