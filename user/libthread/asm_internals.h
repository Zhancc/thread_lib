/**
 * @file asm_internals.h
 * @brief Definitions of atomic functions.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

#ifndef _ASM_INTERNALS_H_
#define _ASM_INTERNALS_H_

/**
 * @brief Atomically increment a value.
 *
 * Equivalent sequence of instructions are:
 *
 *    int old_val;
 *    old_val = *m;
 *    *m = *m + 1;
 *    return old_val;
 *
 * TODO I don't think it is necessary to have volatile since we are the ones
 * writing the assembly.
 *
 * @param m Pointer to value to be incremented.
 * @return Old value stored at the address.
 */
int atomic_inc(volatile int *m);

/**
 * @brief Atomically move a data into an address.
 *
 * Equivalent sequence of instructions are:
 *
 *    int old_val;
 *    old_val = *source;
 *    *source = delta;
 *    return old_val;
 *
 * @param source Pointer to old value.
 * @param delta New value to be stored at address pointed to by source.
 * @return Old value stored at the address.
 */
int xchg(int *source, int delta);

/**
 * @brief Atomically put a value into an address if the value stored there
 *        before was expected. 
 *
 * Equivalent sequence of instructions are:
 *
 *    if (*source == test) {
 *      *source = set;
 *      return 1;
 *    }
 *    return 0;
 *
 * @param source Pointer to the source value.
 * @param test Test value to be compared with.
 * @param set Target value to set if source value is equal to test.
 * @return 1 if equal, 0 otherwise.
 */
int cmpxchg(int *source, int test, int set);

#endif /* _ASM_INTERNALS_H_ */
