/**
 * @file mutex_type.h
 * @brief This file defines the type for mutexes.
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */
#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H

typedef struct mutex {
  volatile int next;  /* The ticket a newcomer will take */
  volatile int owner; /* "Now Serving" number */
} mutex_t;

#endif /* _MUTEX_TYPE_H */
