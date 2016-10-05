/**
 * @file swexn_handler.c
 * @brief Implementations of thread library software exception handlers.
 *
 * TODO create other handlers to handle other exceptions and untimely deaths
 * of threads and recover from there.
 *
 * @author X.D. Zhai (xingdaz)
 * @author Zhan Chan (zhanc1)
 */
#include <swexn_handler.h> 
#include <malloc.h>         /* _malloc */
#include <memlib.h>         /* mem_sbrk */
#include <syscall.h>        /* swexn */
#include <simics.h>

/* Global pointer to ewexn handler's stack pointer */
void *esp3;

/* Global pointer to root thread's page fault handler argument */
pagefault_handler_arg_t *root_thr_pagefault_arg;

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
static int
pagefault(void *arg, ureg_t *ureg)
{
    int new_pages_ret;
    void *new_stack_low;
    unsigned int new_stack_size, stack_fixed_size;
    pagefault_handler_arg_t *stack_data;

	/* make sure we can handle the page fault*/
    stack_data = (pagefault_handler_arg_t *)arg;
    stack_fixed_size = stack_data->fixed_size;
	
	if(	ureg->error_code & 0x1 ||
		ureg->cr2 >= (unsigned int)stack_data->stack_low || 
		ureg->cr2 + PAGE_SIZE < (unsigned int)stack_data->stack_low){
		/* if this is not a non-present fault, or faulting address is too high or too low*/
		panic("segmentation fault!\n");
	}	

    new_stack_low = (void *)((char *) stack_data->stack_low - STACK_EXTENSION);
    new_stack_size = stack_data->stack_high - new_stack_low;

    /* Failure: Outgrown the declared stack. Single threaded application won't
     * run into this problem. */
    if (stack_fixed_size != 0 && new_stack_size >= stack_fixed_size)
        return -1;

    new_pages_ret = new_pages(new_stack_low, STACK_EXTENSION);
    /* Failure: Can't allocate new pages. */
    if (new_pages_ret < 0)
        return -3;

    /* Sucess: Update global stack data */
    stack_data->stack_low = new_stack_low;
    return 0;
}

/* is this always the swe handler of root thread */
void
swexn_handler(void *arg, ureg_t *ureg)
{
    int pagefault_ret = -1;
    if (ureg->cause == SWEXN_CAUSE_PAGEFAULT) {
    	pagefault_ret = pagefault(arg, ureg); 
    }

    /* Only register the handler if there wasn't any problem in the
     * exception handling. Otherwise, let the kernel kill it next time the
     * exception happens. */
    if (pagefault_ret >= 0)
        swexn(esp3, swexn_handler, arg, ureg);
	else
		panic("Panic\n");
}
