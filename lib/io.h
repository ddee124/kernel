#ifndef __IO_H__
#define __IO_H__ 1
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__)) inline
static inline __always_inline void io_out8(uint16_t port,uint8_t value){
	__asm__ __volatile__(
		"outb %%al,%%dx \n\t"
		:
		:"a"(value),"d"(port)
		:"memory"
	);
}
static inline __always_inline uint8_t io_in8(uint16_t port){
	uint8_t ret;
	__asm__ __volatile__(
		"inb %%dx,%%al \n\t"
		:"=a"(ret)
		:"d"(port)
		:"memory"
	);
	return ret;
}
static inline __always_inline void io_out16(uint16_t port,uint16_t value){
	__asm__ __volatile__(
		"outw %%ax,%%dx \n\t"
		:
		:"a"(value),"d"(port)
		:"memory"
	);
}
static inline __always_inline uint16_t io_in16(uint16_t port){
	uint16_t ret;
	__asm__ __volatile__(
		"inw %%dx,%%ax \n\t"
		:"=a"(ret)
		:"d"(port)
		:"memory"
	);
	return ret;
}
static inline __always_inline void io_out32(uint16_t port,uint32_t value){
	__asm__ __volatile__(
		"outl %%eax,%%dx \n\t"
		:
		:"a"(value),"d"(port)
		:"memory"
	);
}
static inline __always_inline uint32_t io_in32(uint16_t port){
	uint32_t ret;
	__asm__ __volatile__(
		"inl %%dx,%%eax \n\t"
		:"=a"(ret)
		:"d"(port)
		:"memory"
	);
	return ret;
}
#endif
