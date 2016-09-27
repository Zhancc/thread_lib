/**
 *  @file swexn.S
 *  @brief Wrapper for int swexn(void *esp3, 
 *                               swexn_handler_t eip, 
 *                               void *arg, 
 *                               ureg_t *newureg)
 *
 *  @author X.D. Zhai (xingdaz)
 *  @bug None known.
 */

#include <syscall_int.h>

.global swexn

swexn:
  PUSH  %ebp       
  MOV   %esp,%ebp         /* Setup preamble */
  PUSH  %esi              /* Callee saved register */
  PUSH  0x14(%ebp)        /* newureg */
  PUSH  0x10(%ebp)        /* arg */
  PUSH  0xC(%ebp)         /* eip */
  PUSH  0x8(%ebp)         /* esp3 */
  MOV   %esp,%esi         /* Move the address of the "argument packet" into 
                             %esi */
  INT   $SWEXN_INT        /* Trap into kernel's swexn */
  LEA   0x10(%esp), %esp  /* Reclaim the call stack */
  POP   %esi              /* Restore %esi */
  POP   %ebp              /* Restore %ebp */
  RET                     /* Return %eax unchanged */
