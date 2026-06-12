#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#include "ptrace.h"
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
extern void IRQ0xc8_interrupt();
extern void IRQ0xc9_interrupt();
extern void IRQ0xca_interrupt();
extern void IRQ0xcb_interrupt();
extern void IRQ0xcc_interrupt();
extern void IRQ0xcd_interrupt();
extern void IRQ0xce_interrupt();
extern void IRQ0xcf_interrupt();
extern void(*SMP_interrupt[24])(void);
struct irq_desc_T{
	struct hw_int_controller* controller;
	char* irq_name;
	unsigned long parameter;
	void(*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs);
	unsigned long flags;
};
#define NR_IRQS 24
struct irq_desc_T interrupt_desc[NR_IRQS]={0};
struct irq_desc_T SMP_IPI_desc[8]={0};
struct hw_int_controller{
	void(*enable)(unsigned long irq);
	void(*disable)(unsigned long irq);
	unsigned long(*install)(unsigned long irq,void* arg);
	void(*uninstall)(unsigned long irq);
	void(*ack)(unsigned long irq);
};
extern int register_irq(unsigned long irq,void* arg,void(*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),unsigned long parameter,struct hw_int_controller* controller,char* irq_name);
extern int unregister_irq(unsigned long irq);
extern void do_IRQ(struct pt_regs *regs,unsigned long nr);
#endif
