/* the implementation assumes some memory consistency model */
#include <syscall.h>
#include <mutex.h>
#include <asm_internals.h>

int mutex_init( mutex_t *mp ){
	mp->next = mp->owner = 0;
	return 0;
}

void mutex_destroy( mutex_t *mp ){
	return;
}

void mutex_lock( mutex_t *mp ){
	int ticket = atomic_inc(&mp->next);
	while(ticket!=mp->owner){
		yield(-1);
	}
}

void mutex_unlock( mutex_t *mp ){
	mp->owner++;
	return;
}
