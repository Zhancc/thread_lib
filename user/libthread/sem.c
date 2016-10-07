/**
 * @file sem.c
 * @brief Implementation of semomphore API as defined in 410user/inc/sem.h.
 * @author Zhan Chen (zhanc1) 
 * @author X.D. Zhai (xingdaz) 
 */

#include <sem.h>
#include <sem_type.h>   /* sem_t */
#include <mutex.h>      /* mutex_init(), mutex_lock() and mutex_unlock() */
#include <cond.h>       /* cond_wait() and cond_signal() */
#include <assert.h>     /* assert() */

/**
 * @brief Initialize semaphore struct.
 * @param sem Pointer to allocated but unintialized sem_t data.
 * @param count Initial value.
 * @return 0 on success; negative value on failure.
 */
int sem_init(sem_t *sem, int count)
{
  if (!sem || count < 0)
    return -1;
  if(cond_init(&sem->none_neg) < 0 )
    return -2;
  if(mutex_init(&sem->sem_data) < 0)
    return -3;

  sem->cnt = count;
  sem->init = 1;
  sem->waiting = 0;
  return 0;
}

/**
 * @brief Decrement counter and wait on the coundition if it is negative.
 * @param sem Pointer to allocated but unintialized sem_t data.
 */
void sem_wait(sem_t *sem)
{  
  mutex_lock(&sem->sem_data);
  assert(sem->init);
  while (sem->cnt <= 0) {
    sem->waiting++;
    cond_wait(&sem->none_neg, &sem->sem_data);
  }
  sem->waiting--;
  sem->cnt--;
  mutex_unlock(&sem->sem_data);
}

/**
 * @brief sem_signal Increment counter and wait up (1) waiting thread.
 * @param sem Pointer to allocated but unintialized sem_t data.
 */
void sem_signal(sem_t *sem)
{
  mutex_lock(&sem->sem_data);
  assert(sem->init);
  if (sem->waiting)
    cond_signal(&sem->none_neg);
  sem->cnt++;
  mutex_unlock(&sem->sem_data);
}

/**
 * @brief Deactives the semaphore.
 * @param sem Pointer to allocated but unintialized sem_t data.
 */
void sem_destroy(sem_t *sem)
{
  mutex_lock(&sem->sem_data);
  assert(sem->waiting == 0);
  sem->cnt = 0;
  sem->init = 0;
  mutex_unlock(&sem->sem_data);
  mutex_destroy(&sem->sem_data);
  cond_destroy(&sem->none_neg);
}
