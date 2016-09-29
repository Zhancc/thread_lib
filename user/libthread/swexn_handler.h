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
 * @brief Software exception handler that responds to exceptions.
 *
 * Fails to actually allocate new page if the stack grows into the heap. When
 * it first enters the handler, we assume that the kernel is being reasonable
 * and aligned the stack_low with the page boundary. Afterwards, it will be
 * aligned too because the extension amount is multiples of PAGE_SIZE.
 *
 * @param arg Pointer to argument.
 * @param ureg Pointer to register set.
 */
void swexn_handler(void *arg, ureg_t *ureg);

#endif /* _SWEXN_HANDLER_H_ */
