/** 
 * @file rwlock_type.h
 * @brief This file defines the type for reader/writer locks.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <list.h>
#include <mutex.h>
#include <cond.h>

#define NO_ONE (-1)

typedef struct rwlock {
	mutex_t data; /* mutex for accessing this data structure */
	list_t queue; /* queue of readers and writers */
	int holder;   /* indicates the state and holder of the lock.
                 * holder = 0: available 
                 *        > 0: in shared state, indicates # of readers
                 *        < 0: in exclusive state, and is -tid of the writer
                 */
	int init;     /* indicates if the lock has been initialized */
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
