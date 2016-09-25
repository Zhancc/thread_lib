/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H


typedef struct cond {
  /* fill this in */
	mutex_t cmutex;

} cond_t;

#endif /* _COND_TYPE_H */