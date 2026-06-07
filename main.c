#include "lib.h"
#include "multiboot2.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"
#include "interrupt.h"
#include "task.h"
#include "cpu.h"
extern char _text;
extern char _etext;
extern char _edata;
extern char _end;
/*void test_memory(){
	void* tmp=0;int i=0;struct Slab* slab;
	color_printk(WHITE,BLACK,"kmalloc test\n");
	for(i = 0;i< 16;i++)
	{
		color_printk(RED,BLACK,"size:%#010x\t",kmalloc_cache_size[i].size);
		color_printk(RED,BLACK,"color_map(before):%#018lx\t",*kmalloc_cache_size[i].cache_pool->color_map);
		tmp = kmalloc(kmalloc_cache_size[i].size,0);
		if(tmp ==0)
			color_printk(RED,BLACK,"kmalloc size:%#010x ERROR\n",kmalloc_cache_size[i].size);
		color_printk(RED,BLACK,"color_map(middle):%#018lx\t",*kmalloc_cache_size[i].cache_pool->color_map);
		kfree(tmp);
		color_printk(RED,BLACK,"color_map(after):%#018lx\n",*kmalloc_cache_size[i].cache_pool->color_map);
	}

	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);
	kmalloc(kmalloc_cache_size[15].size,0);


	color_printk(RED,BLACK,"color_map(0):%#018lx,%#018lx\n",kmalloc_cache_size[15].cache_pool->color_map,*kmalloc_cache_size[15].cache_pool->color_map);
	slab=(struct Slab*)(list_next(&kmalloc_cache_size[15].cache_pool->list));
	color_printk(RED,BLACK,"color_map(1):%#018lx,%#018lx\n",slab->color_map,*slab->color_map);
	slab=(struct Slab*)(list_next(&slab->list));
	color_printk(RED,BLACK,"color_map(2):%#018lx,%#018lx\n",slab->color_map,*slab->color_map);
	slab=(struct Slab*)(list_next(&slab->list));
	color_printk(RED,BLACK,"color_map(3):%#018lx,%#018lx\n",slab->color_map,*slab->color_map);

}*/
void Start_Kernel(unsigned long mbi_addr){
	//Global_CR3=Get_gdt();
	//for(unsigned long i=0;i<0x80000000;i+=0x200000)	map_2M_page(i);
	//flush_tlb();
	struct multiboot2_tag_framebuffer *fb_tag=(struct multiboot2_tag_framebuffer*)get_tag(mbi_addr,8);
	if(fb_tag){
		if(fb_tag->framebuffer_type!=1)	while(1);
		if(fb_tag->bpp!=32)	while(1);
		Pos.FB_addr=(void*)fb_tag->addr;
		Pos.FB_addr+=0xffff800000000000;
		Pos.XResolution=fb_tag->width;
		Pos.YResolution=fb_tag->height;
		Pos.XPosition=0;
		Pos.YPosition=0;
		Pos.XCharSize=8;
		Pos.YCharSize=16;
		Pos.pitch=fb_tag->pitch;
		Pos.FB_length=Pos.pitch*Pos.YResolution;
	}
	else while(1);
	struct multiboot2_tag_mmap *mmap_tag=(struct multiboot2_tag_mmap*)get_tag(mbi_addr,6);
	if(!mmap_tag)	while(1);
	load_TR(10);
	set_tss64(0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
	sys_vector_init();
	memory_management_struct.start_code=(unsigned long)&_text;
	memory_management_struct.end_code=(unsigned long)&_etext;
	memory_management_struct.end_data=(unsigned long)&_edata;
	memory_management_struct.end_brk=(unsigned long)&_end;
	init_memory(mmap_tag);
	color_printk(0xff00,0,"FB_addr:%#018lx\n",Pos.FB_addr);
	init_cpu();
	init_interrupt();
	//test_memory();
	//task_init();
	unsigned long u,v;
	__asm__ __volatile("sidt %0":"=m"(u)::"memory");
	while(1);
}
