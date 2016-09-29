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
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

#include <malloc.h>     /* _malloc */
#include <memlib.h>     /* mem_sbrk */
#include <syscall.h>    /* swexn */

#define ESP3_OFFSET         2
#define SWEXN_STACK_SIZE    64
#define STACK_EXTENSION     PAGE_SIZE

/* Global variable that stores the location of the handler's stack pointer */
void *esp3;

/**
 * @brief Pagefault handler that extents the stack downward.
 *
 * Fails to actually allocate new page if the stack grows into the heap. When
 * it first enters the handler, we assume that the kernel is being reasonable
 * and aligned the stack_low with the page boundary. Afterwards, it will be
 * aligned too because the extension amount is multiples of PAGE_SIZE.
 *
 * @param arg Pointer to argument.
 * @param ureg Pointer to register set.
 */
void
pagefault_handler(void *arg, ureg_t *ureg)
{
    void *stack_low, *brk;
    stack_low = arg;

    if (ureg->cause != SWEXN_CAUSE_PAGEFAULT)
        goto Return;

    /* If the stack grows into the heap, then sorry, none can do. */
    stack_low -= STACK_EXTENSION;
    brk = mem_sbrk(0);
    if (stack_low <= brk)
        goto Return;

    if (new_pages(stack_low, STACK_EXTENSION) < 0) 
        goto Return; 

Return:
    swexn(esp3, pagefault_handler, stack_low, ureg);
}

/**
 * @brief Installs the page fault handler.
 * @param stack_high Highest byte of the kernel allocated stack.
 * @param stack_low Lower byte of the kernel allocated stack. Grows downwards.
 */
void
install_autostack(void *stack_high, void *stack_low)
{
    /* Make room for the exception handler stack, never freed */
    esp3 = _malloc(SWEXN_STACK_SIZE + ESP3_OFFSET);
    if (!esp3)
        return;
    swexn(esp3, pagefault_handler, stack_low, NULL);
}
