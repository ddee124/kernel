#include "printk.h"
#include "softirq.h"
#include "timer.h"
#include "memory.h"
extern unsigned long jiffies;
void init_timer(struct timer_list *timer,void(*func)(void* data),void* data,unsigned long expire_jiffies){
	list_init(&timer->list);
	timer->func=func;timer->data=data;
	timer->expire_jiffies=jiffies+expire_jiffies;
}
void add_timer(struct timer_list *timer){
	struct timer_list* tmp=(struct timer_list*)list_next(&timer_list_head.list);
	if(!list_is_empty(&timer_list_head.list)){
		while(tmp->expire_jiffies<timer->expire_jiffies)	tmp=(struct timer_list*)list_next(&tmp->list);
	}
	list_add_to_before(&tmp->list,&timer->list);
}
void del_timer(struct timer_list *timer){
	list_del(&timer->list);
}
void test_timer(void* data){
	color_printk(BLUE,WHITE,"test_timer");
}
void do_timer(void* data){
	struct timer_list* tmp=(struct timer_list*)list_next(&timer_list_head.list);
	while((!list_is_empty(&timer_list_head.list))&&(tmp->expire_jiffies<=jiffies)){
		del_timer(tmp);
		if(tmp->func)	tmp->func(tmp->data);
		tmp=(struct timer_list*)list_next(&tmp->list);
	}
	color_printk(RED,WHITE,"(HPET:%ld)",jiffies);
}
void timer_init(){
	struct timer_list* tmp=0;
	jiffies=0;
	init_timer(&timer_list_head,0,0,-1ul);
	register_softirq(0,&do_timer,0);
	/*tmp=(struct timer_list*)kmalloc(sizeof(struct timer_list),0);
	init_timer(tmp,&test_timer,0,5000);
	add_timer(tmp);
	tmp=(struct timer_list*)kmalloc(sizeof(struct timer_list),0);
	init_timer(tmp,&test_timer,0,3000);
	add_timer(tmp);*/
}
