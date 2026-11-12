#include "mem/memory.h"
#include "mem/pagetable.h"
#include "lib/multiboot2.h"
#include "kernel/printk.h"
#include "lib/lib.h"
#include "arch/x86_64/trap.h"
#include "arch/x86_64/gate.h"
#include "mem/slab.h"
#include "driver/acpi/acpi.h"
#include "driver/apic/apic.h"
#include "driver/cpu/cpu.h"
#include "driver/ps2/ps2.h"
#include "driver/ps2/keyboard.h"
#include "driver/ps2/mouse.h"
#include "driver/serial/serial.h"
#include "driver/hpet/hpet.h"
#include "driver/rtc/rtc.h"
uint8_t TSS_Table[128];
void Start_Kernel(void* mbi_addr){
	serial_init();
	color_printk(WHITE,BLACK,"\n[kernel] entry,mbi:%#018lx\n",(uint64_t)mbi_addr);
	mbi_addr=((void*)(uintptr_t)(mbi_addr)+0xffff800000000000);
	struct multiboot2_tag_framebuffer* fb_tag=(struct multiboot2_tag_framebuffer*)get_tag(mbi_addr,8);
	if(fb_tag&&fb_tag->framebuffer_type==1&&fb_tag->bpp==32){
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
	else	color_printk(RED,BLACK,"[kernel] no usable framebuffer,serial only\n");
	struct multiboot2_tag_mmap *mmap_tag=(struct multiboot2_tag_mmap*)get_tag(mbi_addr,6);
	if(!mmap_tag){
		color_printk(RED,BLACK,"[kernel] no mmap tag\n");
		while(1)	halt();
	}
	init_memory(mmap_tag,mbi_addr);
	RTC_show();
	display_memory_info();
	set_tss_descriptor(6,TSS_Table);
	struct Page* TSS_page=alloc_pages(1,0,0);
	if(!TSS_page){
		color_printk(RED,BLACK,"[kernel] no page for tss stack\n");
		while(1)	halt();
	}
	uintptr_t tss_stk_addr=Phy_To_Virt(TSS_page->PHY_address+PAGE_4K_SIZE);
	set_tss64((uint32_t*)TSS_Table,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr,tss_stk_addr);
	load_TR(6);
	color_printk(CYAN,BLACK,"tss stack pointer at:%#018lx\n",tss_stk_addr);
	sys_vector_init();
	slab_init();
	//slab_test();
	struct multiboot2_tag_acpi_new* acpi_tag=(struct multiboot2_tag_acpi_new*)get_tag(mbi_addr,15);
	if(!acpi_tag){
		color_printk(RED,BLACK,"[kernel] no acpi tag,stop before apic\n");
		while(1)	halt();
	}
	ACPI_init(acpi_tag);
	cpu_info();
	APIC_init();
	ps2_init();
	keyboard_init();
	mouse_init();
	HPET_init();
	sti();
	color_printk(GREEN,BLACK,"[kernel] boot ok,hpet %luHz\n",HPET_frequency());
	/*uint64_t x;
	x=*(volatile uint64_t*)(0xffff880000000000);*/
	while(1)	halt();
}
uint64_t init_task_union[32768/8];
