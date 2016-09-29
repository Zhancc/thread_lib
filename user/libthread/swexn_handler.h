/**
 * @file swexn_handler.h
 * @brief Contains definitions and declarations of a exception handler.
 * @author X.D. Zhai (xingdaz), Zhan Chan (zhanc1)
 */

#ifndef _SWEXN_HANDLER_H_
#define _SWEXN_HANDLER_H_

#include <ureg.h> /* ureg_t */

#define ESP3_OFFSET         2
#define SWEXN_STACK_SIZE    64
#define STACK_EXTENSION     PAGE_SIZE

/**
 * @brief Structure that holds the data for root thread's pagefault handler's
 *        argument
 */
typedef struct pagefault_handler_arg {
    void *stack_high;           /* Highest byte of the kernel allocated stack */
    void *stack_low;            /* Lower byte of the stack, initialzed to kernel 
                                   allocated low. Gets updated as newpages are 
                                   alloccated */
    unsigned int fixed_size;    /* Initialized to zero to indicate auto growth. 
                                   None zero to inidcate thr_init() has been 
                                   called and there is a limit to stack growth 
                                   */
} pagefault_handler_arg_t;

/**
 * @brief Software exception handler that responds to exceptions.
*
 * @param arg Pointer to argument.
 * @param ureg Pointer to register set.
 */
void swexn_handler(void *arg, ureg_t *ureg);

#endif /* _SWEXN_HANDLER_H_ */
