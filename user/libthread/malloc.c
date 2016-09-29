/*
 * these functions should be thread safe.
 * It is up to you to rewrite them
 * to make them thread safe.
 *
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>
#include <asm_internals.h>
#include <malloc.h>
#include <syscall.h>
static mutex_t big_lock;
static int thread_safe = 0;
#define THREAD_SAFE_ENTRY do{\
			    if(thread_safe == 0){\
				   int tid = gettid();\
			       if(cmpxchg(&thread_safe, 0, tid)){\
			       	mutex_init(&big_lock);\
			       }else{\
					yield(thread_safe);\
				   }\
			    }\
			    mutex_lock(&big_lock);\
			   }while(0)

#define THREAD_SAFE_EXIT mutex_unlock(&big_lock)

void *malloc(size_t __size)
{
  void *ret;
  THREAD_SAFE_ENTRY;
  ret = _malloc(__size);
  THREAD_SAFE_EXIT;
  return ret;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
  void *ret;
  THREAD_SAFE_ENTRY;
  ret = _calloc(__nelt, __eltsize);
  THREAD_SAFE_EXIT;
  return ret;
}

void *realloc(void *__buf, size_t __new_size)
{
  void *ret;
  THREAD_SAFE_ENTRY;
  ret = _realloc(__buf, __new_size);
  THREAD_SAFE_EXIT;
  return ret;
}

void free(void *__buf)
{
  THREAD_SAFE_ENTRY;
  _free(__buf);
  THREAD_SAFE_EXIT;
}
