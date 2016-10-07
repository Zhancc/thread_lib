/** 
 * @file sem_type.h
 * @brief This file defines the type for semaphores.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */
#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <mutex_type.h> /* mutex_t */
#include <cond_type.h>  /* cond_t */

typedef struct sem {
	mutex_t sem_data;     /* mutex for this data structure */
	cond_t none_neg;      /* conditional variable for cnt to be none negative */
	int cnt;              /* count that gets incremented or decrented */
	int init;             /* indicates if the semaphore is initialized */
	int waiting;          /* indicates if there are waiting threads still */
} sem_t;

#endif /* _SEM_TYPE_H */
