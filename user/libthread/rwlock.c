#if 0
#include <rwlock.h>
#include <rwlock_type.h>

int rwlock_init( rwlock_t *rwlock ){
	

}

void rwlock_lock( rwlock_t *rwlock, int type ){
	if(type == read){
		lock(rwlock->lock);
		if(rwlock->queue){
			enqueue(self)
			self->indicator = 0;
			unlock(rwlock->lock);
			deschedule(self)
			//we are waken up
			goto out
		}else{
			//queue is empty
			if(reader < 0){
				enqueue(self)
				self->indicator = 0;
				unlock(rwlock->lock);
				deschedule(self)
				//we are waken up
				goto out
			}else{
				reader++;
				unlock(rwlock->lock);
				goto out
			}
		}
	}else{
		lock(rwlock->lock);
		if(rwlock->queue){
			enqueue(self);
			self->indicator = 0;
			unlock(rwlock->lock);
			deschedue(self)
			goto out
		}else{
			if(reader != 0){
				enqueue(self);
            	self->indicator = 0;
            	unlock(rwlock->lock);
            	deschedue(self)
            	goto out
			}else{
				reader = -1;
				unlock(rwlock->lock);
				goto out
			}
		}
	}
}
void rwlock_unlock( rwlock_t *rwlock ){
	lock(rwlock->lock)
	assert(reader != 0);
	if(reader < 0){
		/* we are a writer lock*/
		if(!relock->queue){
			reader = 0;
			unlock(rwlock->lock);
			goto out
		}else{
			for(s = dequeue; s && s->type == read; s = dequeue){
				reader++;
				makerunnable(s->tid);
			}
			unlock(rwlock->lock);
			goto out;
		}
	}else if(reader > 1){
		/* we are not last reader lock */
		reader--;
		unlock(rwlock->lock);
		goto out;
	}else{
		/* we are last reader */
		reader--;
		if(!rwlock->queue){
			/*no one waiting*/
			unlock(rwlock->lock)
			goto out
		}else{
			s = dequeue(rwlock->queue);
			assert(s->type == write);
			reader = -1;
			unlock(rwlock->queue)
			makerunnable(s->tid);
		}
	}

}
void rwlock_destroy( rwlock_t *rwlock ){
	/*assert on the illegal state*/
}
void rwlock_downgrade( rwlock_t *rwlock){
	assert(reader < 0);
	reader = 1;
	/*dequeue other readers*/
	lock(rwlock->lock);
	for(...);
	unlock(rwlock->lock);

}
#endif
