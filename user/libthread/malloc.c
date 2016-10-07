/**
 * @file malloc.c
 * @brief Thread safe versions of malloc and its variant functions.
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

#include <malloc.h>
#include <types.h>          /* size_t */
#include <stddef.h>         /* NULL */
#include <mutex.h>          /* mutex_t, mutex_lock(), and mutex_unlock() */
#include <asm_internals.h>  /* cmpxchg */
#include <syscall.h>        /* yield */

static mutex_t big_lock;  /* The big lock around all the following funcs */
static int thread_safe = 0;

/* TODO comment */
inline static void thread_safe_entry()
{
  int tid;
  if (thread_safe == 0) {
    tid = gettid();
    if (cmpxchg(&thread_safe, 0, tid)) {
      mutex_init(&big_lock);
    } else {
      yield(thread_safe);
    }
  }
  mutex_lock(&big_lock);
}

inline static void thread_safe_exit()
{
  mutex_unlock(&big_lock);
}

int double_malloc(void **dest1, size_t __size1, void **dest2, size_t __size2)
{
  void *ret1, *ret2;
  int ret = 0;
  thread_safe_entry();
  ret1 = _malloc(__size1);
  if (ret1 != NULL) {
    ret2 = _malloc(__size2);
    if (ret2 != NULL) {
      *dest1 = ret1;
      *dest2 = ret2;
    } else {
      _free(ret1);
      ret = -2;
    }
  } else {
    ret = -1;
  }
  thread_safe_exit();
  return ret;
}

void *malloc(size_t __size)
{
  void *ret;
  thread_safe_entry();
  ret = _malloc(__size);
  thread_safe_exit();
  return ret;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
  void *ret;
  thread_safe_entry();
  ret = _calloc(__nelt, __eltsize);
  thread_safe_exit();
  return ret;
}

void *realloc(void *__buf, size_t __new_size)
{
  void *ret;
  thread_safe_entry();
  ret = _realloc(__buf, __new_size);
  thread_safe_exit();
  return ret;
}

void free(void *__buf)
{
  thread_safe_entry();
  _free(__buf);
  thread_safe_exit();
}
