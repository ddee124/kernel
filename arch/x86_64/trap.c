#include "kernel/printk.h"
#include "arch/x86_64/ptrace.h"
#include "arch/x86_64/trap.h"
#include "arch/x86_64/gate.h"
#include<stdint.h>
#define __always_inline __attribute__((__always_inline__))
void print_regs(struct pt_regs* regs){
	color_printk(RED,WHITE,"RAX:%#018lx RBX:%#018lx RCX:%#018lx RDX:%#018lx\n",regs->rax,regs->rbx,regs->rcx,regs->rdx);
	color_printk(RED,WHITE,"RSI:%#018lx RDI:%#018lx RBP:%#018lx RSP:%#018lx\n",regs->rsi,regs->rdi,regs->rbp,regs->rsp);
	color_printk(RED,WHITE,"R8 :%#018lx R9 :%#018lx R10:%#018lx R11:%#018lx\n",regs->r8,regs->r9,regs->r10,regs->r11);
	color_printk(RED,WHITE,"R12:%#018lx R13:%#018lx R14:%#018lx R15:%#018lx\n",regs->r12,regs->r13,regs->r14,regs->r15);
	color_printk(RED,WHITE,"RIP:%#018lx RFLAGS:%#018lx CS:%#018lx SS:%#018lx\n",regs->rip,regs->rflags,regs->cs,regs->ss);
}
void analysis_error_code(uint64_t errcode){
	if(errcode&0x01)	color_printk(RED,WHITE,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");
	if(errcode&0x02)	color_printk(RED,WHITE,"Refers to a gate descriptor in the IDT;\n");
	else    color_printk(RED,WHITE,"Refers to a descriptor in the GDT or the current LDT;\n");
	if((errcode&0x02)==0){
		if(errcode&0x04)	color_printk(RED,WHITE,"Refers to a segment or gate descriptor in the LDT;\n");
		else    color_printk(RED,WHITE,"Refers to a descriptor in the current GDT;\n");
	}
	color_printk(RED,WHITE,0,"Segment Selector Index:%#010x\n",errcode&0xfff8);
}
void do_divide_error(struct pt_regs* regs){
	color_printk(RED,WHITE,"do_divide_error(0),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_debug(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_debug(1),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_nmi(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_nmi(2),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_int3(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_int3(3),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_overflow(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_overflow(4),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_bound(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_bound(5),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_invalid_opcode(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_invalid_opcode(6),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_dev_not_available(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_dev_not_available(7),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_double_fault(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_double_fault(8),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_coprocessor_segment_overrun(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_coprocessor_segment_overrun(9),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_invalid_tss(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_invalid_TSS(10),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	analysis_error_code(regs->errcode);
	while(1);
}
void do_segment_not_present(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_segment_not_present(11),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	analysis_error_code(regs->errcode);
	while(1);
}
void do_stack_fault(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_stack_fault(12),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	analysis_error_code(regs->errcode);
	while(1);
}
void do_general_protection(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_general_protection(13),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	analysis_error_code(regs->errcode);
	while(1);
}
void do_page_fault(struct pt_regs *regs){
	uint64_t cr2=0;
	__asm__ __volatile__("movq %%cr2,%0":"=r"(cr2)::"memory");
	color_printk(RED,WHITE,"do_page_fault(14),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	if(!(regs->errcode&0x01))	color_printk(RED,WHITE,"Page Not Present,\t");
	if(regs->errcode&0x02)	color_printk(RED,WHITE,"Write Cause Fault,\t");
	else    color_printk(RED,WHITE,"Read Cause Fault,\t");
	if(regs->errcode&0x04)	color_printk(RED,WHITE,"Fault in user(3),\t");
	else    color_printk(RED,WHITE,"Fault in supervisor(0),\t");
	if(regs->errcode&0x08)	color_printk(RED,WHITE,"Reserved Bit Cause Fault,\t");
	if(regs->errcode&0x10)	color_printk(RED,WHITE,"Instruction fetch Cause Fault,\t");
	if(regs->errcode&0x20)	color_printk(RED,WHITE,"Protection key violation Cause Fault,\t");
	if(regs->errcode&0x40)	color_printk(RED,WHITE,"Shadow stack access Cause Fault,\t");
	if(regs->errcode&0x80)	color_printk(RED,WHITE,"HLAT Paging Cause Fault,\t");
	if(regs->errcode&0x8000)	color_printk(RED,WHITE,"SGX Cause Fault,\t");
	color_printk(RED,WHITE,"\n");
	color_printk(RED,WHITE,"CR2:%#018lx\n",cr2);
	while(1);
}
void do_x87_fpu_error(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_x87_fpu_error(16),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_alignment_check(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_alignment_check(17),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_machine_check(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_machine_check(18),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_simd_exception(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_SIMD_exception(19),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_virtualization_exception(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_virtualization_exception(20),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void do_control_protection(struct pt_regs *regs){
	color_printk(RED,WHITE,"do_control_protection(21),errcode:%#018lx\n",regs->errcode);
	print_regs(regs);
	while(1);
}
void sys_vector_init(){
	set_trap_gate(&IDT_Table[0],1,(uintptr_t)divide_error);
	set_trap_gate(&IDT_Table[1],1,(uintptr_t)debug);
	set_intr_gate(&IDT_Table[2],1,(uintptr_t)nmi);
	set_system_gate(&IDT_Table[3],1,(uintptr_t)int3);
	set_system_gate(&IDT_Table[4],1,(uintptr_t)overflow);
	set_system_gate(&IDT_Table[5],1,(uintptr_t)bound);
	set_trap_gate(&IDT_Table[6],1,(uintptr_t)invalid_opcode);
	set_trap_gate(&IDT_Table[7],1,(uintptr_t)dev_not_available);
	set_trap_gate(&IDT_Table[8],1,(uintptr_t)double_fault);
	set_trap_gate(&IDT_Table[9],1,(uintptr_t)coprocessor_segment_overrun);
	set_trap_gate(&IDT_Table[10],1,(uintptr_t)invalid_tss);
	set_trap_gate(&IDT_Table[11],1,(uintptr_t)segment_not_present);
	set_trap_gate(&IDT_Table[12],1,(uintptr_t)stack_fault);
	set_trap_gate(&IDT_Table[13],1,(uintptr_t)general_protection);
	set_trap_gate(&IDT_Table[14],1,(uintptr_t)page_fault);
	//15 Intel reserved. Do not use.
	set_trap_gate(&IDT_Table[16],1,(uintptr_t)x87_fpu_error);
	set_trap_gate(&IDT_Table[17],1,(uintptr_t)alignment_check);
	set_trap_gate(&IDT_Table[18],1,(uintptr_t)machine_check);
	set_trap_gate(&IDT_Table[19],1,(uintptr_t)simd_exception);
	set_trap_gate(&IDT_Table[20],1,(uintptr_t)virtualization_exception);
}
