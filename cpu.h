#ifndef __CPU_H__
#define __CPU_H__
extern inline void get_cpuid(unsigned int mop,unsigned int sop,unsigned int *a,unsigned int *b,unsigned int *c,unsigned int *d){
	__asm__ __volatile__(
		"cpuid \n\t"
		:"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
		:"0"(mop),"2"(sop)
		:"memory"
	);
}
extern void init_cpu();
#endif
