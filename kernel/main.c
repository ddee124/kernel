#include "mem/memory.h"
#include "lib/multiboot2.h"
#include "kernel/printk.h"
#include "lib/lib.h"
#include "arch/x86_64/trap.h"
#include "arch/x86_64/gate.h"
#include "mem/slab.h"
#include "driver/acpi/acpi.h"
#include "driver/cpu/cpu.h"
uint8_t TSS_Table[128];
void Start_Kernel(void* mbi_addr){
	mbi_addr=((void*)(uintptr_t)(mbi_addr)+0xffff800000000000);
	struct multiboot2_tag_framebuffer* fb_tag=(struct multiboot2_tag_framebuffer*)get_tag(mbi_addr,8);
	if(fb_tag){
		if(fb_tag->framebuffer_type!=1)	while(1);
		if(fb_tag->bpp!=32)	while(1);
		Pos.FB_addr=fb_tag->addr;
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
	init_memory(mmap_tag);
	//display_memory_info();
	set_tss_descriptor(6,TSS_Table);
	set_tss64((uint32_t*)TSS_Table,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
	load_TR(6);
	sys_vector_init();
	slab_init();
	//slab_test();
	struct multiboot2_tag_acpi_new* acpi_tag=(struct multiboot2_tag_acpi_new*)get_tag(mbi_addr,15);
	if(!acpi_tag){
		color_printk(RED,BLACK,"no acpi table!");
		while(1);
	}
	ACPI_init(acpi_tag);
	cpu_info();
	uint64_t x;
	x=*(volatile uint64_t*)(0xfffffffffffffff0);
	while(1)	halt();
}
uint64_t init_task_union[32768/8];
