#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__
#include "preempt.h"
struct spinlock_T{
	__volatile__ unsigned long lock;
};
extern inline void spin_init(struct spinlock_T* lock){
	lock->lock=1;
}
extern inline void spin_lock(struct spinlock_T* lock){
	preempt_disable();
	__asm__ __volatile(
		"1: \n\t"
		"lock decq %0 \n\t"
		"jns 3f \n\t"
		"2: \n\t"
		"pause \n\t"
		"cmpq $0,%0 \n\t"
		"jle 2b \n\t"
		"jmp 1b \n\t"
		"3: \n\t"
		:"=m"(lock->lock)
		:
		:"memory"
	);
}
extern inline void spin_unlock(struct spinlock_T* lock){
	__asm__ __volatile(
		"movq $1,%0 \n\t"
		:"=m"(lock->lock)
		:
		:"memory"
	);
	preempt_enable();
}
inline long spin_trylock(struct spinlock_T* lock){
	unsigned long tmp_value=0;
	preempt_disable();
	__asm__ __volatile__(
		"xchgq %0,%1 \n\t"
		:"=q"(tmp_value),"=m"(lock->lock)
		:"0"(0)
		:"memory"
	);
	if(!tmp_value)	preempt_enable();
	return tmp_value;
}
#endif
