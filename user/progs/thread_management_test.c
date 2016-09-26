/** 
 * @file user/progs/thread_management_test.c
 * @author X.D. Zhai (xingdaz)
 * @brief 
 */

#include <syscall.h>
#include <simics.h>

int 
main(int argc, char *argv[])
{
  ureg_t pseudo_ureg;
  int my_tid, pseudo_tid, reject;
  unsigned before_sleep_ticks, post_sleep_ticks;
  my_tid = gettid();
  lprintf("My thread id = %d\n", my_tid);
  pseudo_tid = -2;
  if (yield(pseudo_tid) < 0)
    lprintf("Can't yield to nonexisting thread tid = %d\n", pseudo_tid);

  reject = 1;
  deschedule(&reject);

  if (make_runnable(my_tid) < 0)
    lprintf("Can't make tid = %d runnable. It wan't descheduled\n", my_tid);

  before_sleep_ticks = get_ticks();
  lprintf("%u ticks have elapsed since system booted\n", before_sleep_ticks);

  sleep(100);
  post_sleep_ticks = get_ticks();

  lprintf("Just slept for %d ticks\n", post_sleep_ticks - before_sleep_ticks);

  if (swexn(0x0, 0x0, 0x0, &pseudo_ureg) < 0)
    lprintf("You can't register bullshit\n");
}
