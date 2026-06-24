#ifndef __LIB_H__
#define __LIB_H__ 1
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
#define local_irq_save(flags) __asm__ __volatile__("pushfq \n\t popq %0 \n\t cli \n\t":"=rm"(flags)::"memory")
#define local_irq_restore(flags) __asm__ __volatile__("pushq %0 \n\t popfq \n\t"::"g"(flags):"memory","cc")
static inline __always_inline void memcpy(void* dest,void* src,uint64_t len){
	void* tmp_dest=dest;void* tmp_src=src;uint64_t tmp_len=len;
	__asm__ __volatile__(
		"rep movsb \n\t"
		:"+&D"(tmp_dest),"+&S"(tmp_src),"+&c"(tmp_len)
		:
		:"memory"
	);
}
static inline __always_inline void memset(void* s,uint8_t c,uint64_t len){
	void* tmp_s=s;uint64_t tmp_len=len;
	__asm__ __volatile__(
		"rep stosb \n\t"
		:"+&D"(tmp_s),"+&c"(tmp_len)
		:"a"(c)
		:"memory"
	);
}
static inline __always_inline uint64_t strlen(unsigned char* s){
	uint64_t res=-1ul;unsigned char* tmp_s=s;
	__asm__ __volatile__(
		"xorq %%rax,%%rax \n\t"
		"repne scasb \n\t"
		"notq %%rcx \n\t"
		"decq %%rcx \n\t"
		:"+c"(res),"+&D"(tmp_s)
		:
		:"memory","rax","cc"
	);
	return res;
}
static inline __always_inline void cli(){__asm__ __volatile__("cli \n\t":::"memory");}
static inline __always_inline void sti(){__asm__ __volatile__("sti \n\t":::"memory");}
static inline __always_inline void nop(){__asm__ __volatile__("nop \n\t");}
static inline __always_inline void pause(){__asm__ __volatile__("pause \n\t");}
static inline __always_inline void io_mfence(){__asm__ __volatile__("mfence \n\t":::"memory");}
static inline __always_inline void halt(){__asm__ __volatile__("hlt \n\t");}
static inline __always_inline uint64_t rdmsr(uint32_t addr){
	uint64_t ret;
	__asm__ __volatile__(
		"rdmsr \n\t"
		"shlq $32,%%rdx \n\t"
		"orq %%rax,%%rdx \n\t"
		:"=d"(ret)
		:"c"((uint64_t)addr)
		:"rax"
	);
	return ret;
}
static inline __always_inline void wrmsr(uint32_t addr,uint64_t val){
	__asm__ __volatile__(
		"wrmsr \n\t"
		:
		:"a"(val&0xfffffffful),"d"(val>>32),"c"((uint64_t)addr)
		:
	);
}
static inline __always_inline uint64_t get_rflags(){
	uint64_t ret;
	__asm__ __volatile__(
		"pushfq \n\t"
		"popq %0 \n\t"
		:"=r"(ret)
		:
		:"memory"
	);
	return ret;
}
static inline __always_inline uint64_t is_digit(unsigned char ch){
	return ch>='0'&&ch<='9';
}
#endif
