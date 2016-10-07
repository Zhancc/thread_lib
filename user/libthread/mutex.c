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
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

/* Public APIs and types */
#include <mutex.h>      
#include <syscall.h>        /* yield */
#include <mutex_type.h>     /* mutex_t */
#include <assert.h>         /* assert() */ 

/* Private APIs */
#include "asm_internals.h"  /* atomic_inc, xchg*/

/**
 * @brief Initialize the mutex object. 
 *
 * Behavior is undefined if called after the mutex has already been initialized
 * or called while it is in use.
 *
 * @param mp Pointer to allocated but unintialized mutex_t data.
 * @return 0 on success and negative number on error.
 */
int mutex_init(mutex_t *mp) 
{
  if (!mp)
    return -1;

  mp->next = mp->owner = 0;
  mp->locked = 0;
  mp->init = 1;
  return 0;
}

/**
 * @brief Deactives the mutex object.
 *
 * We are making sure that the mutex is not in locked state and every thread
 * that has asked for the lock has acquired the lock. Then we atomically set
 * the init flag to 0.
 *
 * @param mp Pointer to initialized mutex object.
 */
void mutex_destroy(mutex_t *mp) {
  xchg(&(mp->init), 0);
  assert(mp->owner == mp->next);
  assert(mp->locked == 0);
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
  int ticket;
  ticket = atomic_inc(&(mp->next));
  assert(mp->init == 1);
  while (ticket != mp->owner) {
    yield(-1);
  }
  mp->locked = 1;
}

/**
 * @brief Indicates the end of the mutual exclusion region.
 *
 * Increament the "Now Serving" variable. It is illegal for application 
 * to unlock a mutex that is not locked. 
 *
 * @param mp Pointer to initialized mutex object.
 */
void mutex_unlock(mutex_t *mp) {
  int old_lock = xchg(&(mp->locked), 0);
  assert(mp->init == 1);
  assert(old_lock == 1);
  mp->owner++;
  return;
}
