#include "cpu.h"
#include "printk.h"
#include "gate.h"
#include "lib.h"
#include "memory.h"
#include "APIC.h"
#include "ptrace.h"
#include "interrupt.h"
#include "ACPI.h"
#include "task.h"
int global_i;
unsigned int bsp_id;
unsigned long ioapic_rte_read(unsigned char index){
	unsigned long ret;
	*(ioapic_map.virtual_index_addr)=index+1;
	io_mfence();
	ret=*(ioapic_map.virtual_data_addr);
	ret<<=32;
	io_mfence();
	*(ioapic_map.virtual_index_addr)=index;
	io_mfence();
	ret|=*(ioapic_map.virtual_data_addr);
	io_mfence();
	return ret;
}
void ioapic_rte_write(unsigned char index,unsigned long value){
	*(ioapic_map.virtual_index_addr)=index;
	io_mfence();
	*(ioapic_map.virtual_data_addr)=value&0xfffffffful;
	value>>=32;
	io_mfence();
	*(ioapic_map.virtual_index_addr)=index+1;
	io_mfence();
	*(ioapic_map.virtual_data_addr)=value&0xfffffffful;
	io_mfence();
}
void IOAPIC_enable(unsigned long irq){
	unsigned long value=0;
	value=ioapic_rte_read((irq-32)*2+0x10);
	value=value&(~0x10000ul);
	ioapic_rte_write((irq-32)*2+0x10,value);
}
void IOAPIC_disable(unsigned long irq){
	unsigned long value=0;
	value=ioapic_rte_read((irq-32)*2+0x10);
	value=value|0x10000ul;
	ioapic_rte_write((irq-32)*2+0x10,value);
}
unsigned long IOAPIC_install(unsigned long irq,void* arg){
	struct IO_APIC_RET_entry* entry=(struct IO_APIC_RET_entry*)arg;
	ioapic_rte_write((irq-32)*2+0x10,*(unsigned long*)entry);
	return 1;
}
void IOAPIC_uninstall(unsigned long irq){
	ioapic_rte_write((irq-32)*2+0x10,0x10000UL);
}
void IOAPIC_edge_ack(unsigned long irq){
	__asm__ __volatile__(
		"movq $0x00,%%rdx \n\t"
		"movq $0x00,%%rax \n\t"
		"movq $0x80b,%%rcx \n\t"
		"wrmsr \n\t"
		:::"memory"
	);
}
void IOAPIC_level_ack(unsigned long irq){
	__asm__ __volatile__(
		"movq $0x00,%%rdx \n\t"
		"movq $0x00,%%rax \n\t"
		"movq $0x80b,%%rcx \n\t"
		"wrmsr \n\t"
		:::"memory"
	);
	*ioapic_map.virtual_EOI_addr=0;
}
void(*interrupt[24])(void)={
	IRQ0x20_interrupt,IRQ0x21_interrupt,IRQ0x22_interrupt,IRQ0x23_interrupt,
	IRQ0x24_interrupt,IRQ0x25_interrupt,IRQ0x26_interrupt,IRQ0x27_interrupt,
	IRQ0x28_interrupt,IRQ0x29_interrupt,IRQ0x2a_interrupt,IRQ0x2b_interrupt,
	IRQ0x2c_interrupt,IRQ0x2d_interrupt,IRQ0x2e_interrupt,IRQ0x2f_interrupt,
	IRQ0x30_interrupt,IRQ0x31_interrupt,IRQ0x32_interrupt,IRQ0x33_interrupt,
	IRQ0x34_interrupt,IRQ0x35_interrupt,IRQ0x36_interrupt,IRQ0x37_interrupt
};
void do_IRQ(struct pt_regs *regs,unsigned long nr){
	struct irq_desc_T* irq=&interrupt_desc[nr-32];
	if(irq->handler!=0)	irq->handler(nr,irq->parameter,regs);
	if(irq->controller!=0&&irq->controller->ack!=0)	irq->controller->ack(nr);
}
void Local_APIC_init(){
	unsigned int x,y,a,b,c,d;
	//check x2APIC support
	get_cpuid(1,0,&a,&b,&c,&d);
	if((1<<21)&c)	color_printk(WHITE,BLACK,"Hardware support x2APIC\n");
	else{
		color_printk(RED,BLACK,"Hardware NOT support x2APIC\n");
		while(1);
	}
	color_printk(CYAN,BLACK,"io apic address:%#018lx\n",(unsigned long)Phy_To_Virt(ioapic_map.phy_addr));
	//enable x2APIC
	__asm__ __volatile__(
		"movq $0x1b,%%rcx \n\t"
		"rdmsr \n\t"
		"bts $10,%%rax \n\t"
		"bts $11,%%rax \n\t"
		"wrmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	if(x&0xc00)	color_printk(WHITE,BLACK,"xAPIC & x2APIC enabled\n");
	else{
		color_printk(RED,BLACK," x2APIC enable failed!\n");
		while(1);
	}
	//enable SVR[8] SVR[12]
	__asm__ __volatile__(
		"movq $0x80f,%%rcx \n\t"
		"rdmsr \n\t"
		"bts $8,%%rax \n\t"
		//"bts $12,%%rax \n\t"
		"wrmsr \n\t"
		"movq $0x80f,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	//color_printk(WHITE,BLACK,"eax:%#010x,edx:%#010x\t",x,y);
	if(x&0x100)	color_printk(WHITE,BLACK,"SVR[8] enabled\n");
	//if(x&0x1000)	color_printk(WHITE,BLACK,"SVR[12] enabled\n");
	//get local APIC ID
	__asm__ __volatile__(
		"movq $0x802,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	color_printk(WHITE,BLACK,"eax:%#010x,edx:%#010x\tx2APIC ID:%#010x\n",x,y,x);
	bsp_id=x;
	//get local APIC version
	__asm__ __volatile__(
		"movq $0x803,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	color_printk(WHITE,BLACK,"local APIC Version:%#010x,Max LVT Entry:%#010x,SVR(Suppress EOI Broadcast):%#04x\t",x&0xff,(x>>16&0xff)+1,x>>24&0x1);
	if((x&0xff)<0x10)	color_printk(WHITE,BLACK,"82489DX discrete APIC\n");
	else{
		if((x&0xff)>=0x10&&(x&0xff)<=0x15)	color_printk(WHITE,BLACK,"Integrated APIC\n");
	}
	//mask all LVT
	__asm__ __volatile__(
		//"movq $0x82f,%%rcx \n\t" //CMCI
		//"wrmsr \n\t"
		"movq $0x832,%%rcx \n\t" //Timer
		"wrmsr \n\t"
		"movq $0x833,%%rcx \n\t" //Thermal Monitor
		"wrmsr \n\t"
		"movq $0x834,%%rcx \n\t" //Performance Counter
		"wrmsr \n\t"
		"movq $0x835,%%rcx \n\t" //LINT0
		"wrmsr \n\t"
		"movq $0x836,%%rcx \n\t" //LINT1
		"wrmsr \n\t"
		"movq $0x837,%%rcx \n\t" //Error
		"wrmsr \n\t"
		:
		:"a"(0x10000),"d"(0x00)
		:"memory"
	);
	color_printk(GREEN,BLACK,"Mask ALL LVT\n");
	//TPR
	__asm__ __volatile__(
		"movq $0x808,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	color_printk(GREEN,BLACK,"Set LVT TPR:%#010x\t",x);
	//PPR
	__asm__ __volatile__(
		"movq $0x80a,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	color_printk(GREEN,BLACK,"Set LVT PPR:%#010x\n",x);
}
void APIC_pagetable_map(){
	unsigned long* tmp;
	//ioapic_map.phy_addr=0xfec00000;
	unsigned char* IOAPIC_addr=(unsigned char*)Phy_To_Virt(ioapic_map.phy_addr);
	ioapic_map.virtual_index_addr=IOAPIC_addr;
	ioapic_map.virtual_data_addr=(unsigned int*)(IOAPIC_addr+0x10);
	ioapic_map.virtual_EOI_addr=(unsigned int*)(IOAPIC_addr+0x40);
	Global_CR3=Get_gdt();
	unsigned long pml4_idx=0,pdpt_idx=0,pd_idx=0;
	pml4_idx=((unsigned long)IOAPIC_addr>>PAGE_GDT_SHIFT)&0x1ff;
	pdpt_idx=((unsigned long)IOAPIC_addr>>PAGE_1G_SHIFT)&0x1ff;
	pd_idx=((unsigned long)IOAPIC_addr>>PAGE_2M_SHIFT)&0x1ff;
	tmp=Phy_To_Virt((unsigned long)Global_CR3&(~0xffful));
	if(tmp[pml4_idx]==0){
		unsigned long *virtual=kmalloc(PAGE_4K_SIZE,0);
		memset(virtual,0,PAGE_4K_SIZE);
		tmp[pml4_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
	}
	//color_printk(YELLOW,BLACK,"1:%#018lx\t%#018lx\n",(unsigned long)(tmp+pml4_idx),(unsigned long)tmp[pml4_idx]);
	tmp=Phy_To_Virt((unsigned long)tmp[pml4_idx]&(~0xffful));
	if(tmp[pdpt_idx]==0){
		unsigned long *virtual=kmalloc(PAGE_4K_SIZE,0);
		memset(virtual,0,PAGE_4K_SIZE);
		tmp[pdpt_idx]=Virt_To_Phy(virtual)|PAGE_KERNEL_Dir;
	}
	//color_printk(YELLOW,BLACK,"2:%#018lx\t%#018lx\n",(unsigned long)(tmp+pdpt_idx),(unsigned long)tmp[pdpt_idx]);
	tmp=Phy_To_Virt((unsigned long)tmp[pdpt_idx]&(~0xffful));
	if(!(tmp[pd_idx]&PAGE_Present)) tmp[pd_idx]=Virt_To_Phy(IOAPIC_addr)|PAGE_KERNEL_Page|PAGE_PWT|PAGE_PCD;
	//color_printk(YELLOW,BLACK,"3:%#018lx\t%#018lx\n",(unsigned long)(tmp+pd_idx),(unsigned long)tmp[pd_idx]);
	//color_printk(BLUE,BLACK,"ioapic_map.physical_address:%#010x\t\t\n",ioapic_map.phy_addr);
	//color_printk(BLUE,BLACK,"ioapic_map.virtual_address:%#018lx\t\t\n",(unsigned long)ioapic_map.virtual_index_addr);
	flush_tlb();
}
void IO_APIC_init(){
	int i;
	//I/O APIC ID
	*ioapic_map.virtual_index_addr=0x00;
	io_mfence();
	*ioapic_map.virtual_data_addr=0x0f000000;
	io_mfence();
	color_printk(GREEN,BLACK,"Get IOAPIC ID REG:%#010x,ID:%#010x\n",*ioapic_map.virtual_data_addr,*ioapic_map.virtual_data_addr>>24&0xf);
	io_mfence();
	//I/O APIC Version
	*ioapic_map.virtual_index_addr=0x01;
	io_mfence();
	color_printk(GREEN,BLACK,"Get IOAPIC Version REG:%#010x,MAX redirection enties:%#08d\n",*ioapic_map.virtual_data_addr,((*ioapic_map.virtual_data_addr>>16)&0xff)+1);
	io_mfence();
	//RTE
	for(i=0x10;i<0x40;i+=2)	ioapic_rte_write(i,0x10020+((i-0x10)>>1));
	//ioapic_rte_write(0x12,0x21);//enable keyboard
	//ioapic_rte_write(0x28,0x2c);//enable mouse
	color_printk(GREEN,BLACK,"I/O APIC Redirection Table Entries Set Finished.\n");	
}
void APIC_init(){
	int i;
	unsigned int x;
	unsigned int* p;
	APIC_pagetable_map();
	for(i=32;i<56;i++)	set_intr_gate(i,2,interrupt[i-32]);
	//mask 8259A
	color_printk(GREEN,BLACK,"MASK 8259A\n");
	io_out8(0x21,0xff);
	io_out8(0xa1,0xff);
	//init local apic
	Local_APIC_init();
	//init io apic
	IO_APIC_init();
	memset(interrupt_desc,0,sizeof(struct irq_desc_T)*NR_IRQS);
	boot_ap(bsp_id);
	//enable IF rflags
	sti();
}
