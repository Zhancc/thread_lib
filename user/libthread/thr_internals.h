/**
 * @file thr_internals.h
 * @brief Definitions, macros and utility functions to implement thread API.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

#ifndef _THR_INTERNALS_H_
#define _THR_INTERNALS_H_

#include <list.h>   /* list_t */
#include <cond.h>   /* cond_t */

#define STATUS_RUNNING        0
#define STATUS_RUNNABLE       1
#define STATUS_EXITED         2

#define STACK_ALIGNMENT_MASK  (~0x3)

/* Global pointer to _main()'s ebp */
void **_main_ebp;

/**
 * @brief Thread Control Block.
 */
typedef struct _tcb {
  int tid;            /* Thread ID returned by the kernel */
  int status;         /* One of the above STATUS_x */
  int joined;         /* Indicate if it has been joined by some thread */
  cond_t exited;      /* Indicate if the peer thread has exited */
  void *ret;          /* Pointer to address that holds return status */
  list_t tcb_entry;   /* List entry used to find the previous and next tcb */
  void *stack_high;   /* Limits of the stack */
  void *stack_low;
} tcb_t;

/**
 * @brief A waiting thread populate this structure before enqueue and sleep.
 */
typedef struct waiting_thr_data {
  int tid;  /* tid of the waiting thread */

  /* This is the "reject" argument of the deschedule syscall. The idea is if
   * some thread wants to wake up a thread, it indicates its intent by setting
   * this variable to 1 before calling  make_runnable. A thread about to
   * deschedule itself will atomically check this variable. If it is none
   * zero, i.e. it will runnable soon, then it will not deschedule itself. */
  int about_to_be_runnable;
  list_t list_entry;
  int type; /* Only applicable to rwlock, indicates reader of writer */
} waiting_thr_data_t;

/**
 * @brief System call wrapper for thread_fork.
 * @param esp Top of the peer thread stack.
 * @param tcb Pointer to new thread's TCB.
 * @return tid for invoking thread, 0 for peer thread.
 */
int thread_fork_wrapper(void *esp, tcb_t *tcb);

/**
 * @brief New thread's landing point after thr_create().
 *
 * New peer thread deschedules itself if the invoking thread hasn't got a
 * chance to fill in its tid.
 *
 * @param tcb Pointer to its own tcb.
 */
void peer_thread_init(tcb_t *tcb);

/**
 * @brief Default return point. 
 */
void default_exit_entry();

/**
 * @brief Retrieves the current frame pointer.
 * @return Address that holds the saved %ebp of the calling frame.
 */
void **get_ebp();

#endif /* _THR_INTERNALS_H_ */
