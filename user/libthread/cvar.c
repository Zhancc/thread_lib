#include <cond_type.h>
#include <cond.h>
#include <mutex.h>
#include <list.h>
#include <syscall.h>
typedef struct waiting{
	int tid;
	int indicator;
	list list_entry;
} waiting;

int cond_init( cond_t *cv ){
	int ret;
	ret = mutex_init(&cv->cmutex);
	if(ret < 0)
		return ret;
	init_list(&cv->queue);
	return 0;
}

void cond_destroy( cond_t *cv ){
	mutex_destroy(&cv->cmutex);
}

void cond_wait( cond_t *cv, mutex_t *mp){
	struct waiting self;
	self.tid = gettid();
	self.indicator = 0;
	mutex_lock(&cv->cmutex);
	list_add_tail(&cv->queue, &self.list_entry);
	mutex_unlock(mp);
	mutex_unlock(&cv->cmutex);
	deschedule(&self.indicator);
	mutex_lock(mp);
}

#define dequeue(queue_ptr) (LIST_ENTRY(list_remv_head(queue_ptr), waiting, list_entry))

void cond_signal(cond_t *cv ){
	struct waiting *s;
	mutex_lock(&cv->cmutex);
	s = dequeue(&cv->queue);
	mutex_unlock(&cv->cmutex);
	if(s){
	  s->indicator = 1;
	  make_runnable(s->tid);
	}
}

void cond_broadcast( cond_t *cv ){
	struct waiting *s;
	mutex_lock(&cv->cmutex);
	for(s = dequeue(&cv->queue); !s; s = dequeue(&cv->queue)){
	  s->indicator = 1;
	  make_runnable(s->tid);		
	}
	mutex_unlock(&cv->cmutex);
}
