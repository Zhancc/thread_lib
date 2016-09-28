/**
 * @file thr_internals.h
 * @brief Contains internal definitions, macros and functions to implement
 *        thread API.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 * @date 2016-09-27
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <list.h>
#include <cond.h>

#define STATUS_ON_GOING 0
#define STATUS_BLOCKED  1
#define STATUS_ZOMBIE   2

/* TODO why don't we just call this TCB? */
typedef struct thread_struct {
	int tid;
	int status;
	int joined;
	cond_t cv;

	void *ret;

	list tcb_entry;
	void *stack_high;
	void *stack_low;
} thread_struct;


/**
 * @brief System call wrapper for thread_fork.
 *
 * @param esp
 *
 * @return 
 */
int thread_fork_wrapper(void *esp);

#endif /* THR_INTERNALS_H */
