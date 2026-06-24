#ifndef __PAGETABLE_H__
#define __PAGETABLE_H__ 1
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30
#define PAGE_2M_SHIFT 21
#define PAGE_4K_SHIFT 12
#define PAGE_1G_SIZE 0x40000000ul
#define PAGE_2M_SIZE 0x200000ul
#define PAGE_4K_SIZE 0x1000ul
#define PAGE_1G_MASK 0xffffffffc0000000ul
#define PAGE_2M_MASK 0xffffffffffe00000ul
#define PAGE_4K_MASK 0xfffffffffffff000ul
#define PAGE_XD (1ul<<63)
#define PAGE_PAT (1ul<<12)
#define PAGE_Global (1ul<<8)
#define PAGE_PS (1ul<<7)
#define PAGE_Dirty (1ul<<6)
#define PAGE_Accessed (1ul<<5)
#define PAGE_PCD (1ul<<4)
#define PAGE_PWT (1ul<<3)
#define PAGE_U_S (1ul<<2)
#define PAGE_R_W (1ul<<1)
#define PAGE_Present (1ul<<0)
#define PAGE_KERNEL_GDT (PAGE_R_W|PAGE_Present)
#define PAGE_KERNEL_Dir	(PAGE_R_W|PAGE_Present)
#define	PAGE_KERNEL_Page (PAGE_PS|PAGE_R_W|PAGE_Present)
#define PAGE_USER_Dir (PAGE_U_S|PAGE_R_W|PAGE_Present)
#define	PAGE_USER_Page (PAGE_PS|PAGE_U_S|PAGE_R_W|PAGE_Present)
#define PAGE_OFFSET 0xffff800000000000ul
#define Virt_To_Phy(x) ((uint64_t)(x)-PAGE_OFFSET)
#define Phy_To_Virt(x) ((uintptr_t)((uint64_t)(x)+PAGE_OFFSET))
static inline __always_inline uint64_t get_cr3(){
	uint64_t ret;
	__asm__ __volatile__(
		"movq %%cr3,%0 \n\t"
		:"=r"(ret)
		:
		:
	);
	return ret;
}
static inline __always_inline void flush_tlb(){
	uint64_t tmp;
	__asm__ __volatile__(
		"movq %%cr3,%0 \n\t"
		"movq %0,%%cr3 \n\t"
		:"=r"(tmp)
		:
		:"memory"
	);
}
extern void map_4K_page(uint64_t phy_addr,uint64_t attr);
extern void map_2M_page(uint64_t phy_addr,uint64_t attr);
extern void map_1G_page(uint64_t phy_addr,uint64_t attr);
extern void map_range(uint64_t start,uint64_t end,uint64_t attr);
#endif
