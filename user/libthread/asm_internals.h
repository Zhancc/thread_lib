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

#endif
