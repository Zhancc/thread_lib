/**
 * @file cond_type.h
 * @brief This file defines the type for condition variables.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <mutex_type.h> /* mutex_t */
#include <list.h>       /* list_t */

/**
 * @brief This is a structure that contains a queue into which thread can insert
 *        their list entry and a lock for the queue. 
 *
 * TODO We should have 2 more variables, "initizliaed" and "destroyed".
 */
typedef struct cond {
    mutex_t qmutex; /* Lock around the queue */
    list_t queue;   /* The queue containing the list entry of threads that are 
                       waiting on this particular condition */
	int init_flag;  /* 1 when it's initialized */
} cond_t;

#endif /* _COND_TYPE_H */
