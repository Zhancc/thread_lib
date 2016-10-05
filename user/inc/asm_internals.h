/**
 * @file asm_internals.h
 * @brief Assembly functions mostly for atomic actions.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

#ifndef _ASM_INTERNALS_
#define _ASM_INTERNALS_

/**
 * @brief Atomically increment the value stored at m.
 *
 * @param m Pointer to value to be incremented.
 *
 * @return Old value stored at m.
 */
int atomic_inc(volatile int *m);

/**
 * @brief Atomically swap source's old value and delta.
 *
 * @param source Pointer to old value.
 * @param delta New value to be stored at old value location.
 *
 * @return Old value stored at source.
 */
int xchg(int *source, int delta);

/**
 * @brief Atomically compare the value at source and test, if equal, set value 
 *        at source to set.
 *
 * @param source Pointer to the source value.
 * @param test Test value to be compared with.
 * @param set Target value to which source is set if source is equal to test.
 *
 * @return if equal, return positive; if not, return zero
 */
int cmpxchg(int *source, int test, int set);

/**
 * @brief default return point, which takes return value as argument to 
 *        default_exit
 * @return should never return
 */
void default_exit_entry();

/**
 * @brief Retrieves the current frame pointer  
 *
 * @return Address that holds the saved %ebp of the caller.
 */
void **get_ebp();
#endif
