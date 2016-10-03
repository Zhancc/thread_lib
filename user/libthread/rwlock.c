#include <rwlock.h>
#include <rwlock_type.h>
#include <assert.h>
#include <syscall.h>
#include <list.h>
/**
 *  * @brief A waiting thread populate this structure before enqueue and sleep.
 *   */
typedef struct waiting_thr_data {
    /* tid of the waiting thread */
    int tid;
    /* This is the "reject" argument of the deschedule syscall. The idea is if
 	 * some thread wants to wake up a thread, it indicates its intent by setting
 	 * this variable to 1 before calling  make_runnable. A thread about to
 	 * deschedule itself will atomically check this variable. If it is none
 	 * zero, i.e. it will runnable soon, then it will not deschedule itself. */
    int about_to_be_runnable;
    list_t list_entry;
	int type;
} waiting_thr_data_t;

int rwlock_init( rwlock_t *rwlock ){
	if(mutex_init(&rwlock->qr_mutex) < 0)
		return -1;

	list_init(&rwlock->queue);
	rwlock->reader = 0;
	rwlock->init_flag = 1;
	return 0;
}

void rwlock_lock( rwlock_t *rwlock, int type ){
	waiting_thr_data_t data;
	data.tid = gettid();
	data.about_to_be_runnable = 0;
	data.type = type;
    mutex_lock(&rwlock->qr_mutex);
    assert(rwlock->init_flag);
	if(type == RWLOCK_READ){
		if(!list_empty(&rwlock->queue)){
			list_add_tail(&rwlock->queue, &data.list_entry);
			mutex_unlock(&rwlock->qr_mutex);
			deschedule(&data.about_to_be_runnable);
			//we are waken up
			goto out;
		}else{
			//queue is empty
			if(rwlock->reader < 0){
				list_add_tail(&rwlock->queue, &data.list_entry);
				mutex_unlock(&rwlock->qr_mutex);
				deschedule(&data.about_to_be_runnable);
				//we are waken up
				goto out;
			}else{
				rwlock->reader++;
				mutex_unlock(&rwlock->qr_mutex);
				goto out;
			}
		}
	}else{
		/* this is a write lock request*/
		if(!list_empty(&rwlock->queue)){
			list_add_tail(&rwlock->queue, &data.list_entry);
			mutex_unlock(&rwlock->qr_mutex);
			deschedule(&data.about_to_be_runnable);
			goto out;
		}else{
			if(rwlock->reader != 0){
				list_add_tail(&rwlock->queue, &data.list_entry);
            	mutex_unlock(&rwlock->qr_mutex);
            	deschedule(&data.about_to_be_runnable);
            	goto out;
			}else{
				rwlock->reader = -data.tid;
				mutex_unlock(&rwlock->qr_mutex);
				goto out;
			}
		}
	}
out:
	return;
}
void rwlock_unlock( rwlock_t *rwlock ){
	waiting_thr_data_t *next_in_line;
	list_ptr entry;
	mutex_lock(&rwlock->qr_mutex);
    assert(rwlock->init_flag);	
	assert(rwlock->reader != 0);
	if(rwlock->reader < 0){
		/* we are a writer lock*/
		rwlock->reader = 0;
		if(list_empty(&rwlock->queue)){
			mutex_unlock(&rwlock->qr_mutex);
			goto out;
		}else{
			entry = list_remv_head(&rwlock->queue);
			next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
			if(next_in_line->type == RWLOCK_WRITE){
				rwlock->reader = -gettid();;
				next_in_line->about_to_be_runnable = 1;
				make_runnable(next_in_line->tid);
			}

			while(next_in_line && next_in_line->type == RWLOCK_READ){
				next_in_line->about_to_be_runnable = 1;
				rwlock->reader++;
				make_runnable(next_in_line->tid);
				entry = list_remv_head(&rwlock->queue);
				next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
			}
			mutex_unlock(&rwlock->qr_mutex);
			goto out;
		}
	}else if(rwlock->reader > 1){
		/* we are not last reader lock */
		rwlock->reader--;
		mutex_unlock(&rwlock->qr_mutex);
		goto out;
	}else{
		/* we are last reader */
		rwlock->reader--;
		if(list_empty(&rwlock->queue)){
			/*no one waiting*/
			mutex_unlock(&rwlock->qr_mutex);
			goto out;
		}else{
			entry = list_remv_head(&rwlock->queue);
			next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
			assert(next_in_line->type == RWLOCK_WRITE);
			next_in_line->about_to_be_runnable = 1;
			rwlock->reader = -next_in_line->tid;
			mutex_unlock(&rwlock->qr_mutex);
			make_runnable(next_in_line->tid);
		}
	}
out:
	return;
}

void rwlock_destroy( rwlock_t *rwlock ){
	/*assert on the illegal state*/
	mutex_lock(&rwlock->qr_mutex);
	assert(rwlock->reader == 0);
	rwlock->init_flag = 0;
	mutex_unlock(&rwlock->qr_mutex);
	mutex_destroy(&rwlock->qr_mutex);
}

void rwlock_downgrade( rwlock_t *rwlock){
	waiting_thr_data_t *next_in_line;	
	list_ptr entry;
	mutex_lock(&rwlock->qr_mutex);
    assert(rwlock->init_flag);	
	assert(-rwlock->reader == gettid());
	rwlock->reader = 1;
	/*dequeue other readers*/
	entry = list_remv_head(&rwlock->queue);
	next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
    while(next_in_line && next_in_line->type == RWLOCK_READ){
        next_in_line->about_to_be_runnable = 1;
        rwlock->reader++;
        make_runnable(next_in_line->tid);
		entry = list_remv_head(&rwlock->queue);
		next_in_line = LIST_ENTRY(entry, waiting_thr_data_t, list_entry);
    }	

	mutex_unlock(&rwlock->qr_mutex);
}