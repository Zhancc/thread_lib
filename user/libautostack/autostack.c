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
 * @author X.D. Zhai (xingdaz), Zhan Chan (zhanc1)
 */

#include <malloc.h>         /* _malloc */
#include <syscall.h>        /* swexn */
#include <swexn_handler.h>  /* SWEXN_STACK_SIZE, ESP3_OFFSET */

extern void *esp3;

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
    swexn(esp3, swexn_handler, stack_low, NULL);
}
