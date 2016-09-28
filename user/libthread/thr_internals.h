/**
 * @file thr_internals.h
 * @brief Contains internal definitions, macros and functions to implement
 *        thread API.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

/* list */
#include <list.h>
/* cond_t */
#include <cond.h>

/* TODO we have to conform to the lingo, has to call RUNNING, RUNNABLE 
 * ZOMBIE is used to refer to proceses */
#define STATUS_ON_GOING 0
#define STATUS_BLOCKED  1
#define STATUS_ZOMBIE   2

/**
 * @brief Part of the thread context less the register values.
 */
typedef struct _tcb {
	int tid;          /* Thread ID returned by the kernel */
	int status;       /* One of the above */
	int joined;
	cond_t exited;    /* Indicate if the peer thread has exited */

	void *ret;        /* Pointer to return value */

	list tcb_entry;
	void *stack_high; /* Limits of the stack */
	void *stack_low;
} tcb_t;


/**
 * @brief System call wrapper for thread_fork.
 *
 * @param esp Top of the peer thread stack.
 *
 * @return tid for invoking thread, 0 for peer thread.
 */
int thread_fork_wrapper(void *esp);

/**
 * @brief New thread lands here after thr_create.
 *
 * The new peer thread deschedule itself if the invoking thread hasn't got a
 * chance to fill in its tid.
 *
 * @param tcb_ptr Pointer to its own tcb.
 */
void peer_thread_init(tcb_t *tcb);
#endif /* THR_INTERNALS_H */
