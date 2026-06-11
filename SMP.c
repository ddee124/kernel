#include "cpu.h"
#include "printk.h"
#include "SMP.h"
#include "lib.h"
#include "gate.h"
#include "spinlock.h"
#include "ACPI.h"
#include "APIC.h"
#include "multiboot2.h"
#include "memory.h"
#include "task.h"
extern int global_i;
extern unsigned long _stack_start;
void SMP_Init(){
	int i;
	unsigned int a,b,c,d;
	//get local APIC ID
	for(i=0;;i++){
		get_cpuid(0xb,i,&a,&b,&c,&d);
		if((c>>8&0xff)==0)	break;
		//color_printk(WHITE,BLACK,"local APIC ID Package_../Core_2/SMT_1,type(%x)Width:%#010x,num of logical processor(%x)\n",c>>8&0xff,a&0x1f,b&0xff);
	}
	color_printk(WHITE,BLACK,"x2APIC ID level:%#010x\tx2APIC ID the current logical processor:%#010x\n",c&0xff,d);
	color_printk(WHITE,BLACK,"SMP copy byte:%#010x\n",(unsigned long)&_APU_boot_end-(unsigned long)&_APU_boot_start);
	memcpy(_APU_boot_start,(unsigned char*)0xffff800000020000,(unsigned long)&_APU_boot_end-(unsigned long)&_APU_boot_start);
	spin_init(&SMP_lock);
}
void Start_SMP(){
	color_printk(RED,YELLOW,"APU starting...... ");
	unsigned int x,y,a,b,c,d;
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
	if(x&0xc00)	color_printk(WHITE,BLACK,"xAPIC & x2APIC enabled ");
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
	if(x&0x100)	color_printk(WHITE,BLACK,"SVR[8] enabled ");
	//if(x&0x1000)	color_printk(WHITE,BLACK,"SVR[12] enabled\n");
	//get local APIC ID
	__asm__ __volatile__(
		"movq $0x802,%%rcx \n\t"
		"rdmsr \n\t"
		:"=a"(x),"=d"(y)
		:
		:"memory"
	);
	color_printk(WHITE,BLACK,"x2APIC ID:%#010x\n",x);
	load_TR(10+global_i*2);
	spin_unlock(&SMP_lock);
	while(1){
		__asm__ __volatile__ ("hlt");
	}
	//while(1);
}
void boot_ap(unsigned int bsp_id){
	struct INT_CMD_REG icr_entry;
	icr_entry.vector=0x00;
	icr_entry.deliver_mode=APIC_ICR_IOAPIC_INIT;
	icr_entry.dest_mode=ICR_IOAPIC_DELV_PHYSICAL;
	icr_entry.deliver_status=APIC_ICR_IOAPIC_Idle;
	icr_entry.res_1=0;
	icr_entry.level=ICR_LEVEL_ASSERT;
	icr_entry.trigger=APIC_ICR_IOAPIC_Level;
	icr_entry.res_2=0;
	icr_entry.dest_shorthand=ICR_ALL_EXCLUDE_Self;
	icr_entry.res_3=0;
	icr_entry.destination.x2apic_destination=0x00;
	wrmsr(0x830,*(unsigned long*)&icr_entry);
	unsigned int* tss=0;
	global_i=0;
	unsigned long start=(unsigned long)madt_addr->entries;
	unsigned long end=(unsigned long)madt_addr+madt_addr->header.length;
	while(start<end){
		if(*((unsigned char*)(start))==9){
			//color_printk(0xffff,0,"x2APIC ID:%lu enabled:%u\n",(unsigned long)*((unsigned int*)(start+4)),(unsigned int)(*((unsigned int*)(start+8))&1));
			if((*((unsigned int*)(start+8)))&1){
				if(*((unsigned int*)(start+4))!=bsp_id){
					spin_lock(&SMP_lock);
					global_i++;
					_stack_start=(unsigned long)kmalloc(STACK_SIZE,0)+STACK_SIZE;
					tss=(unsigned int*)kmalloc(128,0);
					set_tss_descriptor(10+global_i*2,tss);
					set_tss64(tss,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start);
					icr_entry.vector=0x20;
					icr_entry.deliver_mode=ICR_Start_up;
					icr_entry.dest_shorthand=ICR_No_Shorthand;
					icr_entry.destination.x2apic_destination=*((unsigned int*)(start+4));
					wrmsr(0x830,*(unsigned long*)&icr_entry);
					wrmsr(0x830,*(unsigned long*)&icr_entry);
				}
			}
		}
		if(*((unsigned char*)(start))==0){
			if((*((unsigned int*)(start+4)))&1){
				if(*((unsigned char*)(start+3))!=bsp_id){
					spin_lock(&SMP_lock);
					global_i++;
					_stack_start=(unsigned long)kmalloc(STACK_SIZE,0)+STACK_SIZE;
					tss=(unsigned int*)kmalloc(128,0);
					set_tss_descriptor(10+global_i*2,tss);
					set_tss64(tss,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start);
					icr_entry.vector=0x20;
					icr_entry.deliver_mode=ICR_Start_up;
					icr_entry.dest_shorthand=ICR_No_Shorthand;
					icr_entry.destination.x2apic_destination=*((unsigned char*)(start+3));
					wrmsr(0x830,*(unsigned long*)&icr_entry);
					wrmsr(0x830,*(unsigned long*)&icr_entry);
				}
			}
		}
		start=start+*((unsigned char*)(start+1));
	}
}
