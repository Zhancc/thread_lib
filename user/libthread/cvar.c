#include <cond_type.h>
#include <mutex.h>

void cond_wait( cond_t *cv, mutex_t *mp){
	self->tid = tid;
	self->indicator = 0;
	mutex_lock(cv->cmutex);
	enque(s)
	mutex_unlock(mp);
	mutex_unlock(cv->cmutex);
	deschedule(&self->indicator);
	mutex_lock(mp);
}

void cond_signal(cond_t *cv ){
	mutex_lock(cv->cmutex);
	s = deque();
	mutex_lock(cv->cmutex);
	s->indicator = 1;
	make_runnable(s->tid);
}
