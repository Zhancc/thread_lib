#include <sem.h>
#include <sem_type.h>

int sem_init( sem_t *sem, int count ){
	sem->cnt = count;
	sem->init_flag = 1;
	sem->waiting = 0;
	cond_init(&sem->cv);
	mutex_init(&sem->cv_mutex);
}

void sem_wait( sem_t *sem ){
	mutex_lock(&sem->cv_mutex);
	assert(sem->init_flag);
	while(sem->cnt <= 0){
		sem->waiting++;
		cond_wait(&sem->cv, &sem->cv_mutex);
	}
	sem->waiting--;
	sem->cnt--;
	mutex_unlock(&sem->cv_mutex);
}

void sem_signal( sem_t *sem ){
	mutex_lock(&sem->cv_mutex);
	assert(sem->init_flag);	
	while(sem->cnt <= 0){
		cond_signal(&sem->cv);
	}
	mutex_unlock(&sem->cv_mutex);	
	sem->cnt++;
}

void sem_destroy( sem_t *sem ){
	mutex_lock(&sem->cv_mutex);
	assert(sem->waiting == 0);
	mutex_unlock(&sem->cv_mutex);
	mutex_destroy(&sem->cv_mutex);
	cond_destroy(&sem->cv);
	sem->cnt = 0;
}
