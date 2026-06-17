#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__
#include "atomic.h"
#include "lib.h"
#include "schedule.h"
struct wait_queue_T{
	struct List wait_list;
	struct task_struct* tsk;
};
void wait_queue_init(struct wait_queue_T* wait_queue,struct task_struct* tsk){
	list_init(&wait_queue->wait_list);
	wait_queue->tsk=tsk;
}
struct semaphore_T{
	struct atomic_T counter;
	struct wait_queue_T wait;
};
void semaphore_init(struct semaphore_T* semaphore,unsigned long count){
	atomic_set(&semaphore->counter,count);
	wait_queue_init(&semaphore->wait,0);
}
void __down(struct semaphore_T* semaphore){
	struct wait_queue_T wait;
	wait_queue_init(&wait,current);
	current->state=TASK_UNINTERRUPTIBLE;
	list_add_to_before(&semaphore->wait.wait_list,&wait->wait_list);
	schedule();
}
void semaphore_down(struct semaphore_T* semaphore){
	if(atomic_read(&semaphore->counter)>0)	atomic_dec(&semaphore->counter);
	else    __down(semaphore);
}
void __up(struct semaphore_T* semaphore){
	struct wait_queue_T* wait=(struct wait_queue_T*)list_next(&semaphore->wait.wait_list);
	list_del(&wait->wait_list);
	wait->tsk->state=TASK_RUNNING;
	insert_task_queue(wait->tsk);
}
void semaphore_up(struct semaphore_T* semaphore){
	if(list_is_empty(&semaphore->wait.wait_list))	atomic_inc(&semaphore->counter);
	else    __up(semaphore);
}
#endif
