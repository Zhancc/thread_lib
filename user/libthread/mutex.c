/**
 * @file mutex.c
 * @brief Implementations of the mutex APIs specified in 410usr/inc/mutex.h
 *
 * We are using a basic ticket lock implementation where each thread has their
 * own ticket number and there is a global "Now Serving" variable. Each thread
 * waits on that number until it increaments to its own ticket number.
 *
 * Threads do not spin while waiting. Instead, they yield their CPU cycle until
 * woken up by the kernel.
 *
 * TODO should try to used deschedule() and make_runnable()
 *
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

/* Public APIs and types */
#include <mutex.h>      
#include <syscall.h>        /* yield */
#include <mutex_type.h>     /* mutex_t */
#include <assert.h>
/* Private APIs */
#include <asm_internals.h>  /* atomic_inc */

/**
 * @brief Initialize the mutex object. 
 *
 * @param mp Pointer to allocated but unintialized mutex_t data.
 *
 * @return 0 on success and negative number on error.
 */
int mutex_init(mutex_t *mp) {
    if (!mp)
        return -1;
	mp->next = mp->owner = 0;
	mp->locked_flag = 0;
	return 0;
}

/**
 * @brief Deactives the mutex object.
 *
 * It is illegal use the mutex object after this function has been called. This
 * can't be called while some thread is holding the lock or trying to aquire it.
 * It is the application's responsibility to ensure legal usage.
 *
 * TODO this is unimplemented. Perhaps we should have a variable in mutex_t to 
 * indicate if it is active.
 *
 * @param mp Pointer to initialized mutex object.
 */
void mutex_destroy(mutex_t *mp) {
	assert(mp->owner == mp->next);
	assert(mp->locked_flag == 0); // this line seems to be verbose
	return;
}

/**
 * @brief Indicate the start of the mutual exclusion region.
 *
 * If it is not your turn yet, yield and have kernel wake you up later.
 * 
 * @param mp Pointer to initialized mutex object.
 */
void mutex_lock(mutex_t *mp) {
	int ticket = atomic_inc(&(mp->next));
	while (ticket != mp->owner) {
		yield(-1);
	}
	mp->locked_flag = 1;
}

/**
 * @brief Indicates the end of the mutual exclusion region.
 *
 * Increament the "Now Serving" variable. 
 *
 * @param mp Pointer to initialized mutex object.
 */
void mutex_unlock(mutex_t *mp) {
	assert(mp->locked_flag);
	mp->locked_flag = 0;
	mp->owner++;
	return;
}
