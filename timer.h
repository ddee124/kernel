#ifndef __TIMER_H__
#define __TIMER_H__
#include "lib.h"
struct timer_list{
	struct List list;
	unsigned long expire_jiffies;
	void(*func)(void* data);
	void* data;
};
struct timer_list timer_list_head;
extern void timer_init();
#endif
