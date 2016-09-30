/**
 * @file rwlock_xd.c
 * @brief  Implementation of readers/writers lock API as specified in 
 *         410user/inc/rwlock.h
 * @author X.D. Zhai (xingdaz)
 * @author Zhan Chen (zhanc1)
 */

/* Public APIs */
#include <rwlock.h>
#include <rwlock_type.h> /* rwlock_t */
#include <mutex.h>
#include <cond.h>
#include <simics.h>

/**
 * @brief Reader lock.
 * 
 * Reader thread waits on the lock as long as there is writer holding the lock 
 * currently or there are writers waiting to acquire it.
 * 
 * @param rwlock Pointer to initialized rwlock.
 */
static void
read_lock(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->data);

    while (rwlock->current_holder == RWLOCK_WRITE || 
           rwlock->num_waiting_writers) {
        rwlock->num_waiting_readers++;
        cond_wait(&rwlock->can_read, &rwlock->data);
        rwlock->num_waiting_readers--;
    }

    /* Has the lock finally. */
    rwlock->current_holder = RWLOCK_READ;
    rwlock->num_lock_holding_readers++;

    mutex_unlock(&rwlock->data);
}

/**
 * @brief Reader unlock.
 * 
 * The wake up policy is as follows:
 *   1. If there are other readers holding the lock, do nothing.
 *   2. If no readers holding the lock,  wake up one waiting writer if there is 
 *   one. In the absence of waiting writers, wake up all waiting readers.
 *
 * @param rwlock Pointer to initialized rwlock.
 */
static void
read_unlock(rwlock_t *rwlock)
{
    rwlock->num_lock_holding_readers--;
    if (!rwlock->num_lock_holding_readers) {
        if (rwlock->num_waiting_writers)
            cond_signal(&rwlock->can_write);
        else
            cond_broadcast(&rwlock->can_read);
    }
}

/**
 * @brief Writer lock.
 *
 * Writer thread waits till no one is holding the lock.
 *
 * @param rwlock
 */
static void
write_lock(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->data);
    while (rwlock->num_lock_holding_readers || 
           rwlock->current_holder == RWLOCK_WRITE) {
        rwlock->num_waiting_writers++;
        cond_wait(&rwlock->can_write, &rwlock->data);
        rwlock->num_waiting_writers--;
    }
    rwlock->current_holder = RWLOCK_WRITE;
    mutex_unlock(&rwlock->data);
}

/**
 * @brief Writer unlock.
 *
 * The wake up policy is as follows:
 *   1. Wake up one waiting writer.
 *   2. Wake up all waitiner readers if no writer is waiting.
 * @param rwlock
 */
static void
write_unlock(rwlock_t *rwlock)
{
    if (rwlock->num_waiting_writers)
        cond_signal(&rwlock->can_write);
    else {
        rwlock->current_holder = NO_ONE;
        cond_broadcast(&rwlock->can_read);
    }
}

/**
 * @brief Initialize rwlock data structure.
 *
 * @param rwlock Allocated but unitialized rwlock.
 *
 * @return 0 on succcess and negative number on failture.
 */
int
rwlock_init(rwlock_t *rwlock)
{
    /* Failure: null pointer */
    if (!rwlock)
        return -1;

    /* Failure: can't initialize conditional variables */
    if (cond_init(&rwlock->can_read) < 0)
        return -2;
    if (cond_init(&rwlock->can_write) < 0)
        return -3;

    /* Failure: can't initialize mutex */
    if (mutex_init(&rwlock->data) < 0)
        return -4;

    /* Success: initialize the state */
    rwlock->current_holder = NO_ONE;
    rwlock->num_lock_holding_readers = 0;
    rwlock->num_waiting_readers = 0;
    rwlock->num_waiting_writers = 0;
    return 0;
}

/**
 * @brief Acquires the rwlock in a specific mode.
 *
 * @param rwlock Pointer to initialized rwlock.
 * @param type Either RWLOCK_READ or RWLOCK_WRITE
 */
void
rwlock_lock(rwlock_t *rwlock, int type)
{
    switch (type) {
    case RWLOCK_READ:
        read_lock(rwlock);
        break;
    case RWLOCK_WRITE:
        write_lock(rwlock);
        break;
    default:
        break;
    }
    return;
}

/**
 * @brief Releases the rwlock and wake up waiting threads appropriately.
 *
 * @param rwlock Pointer to initialized rwlock.
 */
void
rwlock_unlock(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->data);
    switch (rwlock->current_holder) {
    case RWLOCK_READ:
        read_unlock(rwlock);
        break;
    case RWLOCK_WRITE:
        write_unlock(rwlock);
        break;
    default:
        break;
    }
    mutex_unlock(&rwlock->data);
}

/**
 * @brief Deactivates the rwlock.
 *
 * @param rwlock Pointer to initialized rwlock.
 */
void
rwlock_destroy(rwlock_t *rwlock)
{
    cond_destroy(&rwlock->can_read);
    cond_destroy(&rwlock->can_write);
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
void
rwlock_downgrade(rwlock_t *rwlock)
{
    mutex_lock(&rwlock->data);

    if (rwlock->current_holder == RWLOCK_WRITE) {
        /* Switch mode */
        rwlock->current_holder = RWLOCK_READ;
        /* Add yourself to the readers */
        rwlock->num_lock_holding_readers++;
        cond_broadcast(&rwlock->can_read);
    }

    mutex_unlock(&rwlock->data);
}
