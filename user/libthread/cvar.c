/**
 * @file cvar.c
 * @brief Implementation of conditional variable APIs specified in
 *        410usr/inc/cond.h 
 *
 * TODO needs lots of comments and cleanup here
 *
 * @author Zhan Chen (zhanc1), X.D. Zhai (xingdaz)
 */

/* cond_t */
#include <cond_type.h>
/* conditional variable public API */
#include <cond.h>
/* mutex public API */
#include <mutex.h>
/* list private APIs */
#include <list.h>
/* deschedule, make_runnable */
#include <syscall.h>

typedef struct waiting {
	int tid;
	int about_to_be_runnable;
	list list_entry;
} waiting;

/**
 * @brief Initialize the queue lock and the queue itself.
 *
 * @param cv Pointer to uninitialized conditional variable.
 *
 * @return 
 */
int cond_init(cond_t *cv) {
	int ret;

  if (!cv)
    return -1;

	ret = mutex_init(&cv->cmutex);
	if(ret < 0)
		return ret;

	list_init(&cv->queue);
	return 0;
}

/**
 * @brief Deactivates the conditional variable
 * 
 * It is illegal to use the conditional vairable after it has been destroyed
 * or to destroy it when there are still threads waiting on it.
 *
 * @param cv Pointer to conditional variable.
 */
void cond_destroy(cond_t *cv) {
	mutex_destroy(&cv->cmutex);
}

/**
 * @brief Insert the calling thread into the waiting queue, unlocks the mutex,
 *        and put the calling thread to sleep.
 *
 * @param cv Pointer to conditional variable.
 * @param mp Pointer to mutex that the calling thread was holding.
 */
void cond_wait(cond_t *cv, mutex_t *mp) {
	struct waiting self;
	self.tid = gettid();
	self.about_to_be_runnable = 0;
	mutex_lock(&cv->cmutex);
  /* It is ok that we are inserting an address on the stack b/c the stack will 
   * only be cleaned up _after_ the thread wakes up at which point the address
   * is no longer in the queue. */
	list_add_tail(&cv->queue, &self.list_entry);
	mutex_unlock(mp);
	mutex_unlock(&cv->cmutex);
  /* Don't sleep some other thread is going to make this thread runnable soon.
   * Otherwise, go to sleep. */
	deschedule(&self.about_to_be_runnable);
	mutex_lock(mp);
}

/**
 * @brief Wakes up _a_ thread waiting on the condition.
 * 
 * We choose to wait up the first thread if one exists.
 *
 * @param cv Pointer to cond_t structure 
 */
void cond_signal(cond_t *cv) {
	struct waiting *next_in_line;
	list *entry;
	mutex_lock(&cv->cmutex);
	entry = list_remv_head(&cv->queue);
	mutex_unlock(&cv->cmutex);
	if (entry) {
	  next_in_line = LIST_ENTRY(entry, waiting, list_entry);
	  next_in_line->about_to_be_runnable = 1;
	  make_runnable(next_in_line->tid);
	}
}

/**
 * @brief Wakes up _every_ thread waiting on the condition.
 *
 * @param cv Pointer to cond_t structure 
 */
void cond_broadcast(cond_t *cv) {
	struct waiting *next_in_line;
	list *entry;

	mutex_lock(&cv->cmutex);
	entry = list_remv_head(&cv->queue);

  do {
    next_in_line = LIST_ENTRY(entry, waiting, list_entry);
	  next_in_line->about_to_be_runnable = 1;
	  make_runnable(next_in_line->tid);
	  entry = list_remv_head(&cv->queue);
  } while (!entry);

	mutex_unlock(&cv->cmutex);
}
