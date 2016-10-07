/**
 * @file swexn_handler.c
 * @brief Implementations of thread library software exception handlers defined
 *        in swexn_handler.h.
 * @author X.D. Zhai (xingdaz)
 * @author Zhan Chen (zhanc1)
 */
#include <stdlib.h>         /* panic() */
#include <syscall.h>        /* swexn() */
#include <memlib.h>         /* set_brk() */
#include <simics.h>
#include "swexn_handler.h"  /* pagefault_handler_arg_t */

/**
 * @brief Page fault handler. 
 *
 * Fails to actually allocate new page if the stack grows into the heap. When
 * it first enters the handler, we assume that the kernel is being reasonable
 * and aligned the stack_low with the page boundary. Afterwards, it will be
 * aligned too because the extension amount is multiples of PAGE_SIZE.
 * 
 * @param stack_low Pointer to current lowest byte of main thread stack.
 */
static int pagefault(void *arg, ureg_t *ureg)
{
  void *new_stack_low;
  unsigned int stack_fixed_size, old_stack_size;
  pagefault_handler_arg_t *root_stack_data;

  /* Make sure we can handle the page fault */
  root_stack_data = (pagefault_handler_arg_t *) arg;
  stack_fixed_size = root_stack_data->fixed_size;

  /* Failure: non-present fault to indicate possible read/write privilege 
   * violation, or faulting address not in reasonable range */
  if (ureg->error_code & 0x1 ||
      ureg->cr2 >= (unsigned int) root_stack_data->stack_low ||
      MAX_OFFSET < (unsigned int) root_stack_data->stack_low - ureg->cr2) {
    panic("Pagefault handler segmentation fault %p\n", (void *)ureg->cr2);
  }

  old_stack_size = (char *) root_stack_data->stack_high - 
    (char *) root_stack_data->stack_low;

  /* Failure: Outgrown the declared stack. Single threaded application won't
   * run into this problem. Multi-threaded because in thr_init(),
   * stack_fixed_size will be set to none zero. */
  if (stack_fixed_size != 0 && old_stack_size >= stack_fixed_size)
    return -2;

  new_stack_low = (void *)((char *) root_stack_data->stack_low - 
      STACK_EXTENSION);
  /* Failure: Can't allocate new pages. */
  if (new_pages(new_stack_low, STACK_EXTENSION) < 0)
    return -3;

  /* Sucess: Update global stack data */
  root_stack_data->stack_low = new_stack_low;
  return 0;
}

void root_thr_swexn_handler(void *arg, ureg_t *ureg) 
{
  /* Important: arg here is an opaque data type. When this handler was
   * registered by install_autostack(), root_pagefault_arg was passed in. It 
   * is a global variable, points to a pagefault_handler_arg_t on the heap. */
  int pagefault_ret = -1;
  if (ureg->cause == SWEXN_CAUSE_PAGEFAULT) {
    pagefault_ret = pagefault(arg, ureg); 
  }
  /* Only register the handler again if there wasn't any problem in the
   * exception handling. Otherwise, panic() */
  if (pagefault_ret >= 0)
    swexn(root_esp3, root_thr_swexn_handler, arg, ureg);
  else
    panic("Root thread pagefault handler exception.\n");
}
