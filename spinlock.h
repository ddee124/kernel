#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__
typedef struct{
	__volatile__ unsigned long lock;
}spinlock_T;
extern inline void spin_init(spinlock_T* lock){
	lock->lock=1;
}
extern inline void spin_lock(spinlock_T* lock){
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
extern inline void spin_unlock(spinlock_T* lock){
	__asm__ __volatile(
		"movq $1,%0 \n\t"
		:"=m"(lock->lock)
		:
		:"memory"
	);
}
#endif
