/**
 *  @file new_pages.S
 *  @brief Wrapper for int remove_pages(void *base) 
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global remove_pages

remove_pages:
  PUSH  %ebp       
  MOV   %esp,%ebp         /* Setup preamble */
  PUSH  %esi              /* Callee saved register */
  MOV   0x8(%esp),%esi    /* Move base into %esi */ 
  INT   $REMOVE_PAGES_INT /* Trap into kernel's remove_pages */
  POP   %esi              /* Restore %esi */
  POP   %ebp              /* Restore %ebp */
  RET                     /* Return %eax unchanged */
