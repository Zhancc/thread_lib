/**
 * @file cvar.c
 * @brief Implementation of conditional variable APIs specified in 
 *        410usr/inc/cond.h 
 *
 * Threads waiting on a particular conditio will insert their list entry into 
 * the queue and deschedule itself till signaled.
 *
 * @author Zhan Chen (zhanc1), X.D. Zhai (xingdaz)
 */

/* Public APIs */
#include <cond.h>
#include <mutex.h>      /* mutex_init, mutex_lock, and mutex_unlock */

/* Private APIs */
#include <list.h>       /* list_init, list_add_tail, and list_remv_head */
#include <cond_type.h>  /* cont_t */
#include <syscall.h>    /* deschedule and make_runnable */
#include <assert.h>
/**
 * @brief A waiting thread populate this structure before enqueue and sleep.
 */
typedef struct waiting_thr_data {
    /* tid of the waiting thread */
	int tid;
    /* This is the "reject" argument of the deschedule syscall. The idea is if 
     * some thread wants to wake up a thread, it indicates its intent by setting
     * this variable to 1 before calling  make_runnable. A thread about to 
     * deschedule itself will atomically check this variable. If it is none 
     * zero, i.e. it will runnable soon, then it will not deschedule itself. */
	int about_to_be_runnable;   
	list_t list_entry;
} waiting_thr_data_t;

/**
 * @brief Initialize the queue lock and the queue itself.
 *
 * @param cv Pointer to allocated but uninitialized conditional variable struct.
 *
 * @return 
 */
int cond_init(cond_t *cv) {
	int ret;
    
    if (!cv)
        return -1;

	ret = mutex_init(&cv->qmutex);
	if(ret < 0)
		return ret;

	list_init(&cv->queue);
	cv->init_flag = 1;
	return 0;
}

/**
 * @brief Deactivates the conditional variable.
 * 
 * It is illegal to use the conditional vairable after it has been destroyed
 * or to destroy it when there are still threads waiting on it. It is
 * application's responsibility to check for these.
 *
 * @param cv Pointer to initialized conditional variable struct.
 */
void cond_destroy(cond_t *cv) {
	mutex_lock(&cv->qmutex);
	assert(list_empty(&cv->queue));
	cv->init_flag = 0;
	mutex_unlock(&cv->qmutex);
	mutex_destroy(&cv->qmutex);
}

/**
 * @brief Insert the calling thread into the waiting queue, unlocks the mutex,
 *        and put the calling thread to sleep.
 *
 * @param cv Pointer to initialzed conditional variable struct.
 * @param mp Pointer to mutex struct that the calling thread is holding.
 */
void cond_wait(cond_t *cv, mutex_t *mp) {
    waiting_thr_data_t data;
	data.tid = gettid();
	data.about_to_be_runnable = 0;

    /* It is ok that we are inserting an address on the stack b/c the stack will 
     * only be cleaned up _after_ the thread wakes up at which point the address
     * is no longer in the queue. */
	mutex_lock(&cv->qmutex);
	assert(cv->init_flag);
	list_add_tail(&cv->queue, &data.list_entry);
	mutex_unlock(mp);
	mutex_unlock(&cv->qmutex);
	deschedule(&data.about_to_be_runnable);
	mutex_lock(mp);
}

/**
 * @brief Wakes up _a_ thread waiting on the condition.
 * 
 * We choose to wait up the first thread if one exists.
 *
 * @param cv Pointer to initialized conditional variable struct.
 */
void cond_signal(cond_t *cv) {
    waiting_thr_data_t *next_in_line;
	list_ptr entry;

	mutex_lock(&cv->qmutex);
	assert(cv->init_flag);
	entry = list_remv_head(&cv->queue);
	mutex_unlock(&cv->qmutex);

	if (entry) {
	  next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
	  next_in_line->about_to_be_runnable = 1;
	  make_runnable(next_in_line->tid);
	}
}

/**
 * @brief Wakes up _every_ thread waiting on the condition.
 *
 * @param cv Pointer to initialized conditional variable struct.
 */
void cond_broadcast(cond_t *cv) {
    waiting_thr_data_t *next_in_line;
	list_ptr entry;

	mutex_lock(&cv->qmutex);
	assert(cv->init_flag);
	entry = list_remv_head(&cv->queue);

  while(entry) {
      next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
	  next_in_line->about_to_be_runnable = 1;
	  make_runnable(next_in_line->tid);
	  entry = list_remv_head(&cv->queue);
  };

	mutex_unlock(&cv->qmutex);
}
