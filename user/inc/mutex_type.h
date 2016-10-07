/**
 * @file mutex_type.h
 * @brief This file defines the type for mutexes.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */
#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

/**
 * @brief This is a plain vanilla ticket lock.
 */
typedef struct mutex {
  int next;   /* the ticket a newcomer takes */
  int owner;  /* the "Now Serving" number */
	int locked; /* indicates some thread is holding the lock */
  int init;   /* indicates the object has been initialized */
} mutex_t;

#endif /* _MUTEX_TYPE_H */
