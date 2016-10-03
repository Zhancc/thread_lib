/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H
#include <cond_type.h>

typedef struct sem {
  /* fill this in */
	int cnt;
	cond_t cv;
	mutex_t cv_mutex;
	int init_flag;
	int waiting;
} sem_t;

#endif /* _SEM_TYPE_H */
