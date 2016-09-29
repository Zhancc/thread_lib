/**
 * @file swexn_handler.c
 * @brief Implementations of thread library software exception handlers.
 *
 * TODO create other handlers to handle other exceptions and untimely deaths
 * of threads and recover from there.
 *
 * @author X.D. Zhai (xingdaz), Zhan Chan (zhanc1)
 */
#include <swexn_handler.h>
#include <malloc.h>         /* _malloc */
#include <memlib.h>         /* mem_sbrk */
#include <syscall.h>        /* swexn */

/* Global variable that stores the location of the handler's stack pointer */
void *esp3;

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
static void *
pagefault(void *stack_low)
{
    void *brk, *new_stack_low;

    new_stack_low = (void *)((char *) stack_low - STACK_EXTENSION);
    brk = mem_sbrk(0);
    if (new_stack_low <= brk)
        return stack_low;

    /* Can't allocate new pages? Too bad. TODO need to kill thread */
    if (new_pages(new_stack_low, STACK_EXTENSION) < 0)
        return stack_low;

    return new_stack_low;
}

void
swexn_handler(void *arg, ureg_t *ureg)
{
    unsigned int cause;
    void *stack_low;

    cause = ureg->cause;
    stack_low = arg;

    switch (cause) {
    case SWEXN_CAUSE_PAGEFAULT:
        stack_low = pagefault(stack_low); 
        break;
    }
    swexn(esp3, swexn_handler, stack_low, ureg);
}

