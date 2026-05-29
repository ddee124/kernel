#include "trap.h"
#include "gate.h"
#include "printk.h"
#include "ptrace.h"
void do_divide_error(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_divide_error(0),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_debug(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_debug(1),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_nmi(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_nmi(2),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_int3(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_int3(3),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_overflow(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_overflow(4),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_bounds(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_bounds(5),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_undefined_opcode(struct pt_regs *rsp){
	color_printk(0xFF,0,"do_undefined_opcode(6),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_dev_not_available(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_dev_not_available(7),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_double_fault(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_double_fault(8),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_coprocessor_segment_overrun(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_coprocessor_segment_overrun(9),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_invalid_TSS(struct pt_regs *rsp){
	color_printk(0xff0000,0,"do_invalid_TSS(10),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	if(rsp->errcode&0x01)	color_printk(0xff0000,0,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");
	if(rsp->errcode&0x02)	color_printk(0xff0000,0,"Refers to a gate descriptor in the IDT;\n");
	else    color_printk(0xff0000,0,"Refers to a descriptor in the GDT or the current LDT;\n");
	if((rsp->errcode&0x02)==0){
		if(rsp->errcode&0x04)	color_printk(0xff0000,0,"Refers to a segment or gate descriptor in the IDT;\n");
		else    color_printk(0xff0000,0,"Refers to a descriptor in the current GDT;\n");
	}
	color_printk(0xff0000,0,"Segment Selector Index:%#010x\n",rsp->errcode&0xfff8);
	while(1);
}
void do_segment_not_present(struct pt_regs *rsp){
	color_printk(0xff0000,0,"do_segment_not_present(11),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	if(rsp->errcode&0x01)	color_printk(0xff0000,0,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");
	if(rsp->errcode&0x02)	color_printk(0xff0000,0,"Refers to a gate descriptor in the IDT;\n");
	else    color_printk(0xff0000,0,"Refers to a descriptor in the GDT or the current LDT;\n");
	if((rsp->errcode&0x02)==0){
		if(rsp->errcode&0x04)	color_printk(0xff0000,0,"Refers to a segment or gate descriptor in the IDT;\n");
		else    color_printk(0xff0000,0,"Refers to a descriptor in the current GDT;\n");
	}
	color_printk(0xff0000,0,"Segment Selector Index:%#010x\n",rsp->errcode&0xfff8);
	while(1);
}
void do_stack_segment_fault(struct pt_regs *rsp){
	color_printk(0xff0000,0,"do_stack_segment_fault(12),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	if(rsp->errcode&0x01)	color_printk(0xff0000,0,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");
	if(rsp->errcode&0x02)	color_printk(0xff0000,0,"Refers to a gate descriptor in the IDT;\n");
	else    color_printk(0xff0000,0,"Refers to a descriptor in the GDT or the current LDT;\n");
	if((rsp->errcode&0x02)==0){
		if(rsp->errcode&0x04)	color_printk(0xff0000,0,"Refers to a segment or gate descriptor in the IDT;\n");
		else    color_printk(0xff0000,0,"Refers to a descriptor in the current GDT;\n");
	}
	color_printk(0xff0000,0,"Segment Selector Index:%#010x\n",rsp->errcode&0xfff8);
	while(1);
}
void do_general_protection(struct pt_regs *rsp){
	color_printk(0xff0000,0,"do_general_protection(13),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	if(rsp->errcode&0x01)	color_printk(0xff0000,0,"The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");
	if(rsp->errcode&0x02)	color_printk(0xff0000,0,"Refers to a gate descriptor in the IDT;\n");
	else    color_printk(0xff0000,0,"Refers to a descriptor in the GDT or the current LDT;\n");
	if((rsp->errcode&0x02)==0){
		if(rsp->errcode&0x04)	color_printk(0xff0000,0,"Refers to a segment or gate descriptor in the IDT;\n");
		else    color_printk(0xff0000,0,"Refers to a descriptor in the current GDT;\n");
	}
	color_printk(0xff0000,0,"Segment Selector Index:%#010x\n",rsp->errcode&0xfff8);
	while(1);
}
void do_page_fault(struct pt_regs *rsp){
	unsigned long cr2=0;
	__asm__ __volatile__("movq %%cr2,%0":"=r"(cr2)::"memory");
	color_printk(0xff0000,0,"do_page_fault(14),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	if(!(rsp->errcode&0x01))	color_printk(0xff0000,0,"Page Not-Present,\t");
	if(rsp->errcode&0x02)	color_printk(0xff0000,0,"Write Cause Fault,\t");
	else    color_printk(0xff0000,0,"Read Cause Fault,\t");
	if(rsp->errcode&0x04)	color_printk(0xff0000,0,"Fault in user(3)\t");
	else    color_printk(0xff0000,0,"Fault in supervisor(0,1,2)\t");
	if(rsp->errcode&0x08)	color_printk(0xff0000,0,",Reserved Bit Cause Fault\t");
	if(rsp->errcode&0x10)	color_printk(0xff0000,0,",Instruction fetch Cause Fault");
	color_printk(0xff0000,0,"\n");
	color_printk(0xff0000,0,"CR2:%#018lx\n",cr2);
	while(1);
}
void do_x87_FPU_error(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_x87_FPU_error(16),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_alignment_check(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_alignment_check(17),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_machine_check(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_machine_check(18),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_SIMD_exception(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_SIMD_exception(19),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_virtualization_exception(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_virtualization_exception(20),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_control_protection_exception(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_control_protection_exception(21),errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void do_unknown_interrupt(struct pt_regs *rsp){
	color_printk(0xFF0000,0,"do_unknown_interrupt,errcode:%#018lx,RSP:%#018lx,RIP:%#018lx\n",rsp->errcode,rsp->rsp,rsp->rip);
	while(1);
}
void sys_vector_init(){
	set_trap_gate(0,1,divide_error);
	set_trap_gate(1,1,debug);
	set_intr_gate(2,1,nmi);
	set_system_gate(3,1,int3);
	set_system_gate(4,1,overflow);
	set_system_gate(5,1,bounds);
	set_trap_gate(6,1,undefined_opcode);
	set_trap_gate(7,1,dev_not_available);
	set_trap_gate(8,1,double_fault);
	set_trap_gate(9,1,coprocessor_segment_overrun);
	set_trap_gate(10,1,invalid_TSS);
	set_trap_gate(11,1,segment_not_present);
	set_trap_gate(12,1,stack_segment_fault);
	set_trap_gate(13,1,general_protection);
	set_trap_gate(14,1,page_fault);
	//15 Intel reserved. Do not use.
	set_trap_gate(16,1,x87_FPU_error);
	set_trap_gate(17,1,alignment_check);
	set_trap_gate(18,1,machine_check);
	set_trap_gate(19,1,SIMD_exception);
	set_trap_gate(20,1,virtualization_exception);
	//set_system_gate(SYSTEM_CALL_VECTOR,7,system_call);
	for(int i=21;i<256;i++)	set_intr_gate(i,1,unknown_interrupt);
}
