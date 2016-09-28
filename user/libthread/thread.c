#include <thread.h>
#include <thr_internals.h>
#include <cond.h>
#include <mutex.h>
#include <list.h>
#include <syscall.h>
#include <assert.h>
#include <malloc.h>
#include <simics.h>
struct{
	unsigned int stack_size;

	list tcb_list;
	mutex_t tcb_lock;
	cond_t tcb_cv;

	int root_tid;
}gstate;

int thr_init( unsigned int size ){
	thread_struct *root_tcb = (thread_struct *)malloc(sizeof(thread_struct));
	if(root_tcb == 0)
		return -3;
	gstate.stack_size = size;
	init_list(&gstate.tcb_list);
	if(cond_init(&gstate.tcb_cv) < 0)
		return -1;
	if(mutex_init(&gstate.tcb_lock) < 0)
		return -2;
	gstate.root_tid = gettid();

	root_tcb->tid = gstate.root_tid;
	root_tcb->joined = FALSE;
	root_tcb->status = STATUS_ON_GOING;
	root_tcb->stack_low = root_tcb;
	init_list(&root_tcb->tcb_entry);
	cond_init(&root_tcb->cv);
	list_add_tail(&gstate.tcb_list, &root_tcb->tcb_entry);

	return 0;
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

void child_init(thread_struct *tcb){
	deschedule(&tcb->tid);
	/* ok, everything is good now*/
	
}

int thr_create( void *(*func)(void *), void *args ){
	unsigned int size = (gstate.stack_size + sizeof(thread_struct) + PAGE_SIZE - 1)/(PAGE_SIZE) * PAGE_SIZE;
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
	init_list(&tcb->tcb_entry);
	tcb->stack_high = stack_high;
	tcb->stack_low = stack_low;
	if(cond_init(&tcb->cv) < 0)
		return -1;
	
	esp -= 4;
	*(void **)esp = (void *)args;
	esp -= 4;
	*(void **)esp = (void*)default_exit;
	esp -= 4;
	*(void **)esp = (void *)func;
	esp -= 4;
	*(thread_struct **)esp = tcb;

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
	if(statusp)
		*statusp = tcb->ret;
	list_remv(entry);
	free(tcb->stack_low);
out:
	mutex_unlock(&gstate.tcb_lock);
	return ret;
}
