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


void cond_signal(cond_t *cv ){
	struct waiting *s;
	list *entry;
	mutex_lock(&cv->cmutex);
	entry = list_remv_head(&cv->queue);
	mutex_unlock(&cv->cmutex);
	if(entry){
	  s = LIST_ENTRY(entry, waiting, list_entry);
	  s->indicator = 1;
	  make_runnable(s->tid);
	}
}

void cond_broadcast( cond_t *cv ){
	struct waiting *s;
	list *entry;
	mutex_lock(&cv->cmutex);
	entry = list_remv_head(&cv->queue);
	for(s = LIST_ENTRY(entry, waiting, list_entry); !entry; ){
	  s->indicator = 1;
	  make_runnable(s->tid);
	  entry = list_remv_head(&cv->queue);
	  s = LIST_ENTRY(entry, waiting, list_entry);
	}
	mutex_unlock(&cv->cmutex);
}
