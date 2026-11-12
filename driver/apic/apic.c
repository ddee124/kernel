#include "driver/cpu/cpu.h"
#include "kernel/printk.h"
#include "lib/lib.h"
#include "lib/io.h"
#include "mem/pagetable.h"
#include "driver/apic/apic.h"
#include "driver/acpi/acpi.h"
#include "interrupt/interrupt.h"
#include "arch/x86_64/gate.h"
uint64_t bsp_id;
uint64_t ioapic_rte_read(uint8_t index){
	uint64_t ret;
	*(ioapic_map.virtual_index_addr)=0x10+(index<<1)+1;
	io_mfence();
	ret=*(ioapic_map.virtual_data_addr);
	ret<<=32;
	io_mfence();
	*(ioapic_map.virtual_index_addr)=0x10+(index<<1);
	io_mfence();
	ret|=*(ioapic_map.virtual_data_addr);
	io_mfence();
	return ret;
}
void ioapic_rte_write(uint8_t index,uint64_t value){
	*(ioapic_map.virtual_index_addr)=0x10+(index<<1);
	io_mfence();
	*(ioapic_map.virtual_data_addr)=value&0xfffffffful;
	value>>=32;
	io_mfence();
	*(ioapic_map.virtual_index_addr)=0x10+(index<<1)+1;
	io_mfence();
	*(ioapic_map.virtual_data_addr)=value&0xfffffffful;
	io_mfence();
}
void IOAPIC_enable(uint64_t irq){
	uint64_t value=0;
	value=ioapic_rte_read(irq-32);
	value=value&(~0x10000ul);
	ioapic_rte_write(irq-32,value);
}
void IOAPIC_disable(uint64_t irq){
	uint64_t value=0;
	value=ioapic_rte_read(irq-32);
	value=value|0x10000ul;
	ioapic_rte_write(irq-32,value);
}
uint64_t IOAPIC_install(uint64_t irq,void* arg){
	struct IO_APIC_RET_entry* entry=(struct IO_APIC_RET_entry*)arg;
	ioapic_rte_write(irq-32,*(uint64_t*)entry);
	return 1;
}
void IOAPIC_uninstall(uint64_t irq){
	ioapic_rte_write(irq-32,0x10000ul);
}
void IOAPIC_edge_ack(uint64_t irq){
	__asm__ __volatile__(
		"movq $0x00,%%rdx \n\t"
		"movq $0x00,%%rax \n\t"
		"movq $0x80b,%%rcx \n\t"
		"wrmsr \n\t"
		:::"memory"
	);
}
void IOAPIC_level_ack(uint64_t irq){
	__asm__ __volatile__(
		"movq $0x00,%%rdx \n\t"
		"movq $0x00,%%rax \n\t"
		"movq $0x80b,%%rcx \n\t"
		"wrmsr \n\t"
		:::"memory"
	);
	*ioapic_map.virtual_EOI_addr=0;
}
void Local_APIC_edge_level_ack(uint64_t irq){
	__asm__ __volatile__(
		"movq $0x00,%%rdx \n\t"
		"movq $0x00,%%rax \n\t"
		"movq $0x80b,%%rcx \n\t"
		"wrmsr \n\t"
		:::"memory"
	);
}
void Local_APIC_init(){
	uint32_t a,b,c,d;uint64_t x;
	//check x2APIC support
	get_cpuid(1,0,&a,&b,&c,&d);
	if((1<<21)&c)	color_printk(WHITE,BLACK,"Hardware support x2APIC\n");
	else{
		color_printk(RED,BLACK,"Hardware NOT support x2APIC\n");
		while(1);
	}
	color_printk(CYAN,BLACK,"io apic address:%#018lx\n",(uint64_t)Phy_To_Virt(ioapic_map.phy_addr));
	//enable x2APIC
	x=rdmsr(0x1b);
	x|=(1<<10)|(1<<11);
	wrmsr(0x1b,x);
	//enable SVR[8] SVR[12]
	x=rdmsr(0x80f);
	x|=(1<<8)|(1<<12)|0xff;
	wrmsr(0x80f,x);
	x=rdmsr(0x80f);
	//color_printk(WHITE,BLACK,"eax:%#010x,edx:%#010x\t",x,y);
	if(x&0x100)	color_printk(WHITE,BLACK,"SVR[8] enabled\n");
	if(x&0x1000)	color_printk(WHITE,BLACK,"SVR[12] enabled\n");
	//get local APIC ID
	x=rdmsr(0x802);
	color_printk(WHITE,BLACK,"x2APIC ID:%#010x\n",x&0xfffffffful);
	bsp_id=x&0xfffffffful;
	//get local APIC version
	x=rdmsr(0x803);
	color_printk(WHITE,BLACK,"local APIC Version:%#010x,Max LVT Entry:%#010x,SVR(Suppress EOI Broadcast):%#04x\n",x&0xff,(x>>16&0xff)+1,x>>24&0x1);
	//mask all LVT
	//wrmsr(0x82f,0x10000);//CMCI
	wrmsr(0x832,0x10000);//Timer
	wrmsr(0x833,0x10000);//Thermal Monitor
	wrmsr(0x834,0x10000);//Performance Counter
	wrmsr(0x835,0x10000);//LINT0
	wrmsr(0x836,0x10000);//LINT1
	wrmsr(0x837,0x10000);//Error
	color_printk(GREEN,BLACK,"Mask ALL LVT\n");
	//TPR
	x=rdmsr(0x808);
	color_printk(GREEN,BLACK,"LVT TPR:%#010x\t",x&0xfffffffful);
	//PPR
	x=rdmsr(0x80a);
	color_printk(GREEN,BLACK,"LVT PPR:%#010x\n",x);
}
void IO_APIC_init(){
	uint64_t i;
	uint32_t x;
	//I/O APIC ID
	io_mfence();
	*ioapic_map.virtual_index_addr=0x00;
	io_mfence();
	*ioapic_map.virtual_data_addr=0x0f000000;
	io_mfence();
	x=*ioapic_map.virtual_data_addr;
	io_mfence();
	color_printk(GREEN,BLACK,"Get IOAPIC ID REG:%#010x,ID:%#010x\n",x,x>>24&0xf);
	//I/O APIC Version
	io_mfence();
	*ioapic_map.virtual_index_addr=0x01;
	io_mfence();
	x=*ioapic_map.virtual_data_addr;
	io_mfence();
	color_printk(GREEN,BLACK,"Get IOAPIC Version REG:%#010x,MAX redirection enties:%d\n",x,((x>>16)&0xff)+1);
	io_mfence();
	//RTE
	for(i=0;i<=((x>>16)&0xff);i++)	ioapic_rte_write(i,0x10020+i);
	ioapic_rte_write(0x1,0x21);//enable keyboard
	color_printk(GREEN,BLACK,"I/O APIC Redirection Table Entries Set Finished.\n");	
}
void solve_madt(struct acpi_madt* madt){
	if(other_broken(&madt->header)){
		color_printk(RED,BLACK,"MADT Table broken!");
		while(1);
	}
	uintptr_t start=(uintptr_t)madt->entries;
	uintptr_t end=(uintptr_t)madt+madt->header.length;
	while(start<end){
		if(*((uint8_t*)(start))==1){
			ioapic_map.phy_addr=*((uint32_t*)(start+4));
			ioapic_map.virtual_index_addr=(uint8_t*)Phy_To_Virt(ioapic_map.phy_addr);
			ioapic_map.virtual_data_addr=(uint32_t*)Phy_To_Virt(ioapic_map.phy_addr+0x10);
			ioapic_map.virtual_EOI_addr=(uint32_t*)Phy_To_Virt(ioapic_map.phy_addr+0x40);
		}
		start=start+*((uint8_t*)(start+1));
	}
}
void APIC_init(){
	uint64_t i;
	solve_madt(madt_table);
	//APIC_pagetable_map();
	map_4K_page(ioapic_map.phy_addr,PAGE_PCD);
	for(i=32;i<56;i++)	set_intr_gate(IDT_Table+i,0,(uintptr_t)interrupt[i-32]);
	set_intr_gate(IDT_Table+0xff,0,(uintptr_t)IRQ0xff_interrupt);
	//mask 8259A
	color_printk(GREEN,BLACK,"MASK 8259A\n");
	io_out8(0x21,0xff);
	io_out8(0xa1,0xff);
	//init local apic
	Local_APIC_init();
	//init io apic
	IO_APIC_init();
	memset(interrupt_desc,0,sizeof(struct irq_desc_T)*NR_IRQS);
	//boot_ap(bsp_id);
	//enable IF rflags
	//sti();
}
