/**
 * @file thread.c
 * @brief Implementions of the thread APIs specified in 410user/inc/thread.h
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 */

/* thread public facing API interface */
#include <thread.h>
#include <thr_internals.h>
#include <cond.h>
#include <mutex.h>
#include <list.h>
/* NULL */
#include <stddef.h>
/* PAGE_SIZE */
#include <syscall.h>
#include <assert.h>
/* _malloc(), thread unsafe */
#include <malloc.h>
#include <simics.h>

/**
 * @brief 
 * TODO I am not sure if this is the right behavior because the handout says
 * "should be the same as if the function had called thr_exit() specifying
 * the return value from the thread's body function. "
 */
static void default_exit(){
    thr_exit(0);
}

struct {
	unsigned int stack_size;

	list tcb_list;
	mutex_t tcb_lock;
	cond_t tcb_cv;

	int root_tid;
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
  /* TODO we should be calling the _malloc() as we haven't built the thread
   * safe version of the funciton */
	tcb_t *root_tcb = (tcb_t *)malloc(sizeof(tcb_t));
	if(root_tcb == NULL)
		return -3;
	gstate.stack_size = size;
	list_init(&gstate.tcb_list);
	if(cond_init(&gstate.tcb_cv) < 0)
		return -1;
	if(mutex_init(&gstate.tcb_lock) < 0)
		return -2;
	gstate.root_tid = gettid();

	root_tcb->tid = gstate.root_tid;
	root_tcb->joined = FALSE; /* TODO should just use 0 */
	root_tcb->status = STATUS_ON_GOING;
	root_tcb->stack_low = root_tcb;
	list_init(&root_tcb->tcb_entry);
	cond_init(&root_tcb->exited);
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
 *     -------------------- <-- stack_low
 *        Lower Address
 *
 * @param func Pointer to the thread function body.
 * @param args Pointer to the argument to the function body
 *
 * @return 
 */
int thr_create(void *(*func)(void *), void *args) {
	unsigned int size = (gstate.stack_size + 
                       sizeof(tcb_t) + 
                       PAGE_SIZE - 1)/(PAGE_SIZE) * PAGE_SIZE;
	void *stack_low = malloc(size);
	if(stack_low == 0)
	 return -1;

	tcb_t *peer_thr_tcb = stack_low + size - sizeof(tcb_t);
  /* Peer thread's %esp has to be 4 byte aligned */
	void *stack_high = (void *)((unsigned int)peer_thr_tcb & (unsigned int)~0x3);
	void *peer_thr_esp = stack_high;
	int peer_thr_tid;

	/* Populate peer threat's TCB */
	peer_thr_tcb->tid = 0; /* We are assuming that we will not get tid 0.
                            Therefore, we can use it as an indicator to see if 
                            it is safe to deschedule itself. */
	peer_thr_tcb->status = STATUS_ON_GOING;
	peer_thr_tcb->joined = FALSE;
	list_init(&peer_thr_tcb->tcb_entry);
	peer_thr_tcb->stack_high = stack_high;
	peer_thr_tcb->stack_low = stack_low;
	if(cond_init(&peer_thr_tcb->exited) < 0)
		return -1;

	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)args;
	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)default_exit;
	peer_thr_esp -= 4;
	*(void **)peer_thr_esp = (void *)func;
	peer_thr_esp -= 4;
	*(tcb_t **)peer_thr_esp = peer_thr_tcb;
	//peer_thr_esp -=4;
	//*(void **)peer_thr_esp = (void *)peer_thread_init;

	peer_thr_tid = thread_fork_wrapper(peer_thr_esp);
	if(peer_thr_tid < 0){
		free(stack_low);
		return peer_thr_tid;
	}
  /* Populate the tid field of the peer threat's tcb */
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
int thr_join(int tid, void **statusp){
	tcb_t *tcb;
	list *entry = &gstate.tcb_list;
	int ret;
	mutex_lock(&gstate.tcb_lock);
  /* Find the bugger */
	for(entry = entry->next; entry!=&gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, tcb_t, tcb_entry);
		if(tcb->tid == tid){
			break;
		}
    /* TODO how is this necessary */
		tcb = 0;
	}
	if(!tcb){
		ret = -2;
		goto out;
	}

	if(tcb->joined == TRUE){
		ret = -1;
		goto out;
	}

	tcb->joined = TRUE;
	ret = 0;

	while(tcb->status != STATUS_ZOMBIE){
		cond_wait(&tcb->exited, &gstate.tcb_lock);
	}

	if(statusp)
		*statusp = tcb->ret;
	list_remv(entry);
	free(tcb->stack_low);
out:
	mutex_unlock(&gstate.tcb_lock);
	return ret;
}

/**
 * @brief Exits the thread with status
 *
 * @param status Pointer to exit status. Passed unchanged to thr_join
 */
void thr_exit( void *status ){
	/* looking for non-root/root tcb, currently by searching for tid */
	int tid = gettid();
	tcb_t *tcb;
	list *entry = &gstate.tcb_list;
	mutex_lock(&gstate.tcb_lock);
	for(entry = entry->next; entry!=&gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, tcb_t, tcb_entry);
		if(tcb->tid == tid){
			break;
		}
		tcb = 0;
	}
	assert(tcb != 0);
	tcb->ret = status;
	tcb->status = STATUS_ZOMBIE;
	if(tcb->joined == TRUE){
		cond_signal(&tcb->exited);
	}
	mutex_unlock(&gstate.tcb_lock);

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
