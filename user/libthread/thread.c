/**
 * @file thread.c
 * @brief Implementions of the thread APIs specified in 410user/inc/thread.h
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

/* Public APIs */
#include <thread.h>
#include <syscall.h>        /* PAGE_SIZE */
#include <stddef.h>         /* NULL */
#include <assert.h>         /* assert */
#include <malloc.h>         /* malloc() */
#include <cond.h>           /* cond_t, cond_wait, and cond_signal */
#include <mutex.h>          /* mutex_t, mutex_lock, and mutex_unlock */

/* Private APIs */
#include <thr_internals.h>  /* tcb_t, thread_fork_wrapper, and
                               peer_thread_init */
#include <list.h>           /* list_t */
#include "asm_internals.h"
/**
 * @brief 
 * TODO I am not sure if this is the right behavior because the handout says
 * "should be the same as if the function had called thr_exit() specifying
 * the return value from the thread's body function. "
 */
void default_exit(void *ret){
    thr_exit(ret);
}

/**
 * @brief Structure to keep the entry point to the TCB list.
 */
struct {
	unsigned int stack_size;

	list_t tcb_list;
	mutex_t tcb_lock;
	cond_t tcb_cv;

	//int root_tid;
} gstate;

void peer_thread_init(tcb_t *tcb_ptr){
	deschedule(&tcb_ptr->tid);
}

/**
 * @brief Initializes the whole thread library.
 *
 * This function is called exacly once before any other thread library functions
 * are invoked.
 *
 * @param size Amount of stack space available for each thread.
 *
 * @return 0 on success, negative number on error.
 */
int thr_init(unsigned int size) {
	if(cond_init(&gstate.tcb_cv) < 0)
		return -1;
	if(mutex_init(&gstate.tcb_lock) < 0)
		return -2;
	//gstate.root_tid = gettid();
	gstate.stack_size = size;
	list_init(&gstate.tcb_list);

    /* Insert the root thread's TCB into the list */
	tcb_t *root_tcb = (tcb_t *)malloc(sizeof(tcb_t));
	if(root_tcb == NULL)
		return -3;
    if(cond_init(&root_tcb->exited) < 0)
		return -1;
	root_tcb->tid = gettid();
	root_tcb->joined = FALSE;
	root_tcb->status = STATUS_RUNNING;
	root_tcb->stack_low = root_tcb;
	list_init(&root_tcb->tcb_entry);
	list_add_tail(&gstate.tcb_list, &root_tcb->tcb_entry);

	return 0;
}

/**
 * @brief Creates a new thread to run func. 
 * 
 * Set up the peer threat's initial stack as follows.
 *
 *        Higher Address
 *     --------------------
 *     |                  |
 *     |                  |
 *     |       TCB        |
 *     |                  |
 *     |                  |
 *     -------------------- <-- peer_thr_tcb
 *     |   padding space  |
 *     -------------------- <-- stack_high
 *     |       args       |
 *     --------------------
 *     |   default_exit   |
 *     --------------------
 *     |       func       |
 *     --------------------
 *     |   peer_thr_tcb   |
 *     -------------------- <-- peer_thr_esp
 *     |                  |
 *     |                  |
 *     |                  |
 *     |                  |
 *     |                  |
 *     .                  .
 *     .                  .
 *     .                  .
 *     |                  |
 *     -------------------- <-- stack_low
 *        Lower Address
 *
 * @param func Pointer to the thread function body.
 * @param args Pointer to the argument to the function body
 *
 * @return 
 */
int thr_create(void *(*func)(void *), void *args) {
    unsigned int peer_thr_stack_size;
    int peer_thr_tid;
    void *peer_thr_stack_low, *peer_thr_stack_high, *peer_thr_esp;
    tcb_t *peer_thr_tcb;

    /* It is nice to allocate multiples of PAGE_SIZE amount of memory */
	peer_thr_stack_size = (gstate.stack_size + sizeof(tcb_t) + PAGE_SIZE - 1) / 
                          (PAGE_SIZE) * PAGE_SIZE;
	peer_thr_stack_low = malloc(peer_thr_stack_size);
	if(!peer_thr_stack_low)
	    return -1;

    /* Put the thread's TCB on top of the stack */
	peer_thr_tcb = peer_thr_stack_low + peer_thr_stack_size - sizeof(tcb_t);
    /* Peer thread's %esp has to be 4 byte aligned */
	peer_thr_stack_high = 
                    (void *)((unsigned int)peer_thr_tcb & (unsigned int)~0x3);
	peer_thr_esp = peer_thr_stack_high;

	/* Populate peer threat's TCB */
    /* We are assuming that we will not get tid 0. Therefore, we can use it as 
     * an indicator to see if it is safe to deschedule itself. */
    if(cond_init(&peer_thr_tcb->exited) < 0)
		return -1;
	peer_thr_tcb->tid = 0; 
	peer_thr_tcb->status = STATUS_RUNNING;
	peer_thr_tcb->joined = FALSE;
	list_init(&peer_thr_tcb->tcb_entry);
	peer_thr_tcb->stack_high = peer_thr_stack_high;
	peer_thr_tcb->stack_low = peer_thr_stack_low;
  
    /* Prepare calling stack */
	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)args;
	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)default_exit_entry;
	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)func;
	peer_thr_esp -= 4;
	*(tcb_t **)peer_thr_esp = peer_thr_tcb;

    /* Trap into the system call */
	peer_thr_tid = thread_fork_wrapper(peer_thr_esp);
	if(peer_thr_tid < 0){
		free(peer_thr_stack_low);
		return peer_thr_tid;
	}

    /* Populate the tid field of the peer threat's TCB and insert it into the
     * list */
	assert(peer_thr_tid != 0);
	peer_thr_tcb->tid = peer_thr_tid;
	mutex_lock(&gstate.tcb_lock);
	list_add_tail(&gstate.tcb_list, &peer_thr_tcb->tcb_entry);
	mutex_unlock(&gstate.tcb_lock);
	make_runnable(peer_thr_tid);
	return peer_thr_tid;
}

/**
 * @brief Wait on the target thread to exit.
 *
 * @param tid Target tid to wait on.
 * @param statusp Pointer to address where target thread's exit status
 *
 * @return 
 */
int thr_join(int tid, void **statusp) {
	tcb_t *tcb;
	list_ptr entry;
	int ret;

    entry = &gstate.tcb_list;
	mutex_lock(&gstate.tcb_lock);

	for(entry = entry->next; entry != &gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, tcb_t, tcb_entry);
		if(tcb->tid == tid){
			break;
		}
        /* Need to make sure that in the event that we haven't found the tid,
         * tcb is NULL */
		tcb = NULL;
	}

    /* Haven't found it, go home */
	if(!tcb){
		ret = -2;
		goto out;
	}

    /* Some other thread have claimed this thread, go home */
	if(tcb->joined == TRUE){
		ret = -1;
		goto out;
	}

    /* We claim this thread */
	tcb->joined = TRUE;
	ret = 0;
	while(tcb->status != STATUS_EXITED){
		cond_wait(&tcb->exited, &gstate.tcb_lock);
	}

	if(statusp)
		*statusp = tcb->ret;

    /* Housekeeping */
	list_remv(entry);
	free(tcb->stack_low);

out:
	mutex_unlock(&gstate.tcb_lock);
	return ret;
}

/**
 * @brief Exits the thread with status.
 *
 * @param status Pointer to exit status. Just an address on the peer thread's
 *               stack whose stored value will be copied by thr_join() before
 *               freeing the whole stack.
 */
void thr_exit(void *status) {
	int tid = gettid();
	tcb_t *tcb;
	list_ptr entry;
    
    /* Locate its own TCB as this function could be called anywhere */
    entry = &gstate.tcb_list;
	mutex_lock(&gstate.tcb_lock);
	for(entry = entry->next; entry != &gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, tcb_t, tcb_entry);
		if(tcb->tid == tid){
			break;
		}
		tcb = NULL;
	}
	assert(tcb != NULL);

	tcb->ret = status;
	tcb->status = STATUS_EXITED;
	if(tcb->joined == TRUE){
		cond_signal(&tcb->exited);
	}
	mutex_unlock(&gstate.tcb_lock);
    /* Housekeeping */
	vanish();
}

/**
 * @brief Returns the thread id of the invoking thread.
 *
 * @return tid of invoking thread.
 */
int thr_getid(void){
	return gettid();
}

/**
 * @brief Defers the execution of thread w/ tid to a later time.
 *
 * @param tid tid of thread to be deferred.
 *
 * @return 
 */
int thr_yield(int tid){
	return yield(tid);
}
