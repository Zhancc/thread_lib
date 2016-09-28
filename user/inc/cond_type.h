/**
 * @file cond_type.h
 * @brief This file defines the type for condition variables.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <mutex_type.h>
#include <list.h>

typedef struct cond {
	mutex_t cmutex; /* Lock around the queue TODO should really call this qlock 
                     as we are using it to lock the queue */
	list queue;     /* The queue containing the tids of threads that are waiting
                     on this particular condition */
} cond_t;

#endif /* _COND_TYPE_H */
