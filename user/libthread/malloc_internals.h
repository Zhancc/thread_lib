#ifndef _MALLOC_INTERNALS_H_
#define _MALLOC_INTERNALS_H_
#include <types.h> /* size_t */

/**
 * @brief Allocate 2 blocks in succession.
 *
 * This function is created for `thr_init()` where TCB and stack have to be
 * allocated together.
 * 
 * @param dest1 Pointer to address where first malloc()'s address goes into.
 * @param __size1 Size to allocate for first malloc();
 * @param dest2 Pointer to address where second malloc()'s address goes into.
 * @param __size2 Size to allocate for second malloc();
 * @return 0 on success, negative otherwise.
 */
int double_malloc(void **dest1, size_t __size1, void **dest2, size_t __size2);

#endif /* _MALLOC_INTERNALS_H_ */
