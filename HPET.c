#include "ACPI.h"
#include "printk.h"
#include "memory.h"
#include "interrupt.h"
#include "APIC.h"
#include "time.h"
#include "softirq.h"
#include "timer.h"
#include "schedule.h"
extern struct time time_;
unsigned long volatile jiffies=0;
unsigned char* check(struct acpi_hpet* hpet){
	unsigned char x=0;unsigned int i=0;
	for(i=0;i<hpet->header.length;i++)	x+=*((unsigned char*)hpet+i);
	if(x){color_printk(RED,BLACK,"HPET Table broken!");while(1);}
	if(hpet->address_space_id==1){color_printk(RED,BLACK,"HPET not mapped in memory!");while(1);}
	return (unsigned char*)Phy_To_Virt(hpet->address);
}
void HPET_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs){
	//color_printk(RED,WHITE,"(HPET)");
	jiffies++;
	if(((struct timer_list*)list_next(&timer_list_head.list))->expire_jiffies<=jiffies)	set_softirq_status(TIMER_SIRQ);
	switch(current->priority){
		case 0:
		case 1:
			task_schedule.CPU_exec_task_jiffies--;
			current->vrun_time++;
			break;
		case 2:
		default:
			task_schedule.CPU_exec_task_jiffies-=2;
			current->vrun_time+=2;
			break;
	}
	if(task_schedule.CPU_exec_task_jiffies<=0)	current->flags|=NEED_SCHEDULE;
}
struct hw_int_controller HPET_int_controller={
	.enable=IOAPIC_enable,
	.disable=IOAPIC_disable,
	.install=IOAPIC_install,
	.uninstall=IOAPIC_uninstall,
	.ack=IOAPIC_edge_ack,
};
unsigned char* HPET_addr;
unsigned long counter_clk_period;
void HPET_init(){
	unsigned int x;
	unsigned int* p;
	HPET_addr=check(hpet);
	unsigned short pml4_idx,pdpt_idx,pd_idx;
	pml4_idx=(((unsigned long)HPET_addr)>>PAGE_GDT_SHIFT)&0x1ff;
	pdpt_idx=(((unsigned long)HPET_addr)>>PAGE_1G_SHIFT)&0x1ff;
	pd_idx=(((unsigned long)HPET_addr)>>PAGE_2M_SHIFT)&0x1ff;
	unsigned long* tmp=Phy_To_Virt(Global_CR3&(~0xffful));
	if(!(tmp[pml4_idx]&PAGE_Present)){
		unsigned long* virtual=kmalloc(4096,0);
		tmp[pml4_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
	}
	tmp=Phy_To_Virt(tmp[pml4_idx]&(~0xffful));
	if(!(tmp[pdpt_idx]&PAGE_Present)){
		unsigned long* virtual=kmalloc(4096,0);
		tmp[pdpt_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
	}
	tmp=Phy_To_Virt(tmp[pdpt_idx]&(~0xffful));
	if(!(tmp[pd_idx]&PAGE_Present))	tmp[pd_idx]=Virt_To_Phy(HPET_addr)|PAGE_KERNEL_Page|PAGE_PCD|PAGE_PWT;
	flush_tlb();
	struct IO_APIC_RET_entry entry;
	entry.vector=0x22;
	entry.deliver_mode=APIC_ICR_IOAPIC_Fixed;
	entry.dest_mode=ICR_IOAPIC_DELV_PHYSICAL;
	entry.deliver_status=APIC_ICR_IOAPIC_Idle;
	entry.polarity=APIC_IOAPIC_POLARITY_HIGH;
	entry.irr=APIC_IOAPIC_IRR_RESET;
	entry.trigger=APIC_ICR_IOAPIC_Edge;
	entry.mask=APIC_ICR_IOAPIC_Masked;
	entry.reserved=0;
	entry.destination.physical.reserved_1=0;
	entry.destination.physical.phy_dest=0;
	entry.destination.physical.reserved_2=0;
	register_irq(0x22,&entry,&HPET_handler,0,&HPET_int_controller,"HPET");
	unsigned long tmp1=*(unsigned long*)HPET_addr;
	color_printk(RED,BLACK,"HPET - GCAP_ID:<%#018lx>\n",tmp1);
	counter_clk_period=tmp1>>32;
	color_printk(RED,BLACK,"HPET period:%ldfs\n",counter_clk_period);
	//enable and legacy replacement
	*(unsigned long*)(HPET_addr+0x10)=3;
	io_mfence();
	//edge triggered & periodic
	*(unsigned long*)(HPET_addr+0x100)=0x004c;
	io_mfence();
	//1s
	*(unsigned long*)(HPET_addr+0x108)=100000000000000ul/counter_clk_period;
	io_mfence();
	//init MAIN_CNT & get CMOS time
	get_cmos_time(&time_);
	*(unsigned long*)(HPET_addr+0xf0)=0;
	io_mfence();
	color_printk(RED,BLACK,"year:%#010x,month:%#010x,day:%#010x,hour:%#010x,mintue:%#010x,second:%#010x\n",time_.year,time_.month,time_.day,time_.hour,time_.minute,time_.second);
}
