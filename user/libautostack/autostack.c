/**
 * @file autostack.c
 * @brief Installs page fault handler.
 *  
 * It needs to allocate a small stack for the page fault handler. The esp3 is
 * 2 bytes away from the address where the kernel pushes the two arguments for 
 * the handler.
 *
 *         esp3
 *          |
 *          V
 *          -----
 *          |   | 
 *     ----------
 *     |  ureg  |
 *     ----------
 *     |  arg   |
 *     ----------
 *     |        |
 *     |        |
 *     |        |
 *
 * @author X.D. Zhai (xingdaz)
 * @author Zhan Chen (zhanc1)
 */

#include <malloc.h>         /* malloc() */
#include <syscall.h>        /* swexn() */
#include "thr_internals.h"  /* _main_ebp */
#include "swexn_handler.h"  /* SWEXN_STACK_SIZE, ESP3_OFFSET, 
                               and root_swexn_handler() */
#include <simics.h>
/**
 * @brief Installs the page fault handler.
 *
 * If any of the system calls fails, we simply return. When pagefault happens,
 * the kernel will kill the task.
 *
 * @param stack_high Highest byte of the kernel allocated stack.
 * @param stack_low Lower byte of the kernel allocated stack. Grows downwards.
 */
void install_autostack(void *stack_high, void *stack_low) 
{
  /* Make room for the exception handler stack, never freed. At this point
   * we are still single threaded */
  if (!(root_esp3 = malloc(SWEXN_STACK_SIZE + ESP3_OFFSET)))
    panic("memory_pressure\n");
  /* Populate root thread pagefault handler argument */
  if (!(root_pagefault_arg = malloc(sizeof(pagefault_handler_arg_t))))
    panic("memory pressure\n");

  root_pagefault_arg->stack_high = stack_high;
  root_pagefault_arg->stack_low = stack_low;
  root_pagefault_arg->fixed_size = 0;
  
  /* ureg is left as NULL to tell kernel it is the first registration. */
  if (swexn(root_esp3, root_thr_swexn_handler, 
            (void *) root_pagefault_arg, NULL))
    panic("handler installation fail\n");

  /* I say one last thing, let's remember _main()'s ebp for later use.
   * It is used to overwrite main()'s return address. Detail in thread.c's 
   * thr_init() documentation. */
	_main_ebp = (void **) *get_ebp();
}
