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
	list_t queue;
/* reader = 0: available 
 * > 0: # of readers
 * < 0: in exclusive state, and is -tid of the holder
 * */
	int reader;
	mutex_t qr_mutex;
	int init_flag;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
