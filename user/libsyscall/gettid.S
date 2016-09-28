/**
 *  @file gettid.S
 *  @brief Wrapper for void gettid(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global gettid

gettid:
  PUSH %ebp
  MOV %esp,%ebp     /* Setup preamble */
  INT $GETTID_INT   /* Trap into kernel's gettid */
  POP %ebp          /* Restore %ebp */
  RET               /* Return */
  
