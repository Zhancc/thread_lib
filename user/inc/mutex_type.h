/**
 * @file mutex_type.h
 * @brief This file defines the type for mutexes.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

/**
 * @brief This is a plain vanilla ticket lock.
 *
 * TODO do we need volatile qualifier?
 * We should have 2 more variables, "initizliaed" and "destroyed". 
 */
typedef struct mutex {
    volatile int next;  /* The ticket a newcomer takes */
    volatile int owner; /* The "Now Serving" number */
	int locked_flag;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
