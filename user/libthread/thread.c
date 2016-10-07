/**
 * @file thread.c
 * @brief Implementions of the thread APIs specified in `410user/inc/thread.h`
 *
 * Here is the life cycle of a thread, from creation, to exiting, and to 
 * joining.
 *
 * 1. Malloc space for stack and TCB. Size of the stack is rounded up to 
 *    multiples of `PAGE_SIZE`. `stack_high` is aligned to 4 byte. Though 
 *    highly unlikely, there might be some unused space as padding before TCB
 *    as a result of the alignment. That will only happen if page size is not
 *    multiples of 2 in which case `malloc()` doesn't return 4 byte aligned 
 *    address.
 * 2. Populate the data field in TCB.
 * 3. Setup calling stack for `thread_fork_wrapper()`. Please reference
 *    `asm.S`. When in `thread_fork_wrapper()`, and before 
 *    invoking `peer_thr_init()`, the stack looks as follows. `%ebp` is set 
 *    above `stack_high` at TCB for reason that will become clear later.
 *
 *                 Higher Address
 *              --------------------
 *              |                  |
 *              |       TCB        |
 *              |                  |
 *          +-->-------------------- <-- %ebp
 *          |   | Possible Padding |
 *    ----  |   -------------------- <-- stack_high (4 byte aligned)
 *     ^    |   |       args       |
 *     |    |   --------------------
 *          |   |default_exit_entry|
 *          |   --------------------
 *          |   |       func       |
 *          |   --------------------
 *    mult. +---+------ tcb        |
 *     of       -------------------- <-- %esp
 *    PAGE      |                  |
 *    SIZE      |                  |
 *              .                  .
 *              .                  .
 *     |        .                  .
 *     V        |                  |
 *    ----      -------------------- <-- stack_low
 *                 Lower Address
 *
 * 4. New thread lands in `peer_thr_init()` where it deschedules itself to wait
 *    for invoking thread to fill in its TID in TCB. Upon wake up, it registers
 *    an exception handler that vanish the the whole task.
 * 5. Upon `peer_thr_init()`'s return, but before `thread_fork_wrapper()` 
 *    exits, the stack looks as follows. 
 *    
 *                 Higher Address
 *              --------------------
 *              |                  |
 *              |       TCB        |
 *              |                  |
 *              -------------------- <-- %ebp
 *              | Possible Padding |
 *    ----      -------------------- <-- stack_high (4 byte aligned)
 *     ^        |       args       |
 *     |        --------------------
 *              |default_exit_entry|
 *              --------------------
 *              |       func       |
 *    mult.     -------------------- <-- %esp
 *     of       |                  |               
 *    PAGE      |                  |
 *    SIZE      |                  |
 *              .                  .
 *              .                  .
 *     |        .                  .
 *     V        |                  |
 *    ----      -------------------- <-- stack_low
 *                 Lower Address
 *
 * 6. When `thread_fork_wrapper()` returns, `func` will be pop'ed into `%eip`
 *    and starts to execute. Upon entering `func`, the stack predicably looks 
 *    as follows, courtesy of the function setup preamble the compiler puts in. 
 *
 *    push  %ebp;
 *    mov   %esp,%ebp;
 *
 *                 Higher Address
 *              --------------------
 *              |                  |
 *              |       TCB        |
 *              |                  |
 *          +-->--------------------
 *          |   | Possible Padding |
 *    ----  |   -------------------- <-- stack_high (4 byte aligned)
 *     ^    |   |       args       |
 *     |    |   --------------------
 *          |   |default_exit_entry|
 *          |   --------------------
 *          +---+--- saved %ebp    |
 *    mult.     -------------------- <-- %ebp <-- %esp
 *     of       |                  |
 *    PAGE      |                  |
 *    SIZE      |                  |
 *              |                  |
 *              .                  .
 *              .                  .
 *     |        .                  .
 *     V        |                  |
 *    ----      -------------------- <-- stack_low
 *                 Lower Address
 *
 * 7. When `func` is executing, we are less certain about where `%ebp` and 
 *    `%esp` are located.
 * 8. If `thr_exit()` is called before `func` returns, we need to get to TCB
 *    to populate the `ret` there. We simply trace `%ebp` back until it is 4 
 *    bytes below the `default_exit_entry()`.
 * 9. If `thr_exit()` is not called and the `func` returns normally, the thread
 *    will land in `default_exit_entry()`, please refer to `asm.S`. It will 
 *    invoke `default_exit()`, upon whose entering, the stack looks as follows.
 *    `default_exit_entry()` is pushed onto the stack again simply as a way for
 *    `get_tcb()` to be able to find the pointer to TCB. In `default_exit()`, 
 *    `thr_exit()` will be called.
 *
 *                 Higher Address
 *              --------------------
 *              |                  |
 *              |       TCB        |
 *              |                  |
 *          +-->--------------------
 *          |   | Possible Padding |
 *    ----  |   -------------------- <-- stack_high (4 byte aligned)
 *     ^    |   |       args       |
 *     |    |   --------------------
 *          |   |ret value of func |
 *          |   --------------------
 *          |   |default_exit_entry|
  *         |   --------------------
 *          +---+--- saved %ebp    |
 *    mult.     -------------------- <-- %ebp <-- %esp
 *     of       |                  |
 *    PAGE      |                  |
 *    SIZE      |                  |
 *              .                  .
 *              .                  .
 *     |        .                  .
 *     V        |                  |
 *    ----      -------------------- <-- stack_low
 *                 Lower Address
 *
 * 10. If a thread wants to join a particular thread, identified by a TID,
 *     it can start looking for that particular TID starting from the head
 *     entry located in the shared gstate structure. When found, it will
 *     use the offset to locate the first byte of TCB and access the ret of
 *     that thread. Lastly, it will free the TCB, completely cleans up the
 *     joined thread's mess.
 *
 * @author Zhan Chen (zhanc1)
 * @author X.D. Zhai (xingdaz)
 */

/* Public APIs */
#include <thread.h>
#include <syscall.h>        /* PAGE_SIZE, swexn(), panic() */
#include <stddef.h>         /* NULL */
#include <assert.h>         /* assert() */
#include <malloc.h>         /* malloc() */
#include <cond.h>           /* cond_t, cond_wait(), and cond_signal() */
#include <list.h>           /* list_t */
#include <mutex.h>          /* mutex_t, mutex_lock(), and mutex_unlock() */

/* Private APIs */
#include "malloc_internals.h" /* double_malloc() */
#include "thr_internals.h"    /* tcb_t, thread_fork_wrapper(), 
                                 peer_thread_init(), and _main_ebp */
#include "swexn_handler.h"    /* root_pagefault_arg */

/**
 * @brief Global data structure root thread keeps.
 */
struct {
	unsigned int stack_size;  /* Declared stack size for each peer thread */
	list_t tcb_list;          /* Dummy head into the list of TCB  */
	mutex_t tcb_lock;         /* Lock to this data strucure */
  int root_tid;             /* Root thread's TID */
} gstate;

/**
 * @brief Exception handler for each peer thread.
 *
 * Any kind of software exception will cause the whole task to vanish. The 
 * error is, most of the time, irrecoverable b/c the thread could be holding 
 * locks, in the middle of modifying shared data structure or working on to 
 * produce a result that other thread may be depending on. 
 *
 * @param arg Pointer to argument structure. Not used here.
 * @param ureg Pointer to ureg structure. Not used here.
 */
static void peer_thr_swexn_handler(void *arg, ureg_t *ureg)
{
	panic("Peer thread encountered general protection fault.\n");
}

/**
 * @brief Backtrace through %ebp to get to pointer to TCB. 
 * @return Pointer to TCB. Panic if failure.
 */
static tcb_t *get_tcb() 
{
	void **ebp;
  ebp = get_ebp();

  /* As long as we have not reached %ebp 4 byte below default_exit_entry */
	while(*(ebp + 1) != default_exit_entry) {
		ebp = *ebp;
	}

	return *(tcb_t **)ebp;
}

/**
 * @brief If a thread doesn't call thr_exit(), it will eventually land here.
 * @param Never returns.
 */
void default_exit(void *ret)
{
  /* set_status is only useful for root thread return */
	set_status((int) ret);
	thr_exit(ret);
}

/**
 * @brief New thread's landing point.
 *
 * New thread deschedule's itself first, waiting for its TID to be filled in
 * by the invoking thread. Upon wake up, it registers an exception handler 
 * that kills the whole task if any kind of software exception is encountered
 * in the future. 
 *
 * @param tcb Pointer to its TCB.
 */
void peer_thread_init(tcb_t *tcb) {
	void *handler_esp3;
  deschedule(&tcb->tid);
  handler_esp3 = malloc(SWEXN_STACK_SIZE + ESP3_OFFSET);
  if (swexn(handler_esp3, peer_thr_swexn_handler, NULL, NULL) < 0)
      thr_exit((void *) -1);
}

/**
 * @brief Initializes the whole thread library for this task.
 *
 * This function is called exacly once before any other thread library 
 * functions are invoked. One thing that needs to point out is that we don't
 * want to treat the root thread any differently than the other peer threads.
 * So we want it to have the same exit behavior as the rest of them. To do so,
 * we will have to form the stack the same way we do for the rest of them. At
 * the time of `thr_init()`, `_main()` is on top of the calling chain with the
 * stack looking like the following. `_main_ebp` is saved when
 * `install_autostack()` was called.
 * 
 *    exit(main(argc, argv));
 *         
 *                 Higher Address
 *              .                  .
 *              .                  .
 *              |                  |
 *              --------------------
 *              |    stack_low     |
 *              --------------------
 *              |    stack_high    |
 *              --------------------
 *              |       argv       |
 *              --------------------
 *              |       argc       |
 *              --------------------
 *              |       ret        |
 *              --------------------
 *              |    saved %ebp    |
 *          +-->-------------------- <-- _main_ebp
 *          |   |       argv       |
 *          |   --------------------
 *          |   |       argc       |
 *          |   --------------------
 *          |   |       ret        |
 *          |   --------------------
 *          +---+-------           |
 *              .                  .
 *              .                  .
 *                 Lower Address
 *
 * After `thr_init()`, the top of the calling chain looks like follows. As a 
 * result, `main()` returns to `default_exit_entry()`.
 *
 *                 Higher Address
 *              .                  .
 *              .                  .
 *              |                  |
 *              -------------------- <-- _main_ebp
 *              |       argv       |                    Root thread TCB
 *              --------------------         +--------> -----------
 *              |       argc       |         |          |         |
 *              --------------------         |          |         |
 *              |default_exit_entry|         |          |         |
 *              --------------------         |          -----------
 *              |       tcb  ------+---------+  
 *              --------------------
 *              |                  |              
 *              .                  .              
 *              .                  .
 *                 Lower Address
 *
 * @param size Amount of stack space needed for each thread.
 * @return 0 on success, negative number on error.
 */
int thr_init(unsigned int size) {
	void **ebp;

  /* Quit f***ing w/ me */
  if (!size)
    return -1;
	if(mutex_init(&gstate.tcb_lock) < 0)
		return -2;
	gstate.stack_size = size;
	list_init(&gstate.tcb_list);

  /* Insert the root thread's TCB into the list */
	tcb_t *root_tcb = (tcb_t *) malloc(sizeof(tcb_t));
	if(root_tcb == NULL)
		return -3;
  if(cond_init(&root_tcb->exited) < 0)
    return -4;

  /* Coast is clear and we are set to limit the size of the root thread's 
   * stack growth. */
  root_pagefault_arg->fixed_size = size;

	root_tcb->tid = gettid();
  gstate.root_tid = root_tcb->tid;
	root_tcb->joined = FALSE;
	root_tcb->status = STATUS_RUNNING;
	root_tcb->stack_low = root_tcb;
	list_init(&root_tcb->tcb_entry);
	list_add_tail(&gstate.tcb_list, &root_tcb->tcb_entry);

  /* Overwrite root threat's return address and tcb address */
	ebp = get_ebp();
	ebp = (void **)*ebp;
	while( ((void **)*ebp) != _main_ebp)
		ebp = *ebp;
	*(ebp++) = root_tcb;
	*(ebp) = default_exit_entry;
	
	return 0;
}

/**
 * @brief Creates a new thread to run func. 
 * @param func Pointer to the thread function body.
 * @param args Opaque data type.
 * @return New thread's TID.
 */
int thr_create(void *(*func)(void *), void *args) {
  unsigned int stack_size;
  int thr_tid;
  void *stack_low, *stack_high, *thr_esp;
  tcb_t *thr_tcb;

  /* Validate input */
  if (!func)
    return -1;

  /* Round up stack_size to to be multiples of PAGE_SIZE */ 
  stack_size = (gstate.stack_size + PAGE_SIZE - 1) / (PAGE_SIZE) * PAGE_SIZE;

  /* Double malloc gurantees TCB will be on top of the stack cause they are
   * invoked in the same critical section. */
  if (double_malloc((void *) &stack_low, stack_size, 
                    (void *) &thr_tcb, sizeof(tcb_t)) < 0)
    return -2;

  /* Peer thread's %esp has to be 4 byte aligned */
  stack_high = (void *)((int) ((char *) stack_low + stack_size) & 
                         STACK_ALIGNMENT_MASK);
  thr_esp = stack_high;

  /* Populate peer threat's TCB */
  /* We are assuming that we will not get tid 0. Therefore, we can use it as 
   * an indicator to see if it is safe to deschedule itself. */
  if (cond_init(&thr_tcb->exited) < 0)
    return -3;
	thr_tcb->tid = 0; 
	thr_tcb->status = STATUS_RUNNING;
	thr_tcb->joined = FALSE;
	list_init(&thr_tcb->tcb_entry);
	thr_tcb->stack_high = stack_high;
	thr_tcb->stack_low = stack_low;
  
  /* Prepare the calling stack for thread_fork. */
	thr_esp -= 4;
	*(void **)thr_esp = (void *)args;
	thr_esp -= 4;
	*(void **)thr_esp = (void *)default_exit_entry;
	thr_esp -= 4;
	*(void **)thr_esp = (void *)func;
	thr_esp -= 4;
	*(tcb_t **)thr_esp = thr_tcb;

  /* Trap into the system call */
	thr_tid = thread_fork_wrapper(thr_esp, thr_tcb);
	if(thr_tid < 0){
		free(stack_low);
		free(thr_tcb);
		return -4;
	}

  /* Populate the tid field of the peer threat's TCB and insert it into the
   * list */
	assert(thr_tid != 0);
	thr_tcb->tid = thr_tid;
	mutex_lock(&gstate.tcb_lock);
	list_add_tail(&gstate.tcb_list, &thr_tcb->tcb_entry);
	mutex_unlock(&gstate.tcb_lock);
	make_runnable(thr_tid);
	return thr_tid;
}

/**
 * @brief Wait on the target thread to exit.
 * @param tid Target tid to wait on.
 * @param statusp Pointer to address where target thread's exit status
 * @return 
 */
int thr_join(int tid, void **statusp) {
	tcb_t *tcb;
	list_ptr entry;
	int ret;

  entry = &gstate.tcb_list;
	mutex_lock(&gstate.tcb_lock);

	for (entry = entry->next; entry != &gstate.tcb_list; entry = entry->next) {
		tcb = LIST_ENTRY(entry, tcb_t, tcb_entry);
		if (tcb->tid == tid) {
			break;
		}
    /* Need to make sure that in the event that we haven't found the tid,
     * tcb is NULL */
		tcb = NULL;
	}

  /* Failure: Haven't found it, go home */
	if (!tcb) {
		ret = -2;
		goto Exit;
	}

  /* Failure: Some other thread have claimed this thread, go home */
	if (tcb->joined == TRUE) {
		ret = -1;
		goto Exit;
	}

  /* Success: We claim this thread */
	tcb->joined = TRUE;
	ret = 0;
	while (tcb->status != STATUS_EXITED) {
		cond_wait(&tcb->exited, &gstate.tcb_lock);
	}
  
  if (statusp)
    *statusp = tcb->ret;

  /* Housekeeping */
	list_remv(entry);
	free(tcb);

Exit:
	mutex_unlock(&gstate.tcb_lock);
	return ret;
}

/**
 * @brief Exits the thread with status.
 *
 * We don't have to validate status here b/c it is not a pointer. It is 
 * a pointer-sized opaque data type thr_exit transports uninterpreted to 
 * the ret member of the TCB.
 *
 * @param status Pointer-sized opaque data type.
 */
void thr_exit(void *status)
{
	tcb_t *tcb = get_tcb();

	mutex_lock(&gstate.tcb_lock);
	assert(tcb != NULL);
	tcb->ret = status;
	tcb->status = STATUS_EXITED;
	if(tcb->joined == TRUE){
		cond_signal(&tcb->exited);
	}
	mutex_unlock(&gstate.tcb_lock);

  /* Leave TCB alone because thr_join might be called later. Also, if root
   * thread gets here, we don't free its stack b/c it is not ours to free */
  if (tcb->tid != gstate.root_tid)
		free(tcb->stack_low);

  /* Vanish thyself */
	vanish();
}

/**
 * @brief Returns the thread id of the invoking thread.
 * @return tid of invoking thread.
 */
int thr_getid(void) 
{
	return get_tcb()->tid;
}

/**
 * @brief Defers the execution of thread w/ tid to a later time.
 * @param tid tid of thread to be deferred.
 * @return 0 on success, negative on failure.
 */
int thr_yield(int tid) 
{
	return yield(tid);
}
