/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <list.h>
#include <mutex.h>

typedef struct rwlock {
	list_t queue;
/* reader = 0: available 
 * > 0: # of readers
 * < 0: in exclusive state
 * */
	int reader;
	mutex_t qr_mutex; //protect reader and queue
	int init_flag;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
