#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__
#define TIMER_SIRQ (1<<0)
unsigned long softirq_status=0;
struct softirq{
	void(*action)(void* data);
	void* data;
};
struct softirq softirq_vector[64]={0};
extern void softirq_init();
extern void set_softirq_status(unsigned long status);
extern unsigned long get_softirq_status();
extern void register_softirq(int nr,void(*action)(void* data),void* data);
extern void unregister_softirq(int nr);
#endif
