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


/* malloc two objects simultaneously, return negative in case any of them fails */
int double_malloc(void **dest1, size_t __size1, void **dest2, size_t __size2){
	void *ret1, *ret2;
	int ret = 0;
	THREAD_SAFE_ENTRY;
	ret1 = _malloc(__size1);
	if(ret1 != NULL){
		ret2 = _malloc(__size2);
		if(ret2 != NULL){
			*dest1 = ret1;
			*dest2 = ret2;
		}else{
			_free(ret1);
			ret = -2;
		}
	}else{
		ret = -1;
	}
	THREAD_SAFE_EXIT;
	return ret;
}

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
