/** 
 * @file user/progs/life_cycle_test.c
 * @author X.D. Zhai (xingdaz)
 * @brief Wrapper tests for fork, exec, set_status, vanish, wait and task_vanish.
 *
 * Parent forks a child and wait on the child to exit.
 */

#include <syscall.h>
#include <simics.h>

#define NULL 0x0

int 
main(int argc, char *argv[])
{
  int task_id;
  int child_thread_id;
  int child_exit_status;
  char *exec_prog = "virgin";
  char *exec_argv[2];
  int exec_status;


  lprintf("Parent -- Prefork\n");
  if ((task_id = fork())) {
    /* In parent */
    lprintf("Parent -- Forked a child w/ tid = %d\n", task_id);
    if ((child_thread_id = wait(&child_exit_status)) < 0)
      lprintf("Parent -- Failed to wait on child.");
    else
      lprintf("Parent -- Reaped child tid = %d with exit status %d\n", 
            child_thread_id, child_exit_status);
  } else { 
    /* In child */
    lprintf("Child  -- Going to exec virgin\n");
    exec_argv[0] = exec_prog;
    exec_argv[1] = NULL;
    if ((exec_status = exec(exec_prog, exec_argv)) < 0) {
      lprintf("Child  -- Error in exec\n");
      set_status(-99);
      vanish();
    }
  }
  task_vanish(69);
}
