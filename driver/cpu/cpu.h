#ifndef __CPU_H__
#define __CPU_H__
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
static inline __always_inline void get_cpuid(uint32_t mop,uint32_t sop,uint32_t* a,uint32_t* b,uint32_t* c,uint32_t* d){
	__asm__ __volatile__(
		"cpuid \n\t"
		:"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
		:"a"(mop),"c"(sop)
		:"memory"
	);
}
extern void cpu_info();
#endif
