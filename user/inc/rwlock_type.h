/** 
 * @file rwlock_type.h
 * @brief This file defines the type for reader/writer locks.
 * @author X.D. Zhai (xingdaz)
 * @author Zhan Chen (zhanc1)
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <list.h>
#include <mutex.h>
#include <cond.h>

#define NO_ONE (-1)

typedef struct rwlock {
    int current_holder;
    int num_lock_holding_readers;
    int num_waiting_readers;
    int num_waiting_writers;
    cond_t can_read;
    cond_t can_write;
    mutex_t data;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
