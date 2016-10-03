/**
 * @file thr_internals.h
 * @brief Contains internal definitions, macros and functions to implement
 *        thread API.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <list.h>   /* list_t */
#include <cond.h>   /* cond_t */

#define STATUS_RUNNING  0
#define STATUS_RUNNABLE 1
#define STATUS_EXITED   2

/**
 * @brief Thread Control Block 
 */
typedef struct _tcb {
	int tid;            /* Thread ID returned by the kernel */
	int status;         /* One of the above */
	int joined;         /* Indicate if it has been joined by some thread */
	cond_t exited;      /* Indicate if the peer thread has exited */
	void *ret;          /* Pointer to address that holds return status */
	list_t tcb_entry;   /* List entry used to find the previous and next tcb */
	void *stack_high;   /* Limits of the stack */
	void *stack_low;
} tcb_t;


/**
 * @brief System call wrapper for thread_fork.
 *
 * @param esp Top of the peer thread stack.
 *
 * @return tid for invoking thread, 0 for peer thread.
 */
int thread_fork_wrapper(void *esp, tcb_t *tcb);

/**
 * @brief New thread lands here after thr_create.
 *
 * The new peer thread deschedule itself if the invoking thread hasn't got a
 * chance to fill in its tid.
 *
 * @param tcb_ptr Pointer to its own tcb.
 */
void peer_thread_init(tcb_t *tcb);
/**
 * @brief get the tcb ptr
 * we have arranged the stack in such a way  that the field below the return
 * address default_exit_entry is tcb ptr
 * @return return the tcb ptr. panic if failed
 */
tcb_t *get_tcb();
#endif /* THR_INTERNALS_H */
