/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */



#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H
#include <list.h>
#include <cond.h>

#define STATUS_ON_GOING 0
#define STATUS_BLOCKED 1
#define STATUS_ZOMBIE 2

typedef struct thread_struct{
	int tid;
	int status;
	int joined;
	cond_t cv;

	void *ret;

	list tcb_entry;
	void *stack_high;
	void *stack_low;
} thread_struct;
int thread_fork_wrapper(void *esp);
#endif /* THR_INTERNALS_H */
