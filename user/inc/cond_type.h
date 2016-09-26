/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <mutex_type.h>
#include <list.h>

typedef struct cond {
  /* fill this in */
	mutex_t cmutex;
	list queue;
} cond_t;

#endif /* _COND_TYPE_H */
