#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "arch/x86_64/ptrace.h"
extern void IRQ0x20_interrupt();
extern void IRQ0x21_interrupt();
extern void IRQ0x22_interrupt();
extern void IRQ0x23_interrupt();
extern void IRQ0x24_interrupt();
extern void IRQ0x25_interrupt();
extern void IRQ0x26_interrupt();
extern void IRQ0x27_interrupt();
extern void IRQ0x28_interrupt();
extern void IRQ0x29_interrupt();
extern void IRQ0x2a_interrupt();
extern void IRQ0x2b_interrupt();
extern void IRQ0x2c_interrupt();
extern void IRQ0x2d_interrupt();
extern void IRQ0x2e_interrupt();
extern void IRQ0x2f_interrupt();
extern void IRQ0x30_interrupt();
extern void IRQ0x31_interrupt();
extern void IRQ0x32_interrupt();
extern void IRQ0x33_interrupt();
extern void IRQ0x34_interrupt();
extern void IRQ0x35_interrupt();
extern void IRQ0x36_interrupt();
extern void IRQ0x37_interrupt();
extern void(*interrupt[24])(void);
/*extern void IRQ0xc8_interrupt();
extern void IRQ0xc9_interrupt();
extern void IRQ0xca_interrupt();
extern void IRQ0xcb_interrupt();
extern void IRQ0xcc_interrupt();
extern void IRQ0xcd_interrupt();
extern void IRQ0xce_interrupt();
extern void IRQ0xcf_interrupt();
extern void(*SMP_interrupt[24])(void);*/
extern void IRQ0xff_interrupt();
struct irq_desc_T{
	struct hw_int_controller* controller;
	unsigned char* irq_name;
	uint64_t parameter;
	void(*handler)(uint64_t nr,uint64_t parameter,struct pt_regs* regs);
	uint64_t flags;
};
#define NR_IRQS 24
struct irq_desc_T interrupt_desc[NR_IRQS]={0};
//struct irq_desc_T SMP_IPI_desc[8]={0};
struct hw_int_controller{
	void(*enable)(uint64_t irq);
	void(*disable)(uint64_t irq);
	uint64_t(*install)(uint64_t irq,void* arg);
	void(*uninstall)(uint64_t irq);
	void(*ack)(uint64_t irq);
};
extern uint64_t register_irq(uint64_t irq,void* arg,void(*handler)(uint64_t nr,uint64_t parameter,struct pt_regs* regs),uint64_t parameter,struct hw_int_controller* controller,unsigned char* irq_name);
extern uint64_t unregister_irq(uint64_t irq);
extern void do_IRQ(struct pt_regs *regs,uint64_t nr);
#endif
