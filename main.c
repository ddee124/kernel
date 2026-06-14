#include "lib.h"
#include "multiboot2.h"
#include "printk.h"
#include "memory.h"
#include "gate.h"
#include "task.h"
#include "cpu.h"
#include "APIC.h"
#include "keyboard.h"
#include "mouse.h"
#include "ACPI.h"
#include "time.h"
#include "softirq.h"
#include "timer.h"
#include "schedule.h"
extern void SMP_Init();
extern char _text;
extern char _etext;
extern char _edata;
extern char _end;
extern unsigned long _stack_start;
struct time time_;
extern void HPET_init();
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
	spin_init(&Pos.printk_lock);
	load_TR(10);
	set_tss64(TSS64_Table,_stack_start,_stack_start,_stack_start,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
	sys_vector_init();
	memory_management_struct.start_code=(unsigned long)&_text;
	memory_management_struct.end_code=(unsigned long)&_etext;
	memory_management_struct.end_data=(unsigned long)&_edata;
	memory_management_struct.end_brk=(unsigned long)&_end;
	init_memory(mmap_tag);
	color_printk(0xff00,0,"mbi_addr:%#018lx\n",mbi_addr);
	color_printk(0xff00,0,"FB_addr:%#018lx\n",Pos.FB_addr);
	struct multiboot2_tag_acpi_new* acpi_tag=(struct multiboot2_tag_acpi_new*)get_tag(mbi_addr,15);
	if(!acpi_tag){
		color_printk(0xff0000,0," No ACPI v2.0 Support!");
		while(1);
	}
	ACPI_init(acpi_tag);
	color_printk(0xff00,0,"Display CPU information\n");
	init_cpu();
	SMP_Init();
	color_printk(0xff00,0,"Init APIC\n");
	APIC_init();
	//this should be done after ap booted
	spin_lock(&SMP_lock);
	for(int i=0;i<10;i++)	*(Phy_To_Virt(Global_CR3)+i)=0ul;
	flush_tlb();
	spin_unlock(&SMP_lock);
	/*struct INT_CMD_REG icr_entry;
	icr_entry.vector=0xc8;
	icr_entry.deliver_mode=APIC_ICR_IOAPIC_Fixed;
	icr_entry.destination.x2apic_destination=1;
	icr_entry.dest_mode=ICR_IOAPIC_DELV_PHYSICAL;
	icr_entry.deliver_status=APIC_ICR_IOAPIC_Idle;
	icr_entry.res_1=0;
	icr_entry.level=ICR_LEVEL_DE_ASSERT;
	icr_entry.trigger=APIC_ICR_IOAPIC_Edge;
	icr_entry.res_2=0;
	icr_entry.res_3=0;
	icr_entry.dest_shorthand=ICR_No_Shorthand;
	wrmsr(0x830,*(unsigned long*)&icr_entry);*/
	color_printk(0xff00,0,"Init keyboard\n");
	keyboard_init();
	color_printk(0xff00,0,"Init mouse\n");
	mouse_init();
	schedule_init();
	softirq_init();
	timer_init();
	HPET_init();
	sti();
	task_init();
	while(1){
		if(p_kb->count)	analysis_keycode();
		if(p_mouse->count)	analysis_mousecode();
	}
}
