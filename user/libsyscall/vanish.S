/**
 *  @file vanish.S
 *  @brief Wrapper for void vanish(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global vanish

vanish:
  PUSH %ebp
  MOV %esp,%ebp     /* Setup preamble */
  INT $VANISH_INT   /* Trap into kernel's vanish */
  POP %ebp          /* Restore %ebp */
  RET               /* Return */
