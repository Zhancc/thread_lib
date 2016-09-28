/**
 * @file thread.c
 * @brief Implementions of the thread APIs specified in 410user/inc/thread.h
 * @author Zhan Chan (zhanc1), X.D. Zhai (xingdaz)
 * @date 2016-09-27
 */

#include <thread.h>
#include <thr_internals.h>
#include <cond.h>
#include <mutex.h>
#include <list.h>
#include <syscall.h>
#include <assert.h>
/* _malloc(), thread unsafe */
#include <malloc.h>

struct {
	unsigned int stack_size;

	list tcb_list;
	mutex_t tcb_lock;
	cond_t tcb_cv;

	int root_tid;
} gstate;

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
   * save version of the funciton */
	thread_struct *root_tcb = (thread_struct *)malloc(sizeof(thread_struct));
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
	root_tcb->joined = FALSE;
	root_tcb->status = STATUS_ON_GOING;
	root_tcb->stack_low = root_tcb;
	list_init(&root_tcb->tcb_entry);
	cond_init(&root_tcb->cv);
	list_add_tail(&gstate.tcb_list, &root_tcb->tcb_entry);

	return 0;
}

/**
 * @brief 
 *
 * @param func
 * @param args
 *
 * @return 
 */
int thr_create(void *(*func)(void *), void *args) {
	unsigned int size = (gstate.stack_size + 
                       sizeof(thread_struct) + 
                       PAGE_SIZE - 1)/(PAGE_SIZE) * PAGE_SIZE;
	void *stack_low = malloc(size);
	thread_struct *tcb = stack_low + size - sizeof(thread_struct);
	void *stack_high = (void *)((unsigned int)tcb & (unsigned int)~0x3);
	void *esp = stack_high;
	int tid;

	if(stack_low == 0)
	 return -1;

	/* init tcb */
	tcb->tid = 0; //to make use deschedule call. We shouldn't get tid 0 anyway
	tcb->status = STATUS_ON_GOING;
	tcb->joined = FALSE;
	list_init(&tcb->tcb_entry);
	tcb->stack_high = stack_high;
	tcb->stack_low = stack_low;
	if(cond_init(&tcb->cv) < 0)
		return -1;

  /* TODO let's use assembly here. */
	esp -= 4;
	*(void **)esp = (void *)args;
	esp -= 4;
	*(void **)esp = (void*)default_exit;
	esp -= 4;
	*(void **)esp = (void *)func;
	esp -= 4;
	*(thread_struct **)esp = tcb;
	esp -=4;
	*(void **)esp = (void *)peer_thread_init;

	tid = thread_fork_wrapper(esp);
	if(tid < 0){
		free(stack_low);
		return tid;
	}
	tcb->tid = tid;
	/* add tcb to the list */
	mutex_lock(&gstate.tcb_lock);
	list_add_tail(&gstate.tcb_list, &tcb->tcb_entry);
	mutex_unlock(&gstate.tcb_lock);
	make_runnable(tid);
	return tid;
}

int thr_gettid(void){
	return gettid();
}

int thr_yield(int tid){
	/* just use yield syscall?? */
	return yield(tid);
}

void thr_exit( void *status ){
	/* looking for non-root/root tcb, currently by searching for tid */
	int tid = gettid();
	thread_struct *tcb;
	list *entry = &gstate.tcb_list;
	mutex_lock(&gstate.tcb_lock);
	for(entry = entry->next; entry!=&gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, thread_struct, tcb_entry);
		if(tcb->tid == tid){
			entry = list_remv(entry);
			break;
		}
		tcb = 0;
	}
	assert(tcb != 0);
	tcb->ret = status;
	tcb->status = STATUS_ZOMBIE;
	if(tcb->joined == TRUE){
		cond_signal(&tcb->cv);
	}
	mutex_unlock(&gstate.tcb_lock);

	vanish();
}

static void default_exit(){
	thr_exit(0);
}

/* TODO let's call them "peer thread" because parent and child are used to refer
 * processes. Refer to page 986 of CSAPP*/
static void peer_thread_init(thread_struct *tcb){
  /* TODO Why do you want to descedule it? */
	deschedule(&tcb->tid);
	/* ok, everything is good now*/
	
}

int thr_join( int tid, void **statusp ){
	thread_struct *tcb;
	list *entry = &gstate.tcb_list;
	int ret;
	mutex_lock(&gstate.tcb_lock);
	for(entry = entry->next; entry!=&gstate.tcb_list; entry = entry->next){
		tcb = LIST_ENTRY(entry, thread_struct, tcb_entry);
		if(tcb->tid == tid){
			break;
		}
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
		cond_wait(&tcb->cv, &gstate.tcb_lock);
	}
	if(!statusp)
		*statusp = tcb->ret;
	free(tcb->stack_low);
out:
	mutex_unlock(&gstate.tcb_lock);
	return ret;
}
