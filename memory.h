#ifndef __MEMORY_H__
#define __MEMORY_H__
#define PTRS_PER_PAGE 512
#define PAGE_OFFSET 0xffff800000000000
#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30
#define PAGE_2M_SHIFT 21
#define PAGE_4K_SHIFT 12
#define PAGE_2M_SIZE 0x200000ul
#define PAGE_4K_SIZE 0x1000ul
#define PAGE_2M_MASK 0xffffffffffe00000ul
#define PAGE_4K_MASK 0xfffffffffffff000ul
#define PAGE_2M_ALIGN(x) (((unsigned long)(x)+PAGE_2M_SIZE-1)&PAGE_2M_MASK)
#define PAGE_4K_ALIGN(x) (((unsigned long)(x)+PAGE_4K_SIZE-1)&PAGE_4K_MASK)
#define Virt_To_Phy(x) ((unsigned long)(x)-PAGE_OFFSET)
#define Phy_To_Virt(x) ((unsigned long*)((unsigned long)(x)+PAGE_OFFSET))
#include "multiboot2.h"
#include "lib.h"
int ZONE_DMA_INDEX=0;
int ZONE_NORMAL_INDEX=0;
int ZONE_UNMAPED_INDEX=0;
extern unsigned long Global_CR3;
extern struct Global_Memory_Descriptor memory_management_struct;
extern void init_memory(struct multiboot2_tag_mmap *mmap_tag);
struct E820{
	unsigned long address;
	unsigned long length;
	unsigned int type;
}__attribute__((packed));
struct Zone{
	struct Page* pages_group;
	unsigned long pages_length;
	unsigned long zone_start_address;
	unsigned long zone_end_address;
	unsigned long zone_length;
	unsigned long attribute;
	struct Global_Memory_Descriptor* GMD_struct;
	unsigned long page_using_count;
	unsigned long page_free_count;
	unsigned long total_pages_link;
};
struct Page{
	struct Zone* zone_struct;
	unsigned long PHY_address;
	unsigned long attribute;
	unsigned long reference_count;
	unsigned long age;
};
struct Global_Memory_Descriptor{
	struct E820 e820[32];
	unsigned long e820_length;
	unsigned long* bits_map;
	unsigned long bits_size;
	unsigned long bits_length;
	struct Page* pages_struct;
	unsigned long pages_size;
	unsigned long pages_length;
	struct Zone* zones_struct;
	unsigned long zones_size;
	unsigned long zones_length;
	unsigned long start_code,end_code,end_data,end_brk;
	unsigned long end_of_struct;
};
#define flush_tlb()\
do{\
	unsigned long tmpreg;\
	__asm__ __volatile__ (\
	    "movq %%cr3,%0 \n\t"\
	    "movq %0,%%cr3 \n\t"\
	    :"=r"(tmpreg)\
	    :\
	    :"memory"\
	);\
}while(0)

inline unsigned long Get_gdt(){
	unsigned long tmp;
	__asm__ __volatile__(
	    "movq %%cr3,%0 \n\t"
	    :"=r"(tmp)
	    :
	    :"memory"
	);
	return tmp;
}
extern inline unsigned long Get_gdt();
typedef struct{unsigned long pml4t;} pml4t_t;
#define PG_PTable_Maped 1
#define PG_Kernel_Init 2
#define PG_Device 4
#define PG_Kernel 8
#define PG_Shared 16
/*#define PG_PTable_Maped 1
#define PG_Kernel_Init 2
#define PG_Referenced 4
#define PG_Dirty 8
#define PG_Active 16
#define PG_Up_To_Date 32
#define PG_Device 64
#define PG_Kernel 128
#define PG_K_Share_To_U 256
#define PG_Slab 512*/

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

#define SIZEOF_LONG_ALIGN(size) ((size+sizeof(long)-1)&~(sizeof(long)-1))
#define SIZEOF_INT_ALIGN(size) ((size+sizeof(int)-1)&~(sizeof(int)-1))


#define ZONE_DMA 1
#define ZONE_NORMAL 2
#define ZONE_UNMAPED 4

struct Slab{
	struct List list;
	struct Page* page;
	unsigned long using_count;
	unsigned long free_count;
	void* Vaddress;
	unsigned long color_length;
	unsigned long color_count;
	unsigned long *color_map;
};
struct Slab_cache{
	unsigned long size;
	unsigned long total_using;
	unsigned long total_free;
	struct Slab* cache_pool;
	struct Slab* cache_dma_pool;
	void*(*constructor)(void* Vaddress,unsigned long arg);
	void*(*destructor)(void* Vaddress,unsigned long arg);
};
struct Slab_cache kmalloc_cache_size[16]={
	{32,0,0,0,0,0,0},
	{64,0,0,0,0,0,0},
	{128,0,0,0,0,0,0},
	{256,0,0,0,0,0,0},
	{512,0,0,0,0,0,0},
	{1024,0,0,0,0,0,0},
	{2048,0,0,0,0,0,0},
	{4096,0,0,0,0,0,0},
	{8192,0,0,0,0,0,0},
	{16384,0,0,0,0,0,0},
	{32768,0,0,0,0,0,0},
	{65536,0,0,0,0,0,0},
	{131072,0,0,0,0,0,0},
	{262144,0,0,0,0,0,0},
	{524288,0,0,0,0,0,0},
	{1048576,0,0,0,0,0,0}
};
extern unsigned long page_init(struct Page* page,unsigned long flags);
extern unsigned long page_clean(struct Page* page);
extern unsigned long get_page_attribute(struct Page* page);
extern unsigned long set_page_attribute(struct Page* page,unsigned long flags);
extern struct Page* alloc_pages(int zone_select,int number,unsigned long page_flags);
extern void free_pages(struct Page* page,int number);
extern void* kmalloc(unsigned long size,unsigned long gfp_flags);
extern unsigned long kfree(void* address);

#endif
