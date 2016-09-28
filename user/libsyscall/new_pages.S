/**
 *  @file new_pages.S
 *  @brief Wrapper for int new_pages(void *base, int len) 
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global new_pages

new_pages:
  PUSH  %ebp       
  MOV   %esp,%ebp       /* Setup preamble */
  PUSH  %esi            /* Callee saved register */
  PUSH  0xC(%ebp)       /* len */
  PUSH  0x8(%ebp)       /* base */
  MOV   %esp,%esi       /* Move the address of the "argument packet" into %esi */
  INT   $NEW_PAGES_INT  /* Trap into kernel's new_page */
  LEA   0x8(%esp), %esp /* Reclaim the call stack */
  POP   %esi            /* Restore %esi */
  POP   %ebp            /* Restore %ebp */
  RET                   /* Return %eax unchanged */
