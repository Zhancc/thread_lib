/**
 * @file rwlock.c
 * @brief Implementation of reader writer lock API as defined in 
 *        410user/inc/rwlock.h
 *
 * This is a fair implementation of reader writer lock where no one gets
 * starved. Readers and writers share 1 queue. Upon unlock, the last holder 
 * wakes up one writer if there is one in line, or as many readers as
 * possible till hitting a writer. 
 *
 * Large part of the implementation is lifted from the conditional variable 
 * code because they are very similiar.
 *
 * @author Zhan Chen (zhanc1)
 * @author Xingda Zhai (xingdaz)
 */
#include <rwlock.h>
#include <rwlock_type.h>    /* rwlock_t */
#include <assert.h>         /* assert() */
#include <syscall.h>        /* deschedule() and make_runnable() */
#include <list.h>           /* list_init, list_add_tail, and list_remv_head */
#include "thr_internals.h"  /* waiting_thr_data_t */

/**
 * @brief Initialize rwlock data structure.
 * @param rwlock Allocated but unitialized rwlock.
 * @return 0 on succcess and negative number on failture.
 */
int rwlock_init(rwlock_t *rwlock)
{
  if (!rwlock)
    return -1;
  if (rwlock->init)
    return -2;
  if(mutex_init(&rwlock->data) < 0)
    return -3;

  list_init(&rwlock->queue);
  rwlock->holder = 0;
  rwlock->init = 1;
  return 0;
}

/**
 * @brief Reader lock.
 * 
 * Reader thread waits on the lock as long as the queue is not empty or the 
 * current holder is a writer
 * 
 * @param rwlock Pointer to initialized rwlock.
 * @param waiting_data Pointer to struc holding waiting thread's data.
 */
static void reader_lock(rwlock_t *rwlock, waiting_thr_data_t *waiting_data)
{
  if (!list_empty(&rwlock->queue) || rwlock->holder < 0) {
    list_add_tail(&rwlock->queue, &waiting_data->list_entry);
    deschedule(&waiting_data->about_to_be_runnable);
    return;
  } else {
    rwlock->holder++;
    return;
  }
}

/**
 * @brief Writer lock.
 * 
 * Writer threads waits on the lock as long as the queue is not empty or there
 * is a current holder.
 * 
 * @param rwlock Pointer to initialized rwlock.
 * @param waiting_data Pointer to struc holding waiting thread's data.
 */
static void writer_lock(rwlock_t *rwlock, waiting_thr_data_t *waiting_data)
{
  if (!list_empty(&rwlock->queue) || rwlock->holder != 0) {
    list_add_tail(&rwlock->queue, &waiting_data->list_entry);
    deschedule(&waiting_data->about_to_be_runnable);
    return;
  } else {
    rwlock->holder = -waiting_data->tid;
    return;
  }
}
/**
 * @brief Acquires the rwlock in a specific mode.
 *
 * @param rwlock Pointer to initialized rwlock.
 * @param type Either RWLOCK_READ or RWLOCK_WRITE
 */
void rwlock_lock(rwlock_t *rwlock, int type)
{
  waiting_thr_data_t data;
  data.tid = gettid();
  data.about_to_be_runnable = 0;
  data.type = type;

  mutex_lock(&rwlock->data);
  assert(rwlock->init);
  assert(type == RWLOCK_READ || type == RWLOCK_WRITE);
  if (type == RWLOCK_READ) {
    reader_lock(rwlock, &data);
  } else {
    writer_lock(rwlock, &data);
  }
  mutex_unlock(&rwlock->data);
}

/**
 * @brief Reader unlock.
 * 
 * The wake up policy is as follows:
 *   1. If there are other readers holding the lock, do nothing.
 *   2. If no readers holding the lock,  wake up one waiting writer if there is 
 *      one. In the absence of waiting writers, wake up all waiting readers.
 *
 * @param rwlock Pointer to initialized rwlock.
 */
static void reader_unlock(rwlock_t *rwlock)
{
  waiting_thr_data_t *next_in_line;
  list_ptr entry;

  /* Other readers still holding the lock */
  if(rwlock->holder > 1) {
    rwlock->holder--;
    return;
  }

  /* We are last reader standing */
  rwlock->holder--;
  if (list_empty(&rwlock->queue)) {
    return;
  } else {
    /* One lone writer waiting */
    entry = list_remv_head(&rwlock->queue);
    next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
    assert(next_in_line->type == RWLOCK_WRITE);
    next_in_line->about_to_be_runnable = 1;
    rwlock->holder = -next_in_line->tid;
    make_runnable(next_in_line->tid);
  }
}

/**
 * @brief Writer unlock.
 *
 * The wake up policy is as follows:
 *   1. Wake up one waiting writer if there is one.
 *   2. Wake up as many readers as you can if no writer waiting. 
 *
 * @param rwlock Pointer to initialized rwlock.
 */
static void writer_unlock(rwlock_t *rwlock)
{
  waiting_thr_data_t *next_in_line;
  list_ptr entry;

  rwlock->holder = 0;

  /* Quiet day at the office */
  if (list_empty(&rwlock->queue)) {
    return;
  }

  entry = list_remv_head(&rwlock->queue);
  next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);

  /* Wake up one writer */
  if (next_in_line->type == RWLOCK_WRITE) {
    rwlock->holder = -gettid();;
    next_in_line->about_to_be_runnable = 1;
    make_runnable(next_in_line->tid);
    return;
  }

  /* Or all readers */
  while (next_in_line && next_in_line->type == RWLOCK_READ) {
    next_in_line->about_to_be_runnable = 1;
    rwlock->holder++;
    make_runnable(next_in_line->tid);
    entry = list_remv_head(&rwlock->queue);
    next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
  }
}

/**
 * @brief Releases the rwlock and wake up waiting threads appropriately.
 * @param rwlock Pointer to initialized rwlock.
 */
void rwlock_unlock(rwlock_t *rwlock)
{
  mutex_lock(&rwlock->data);
  assert(rwlock->init);	
  assert(rwlock->holder != 0);
  if (rwlock->holder < 0) {
    writer_unlock(rwlock);
  } else {
    reader_unlock(rwlock);
  }
  mutex_unlock(&rwlock->data);
}

/**
 * @brief Deactivates the rwlock.
 * @param rwlock Pointer to initialized rwlock.
 */
void rwlock_destroy(rwlock_t *rwlock)
{
  /*assert on the illegal state*/
  mutex_lock(&rwlock->data);
  assert(rwlock->holder == 0);
  rwlock->init = 0;
  mutex_unlock(&rwlock->data);
  mutex_destroy(&rwlock->data);
}

/**
 * @brief Downgrade from an exclusive access to a share access.
 *
 * Only a writer can downgrade to a read access. At the same time, it wakes up
 * all the waiting readers.
 *
 * @param rwlock Pointer to initialized rwlock.
 */
void rwlock_downgrade(rwlock_t *rwlock)
{
  waiting_thr_data_t *next_in_line;	
  list_ptr entry;

  mutex_lock(&rwlock->data);
  assert(rwlock->init);	
  assert(-rwlock->holder == gettid());
  rwlock->holder = 1;

  /* Wake up all other waiting readers */
  entry = list_remv_head(&rwlock->queue);
  next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
  while (next_in_line && next_in_line->type == RWLOCK_READ) {
    next_in_line->about_to_be_runnable = 1;
    rwlock->holder++;
    make_runnable(next_in_line->tid);
    entry = list_remv_head(&rwlock->queue);
    next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
  }	

  mutex_unlock(&rwlock->data);
}
