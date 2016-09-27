/**
 *  @file halt.S
 *  @brief Wrapper for void halt(void)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global halt

halt:
  INT   $HALT_INT     /* Trap into kernel's halt */
