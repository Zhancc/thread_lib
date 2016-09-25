#include <cond_type.h>
#include <cond.h>
#include <mutex.h>

struct waiting{
	int tid;
	int indicator;
};

int cond_init( cond_t *cv ){
	int ret;
	ret = mutex_init(&cv->cmutex);
	if(ret < 0)
		return ret;
}

void cond_destroy( cond_t *cv ){
	mutex_destroy(&cv->cmutex);
}

void cond_wait( cond_t *cv, mutex_t *mp){
	struct waiting self;
	self.tid = tid;
	self.indicator = 0;
	mutex_lock(cv->cmutex);
	enque(&self);
	mutex_unlock(mp);
	mutex_unlock(cv->cmutex);
	deschedule(&self.indicator);
	mutex_lock(mp);
}

void cond_signal(cond_t *cv ){
	struct waiting *s;
	mutex_lock(cv->cmutex);
	s = deque();
	mutex_unlock(cv->cmutex);
	if(s){
	  s->indicator = 1;
	  make_runnable(s->tid);
	}
}

void cond_broadcast( cond_t *cv ){
	struct waiting *s;
	mutex_lock(cv->cmutex);
	while(s = deque(); !s; s = deque()){
	  s->indicator = 1;
	  make_runnable(s->tid);		
	}
	mutex_unlock(cv->cmutex);
}